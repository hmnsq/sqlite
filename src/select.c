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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** An instance of the following object is used to record information about
** how to process the DISTINCT keyword, to simplify passing that information
** into the selectInnerLoop() routine.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ DistinctCtx DistinctCtx;
ਬਣਤਰ DistinctCtx {
  u8 isTnct;      /* 0: Not distinct. 1: DISTINCT  2: DISTINCT and ORDER BY */
  u8 eTnctType;   /* One of the WHERE_DISTINCT_* operators */
  ਪੂਰਨ tabTnct;    /* Ephemeral table used for DISTINCT processing */
  ਪੂਰਨ addrTnct;   /* Address of OP_OpenEphemeral opcode for tabTnct */
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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਛਾਂਟੋ_Ctx ਛਾਂਟੋ_Ctx;
ਬਣਤਰ ਛਾਂਟੋ_Ctx {
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;   /* The ORDER BY (or GROUP BY clause) */
  ਪੂਰਨ nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  ਪੂਰਨ iECursor;         /* Cursor number for the sorter */
  ਪੂਰਨ regReturn;        /* Register holding block-output return address */
  ਪੂਰਨ label_Bk_ਬਾਹਰਲਾ;       /* Start label for the block-output subroutine */
  ਪੂਰਨ addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  ਪੂਰਨ label_ਪੂਰਾ;        /* Jump here when done, ex: LIMIT reached */
  ਪੂਰਨ labelOBLopt;      /* Jump here when sorter is full */
  u8 ਛਾਂਟੋ_Flags;         /* Zero or more SORTFLAG_* bits */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  ਬਣਤਰ DeferredCsr {
    ਸਾਰਣੀ *pTab;        /* Table definition */
    ਪੂਰਨ iCsr;           /* Cursor number for table */
    ਪੂਰਨ n_ਕੁੰਜੀ;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#ਜੇ_ਅੰਤ
  ਬਣਤਰ ਪੰਕਤੀ_ਲੱਦੋ_Info *p_Deferred_ਪੰਕਤੀ_ਲੱਦੋ;  /* Deferred row loading info or NULL */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  ਪੂਰਨ addr_ਧੱਕੋ;         /* First instruction to push data into sorter */
  ਪੂਰਨ addr_ਧੱਕੋ_ਸਿਰਾ;      /* Last instruction that pushes data into sorter */
#ਜੇ_ਅੰਤ
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SORTFLAG_UseSorter  0x01   /* Use SorterOpen instead of OpenEphemeral */

/*
** Delete all the content of a Select structure.  Deallocate the structure
** itself depending on the value of bFree
**
** If bFree==1, call sqlite3DbFree() on the p object.
** If bFree==0, Leave the first Select object unfreed
*/
ਸਥਿਰ ਖਾਲੀ clearSelect(sqlite3 *db, Select *p, ਪੂਰਨ bFree){
  ਪ੍ਰਤਿਜ੍ਞਾ( db!=0 );
  ਜਦੋਂ( p ){
    Select *p_ਪੂਰਵ = p->p_ਪੂਰਵ;
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p->pWhere);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p->p_ਸਮੂਹ_By);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p->pHaving);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p->p_ਕ੍ਰਮ_By);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p->p_ਹੱਦਬੰਦੀ);
    ਜੇ( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3_ਖਿੜਕੀ_List_Delete(db, p->pWinDefn);
    }
    ਜਦੋਂ( p->pWin ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p->pWin->ppThis==&p->pWin );
      sqlite3_ਖਿੜਕੀ_Unlink_From_Select(p->pWin);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( bFree ) sqlite3DbNNFreeNN(db, p);
    p = p_ਪੂਰਵ;
    bFree = 1;
  }
}

/*
** Initialize a SelectDest structure.
*/
ਖਾਲੀ sqlite3SelectDestInit(SelectDest *pDest, ਪੂਰਨ eDest, ਪੂਰਨ iParm){
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
  ਵਿਅੰਜਕ_List *pEList,     /* which columns to include in the result */
  SrcList *pSrc,        /* the FROM clause -- which tables to scan */
  ਵਿਅੰਜਕ *pWhere,         /* the WHERE clause */
  ਵਿਅੰਜਕ_List *p_ਸਮੂਹ_By,   /* the GROUP BY clause */
  ਵਿਅੰਜਕ *pHaving,        /* the HAVING clause */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, ਆਕਾਰ(*pNew) );
  ਜੇ( pNew==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed );
    pNew = &standin;
  }
  ਜੇ( pEList==0 ){
    pEList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0,
                                   sqlite3_ਵਿਅੰਜਕ(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->i_ਹੱਦਬੰਦੀ = 0;
  pNew->i_ਵਿੱਥ = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->n_Select_ਪੰਕਤੀ = 0;
  ਜੇ( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->p_ਸਮੂਹ_By = p_ਸਮੂਹ_By;
  pNew->pHaving = pHaving;
  pNew->p_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By;
  pNew->p_ਪੂਰਵ = 0;
  pNew->p_ਅਗਲਾ = 0;
  pNew->p_ਹੱਦਬੰਦੀ = p_ਹੱਦਬੰਦੀ;
  pNew->pWith = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  pNew->pWin = 0;
  pNew->pWinDefn = 0;
#ਜੇ_ਅੰਤ
  ਜੇ( pParse->db->mallocFailed ) {
    clearSelect(pParse->db, pNew, pNew!=&standin);
    pAllocated = 0;
  }ਨਹੀਂਤਾਂ{
    ਪ੍ਰਤਿਜ੍ਞਾ( pNew->pSrc!=0 || pParse->nErr>0 );
  }
  ਵਾਪਸ pAllocated;
}


/*
** Delete the given Select structure and all of its substructures.
*/
ਖਾਲੀ sqlite3SelectDelete(sqlite3 *db, Select *p){
  ਜੇ( OK_IF_ALWAYS_TRUE(p) ) clearSelect(db, p, 1);
}
ਖਾਲੀ sqlite3SelectDeleteGeneric(sqlite3 *db, ਖਾਲੀ *p){
  ਜੇ( ALWAYS(p) ) clearSelect(db, (Select*)p, 1);
}

/*
** Return a pointer to the right-most SELECT statement in a compound.
*/
ਸਥਿਰ Select *ਲੱਭੋ_Rightmost(Select *p){
  ਜਦੋਂ( p->p_ਅਗਲਾ ) p = p->p_ਅਗਲਾ;
  ਵਾਪਸ p;
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
ਪੂਰਨ sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  ਪੂਰਨ jointype = 0;
  Token *apAll[3];
  Token *p;
                             /*   0123456789 123456789 123456789 123 */
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ z_ਕੁੰਜੀ_ਪਾਠ[] = "naturaleftouterightfullinnercross";
  ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 ਕੋਡ;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_ਖੱਬਾ|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_ਸੱਜਾ|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_ਖੱਬਾ|JT_ਸੱਜਾ|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  ਪੂਰਨ i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  ਲਈ(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    ਲਈ(j=0; j<Array_ਮਾਪ(aKeyword); j++){
      ਜੇ( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((ਅੱਖਰ*)p->z, &z_ਕੁੰਜੀ_ਪਾਠ[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].ਕੋਡ;
        ਰੋਕੋ;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    ਜੇ( j>=Array_ਮਾਪ(aKeyword) ){
      jointype |= JT_ERROR;
      ਰੋਕੋ;
    }
  }
  ਜੇ(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_ਖੱਬਾ|JT_ਸੱਜਾ))==JT_OUTER
  ){
    ਅਚਲ ਅੱਖਰ *zSp1 = " ";
    ਅਚਲ ਅੱਖਰ *zSp2 = " ";
    ਜੇ( pB==0 ){ zSp1++; }
    ਜੇ( pC==0 ){ zSp2++; }
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  ਵਾਪਸ jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
ਪੂਰਨ sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(ਸਾਰਣੀ *pTab, ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ){
  ਪੂਰਨ i;
  u8 h;
  ਅਚਲ ਕਾਲਮ *a_ਖਾਨਾ;
  ਪੂਰਨ n_ਖਾਨਾ;

  h = sqlite3StrIHash(z_ਖਾਨਾ);
  a_ਖਾਨਾ = pTab->a_ਖਾਨਾ;
  n_ਖਾਨਾ = pTab->n_ਖਾਨਾ;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % ਆਕਾਰ(pTab->aHx)];
  ਪ੍ਰਤਿਜ੍ਞਾ( i<n_ਖਾਨਾ );
  ਜੇ( a_ਖਾਨਾ[i].hName==h
   && sqlite3StrICmp(a_ਖਾਨਾ[i].zCnName, z_ਖਾਨਾ)==0
  ){
    ਵਾਪਸ i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  ਜਦੋਂ( 1 /*exit-by-break*/ ){
    ਜੇ( a_ਖਾਨਾ[i].hName==h
     && sqlite3StrICmp(a_ਖਾਨਾ[i].zCnName, z_ਖਾਨਾ)==0
    ){
      ਵਾਪਸ i;
    }
    i++;
    ਜੇ( i>=n_ਖਾਨਾ ) ਰੋਕੋ;
  }
  ਵਾਪਸ -1;
}

/*
** Mark a subquery result column as having been used.
*/
ਖਾਲੀ sqlite3_Src_ਵਸਤੂ_ਕਾਲਮ_ਵਰਤਿਆ(Src_ਵਸਤੂ *p_ਵਸਤੂ, ਪੂਰਨ i_ਖਾਨਾ){
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੂਰਨ)p_ਵਸਤੂ->fg.isNestedFrom == IsNestedFrom(p_ਵਸਤੂ) );
  ਜੇ( p_ਵਸਤੂ->fg.isNestedFrom ){
    ਵਿਅੰਜਕ_List *pResults;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.isSubquery );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->u4.pSubq!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->u4.pSubq->pSelect!=0 );
    pResults = p_ਵਸਤੂ->u4.pSubq->pSelect->pEList;
    ਪ੍ਰਤਿਜ੍ਞਾ( pResults!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=0 && i_ਖਾਨਾ<pResults->n_ਵਿਅੰਜਕ );
    pResults->a[i_ਖਾਨਾ].fg.b_ਵਰਤਿਆ = 1;
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
ਸਥਿਰ ਪੂਰਨ ਸਾਰਣੀ_And_ਕਾਲਮ_ਸੂਚਕਾਂਕ(
  SrcList *pSrc,       /* Array of tables to search */
  ਪੂਰਨ i_ਸ਼ੁਰੂ,          /* First member of pSrc->a[] to check */
  ਪੂਰਨ i_ਸਿਰਾ,            /* Last member of pSrc->a[] to check */
  ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ,    /* Name of the column we are looking for */
  ਪੂਰਨ *piTab,          /* Write index of pSrc->a[] here */
  ਪੂਰਨ *pi_ਖਾਨਾ,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  ਪੂਰਨ bIgnoreHidden    /* Ignore hidden columns */
){
  ਪੂਰਨ i;               /* For looping over tables in pSrc */
  ਪੂਰਨ i_ਖਾਨਾ;            /* Index of column matching zCol */

  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਸਿਰਾ<pSrc->nSrc );
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਸ਼ੁਰੂ>=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (piTab==0)==(pi_ਖਾਨਾ==0) );  /* Both or neither are NULL */

  ਲਈ(i=i_ਸ਼ੁਰੂ; i<=i_ਸਿਰਾ; i++){
    i_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pSrc->a[i].pSTab, z_ਖਾਨਾ);
    ਜੇ( i_ਖਾਨਾ>=0
     && (bIgnoreHidden==0 || Is_Hidden_ਕਾਲਮ(&pSrc->a[i].pSTab->a_ਖਾਨਾ[i_ਖਾਨਾ])==0)
    ){
      ਜੇ( piTab ){
        sqlite3_Src_ਵਸਤੂ_ਕਾਲਮ_ਵਰਤਿਆ(&pSrc->a[i], i_ਖਾਨਾ);
        *piTab = i;
        *pi_ਖਾਨਾ = i_ਖਾਨਾ;
      }
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
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
ਖਾਲੀ sqlite3_ਸੈੱਟ_Join_ਵਿਅੰਜਕ(ਵਿਅੰਜਕ *p, ਪੂਰਨ i_ਸਾਰਣੀ, u32 joinFlag){
  ਪ੍ਰਤਿਜ੍ਞਾ( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  ਜਦੋਂ( p ){
    ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p, joinFlag);
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ਵਿਅੰਜਕ_ਸੈੱਟ_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = i_ਸਾਰਣੀ;
    ਜੇ( ਵਿਅੰਜਕ_Use_XList(p) ){
      ਜੇ( p->x.pList ){
        ਪੂਰਨ i;
        ਲਈ(i=0; i<p->x.pList->n_ਵਿਅੰਜਕ; i++){
          sqlite3_ਸੈੱਟ_Join_ਵਿਅੰਜਕ(p->x.pList->a[i].p_ਵਿਅੰਜਕ, i_ਸਾਰਣੀ, joinFlag);
        }
      }
    }
    sqlite3_ਸੈੱਟ_Join_ਵਿਅੰਜਕ(p->p_ਖੱਬਾ, i_ਸਾਰਣੀ, joinFlag);
    p = p->p_ਸੱਜਾ;
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
ਸਥਿਰ ਖਾਲੀ unset_Join_ਵਿਅੰਜਕ(ਵਿਅੰਜਕ *p, ਪੂਰਨ i_ਸਾਰਣੀ, ਪੂਰਨ nullable){
  ਜਦੋਂ( p ){
    ਜੇ( i_ਸਾਰਣੀ<0 || (ਵਿਅੰਜਕ_Has_Property(p, EP_OuterON) && p->w.iJoin==i_ਸਾਰਣੀ) ){
      ਵਿਅੰਜਕ_Clear_Property(p, EP_OuterON|EP_InnerON);
      ਜੇ( i_ਸਾਰਣੀ>=0 ) ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p, EP_InnerON);
    }
    ਜੇ( p->op==TK_ਕਾਲਮ && p->i_ਸਾਰਣੀ==i_ਸਾਰਣੀ && !nullable ){
      ਵਿਅੰਜਕ_Clear_Property(p, EP_CanBeNull);
    }
    ਜੇ( p->op==TK_FUNCTION ){
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਖੱਬਾ==0 );
      ਜੇ( p->x.pList ){
        ਪੂਰਨ i;
        ਲਈ(i=0; i<p->x.pList->n_ਵਿਅੰਜਕ; i++){
          unset_Join_ਵਿਅੰਜਕ(p->x.pList->a[i].p_ਵਿਅੰਜਕ, i_ਸਾਰਣੀ, nullable);
        }
      }
    }
    unset_Join_ਵਿਅੰਜਕ(p->p_ਖੱਬਾ, i_ਸਾਰਣੀ, nullable);
    p = p->p_ਸੱਜਾ;
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
ਸਥਿਰ ਪੂਰਨ sqlite3ProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  ਪੂਰਨ i, j;                       /* Loop counters */
  Src_ਵਸਤੂ *p_ਖੱਬਾ;                 /* Left table being joined */
  Src_ਵਸਤੂ *p_ਸੱਜਾ;                /* Right table being joined */
  ਪੂਰਨ b_ਵੇਖਿਆ_TFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_ਖੱਬਾ = &pSrc->a[0];
  p_ਸੱਜਾ = &p_ਖੱਬਾ[1];
  ਲਈ(i=0; i<pSrc->nSrc-1; i++, p_ਸੱਜਾ++, p_ਖੱਬਾ++){
    ਸਾਰਣੀ *p_ਸੱਜਾ_Tab = p_ਸੱਜਾ->pSTab;
    u32 joinType;

    ਜੇ( NEVER(p_ਖੱਬਾ->pSTab==0 || p_ਸੱਜਾ_Tab==0) ) ਜਾਰੀ;
    joinType = (p_ਸੱਜਾ->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    ਜੇ( p_ਸੱਜਾ->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      ਜੇ( p_ਸੱਜਾ->fg.isUsing || p_ਸੱਜਾ->u3.pOn ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        ਵਾਪਸ 1;
      }
      ਲਈ(j=0; j<p_ਸੱਜਾ_Tab->n_ਖਾਨਾ; j++){
        ਅੱਖਰ *zName;   /* Name of column in the right table */

        ਜੇ( Is_Hidden_ਕਾਲਮ(&p_ਸੱਜਾ_Tab->a_ਖਾਨਾ[j]) ) ਜਾਰੀ;
        zName = p_ਸੱਜਾ_Tab->a_ਖਾਨਾ[j].zCnName;
        ਜੇ( ਸਾਰਣੀ_And_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          ਜੇ( pUsing ){
            ਪ੍ਰਤਿਜ੍ਞਾ( pUsing->nId>0 );
            ਪ੍ਰਤਿਜ੍ਞਾ( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      ਜੇ( pUsing ){
        p_ਸੱਜਾ->fg.isUsing = 1;
        p_ਸੱਜਾ->fg.isSynthUsing = 1;
        p_ਸੱਜਾ->u3.pUsing = pUsing;
      }
      ਜੇ( pParse->nErr ) ਵਾਪਸ 1;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    ਜੇ( p_ਸੱਜਾ->fg.isUsing ){
      IdList *pList = p_ਸੱਜਾ->u3.pUsing;
      sqlite3 *db = pParse->db;
      ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );
      ਲਈ(j=0; j<pList->nId; j++){
        ਅੱਖਰ *zName;     /* Name of the term in the USING clause */
        ਪੂਰਨ i_ਖੱਬਾ;       /* Table on the left with matching column name */
        ਪੂਰਨ i_ਖੱਬਾ_ਖਾਨਾ;    /* Column number of matching column on the left */
        ਪੂਰਨ i_ਸੱਜਾ_ਖਾਨਾ;   /* Column number of matching column on the right */
        ਵਿਅੰਜਕ *pE1;       /* Reference to the column on the LEFT of the join */
        ਵਿਅੰਜਕ *pE2;       /* Reference to the column on the RIGHT of the join */
        ਵਿਅੰਜਕ *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_ਸੱਜਾ_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(p_ਸੱਜਾ_Tab, zName);
        ਜੇ( i_ਸੱਜਾ_ਖਾਨਾ<0
         || ਸਾਰਣੀ_And_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pSrc, 0, i, zName, &i_ਖੱਬਾ, &i_ਖੱਬਾ_ਖਾਨਾ,
                                p_ਸੱਜਾ->fg.isSynthUsing)==0
        ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          ਵਾਪਸ 1;
        }
        pE1 = sqlite3_Create_ਕਾਲਮ_ਵਿਅੰਜਕ(db, pSrc, i_ਖੱਬਾ, i_ਖੱਬਾ_ਖਾਨਾ);
        sqlite3_Src_ਵਸਤੂ_ਕਾਲਮ_ਵਰਤਿਆ(&pSrc->a[i_ਖੱਬਾ], i_ਖੱਬਾ_ਖਾਨਾ);
        ਜੇ( (pSrc->a[0].fg.jointype & JT_LTORJ)!=0 && pParse->nErr==0 ){
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
          ਵਿਅੰਜਕ_List *p_Func_ਦਲੀਲਾਂ = 0;   /* Arguments to the coalesce() */
          ਸਥਿਰ ਅਚਲ Token tkCoalesce = { "coalesce", 8 };
          ਪ੍ਰਤਿਜ੍ਞਾ( pE1!=0 );
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pE1, EP_CanBeNull);
          ਜਦੋਂ( ਸਾਰਣੀ_And_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pSrc, i_ਖੱਬਾ+1, i, zName, &i_ਖੱਬਾ, &i_ਖੱਬਾ_ਖਾਨਾ,
                                     p_ਸੱਜਾ->fg.isSynthUsing)!=0 ){
            ਜੇ( pSrc->a[i_ਖੱਬਾ].fg.isUsing==0
             || sqlite3_Id_List_ਸੂਚਕਾਂਕ(pSrc->a[i_ਖੱਬਾ].u3.pUsing, zName)<0
            ){
              sqlite3_Error_ਸੁਨੇਹਾ(pParse, "ambiguous reference to %s in USING()",
                              zName);
              ਰੋਕੋ;
            }
            p_Func_ਦਲੀਲਾਂ = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, p_Func_ਦਲੀਲਾਂ, pE1);
            pE1 = sqlite3_Create_ਕਾਲਮ_ਵਿਅੰਜਕ(db, pSrc, i_ਖੱਬਾ, i_ਖੱਬਾ_ਖਾਨਾ);
            sqlite3_Src_ਵਸਤੂ_ਕਾਲਮ_ਵਰਤਿਆ(&pSrc->a[i_ਖੱਬਾ], i_ਖੱਬਾ_ਖਾਨਾ);
          }
          ਜੇ( p_Func_ਦਲੀਲਾਂ ){
            p_Func_ਦਲੀਲਾਂ = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, p_Func_ਦਲੀਲਾਂ, pE1);
            pE1 = sqlite3_ਵਿਅੰਜਕ_Function(pParse, p_Func_ਦਲੀਲਾਂ, &tkCoalesce, 0);
            ਜੇ( pE1 ){
              pE1->aff_ਵਿਅੰਜਕ = SQLITE_AFF_DEFER;
            }
          }
        }ਨਹੀਂਤਾਂ ਜੇ( (pSrc->a[i+1].fg.jointype & JT_ਖੱਬਾ)!=0 && pParse->nErr==0 ){
          ਪ੍ਰਤਿਜ੍ਞਾ( pE1!=0 );
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_ਕਾਲਮ_ਵਿਅੰਜਕ(db, pSrc, i+1, i_ਸੱਜਾ_ਖਾਨਾ);
        sqlite3_Src_ਵਸਤੂ_ਕਾਲਮ_ਵਰਤਿਆ(p_ਸੱਜਾ, i_ਸੱਜਾ_ਖਾਨਾ);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        ਪ੍ਰਤਿਜ੍ਞਾ( pE2!=0 || pEq==0 );
        ਜੇ( pEq ){
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pEq, joinType);
          ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          ਵਿਅੰਜਕ_ਸੈੱਟ_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->i_ਸਾਰਣੀ;
        }
        p->pWhere = sqlite3_ਵਿਅੰਜਕ_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    ਨਹੀਂਤਾਂ ਜੇ( p_ਸੱਜਾ->u3.pOn ){
      sqlite3_ਸੈੱਟ_Join_ਵਿਅੰਜਕ(p_ਸੱਜਾ->u3.pOn, p_ਸੱਜਾ->iCursor, joinType);
      p->pWhere = sqlite3_ਵਿਅੰਜਕ_And(pParse, p->pWhere, p_ਸੱਜਾ->u3.pOn);
      p_ਸੱਜਾ->u3.pOn = 0;
      p_ਸੱਜਾ->fg.isOn = 1;
      p->selFlags |= SF_On_ਮੰਜ਼ਿਲ_Where;
    }

    ਜੇ( b_ਵੇਖਿਆ_TFunc==0 ){
      b_ਵੇਖਿਆ_TFunc = (p_ਖੱਬਾ->fg.isTabFunc && p_ਖੱਬਾ->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    ਜੇ( (p_ਸੱਜਾ->fg.isTabFunc && joinType==EP_OuterON && p_ਸੱਜਾ->u1.pFuncArg)
     || (b_ਵੇਖਿਆ_TFunc && joinType==EP_OuterON)
    ){
      p->selFlags |= SF_On_ਮੰਜ਼ਿਲ_Where;
    }
  }
  ਵਾਪਸ 0;
}

/*
** An instance of this object holds information (beyond pParse and pSelect)
** needed to load the next result row that is to be added to the sorter.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਪੰਕਤੀ_ਲੱਦੋ_Info ਪੰਕਤੀ_ਲੱਦੋ_Info;
ਬਣਤਰ ਪੰਕਤੀ_ਲੱਦੋ_Info {
  ਪੂਰਨ reg_ਨਤੀਜਾ;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
  ਵਿਅੰਜਕ_List *pExtra;            /* Extra columns needed by sorter refs */
  ਪੂਰਨ reg_Extra_ਨਤੀਜਾ;          /* Where to load the extra columns */
#ਜੇ_ਅੰਤ
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
ਸਥਿਰ ਖਾਲੀ inner_Loop_ਲੱਦੋ_ਪੰਕਤੀ(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  ਪੰਕਤੀ_ਲੱਦੋ_Info *pInfo         /* Info needed to complete the row load */
){
  sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(pParse, pSelect->pEList, pInfo->reg_ਨਤੀਜਾ,
                          0, pInfo->ecelFlags);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
  ਜੇ( pInfo->pExtra ){
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(pParse, pInfo->pExtra, pInfo->reg_Extra_ਨਤੀਜਾ, 0, 0);
    sqlite3_ਵਿਅੰਜਕ_List_Delete(pParse->db, pInfo->pExtra);
  }
#ਜੇ_ਅੰਤ
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
ਸਥਿਰ ਪੂਰਨ make_Sorter_ਰਿਕਾਰਡ(
  Parse *pParse,
  ਛਾਂਟੋ_Ctx *p_ਛਾਂਟੋ,
  Select *pSelect,
  ਪੂਰਨ reg_ਅਧਾਰ,
  ਪੂਰਨ n_ਅਧਾਰ
){
  ਪੂਰਨ nOBSat = p_ਛਾਂਟੋ->nOBSat;
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ reg_ਬਾਹਰਲਾ = ++pParse->nMem;
  ਜੇ( p_ਛਾਂਟੋ->p_Deferred_ਪੰਕਤੀ_ਲੱਦੋ ){
    inner_Loop_ਲੱਦੋ_ਪੰਕਤੀ(pParse, pSelect, p_ਛਾਂਟੋ->p_Deferred_ਪੰਕਤੀ_ਲੱਦੋ);
  }
  sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg_ਅਧਾਰ+nOBSat, n_ਅਧਾਰ-nOBSat, reg_ਬਾਹਰਲਾ);
  ਵਾਪਸ reg_ਬਾਹਰਲਾ;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
ਸਥਿਰ ਖਾਲੀ ਧੱਕੋ_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  ਛਾਂਟੋ_Ctx *p_ਛਾਂਟੋ,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  ਪੂਰਨ reg_ਅੰਕੜੇ,           /* First register holding data to be sorted */
  ਪੂਰਨ reg_Orig_ਅੰਕੜੇ,       /* First register holding data before packing */
  ਪੂਰਨ n_ਅੰਕੜੇ,             /* Number of elements in the regData data array */
  ਪੂਰਨ n_ਅਗੇਤਰ_Reg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  ਪੂਰਨ bSeq = ((p_ਛਾਂਟੋ->ਛਾਂਟੋ_Flags & SORTFLAG_UseSorter)==0);
  ਪੂਰਨ n_ਵਿਅੰਜਕ = p_ਛਾਂਟੋ->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ;              /* No. of ORDER BY terms */
  ਪੂਰਨ n_ਅਧਾਰ = n_ਵਿਅੰਜਕ + bSeq + n_ਅੰਕੜੇ;                /* Fields in sorter record */
  ਪੂਰਨ reg_ਅਧਾਰ;                                     /* Regs for sorter record */
  ਪੂਰਨ reg_ਰਿਕਾਰਡ = 0;                               /* Assembled sorter record */
  ਪੂਰਨ nOBSat = p_ਛਾਂਟੋ->nOBSat;                      /* ORDER BY terms to skip */
  ਪੂਰਨ op;                            /* Opcode to add sorter record to sorter */
  ਪੂਰਨ i_ਹੱਦਬੰਦੀ;                        /* LIMIT counter */
  ਪੂਰਨ iSkip = 0;                     /* End of the sorter insert loop */

  ਪ੍ਰਤਿਜ੍ਞਾ( bSeq==0 || bSeq==1 );

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
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਅੰਕੜੇ==1 || reg_ਅੰਕੜੇ==reg_Orig_ਅੰਕੜੇ || reg_Orig_ਅੰਕੜੇ==0 );

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  p_ਛਾਂਟੋ->addr_ਧੱਕੋ = sqlite3VdbeCurrentAddr(v);
#ਜੇ_ਅੰਤ

  ਜੇ( n_ਅਗੇਤਰ_Reg ){
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਅਗੇਤਰ_Reg==n_ਵਿਅੰਜਕ+bSeq );
    reg_ਅਧਾਰ = reg_ਅੰਕੜੇ - n_ਅਗੇਤਰ_Reg;
  }ਨਹੀਂਤਾਂ{
    reg_ਅਧਾਰ = pParse->nMem + 1;
    pParse->nMem += n_ਅਧਾਰ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pSelect->i_ਵਿੱਥ==0 || pSelect->i_ਹੱਦਬੰਦੀ!=0 );
  i_ਹੱਦਬੰਦੀ = pSelect->i_ਵਿੱਥ ? pSelect->i_ਵਿੱਥ+1 : pSelect->i_ਹੱਦਬੰਦੀ;
  p_ਛਾਂਟੋ->label_ਪੂਰਾ = sqlite3VdbeMakeLabel(pParse);
  sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(pParse, p_ਛਾਂਟੋ->p_ਕ੍ਰਮ_By, reg_ਅਧਾਰ, reg_Orig_ਅੰਕੜੇ,
                          SQLITE_ECEL_DUP | (reg_Orig_ਅੰਕੜੇ? SQLITE_ECEL_REF : 0));
  ਜੇ( bSeq ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Sequence, p_ਛਾਂਟੋ->iECursor, reg_ਅਧਾਰ+n_ਵਿਅੰਜਕ);
  }
  ਜੇ( n_ਅਗੇਤਰ_Reg==0 && n_ਅੰਕੜੇ>0 ){
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Move(pParse, reg_ਅੰਕੜੇ, reg_ਅਧਾਰ+n_ਵਿਅੰਜਕ+bSeq, n_ਅੰਕੜੇ);
  }
  ਜੇ( nOBSat>0 ){
    ਪੂਰਨ reg_Prev_ਕੁੰਜੀ;   /* The first nOBSat columns of the previous row */
    ਪੂਰਨ addr_ਪਹਿਲਾ;    /* Address of the OP_IfNot opcode */
    ਪੂਰਨ addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    ਪੂਰਨ n_ਕੁੰਜੀ;         /* Number of sorting key columns, including OP_Sequence */
    ਕੁੰਜੀ_Info *pKI;     /* Original KeyInfo on the sorter table */

    reg_ਰਿਕਾਰਡ = make_Sorter_ਰਿਕਾਰਡ(pParse, p_ਛਾਂਟੋ, pSelect, reg_ਅਧਾਰ, n_ਅਧਾਰ);
    reg_Prev_ਕੁੰਜੀ = pParse->nMem+1;
    pParse->nMem += p_ਛਾਂਟੋ->nOBSat;
    n_ਕੁੰਜੀ = n_ਵਿਅੰਜਕ - p_ਛਾਂਟੋ->nOBSat + bSeq;
    ਜੇ( bSeq ){
      addr_ਪਹਿਲਾ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IfNot, reg_ਅਧਾਰ+n_ਵਿਅੰਜਕ);
    }ਨਹੀਂਤਾਂ{
      addr_ਪਹਿਲਾ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_SequenceTest, p_ਛਾਂਟੋ->iECursor);
    }
    VdbeCoverage(v);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Compare, reg_Prev_ਕੁੰਜੀ, reg_ਅਧਾਰ, p_ਛਾਂਟੋ->nOBSat);
    pOp = sqlite3VdbeGetOp(v, p_ਛਾਂਟੋ->addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ);
    ਜੇ( pParse->db->mallocFailed ) ਵਾਪਸ;
    pOp->p2 = n_ਕੁੰਜੀ + n_ਅੰਕੜੇ;
    pKI = pOp->p4.p_ਕੁੰਜੀ_Info;
    memset(pKI->a_ਛਾਂਟੋ_Flags, 0, pKI->n_ਕੁੰਜੀ_ਖੇਤਰ); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (ਅੱਖਰ*)pKI, P4_KEYINFO);
    testcase( pKI->n_All_ਖੇਤਰ > pKI->n_ਕੁੰਜੀ_ਖੇਤਰ+2 );
    pOp->p4.p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_From_ਵਿਅੰਜਕ_List(pParse,p_ਛਾਂਟੋ->p_ਕ੍ਰਮ_By,nOBSat,
                                           pKI->n_All_ਖੇਤਰ-pKI->n_ਕੁੰਜੀ_ਖੇਤਰ-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    p_ਛਾਂਟੋ->label_Bk_ਬਾਹਰਲਾ = sqlite3VdbeMakeLabel(pParse);
    p_ਛਾਂਟੋ->regReturn = ++pParse->nMem;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, p_ਛਾਂਟੋ->regReturn, p_ਛਾਂਟੋ->label_Bk_ਬਾਹਰਲਾ);
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_ResetSorter, p_ਛਾਂਟੋ->iECursor);
    ਜੇ( i_ਹੱਦਬੰਦੀ ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IfNot, i_ਹੱਦਬੰਦੀ, p_ਛਾਂਟੋ->label_ਪੂਰਾ);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_ਪਹਿਲਾ);
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Move(pParse, reg_ਅਧਾਰ, reg_Prev_ਕੁੰਜੀ, p_ਛਾਂਟੋ->nOBSat);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrJmp);
  }
  ਜੇ( i_ਹੱਦਬੰਦੀ ){
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
    ਪੂਰਨ iCsr = p_ਛਾਂਟੋ->iECursor;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IfNotZero, i_ਹੱਦਬੰਦੀ, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਆਖਰੀ, iCsr, 0);
    iSkip = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IdxLE,
                                 iCsr, 0, reg_ਅਧਾਰ+nOBSat, n_ਵਿਅੰਜਕ-nOBSat);
    VdbeCoverage(v);
    sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Delete, iCsr);
  }
  ਜੇ( reg_ਰਿਕਾਰਡ==0 ){
    reg_ਰਿਕਾਰਡ = make_Sorter_ਰਿਕਾਰਡ(pParse, p_ਛਾਂਟੋ, pSelect, reg_ਅਧਾਰ, n_ਅਧਾਰ);
  }
  ਜੇ( p_ਛਾਂਟੋ->ਛਾਂਟੋ_Flags & SORTFLAG_UseSorter ){
    op = OP_Sorter_ਪਾਓ;
  }ਨਹੀਂਤਾਂ{
    op = OP_Idx_ਪਾਓ;
  }
  sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, op, p_ਛਾਂਟੋ->iECursor, reg_ਰਿਕਾਰਡ,
                       reg_ਅਧਾਰ+nOBSat, n_ਅਧਾਰ-nOBSat);
  ਜੇ( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         p_ਛਾਂਟੋ->labelOBLopt ? p_ਛਾਂਟੋ->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  p_ਛਾਂਟੋ->addr_ਧੱਕੋ_ਸਿਰਾ = sqlite3VdbeCurrentAddr(v)-1;
#ਜੇ_ਅੰਤ
}

/*
** Add code to implement the OFFSET
*/
ਸਥਿਰ ਖਾਲੀ ਕੋਡ_ਵਿੱਥ(
  Vdbe *v,          /* Generate code into this VM */
  ਪੂਰਨ i_ਵਿੱਥ,      /* Register holding the offset counter */
  ਪੂਰਨ iContinue     /* Jump here to skip the current record */
){
  ਜੇ( i_ਵਿੱਥ>0 ){
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_IfPos, i_ਵਿੱਥ, iContinue, 1); VdbeCoverage(v);
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
ਸਥਿਰ ਪੂਰਨ ਕੋਡ_Distinct(
  Parse *pParse,     /* Parsing and code generating context */
  ਪੂਰਨ eTnctType,     /* WHERE_DISTINCT_* value */
  ਪੂਰਨ iTab,          /* A sorting index used to test for distinctness */
  ਪੂਰਨ addrRepeat,    /* Jump to here if not distinct */
  ਵਿਅੰਜਕ_List *pEList,  /* Expression for each element */
  ਪੂਰਨ regElem        /* First element */
){
  ਪੂਰਨ iRet = 0;
  ਪੂਰਨ n_ਨਤੀਜਾ_ਖਾਨਾ = pEList->n_ਵਿਅੰਜਕ;
  Vdbe *v = pParse->pVdbe;

  ਚੋਣ( eTnctType ){
    ਹਾਲਤ WHERE_DISTINCT_ORDERED: {
      ਪੂਰਨ i;
      ਪੂਰਨ iJump;              /* Jump destination */
      ਪੂਰਨ regPrev;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = regPrev = pParse->nMem+1;
      pParse->nMem += n_ਨਤੀਜਾ_ਖਾਨਾ;

      iJump = sqlite3VdbeCurrentAddr(v) + n_ਨਤੀਜਾ_ਖਾਨਾ;
      ਲਈ(i=0; i<n_ਨਤੀਜਾ_ਖਾਨਾ; i++){
        CollSeq *pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, pEList->a[i].p_ਵਿਅੰਜਕ);
        ਜੇ( i<n_ਨਤੀਜਾ_ਖਾਨਾ-1 ){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Ne, regElem+i, iJump, regPrev+i);
          VdbeCoverage(v);
        }ਨਹੀਂਤਾਂ{
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Eq, regElem+i, addrRepeat, regPrev+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (ਅਚਲ ਅੱਖਰ *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Copy, regElem, regPrev, n_ਨਤੀਜਾ_ਖਾਨਾ-1);
      ਰੋਕੋ;
    }

    ਹਾਲਤ WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      ਰੋਕੋ;
    }

    ਮੂਲ: {
      ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, iTab, addrRepeat, regElem, n_ਨਤੀਜਾ_ਖਾਨਾ);
      VdbeCoverage(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, regElem, n_ਨਤੀਜਾ_ਖਾਨਾ, r1);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iTab, r1, regElem, n_ਨਤੀਜਾ_ਖਾਨਾ);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
      iRet = iTab;
      ਰੋਕੋ;
    }
  }

  ਵਾਪਸ iRet;
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
ਸਥਿਰ ਖਾਲੀ fixDistinctOpenEph(
  Parse *pParse,     /* Parsing and code generating context */
  ਪੂਰਨ eTnctType,     /* WHERE_DISTINCT_* value */
  ਪੂਰਨ iVal,          /* Value returned by codeDistinct() */
  ਪੂਰਨ iOpenEphAddr   /* Address of OP_OpenEphemeral instruction for iTab */
){
  ਜੇ( pParse->nErr==0
   && (eTnctType==WHERE_DISTINCT_UNIQUE || eTnctType==WHERE_DISTINCT_ORDERED)
  ){
    Vdbe *v = pParse->pVdbe;
    sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, iOpenEphAddr);
    ਜੇ( sqlite3VdbeGetOp(v, iOpenEphAddr+1)->opcode==OP_Explain ){
      sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, iOpenEphAddr+1);
    }
    ਜੇ( eTnctType==WHERE_DISTINCT_ORDERED ){
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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
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
ਸਥਿਰ ਖਾਲੀ select_ਵਿਅੰਜਕ_Defer(
  Parse *pParse,                  /* Leave any error here */
  ਛਾਂਟੋ_Ctx *p_ਛਾਂਟੋ,                 /* Sorter context */
  ਵਿਅੰਜਕ_List *pEList,               /* Expressions destined for sorter */
  ਵਿਅੰਜਕ_List **ppExtra              /* Expressions to append to sorter record */
){
  ਪੂਰਨ i;
  ਪੂਰਨ nDefer = 0;
  ਵਿਅੰਜਕ_List *pExtra = 0;
  ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ = &pEList->a[i];
    ਜੇ( p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ==0 ){
      ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = p_ਵਸਤੂ->p_ਵਿਅੰਜਕ;
      ਸਾਰਣੀ *pTab;
      ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ
       && p_ਵਿਅੰਜਕ->i_ਕਾਲਮ>=0
       && ALWAYS( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) )
       && (pTab = p_ਵਿਅੰਜਕ->y.pTab)!=0
       && Is_Ordinary_ਸਾਰਣੀ(pTab)
       && (pTab->a_ਖਾਨਾ[p_ਵਿਅੰਜਕ->i_ਕਾਲਮ].ਖਾਨਾ_Flags & COLFLAG_SORTERREF)!=0
      ){
        ਪੂਰਨ j;
        ਲਈ(j=0; j<nDefer; j++){
          ਜੇ( p_ਛਾਂਟੋ->aDefer[j].iCsr==p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ ) ਰੋਕੋ;
        }
        ਜੇ( j==nDefer ){
          ਜੇ( nDefer==Array_ਮਾਪ(p_ਛਾਂਟੋ->aDefer) ){
            ਜਾਰੀ;
          }ਨਹੀਂਤਾਂ{
            ਪੂਰਨ n_ਕੁੰਜੀ = 1;
            ਪੂਰਨ k;
            ਸੂਚਕਾਂਕ *pPk = 0;
            ਜੇ( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
              n_ਕੁੰਜੀ = pPk->n_ਕੁੰਜੀ_ਖਾਨਾ;
            }
            ਲਈ(k=0; k<n_ਕੁੰਜੀ; k++){
              ਵਿਅੰਜਕ *pNew = sqlite3PExpr(pParse, TK_ਕਾਲਮ, 0, 0);
              ਜੇ( pNew ){
                pNew->i_ਸਾਰਣੀ = p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ;
                ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(pNew) );
                pNew->y.pTab = p_ਵਿਅੰਜਕ->y.pTab;
                pNew->i_ਕਾਲਮ = pPk ? pPk->ai_ਕਾਲਮ[k] : -1;
                pExtra = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pExtra, pNew);
              }
            }
            p_ਛਾਂਟੋ->aDefer[nDefer].pTab = p_ਵਿਅੰਜਕ->y.pTab;
            p_ਛਾਂਟੋ->aDefer[nDefer].iCsr = p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ;
            p_ਛਾਂਟੋ->aDefer[nDefer].n_ਕੁੰਜੀ = n_ਕੁੰਜੀ;
            nDefer++;
          }
        }
        p_ਵਸਤੂ->fg.bSorterRef = 1;
      }
    }
  }
  p_ਛਾਂਟੋ->nDefer = (u8)nDefer;
  *ppExtra = pExtra;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
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
ਸਥਿਰ ਪੂਰਨ sqlite3CursorBloomable(Parse *pParse, ਪੂਰਨ iCur, ਪੂਰਨ n_ਖਾਨਾ){
  ਪੂਰਨ i,k;
  Vdbe *v = pParse->pVdbe;
  ਜੇ( pParse->nErr ) ਵਾਪਸ 1;
  ਪ੍ਰਤਿਜ੍ਞਾ( v );
  ਲਈ(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    ਅਚਲ VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    ਅਚਲ ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;
    ਜੇ( pOp->p1!=iCur ) ਜਾਰੀ;
    ਜੇ( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      ਜਾਰੀ;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pOp->p4type==P4_KEYINFO );
    p_ਕੁੰਜੀ_Info = (ਅਚਲ ਕੁੰਜੀ_Info*)pOp->p4.p_ਕੁੰਜੀ_Info;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ_Info!=0 );
    ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Is_ਦੁਆਧਾਰੀ(p_ਕੁੰਜੀ_Info->aColl[i]) );
    }
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */


/*
** This routine generates the code for the inside of the inner loop
** of a SELECT.
**
** If srcTab is negative, then the p->pEList expressions
** are evaluated in order to get the data for this row.  If srcTab is
** zero or more, then data is pulled from srcTab and p->pEList is used only
** to get the number of columns and the collation sequence for each column.
*/
ਸਥਿਰ ਖਾਲੀ selectInnerLoop(
  Parse *pParse,          /* The parser context */
  Select *p,              /* The complete select statement being coded */
  ਪੂਰਨ srcTab,             /* Pull data from this table if non-negative */
  ਛਾਂਟੋ_Ctx *p_ਛਾਂਟੋ,         /* If not NULL, info on how to process ORDER BY */
  DistinctCtx *pDistinct, /* If not NULL, info on how to process DISTINCT */
  SelectDest *pDest,      /* How to dispose of the results */
  ਪੂਰਨ iContinue,          /* Jump here to continue with next row */
  ਪੂਰਨ iBreak              /* Jump here to break out of the inner loop */
){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ i;
  ਪੂਰਨ hasDistinct;            /* True if the DISTINCT keyword is present */
  ਪੂਰਨ eDest = pDest->eDest;   /* How to dispose of results */
  ਪੂਰਨ iParm = pDest->iSDParm; /* First argument to disposal method */
  ਪੂਰਨ n_ਨਤੀਜਾ_ਖਾਨਾ;             /* Number of result columns */
  ਪੂਰਨ n_ਅਗੇਤਰ_Reg = 0;         /* Number of extra registers before regResult */
  ਪੰਕਤੀ_ਲੱਦੋ_Info s_ਪੰਕਤੀ_ਲੱਦੋ_Info;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  ਪੂਰਨ reg_ਨਤੀਜਾ;              /* Start of memory holding current results */
  ਪੂਰਨ regOrig;                /* Start of memory holding full result (or 0) */

  ਪ੍ਰਤਿਜ੍ਞਾ( v );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  ਜੇ( p_ਛਾਂਟੋ && p_ਛਾਂਟੋ->p_ਕ੍ਰਮ_By==0 ) p_ਛਾਂਟੋ = 0;
  ਜੇ( p_ਛਾਂਟੋ==0 && !hasDistinct ){
    ਪ੍ਰਤਿਜ੍ਞਾ( iContinue!=0 );
    ਕੋਡ_ਵਿੱਥ(v, p->i_ਵਿੱਥ, iContinue);
  }

  /* Pull the requested columns.
  */
  n_ਨਤੀਜਾ_ਖਾਨਾ = p->pEList->n_ਵਿਅੰਜਕ;

  ਜੇ( pDest->iSdst==0 ){
    ਜੇ( p_ਛਾਂਟੋ ){
      n_ਅਗੇਤਰ_Reg = p_ਛਾਂਟੋ->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ;
      ਜੇ( !(p_ਛਾਂਟੋ->ਛਾਂਟੋ_Flags & SORTFLAG_UseSorter) ) n_ਅਗੇਤਰ_Reg++;
      pParse->nMem += n_ਅਗੇਤਰ_Reg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += n_ਨਤੀਜਾ_ਖਾਨਾ;
  }ਨਹੀਂਤਾਂ ਜੇ( pDest->iSdst+n_ਨਤੀਜਾ_ਖਾਨਾ > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += n_ਨਤੀਜਾ_ਖਾਨਾ;
  }
  pDest->nSdst = n_ਨਤੀਜਾ_ਖਾਨਾ;
  regOrig = reg_ਨਤੀਜਾ = pDest->iSdst;
  ਜੇ( srcTab>=0 ){
    ਲਈ(i=0; i<n_ਨਤੀਜਾ_ਖਾਨਾ; i++){
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, srcTab, i, reg_ਨਤੀਜਾ+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }ਨਹੀਂਤਾਂ ਜੇ( eDest!=SRT_Exists ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
    ਵਿਅੰਜਕ_List *pExtra = 0;
#ਜੇ_ਅੰਤ
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    ਵਿਅੰਜਕ_List *pEList;
    ਜੇ( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }ਨਹੀਂਤਾਂ{
      ecelFlags = 0;
    }
    ਜੇ( p_ਛਾਂਟੋ && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_ਸਾਰਣੀ ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      ਲਈ(i=p_ਛਾਂਟੋ->nOBSat; i<p_ਛਾਂਟੋ->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; i++){
        ਪੂਰਨ j;
        ਜੇ( (j = p_ਛਾਂਟੋ->p_ਕ੍ਰਮ_By->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ)>0 ){
          p->pEList->a[j-1].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = i+1-p_ਛਾਂਟੋ->nOBSat;
        }
      }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
      select_ਵਿਅੰਜਕ_Defer(pParse, p_ਛਾਂਟੋ, p->pEList, &pExtra);
      ਜੇ( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, p_ਛਾਂਟੋ->addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ);
        pOp->p2 += (pExtra->n_ਵਿਅੰਜਕ - p_ਛਾਂਟੋ->nDefer);
        pOp->p4.p_ਕੁੰਜੀ_Info->n_All_ਖੇਤਰ += (pExtra->n_ਵਿਅੰਜਕ - p_ਛਾਂਟੋ->nDefer);
        pParse->nMem += pExtra->n_ਵਿਅੰਜਕ;
      }
#ਜੇ_ਅੰਤ

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
        ਜੇ( pEList->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ>0
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
         || pEList->a[i].fg.bSorterRef
#ਜੇ_ਅੰਤ
        ){
          n_ਨਤੀਜਾ_ਖਾਨਾ--;
          regOrig = 0;
        }
      }

      testcase( regOrig );
      testcase( eDest==SRT_ਸੈੱਟ );
      testcase( eDest==SRT_Mem );
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      ਪ੍ਰਤਿਜ੍ਞਾ( eDest==SRT_ਸੈੱਟ || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    s_ਪੰਕਤੀ_ਲੱਦੋ_Info.reg_ਨਤੀਜਾ = reg_ਨਤੀਜਾ;
    s_ਪੰਕਤੀ_ਲੱਦੋ_Info.ecelFlags = ecelFlags;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
    s_ਪੰਕਤੀ_ਲੱਦੋ_Info.pExtra = pExtra;
    s_ਪੰਕਤੀ_ਲੱਦੋ_Info.reg_Extra_ਨਤੀਜਾ = reg_ਨਤੀਜਾ + n_ਨਤੀਜਾ_ਖਾਨਾ;
    ਜੇ( pExtra ) n_ਨਤੀਜਾ_ਖਾਨਾ += pExtra->n_ਵਿਅੰਜਕ;
#ਜੇ_ਅੰਤ
    ਜੇ( p->i_ਹੱਦਬੰਦੀ
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && n_ਅਗੇਤਰ_Reg>0
    ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਛਾਂਟੋ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( hasDistinct==0 );
      p_ਛਾਂਟੋ->p_Deferred_ਪੰਕਤੀ_ਲੱਦੋ = &s_ਪੰਕਤੀ_ਲੱਦੋ_Info;
      regOrig = 0;
    }ਨਹੀਂਤਾਂ{
      inner_Loop_ਲੱਦੋ_ਪੰਕਤੀ(pParse, p, &s_ਪੰਕਤੀ_ਲੱਦੋ_Info);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  ਜੇ( hasDistinct ){
    ਪੂਰਨ eType = pDistinct->eTnctType;
    ਪੂਰਨ iTab = pDistinct->tabTnct;
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਨਤੀਜਾ_ਖਾਨਾ==p->pEList->n_ਵਿਅੰਜਕ );
    iTab = ਕੋਡ_Distinct(pParse, eType, iTab, iContinue, p->pEList, reg_ਨਤੀਜਾ);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    ਜੇ( p_ਛਾਂਟੋ==0 ){
      ਕੋਡ_ਵਿੱਥ(v, p->i_ਵਿੱਥ, iContinue);
    }
  }

  ਚੋਣ( eDest ){
    /* Store the result as data using a unique key.
    */
    ਹਾਲਤ SRT_Fifo:
    ਹਾਲਤ SRT_DistFifo:
    ਹਾਲਤ SRT_ਸਾਰਣੀ:
    ਹਾਲਤ SRT_EphemTab: {
      ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_ਅਗੇਤਰ_Reg+1);
      testcase( eDest==SRT_ਸਾਰਣੀ );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg_ਨਤੀਜਾ, n_ਨਤੀਜਾ_ਖਾਨਾ, r1+n_ਅਗੇਤਰ_Reg);
#ਜੇ !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ਜੇ( eDest==SRT_ਸਾਰਣੀ && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
      ਜੇ( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        ਪੂਰਨ addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iParm+1, r1,reg_ਨਤੀਜਾ,n_ਨਤੀਜਾ_ਖਾਨਾ);
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਛਾਂਟੋ==0 );
      }
#ਜੇ_ਅੰਤ
      ਜੇ( p_ਛਾਂਟੋ ){
        ਪ੍ਰਤਿਜ੍ਞਾ( reg_ਨਤੀਜਾ==regOrig );
        ਧੱਕੋ_Onto_Sorter(pParse, p_ਛਾਂਟੋ, p, r1+n_ਅਗੇਤਰ_Reg, regOrig, 1, n_ਅਗੇਤਰ_Reg);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ r2 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, iParm, r2);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r2);
      }
      sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, r1, n_ਅਗੇਤਰ_Reg+1);
      ਰੋਕੋ;
    }

    ਹਾਲਤ SRT_Upfrom: {
      ਜੇ( p_ਛਾਂਟੋ ){
        ਧੱਕੋ_Onto_Sorter(
            pParse, p_ਛਾਂਟੋ, p, reg_ਨਤੀਜਾ, regOrig, n_ਨਤੀਜਾ_ਖਾਨਾ, n_ਅਗੇਤਰ_Reg);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ i2 = pDest->iSDParm2;
        ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IsNull, reg_ਨਤੀਜਾ, iBreak); VdbeCoverage(v);

        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ,
                          reg_ਨਤੀਜਾ+(i2<0), n_ਨਤੀਜਾ_ਖਾਨਾ-(i2<0), r1);
        ਜੇ( i2<0 ){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iParm, r1, reg_ਨਤੀਜਾ);
        }ਨਹੀਂਤਾਂ{
          sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iParm, r1, reg_ਨਤੀਜਾ, i2);
        }
      }
      ਰੋਕੋ;
    }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    ਹਾਲਤ SRT_ਸੈੱਟ: {
      ਜੇ( p_ਛਾਂਟੋ ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        ਧੱਕੋ_Onto_Sorter(
            pParse, p_ਛਾਂਟੋ, p, reg_ਨਤੀਜਾ, regOrig, n_ਨਤੀਜਾ_ਖਾਨਾ, n_ਅਗੇਤਰ_Reg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3Strlen30(pDest->zAffSdst)==n_ਨਤੀਜਾ_ਖਾਨਾ );
        sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Make_ਰਿਕਾਰਡ, reg_ਨਤੀਜਾ, n_ਨਤੀਜਾ_ਖਾਨਾ,
            r1, pDest->zAffSdst, n_ਨਤੀਜਾ_ਖਾਨਾ);
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iParm, r1, reg_ਨਤੀਜਾ, n_ਨਤੀਜਾ_ਖਾਨਾ);
        ਜੇ( pDest->iSDParm2 ){
          ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3CursorBloomable(pParse,iParm,n_ਨਤੀਜਾ_ਖਾਨਾ) );
          sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter_ਜੋੜੋ, pDest->iSDParm2, 0,
                               reg_ਨਤੀਜਾ, n_ਨਤੀਜਾ_ਖਾਨਾ);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
      }
      ਰੋਕੋ;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    ਹਾਲਤ SRT_Exists: {
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      ਰੋਕੋ;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    ਹਾਲਤ SRT_Mem: {
      ਜੇ( p_ਛਾਂਟੋ ){
        ਪ੍ਰਤਿਜ੍ਞਾ( n_ਨਤੀਜਾ_ਖਾਨਾ<=pDest->nSdst );
        ਧੱਕੋ_Onto_Sorter(
            pParse, p_ਛਾਂਟੋ, p, reg_ਨਤੀਜਾ, regOrig, n_ਨਤੀਜਾ_ਖਾਨਾ, n_ਅਗੇਤਰ_Reg);
        pDest->iSDParm = reg_ਨਤੀਜਾ;
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( n_ਨਤੀਜਾ_ਖਾਨਾ==pDest->nSdst );
        ਜੇ( reg_ਨਤੀਜਾ!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Copy, reg_ਨਤੀਜਾ, iParm, n_ਨਤੀਜਾ_ਖਾਨਾ-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* #ifndef SQLITE_OMIT_SUBQUERY */

    ਹਾਲਤ SRT_Coroutine:       /* Send data to a co-routine */
    ਹਾਲਤ SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      ਜੇ( p_ਛਾਂਟੋ ){
        ਧੱਕੋ_Onto_Sorter(pParse, p_ਛਾਂਟੋ, p, reg_ਨਤੀਜਾ, regOrig, n_ਨਤੀਜਾ_ਖਾਨਾ,
                       n_ਅਗੇਤਰ_Reg);
      }ਨਹੀਂਤਾਂ ਜੇ( eDest==SRT_Coroutine ){
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Yield, pDest->iSDParm);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, reg_ਨਤੀਜਾ, n_ਨਤੀਜਾ_ਖਾਨਾ);
      }
      ਰੋਕੋ;
    }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    ਹਾਲਤ SRT_Dist_ਕਤਾਰ:
    ਹਾਲਤ SRT_ਕਤਾਰ: {
      ਪੂਰਨ n_ਕੁੰਜੀ;
      ਪੂਰਨ r1, r2, r3;
      ਪੂਰਨ addrTest = 0;
      ਵਿਅੰਜਕ_List *pSO;
      pSO = pDest->p_ਕ੍ਰਮ_By;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSO );
      n_ਕੁੰਜੀ = pSO->n_ਵਿਅੰਜਕ;
      r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      r2 = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_ਕੁੰਜੀ+2);
      r3 = r2+n_ਕੁੰਜੀ+1;
      ਜੇ( eDest==SRT_Dist_ਕਤਾਰ ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_ਮਿਲਿਆ, iParm+1, 0,
                                        reg_ਨਤੀਜਾ, n_ਨਤੀਜਾ_ਖਾਨਾ);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg_ਨਤੀਜਾ, n_ਨਤੀਜਾ_ਖਾਨਾ, r3);
      ਜੇ( eDest==SRT_Dist_ਕਤਾਰ ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Idx_ਪਾਓ, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      ਲਈ(i=0; i<n_ਕੁੰਜੀ; i++){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_SCopy,
                          reg_ਨਤੀਜਾ + pSO->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ - 1,
                          r2+i);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Sequence, iParm, r2+n_ਕੁੰਜੀ);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, r2, n_ਕੁੰਜੀ+2, r1);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iParm, r1, r2, n_ਕੁੰਜੀ+2);
      ਜੇ( addrTest ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrTest);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
      sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, r2, n_ਕੁੰਜੀ+2);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_CTE */



#ਜੇ !defined(SQLITE_OMIT_TRIGGER)
    /* Discard the results.  This is used for SELECT statements inside
    ** the body of a TRIGGER.  The purpose of such selects is to call
    ** user-defined functions that have side effects.  We do not care
    ** about the actual results of the select.
    */
    ਮੂਲ: {
      ਪ੍ਰਤਿਜ੍ਞਾ( eDest==SRT_Discard );
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  ਜੇ( p_ਛਾਂਟੋ==0 && p->i_ਹੱਦਬੰਦੀ ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_DecrJumpZero, p->i_ਹੱਦਬੰਦੀ, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
ਕੁੰਜੀ_Info *sqlite3_ਕੁੰਜੀ_Info_Alloc(sqlite3 *db, ਪੂਰਨ N, ਪੂਰਨ X){
  ਪੂਰਨ nExtra = (N+X)*(ਆਕਾਰ(CollSeq*)+1);
  ਕੁੰਜੀ_Info *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( X>=0 );
  ਜੇ( NEVER(N+X>0xffff) ) ਵਾਪਸ (ਕੁੰਜੀ_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  ਜੇ( p ){
    p->a_ਛਾਂਟੋ_Flags = (u8*)&p->aColl[N+X];
    p->n_ਕੁੰਜੀ_ਖੇਤਰ = (u16)N;
    p->n_All_ਖੇਤਰ = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ (ਕੁੰਜੀ_Info*)sqlite3OomFault(db);
  }
  ਵਾਪਸ p;
}

/*
** Deallocate a KeyInfo object
*/
ਖਾਲੀ sqlite3_ਕੁੰਜੀ_Info_Unref(ਕੁੰਜੀ_Info *p){
  ਜੇ( p ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->db!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef>0 );
    p->nRef--;
    ਜੇ( p->nRef==0 ) sqlite3DbNNFreeNN(p->db, p);
  }
}

/*
** Make a new pointer to a KeyInfo object
*/
ਕੁੰਜੀ_Info *sqlite3_ਕੁੰਜੀ_Info_Ref(ਕੁੰਜੀ_Info *p){
  ਜੇ( p ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->nRef>0 );
    p->nRef++;
  }
  ਵਾਪਸ p;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
ਪੂਰਨ sqlite3_ਕੁੰਜੀ_Info_Is_Writeable(ਕੁੰਜੀ_Info *p){ ਵਾਪਸ p->nRef==1; }
#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */

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
ਕੁੰਜੀ_Info *sqlite3_ਕੁੰਜੀ_Info_From_ਵਿਅੰਜਕ_List(
  Parse *pParse,       /* Parsing context */
  ਵਿਅੰਜਕ_List *pList,     /* Form the KeyInfo object from this ExprList */
  ਪੂਰਨ i_ਸ਼ੁਰੂ,          /* Begin with this column of pList */
  ਪੂਰਨ nExtra           /* Add this many extra columns to the end */
){
  ਪੂਰਨ n_ਵਿਅੰਜਕ;
  ਕੁੰਜੀ_Info *pInfo;
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
  sqlite3 *db = pParse->db;
  ਪੂਰਨ i;

  n_ਵਿਅੰਜਕ = pList->n_ਵਿਅੰਜਕ;
  pInfo = sqlite3_ਕੁੰਜੀ_Info_Alloc(db, n_ਵਿਅੰਜਕ-i_ਸ਼ੁਰੂ, nExtra+1);
  ਜੇ( pInfo ){
    ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਕੁੰਜੀ_Info_Is_Writeable(pInfo) );
    ਲਈ(i=i_ਸ਼ੁਰੂ, p_ਵਸਤੂ=pList->a+i_ਸ਼ੁਰੂ; i<n_ਵਿਅੰਜਕ; i++, p_ਵਸਤੂ++){
      pInfo->aColl[i-i_ਸ਼ੁਰੂ] = sqlite3_ਵਿਅੰਜਕ_NNColl_Seq(pParse, p_ਵਸਤੂ->p_ਵਿਅੰਜਕ);
      pInfo->a_ਛਾਂਟੋ_Flags[i-i_ਸ਼ੁਰੂ] = p_ਵਸਤੂ->fg.ਛਾਂਟੋ_Flags;
    }
  }
  ਵਾਪਸ pInfo;
}

/*
** Name of the connection operator, used for error messages.
*/
ਅਚਲ ਅੱਖਰ *sqlite3SelectOpName(ਪੂਰਨ id){
  ਅੱਖਰ *z;
  ਚੋਣ( id ){
    ਹਾਲਤ TK_ALL:       z = "UNION ALL";   ਰੋਕੋ;
    ਹਾਲਤ TK_INTERSECT: z = "INTERSECT";   ਰੋਕੋ;
    ਹਾਲਤ TK_EXCEPT:    z = "EXCEPT";      ਰੋਕੋ;
    ਮੂਲ:           z = "UNION";       ਰੋਕੋ;
  }
  ਵਾਪਸ z;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
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
ਸਥਿਰ ਖਾਲੀ explain_ਆਰਜ਼ੀ_ਸਾਰਣੀ(Parse *pParse, ਅਚਲ ਅੱਖਰ *zUsage){
  ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s", zUsage));
}

/*
** Assign expression b to lvalue a. A second, no-op, version of this macro
** is provided when SQLITE_OMIT_EXPLAIN is defined. This allows the code
** in sqlite3Select() to assign values to structure member variables that
** only exist if SQLITE_OMIT_EXPLAIN is not defined without polluting the
** code with #ifndef directives.
*/
# define explain_ਸੈੱਟ_Integer(a, b) a = b

#ਨਹੀਂਤਾਂ
/* No-op versions of the explainXXX() functions and macros. */
# define explain_ਆਰਜ਼ੀ_ਸਾਰਣੀ(y,z)
# define explain_ਸੈੱਟ_Integer(y,z)
#ਜੇ_ਅੰਤ


/*
** If the inner loop was generated using a non-null pOrderBy argument,
** then the results were placed in a sorter.  After the loop is terminated
** we need to run the sorter and output the results.  The following
** routine generates the code needed to do that.
*/
ਸਥਿਰ ਖਾਲੀ generate_ਛਾਂਟੋ_ਪੂਛ(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  ਛਾਂਟੋ_Ctx *p_ਛਾਂਟੋ,   /* Information on the ORDER BY clause */
  ਪੂਰਨ n_ਕਾਲਮ,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  ਪੂਰਨ addrBreak = p_ਛਾਂਟੋ->label_ਪੂਰਾ;            /* Jump here to exit loop */
  ਪੂਰਨ addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  ਪੂਰਨ addr;                       /* Top of output loop. Jump for Next. */
  ਪੂਰਨ addrOnce = 0;
  ਪੂਰਨ iTab;
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By = p_ਛਾਂਟੋ->p_ਕ੍ਰਮ_By;
  ਪੂਰਨ eDest = pDest->eDest;
  ਪੂਰਨ iParm = pDest->iSDParm;
  ਪੂਰਨ reg_ਪੰਕਤੀ;
  ਪੂਰਨ regRowid;
  ਪੂਰਨ i_ਖਾਨਾ;
  ਪੂਰਨ n_ਕੁੰਜੀ;                       /* Number of key columns in sorter record */
  ਪੂਰਨ i_ਛਾਂਟੋ_Tab;                   /* Sorter cursor to read from */
  ਪੂਰਨ i;
  ਪੂਰਨ bSeq;                       /* True if sorter record includes seq. no. */
  ਪੂਰਨ n_Ref_ਕੁੰਜੀ = 0;
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *a_ਬਾਹਰਲਾ_Ex = p->pEList->a;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
  ਪੂਰਨ addrExplain;                /* Address of OP_Explain instruction */
#ਜੇ_ਅੰਤ

  n_ਕੁੰਜੀ = p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ - p_ਛਾਂਟੋ->nOBSat;
  ਜੇ( p_ਛਾਂਟੋ->nOBSat==0 || n_ਕੁੰਜੀ==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", p_ਛਾਂਟੋ->nOBSat?"LAST TERM OF ":""
    ));
  }ਨਹੀਂਤਾਂ{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_ਕੁੰਜੀ
    ));
  }
  sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExplain,p_ਛਾਂਟੋ->addr_ਧੱਕੋ,p_ਛਾਂਟੋ->addr_ਧੱਕੋ_ਸਿਰਾ);
  sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Counters(v, addrExplain, addrExplain, p_ਛਾਂਟੋ->addr_ਧੱਕੋ);


  ਪ੍ਰਤਿਜ੍ਞਾ( addrBreak<0 );
  ਜੇ( p_ਛਾਂਟੋ->label_Bk_ਬਾਹਰਲਾ ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, p_ਛਾਂਟੋ->regReturn, p_ਛਾਂਟੋ->label_Bk_ਬਾਹਰਲਾ);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, p_ਛਾਂਟੋ->label_Bk_ਬਾਹਰਲਾ);
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  ਲਈ(i=0; i<p_ਛਾਂਟੋ->nDefer; i++){
    ਸਾਰਣੀ *pTab = p_ਛਾਂਟੋ->aDefer[i].pTab;
    ਪੂਰਨ iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, pTab->pSchema);
    sqlite3_Open_ਸਾਰਣੀ(pParse, p_ਛਾਂਟੋ->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, p_ਛਾਂਟੋ->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_ਕੁੰਜੀ = MAX(n_Ref_ਕੁੰਜੀ, p_ਛਾਂਟੋ->aDefer[i].n_ਕੁੰਜੀ);
  }
#ਜੇ_ਅੰਤ

  iTab = p_ਛਾਂਟੋ->iECursor;
  ਜੇ( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    ਜੇ( eDest==SRT_Mem && p->i_ਵਿੱਥ ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    reg_ਪੰਕਤੀ = pDest->iSdst;
  }ਨਹੀਂਤਾਂ{
    regRowid = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
    ਜੇ( eDest==SRT_EphemTab || eDest==SRT_ਸਾਰਣੀ ){
      reg_ਪੰਕਤੀ = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      n_ਕਾਲਮ = 0;
    }ਨਹੀਂਤਾਂ{
      reg_ਪੰਕਤੀ = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_ਕਾਲਮ);
    }
  }
  ਜੇ( p_ਛਾਂਟੋ->ਛਾਂਟੋ_Flags & SORTFLAG_UseSorter ){
    ਪੂਰਨ reg_ਛਾਂਟੋ_ਬਾਹਰਲਾ = ++pParse->nMem;
    i_ਛਾਂਟੋ_Tab = pParse->nTab++;
    ਜੇ( p_ਛਾਂਟੋ->label_Bk_ਬਾਹਰਲਾ ){
      addrOnce = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenPseudo, i_ਛਾਂਟੋ_Tab, reg_ਛਾਂਟੋ_ਬਾਹਰਲਾ,
        n_ਕੁੰਜੀ+1+n_ਕਾਲਮ+n_Ref_ਕੁੰਜੀ);
    ਜੇ( addrOnce ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addrOnce);
    addr = 1 + sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Sorter_ਛਾਂਟੋ, iTab, addrBreak);
    VdbeCoverage(v);
    ਪ੍ਰਤਿਜ੍ਞਾ( p->i_ਹੱਦਬੰਦੀ==0 && p->i_ਵਿੱਥ==0 );
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Sorter_ਅੰਕੜੇ, iTab, reg_ਛਾਂਟੋ_ਬਾਹਰਲਾ, i_ਛਾਂਟੋ_Tab);
    bSeq = 0;
  }ਨਹੀਂਤਾਂ{
    addr = 1 + sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਛਾਂਟੋ, iTab, addrBreak); VdbeCoverage(v);
    ਕੋਡ_ਵਿੱਥ(v, p->i_ਵਿੱਥ, addrContinue);
    i_ਛਾਂਟੋ_Tab = iTab;
    bSeq = 1;
    ਜੇ( p->i_ਵਿੱਥ>0 ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਜੋੜੋ_Imm, p->i_ਹੱਦਬੰਦੀ, -1);
    }
  }
  ਲਈ(i=0, i_ਖਾਨਾ=n_ਕੁੰਜੀ+bSeq-1; i<n_ਕਾਲਮ; i++){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
    ਜੇ( a_ਬਾਹਰਲਾ_Ex[i].fg.bSorterRef ) ਜਾਰੀ;
#ਜੇ_ਅੰਤ
    ਜੇ( a_ਬਾਹਰਲਾ_Ex[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ==0 ) i_ਖਾਨਾ++;
  }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
  ਜੇ( p_ਛਾਂਟੋ->nDefer ){
    ਪੂਰਨ i_ਕੁੰਜੀ = i_ਖਾਨਾ+1;
    ਪੂਰਨ reg_ਕੁੰਜੀ = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_Ref_ਕੁੰਜੀ);

    ਲਈ(i=0; i<p_ਛਾਂਟੋ->nDefer; i++){
      ਪੂਰਨ iCsr = p_ਛਾਂਟੋ->aDefer[i].iCsr;
      ਸਾਰਣੀ *pTab = p_ਛਾਂਟੋ->aDefer[i].pTab;
      ਪੂਰਨ n_ਕੁੰਜੀ = p_ਛਾਂਟੋ->aDefer[i].n_ਕੁੰਜੀ;

      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, iCsr);
      ਜੇ( HasRowid(pTab) ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, i_ਛਾਂਟੋ_Tab, i_ਕੁੰਜੀ++, reg_ਕੁੰਜੀ);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_ਕੁੰਜੀ);
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ k;
        ਪੂਰਨ iJmp;
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab)->n_ਕੁੰਜੀ_ਖਾਨਾ==n_ਕੁੰਜੀ );
        ਲਈ(k=0; k<n_ਕੁੰਜੀ; k++){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, i_ਛਾਂਟੋ_Tab, i_ਕੁੰਜੀ++, reg_ਕੁੰਜੀ+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_SeekGE, iCsr, iJmp+2, reg_ਕੁੰਜੀ, n_ਕੁੰਜੀ);
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_IdxLE, iCsr, iJmp+3, reg_ਕੁੰਜੀ, n_ਕੁੰਜੀ);
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, iCsr);
      }
    }
    sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, reg_ਕੁੰਜੀ, n_Ref_ਕੁੰਜੀ);
  }
#ਜੇ_ਅੰਤ
  ਲਈ(i=n_ਕਾਲਮ-1; i>=0; i--){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_SORTER_REFERENCES
    ਜੇ( a_ਬਾਹਰਲਾ_Ex[i].fg.bSorterRef ){
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, a_ਬਾਹਰਲਾ_Ex[i].p_ਵਿਅੰਜਕ, reg_ਪੰਕਤੀ+i);
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      ਪੂਰਨ iRead;
      ਜੇ( a_ਬਾਹਰਲਾ_Ex[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ ){
        iRead = a_ਬਾਹਰਲਾ_Ex[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ-1;
      }ਨਹੀਂਤਾਂ{
        iRead = i_ਖਾਨਾ--;
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, i_ਛਾਂਟੋ_Tab, iRead, reg_ਪੰਕਤੀ+i);
      VdbeComment((v, "%s", a_ਬਾਹਰਲਾ_Ex[i].zEName));
    }
  }
  sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExplain, addrExplain, -1);
  ਚੋਣ( eDest ){
    ਹਾਲਤ SRT_ਸਾਰਣੀ:
    ਹਾਲਤ SRT_EphemTab: {
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, i_ਛਾਂਟੋ_Tab, n_ਕੁੰਜੀ+bSeq, reg_ਪੰਕਤੀ);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, iParm, regRowid);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iParm, reg_ਪੰਕਤੀ, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    ਹਾਲਤ SRT_ਸੈੱਟ: {
      ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕਾਲਮ==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Make_ਰਿਕਾਰਡ, reg_ਪੰਕਤੀ, n_ਕਾਲਮ, regRowid,
                        pDest->zAffSdst, n_ਕਾਲਮ);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iParm, regRowid, reg_ਪੰਕਤੀ, n_ਕਾਲਮ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ SRT_Upfrom: {
      ਪੂਰਨ i2 = pDest->iSDParm2;
      ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ,reg_ਪੰਕਤੀ+(i2<0),n_ਕਾਲਮ-(i2<0),r1);
      ਜੇ( i2<0 ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iParm, r1, reg_ਪੰਕਤੀ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iParm, r1, reg_ਪੰਕਤੀ, i2);
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      ਪ੍ਰਤਿਜ੍ਞਾ( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      ਜੇ( eDest==SRT_Output ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, pDest->iSdst, n_ਕਾਲਮ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Yield, pDest->iSDParm);
      }
      ਰੋਕੋ;
    }
  }
  ਜੇ( regRowid ){
    ਜੇ( eDest==SRT_ਸੈੱਟ ){
      sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, reg_ਪੰਕਤੀ, n_ਕਾਲਮ);
    }ਨਹੀਂਤਾਂ{
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, reg_ਪੰਕਤੀ);
    }
    sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  ਜੇ( p_ਛਾਂਟੋ->ਛਾਂਟੋ_Flags & SORTFLAG_UseSorter ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Sorter_ਅਗਲਾ, iTab, addr); VdbeCoverage(v);
  }ਨਹੀਂਤਾਂ{
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, iTab, addr); VdbeCoverage(v);
  }
  sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  ਜੇ( p_ਛਾਂਟੋ->regReturn ) sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Return, p_ਛਾਂਟੋ->regReturn);
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_METADATA
# define ਕਾਲਮ_Type(A,B,C,D,E) ਕਾਲਮ_Type_Impl(A,B,C,D,E)
#ਨਹੀਂਤਾਂ /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define ਕਾਲਮ_Type(A,B,C,D,E) ਕਾਲਮ_Type_Impl(A,B)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਕਾਲਮ_Type_Impl(
  NameContext *pNC,
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_METADATA
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ
#ਨਹੀਂਤਾਂ
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,
  ਅਚਲ ਅੱਖਰ **pzOrigDb,
  ਅਚਲ ਅੱਖਰ **pzOrigTab,
  ਅਚਲ ਅੱਖਰ **pz_Orig_ਖਾਨਾ
#ਜੇ_ਅੰਤ
){
  ਅੱਖਰ ਅਚਲ *zType = 0;
  ਪੂਰਨ j;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_METADATA
  ਅੱਖਰ ਅਚਲ *zOrigDb = 0;
  ਅੱਖਰ ਅਚਲ *zOrigTab = 0;
  ਅੱਖਰ ਅਚਲ *z_Orig_ਖਾਨਾ = 0;
#ਜੇ_ਅੰਤ

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pNC->pSrcList!=0 );
  ਚੋਣ( p_ਵਿਅੰਜਕ->op ){
    ਹਾਲਤ TK_ਕਾਲਮ: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      ਸਾਰਣੀ *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      ਪੂਰਨ i_ਖਾਨਾ = p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;  /* Index of column in pTab */
      ਜਦੋਂ( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        ਲਈ(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ;j++);
        ਜੇ( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          ਜੇ( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }ਨਹੀਂਤਾਂ{
            pS = 0;
          }
        }ਨਹੀਂਤਾਂ{
          pNC = pNC->p_ਅਗਲਾ;
        }
      }

      ਜੇ( pTab==0 ){
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
        ਰੋਕੋ;
      }

      ਪ੍ਰਤਿਜ੍ਞਾ( pTab && ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) && p_ਵਿਅੰਜਕ->y.pTab==pTab );
      ਜੇ( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        ਜੇ( i_ਖਾਨਾ<pS->pEList->n_ਵਿਅੰਜਕ
         && (!ViewCanHaveRowid || i_ਖਾਨਾ>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          ਵਿਅੰਜਕ *p = pS->pEList->a[i_ਖਾਨਾ].p_ਵਿਅੰਜਕ;
          sNC.pSrcList = pS->pSrc;
          sNC.p_ਅਗਲਾ = pNC;
          sNC.pParse = pNC->pParse;
          zType = ਕਾਲਮ_Type(&sNC, p,&zOrigDb,&zOrigTab,&z_Orig_ਖਾਨਾ);
        }
      }ਨਹੀਂਤਾਂ{
        /* A real table or a CTE table */
        ਪ੍ਰਤਿਜ੍ਞਾ( !pS );
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_METADATA
        ਜੇ( i_ਖਾਨਾ<0 ) i_ਖਾਨਾ = pTab->iPKey;
        ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ==XN_ROWID || (i_ਖਾਨਾ>=0 && i_ਖਾਨਾ<pTab->n_ਖਾਨਾ) );
        ਜੇ( i_ਖਾਨਾ<0 ){
          zType = "INTEGER";
          z_Orig_ਖਾਨਾ = "rowid";
        }ਨਹੀਂਤਾਂ{
          z_Orig_ਖਾਨਾ = pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName;
          zType = sqlite3_ਕਾਲਮ_Type(&pTab->a_ਖਾਨਾ[i_ਖਾਨਾ],0);
        }
        zOrigTab = pTab->zName;
        ਜੇ( pNC->pParse && pTab->pSchema ){
          ਪੂਰਨ iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#ਨਹੀਂਤਾਂ
        ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ==XN_ROWID || (i_ਖਾਨਾ>=0 && i_ਖਾਨਾ<pTab->n_ਖਾਨਾ) );
        ਜੇ( i_ਖਾਨਾ<0 ){
          zType = "INTEGER";
        }ਨਹੀਂਤਾਂ{
          zType = sqlite3_ਕਾਲਮ_Type(&pTab->a_ਖਾਨਾ[i_ਖਾਨਾ],0);
        }
#ਜੇ_ਅੰਤ
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    ਹਾਲਤ TK_SELECT: {
      /* The expression is a sub-select. Return the declaration type and
      ** origin info for the single column in the result set of the SELECT
      ** statement.
      */
      NameContext sNC;
      Select *pS;
      ਵਿਅੰਜਕ *p;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) );
      pS = p_ਵਿਅੰਜਕ->x.pSelect;
      p = pS->pEList->a[0].p_ਵਿਅੰਜਕ;
      sNC.pSrcList = pS->pSrc;
      sNC.p_ਅਗਲਾ = pNC;
      sNC.pParse = pNC->pParse;
      zType = ਕਾਲਮ_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_ਖਾਨਾ);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_METADATA 
  ਜੇ( pzOrigDb ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pzOrigTab && pz_Orig_ਖਾਨਾ );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pz_Orig_ਖਾਨਾ = z_Orig_ਖਾਨਾ;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
ਸਥਿਰ ਖਾਲੀ generate_ਕਾਲਮ_Types(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  ਵਿਅੰਜਕ_List *pEList    /* Expressions defining the result set */
){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.p_ਅਗਲਾ = 0;
  ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
    ਵਿਅੰਜਕ *p = pEList->a[i].p_ਵਿਅੰਜਕ;
    ਅਚਲ ਅੱਖਰ *zType;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_ਕਾਲਮ_METADATA
    ਅਚਲ ਅੱਖਰ *zOrigDb = 0;
    ਅਚਲ ਅੱਖਰ *zOrigTab = 0;
    ਅਚਲ ਅੱਖਰ *z_Orig_ਖਾਨਾ = 0;
    zType = ਕਾਲਮ_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_ਖਾਨਾ);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, i, COLNAME_ਸਾਰਣੀ, zOrigTab, SQLITE_TRANSIENT);
    sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, i, COLNAME_ਕਾਲਮ, z_Orig_ਖਾਨਾ, SQLITE_TRANSIENT);
#ਨਹੀਂਤਾਂ
    zType = ਕਾਲਮ_Type(&sNC, p, 0, 0, 0);
#ਜੇ_ਅੰਤ
    sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(pParse);
  ਅਣਵਰਤਿਆ_PARAMETER(pTabList);
  ਅਣਵਰਤਿਆ_PARAMETER(pEList);
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_DECLTYPE) */
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
ਖਾਲੀ sqlite3_Generate_ਕਾਲਮ_ਨਾਂ(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ i;
  ਸਾਰਣੀ *pTab;
  SrcList *pTabList;
  ਵਿਅੰਜਕ_List *pEList;
  sqlite3 *db = pParse->db;
  ਪੂਰਨ ਭਰਿਆ_Name;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  ਪੂਰਨ srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  ਜੇ( pParse->ਖਾਨਾ_ਨਾਂ_ਸੈੱਟ ) ਵਾਪਸ;
  /* Column names are determined by the left-most term of a compound select */
  ਜਦੋਂ( pSelect->p_ਪੂਰਵ ) pSelect = pSelect->p_ਪੂਰਵ;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pTabList!=0 );
  pParse->ਖਾਨਾ_ਨਾਂ_ਸੈੱਟ = 1;
  ਭਰਿਆ_Name = (db->flags & SQLITE_ਭਰਿਆ_ਖਾਨਾ_ਨਾਂ)!=0;
  srcName = (db->flags & SQLITE_Short_ਖਾਨਾ_ਨਾਂ)!=0 || ਭਰਿਆ_Name;
  sqlite3_Vdbe_ਸੈੱਟ_ਅੰਕ_ਖਾਨੇ_ਗਿਣਤੀ(v, pEList->n_ਵਿਅੰਜਕ);
  ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
    ਵਿਅੰਜਕ *p = pEList->a[i].p_ਵਿਅੰਜਕ;

    ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->op!=TK_AGG_ਕਾਲਮ );  /* Agg processing has not run yet */
    ਪ੍ਰਤਿਜ੍ਞਾ( p->op!=TK_ਕਾਲਮ
        || (ਵਿਅੰਜਕ_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    ਜੇ( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      ਅੱਖਰ *zName = pEList->a[i].zEName;
      sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }ਨਹੀਂਤਾਂ ਜੇ( srcName && p->op==TK_ਕਾਲਮ ){
      ਅੱਖਰ *z_ਖਾਨਾ;
      ਪੂਰਨ i_ਖਾਨਾ = p->i_ਕਾਲਮ;
      pTab = p->y.pTab;
      ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
      ਜੇ( i_ਖਾਨਾ<0 ) i_ਖਾਨਾ = pTab->iPKey;
      ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ==-1 || (i_ਖਾਨਾ>=0 && i_ਖਾਨਾ<pTab->n_ਖਾਨਾ) );
      ਜੇ( i_ਖਾਨਾ<0 ){
        z_ਖਾਨਾ = "rowid";
      }ਨਹੀਂਤਾਂ{
        z_ਖਾਨਾ = pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName;
      }
      ਜੇ( ਭਰਿਆ_Name ){
        ਅੱਖਰ *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, z_ਖਾਨਾ);
        sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }ਨਹੀਂਤਾਂ{
        sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, i, COLNAME_NAME, z_ਖਾਨਾ, SQLITE_TRANSIENT);
      }
    }ਨਹੀਂਤਾਂ{
      ਅਚਲ ਅੱਖਰ *z = pEList->a[i].zEName;
      z = z==0 ? sqlite3MPrintf(db, "column%d", i+1) : sqlite3DbStrDup(db, z);
      sqlite3_Vdbe_ਸੈੱਟ_ਖਾਨਾ_Name(v, i, COLNAME_NAME, z, SQLITE_DYNAMIC);
    }
  }
  generate_ਕਾਲਮ_Types(pParse, pTabList, pEList);
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
ਪੂਰਨ sqlite3_Columns_From_ਵਿਅੰਜਕ_List(
  Parse *pParse,          /* Parsing context */
  ਵਿਅੰਜਕ_List *pEList,       /* Expr list from which to derive column names */
  i16 *pn_ਖਾਨਾ,             /* Write the number of columns here */
  ਕਾਲਮ **pa_ਖਾਨਾ          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  ਪੂਰਨ i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  ਕਾਲਮ *a_ਖਾਨਾ, *p_ਖਾਨਾ;        /* For looping over result columns */
  ਪੂਰਨ n_ਖਾਨਾ;                   /* Number of columns in the result set */
  ਅੱਖਰ *zName;                /* Column name */
  ਪੂਰਨ nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  ਸਾਰਣੀ *pTab;

  sqlite3HashInit(&ht);
  ਜੇ( pEList ){
    n_ਖਾਨਾ = pEList->n_ਵਿਅੰਜਕ;
    a_ਖਾਨਾ = sqlite3DbMallocZero(db, ਆਕਾਰ(a_ਖਾਨਾ[0])*n_ਖਾਨਾ);
    testcase( a_ਖਾਨਾ==0 );
    ਜੇ( n_ਖਾਨਾ>32767 ) n_ਖਾਨਾ = 32767;
  }ਨਹੀਂਤਾਂ{
    n_ਖਾਨਾ = 0;
    a_ਖਾਨਾ = 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( n_ਖਾਨਾ==(i16)n_ਖਾਨਾ );
  *pn_ਖਾਨਾ = n_ਖਾਨਾ;
  *pa_ਖਾਨਾ = a_ਖਾਨਾ;

  ਲਈ(i=0, p_ਖਾਨਾ=a_ਖਾਨਾ; i<n_ਖਾਨਾ && !pParse->nErr; i++, p_ਖਾਨਾ++){
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *pX = &pEList->a[i];
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *pCollide;
    /* Get an appropriate name for the column
    */
    ਜੇ( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }ਨਹੀਂਤਾਂ{
      ਵਿਅੰਜਕ *p_ਖਾਨਾ_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(pX->p_ਵਿਅੰਜਕ);
      ਜਦੋਂ( ALWAYS(p_ਖਾਨਾ_ਵਿਅੰਜਕ!=0) && p_ਖਾਨਾ_ਵਿਅੰਜਕ->op==TK_DOT ){
        p_ਖਾਨਾ_ਵਿਅੰਜਕ = p_ਖਾਨਾ_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖਾਨਾ_ਵਿਅੰਜਕ!=0 );
      }
      ਜੇ( p_ਖਾਨਾ_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ
       && ALWAYS( ਵਿਅੰਜਕ_Use_YTab(p_ਖਾਨਾ_ਵਿਅੰਜਕ) )
       && ALWAYS( p_ਖਾਨਾ_ਵਿਅੰਜਕ->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        ਪੂਰਨ i_ਖਾਨਾ = p_ਖਾਨਾ_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
        pTab = p_ਖਾਨਾ_ਵਿਅੰਜਕ->y.pTab;
        ਜੇ( i_ਖਾਨਾ<0 ) i_ਖਾਨਾ = pTab->iPKey;
        zName = i_ਖਾਨਾ>=0 ? pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].zCnName : "rowid";
      }ਨਹੀਂਤਾਂ ਜੇ( p_ਖਾਨਾ_ਵਿਅੰਜਕ->op==TK_ID ){
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਖਾਨਾ_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
        zName = p_ਖਾਨਾ_ਵਿਅੰਜਕ->u.zToken;
      }ਨਹੀਂਤਾਂ{
        /* Use the original text of the column expression as its name */
        ਪ੍ਰਤਿਜ੍ਞਾ( zName==pX->zEName );  /* pointer comparison intended */
      }
    }
    ਜੇ( zName && !sqlite3IsTrueOrFalse(zName) ){
      zName = sqlite3DbStrDup(db, zName);
    }ਨਹੀਂਤਾਂ{
      zName = sqlite3MPrintf(db,"column%d",i+1);
    }

    /* Make sure the column name is unique.  If the name is not unique,
    ** append an integer to the name so that it becomes unique.
    */
    cnt = 0;
    ਜਦੋਂ( zName && (pCollide = sqlite3_Hash_ਲੱਭੋ(&ht, zName))!=0 ){
      ਜੇ( pCollide->fg.b_Using_ਪਦ ){
        p_ਖਾਨਾ->ਖਾਨਾ_Flags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      ਜੇ( nName>0 ){
        ਲਈ(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        ਜੇ( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3_ਤਰੱਕੀ_Check(pParse);
      ਜੇ( cnt>3 ){
        sqlite3_randomness(ਆਕਾਰ(cnt), &cnt);
      }
    }
    p_ਖਾਨਾ->zCnName = zName;
    p_ਖਾਨਾ->hName = sqlite3StrIHash(zName);
    ਜੇ( pX->fg.bNoExpand ){
      p_ਖਾਨਾ->ਖਾਨਾ_Flags |= COLFLAG_NOEXPAND;
    }
    sqlite3_ਕਾਲਮ_Properties_From_Name(0, p_ਖਾਨਾ);
    ਜੇ( zName && sqlite3_Hash_ਪਾਓ(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  ਜੇ( pParse->nErr ){
    ਲਈ(j=0; j<i; j++){
      sqlite3DbFree(db, a_ਖਾਨਾ[j].zCnName);
    }
    sqlite3DbFree(db, a_ਖਾਨਾ);
    *pa_ਖਾਨਾ = 0;
    *pn_ਖਾਨਾ = 0;
    ਵਾਪਸ pParse->rc;
  }
  ਵਾਪਸ SQLITE_OK;
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
ਖਾਲੀ sqlite3_Subquery_ਕਾਲਮ_Types(
  Parse *pParse,      /* Parsing contexts */
  ਸਾਰਣੀ *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  ਅੱਖਰ aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  ਕਾਲਮ *p_ਖਾਨਾ;
  CollSeq *pColl;
  ਪੂਰਨ i,j;
  ਵਿਅੰਜਕ *p;
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *a;
  NameContext sNC;

  ਪ੍ਰਤਿਜ੍ਞਾ( pSelect!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pSelect->selFlags & SF_Resolved)!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab->n_ਖਾਨਾ==pSelect->pEList->n_ਵਿਅੰਜਕ || pParse->nErr>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  ਜੇ( db->mallocFailed || ਵਿੱਚ_RENAME_OBJECT ) ਵਾਪਸ;
  ਜਦੋਂ( pSelect->p_ਪੂਰਵ ) pSelect = pSelect->p_ਪੂਰਵ;
  a = pSelect->pEList->a;
  memset(&sNC, 0, ਆਕਾਰ(sNC));
  sNC.pSrcList = pSelect->pSrc;
  ਲਈ(i=0, p_ਖਾਨਾ=pTab->a_ਖਾਨਾ; i<pTab->n_ਖਾਨਾ; i++, p_ਖਾਨਾ++){
    ਅਚਲ ਅੱਖਰ *zType;
    i64 n;
    ਪੂਰਨ m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (p_ਖਾਨਾ->ਖਾਨਾ_Flags & COLFLAG_NOINSERT);
    p = a[i].p_ਵਿਅੰਜਕ;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    p_ਖਾਨਾ->affinity = sqlite3_ਵਿਅੰਜਕ_Affinity(p);
    ਜਦੋਂ( p_ਖਾਨਾ->affinity<=SQLITE_AFF_NONE && pS2->p_ਅਗਲਾ!=0 ){
      m |= sqlite3_ਵਿਅੰਜਕ_ਅੰਕੜੇ_Type(pS2->pEList->a[i].p_ਵਿਅੰਜਕ);
      pS2 = pS2->p_ਅਗਲਾ;
      p_ਖਾਨਾ->affinity = sqlite3_ਵਿਅੰਜਕ_Affinity(pS2->pEList->a[i].p_ਵਿਅੰਜਕ);
    }
    ਜੇ( p_ਖਾਨਾ->affinity<=SQLITE_AFF_NONE ){
      p_ਖਾਨਾ->affinity = aff;
    }
    ਜੇ( p_ਖਾਨਾ->affinity>=SQLITE_AFF_ਪਾਠ && (pS2->p_ਅਗਲਾ || pS2!=pSelect) ){
      ਲਈ(pS2=pS2->p_ਅਗਲਾ; pS2; pS2=pS2->p_ਅਗਲਾ){
        m |= sqlite3_ਵਿਅੰਜਕ_ਅੰਕੜੇ_Type(pS2->pEList->a[i].p_ਵਿਅੰਜਕ);
      }
      ਜੇ( p_ਖਾਨਾ->affinity==SQLITE_AFF_ਪਾਠ && (m&0x01)!=0 ){
        p_ਖਾਨਾ->affinity = SQLITE_AFF_BLOB;
      }ਨਹੀਂਤਾਂ
      ਜੇ( p_ਖਾਨਾ->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        p_ਖਾਨਾ->affinity = SQLITE_AFF_BLOB;
      }
      ਜੇ( p_ਖਾਨਾ->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        p_ਖਾਨਾ->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = ਕਾਲਮ_Type(&sNC, p, 0, 0, 0);
    ਜੇ( zType==0 || p_ਖਾਨਾ->affinity!=sqlite3AffinityType(zType, 0) ){
      ਜੇ( p_ਖਾਨਾ->affinity==SQLITE_AFF_NUMERIC
       || p_ਖਾਨਾ->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }ਨਹੀਂਤਾਂ{
        zType = 0;
        ਲਈ(j=1; j<SQLITE_N_STDTYPE; j++){
          ਜੇ( sqlite3StdTypeAffinity[j]==p_ਖਾਨਾ->affinity ){
            zType = sqlite3StdType[j];
            ਰੋਕੋ;
          }
        }
      }
    }
    ਜੇ( zType ){
      ਅਚਲ i64 k = ਲੜੀ_ਲੰਬਾਈ(zType);
      n = ਲੜੀ_ਲੰਬਾਈ(p_ਖਾਨਾ->zCnName);
      p_ਖਾਨਾ->zCnName = sqlite3DbReallocOrFree(db, p_ਖਾਨਾ->zCnName, n+k+2);
      p_ਖਾਨਾ->ਖਾਨਾ_Flags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      ਜੇ( p_ਖਾਨਾ->zCnName ){
        ਸਮ੍ਰਿਤੀ_ਨਕਲ(&p_ਖਾਨਾ->zCnName[n+1], zType, k+1);
        p_ਖਾਨਾ->ਖਾਨਾ_Flags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p);
    ਜੇ( pColl ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pTab->p_ਸੂਚਕਾਂਕ==0 );
      sqlite3_ਕਾਲਮ_ਸੈੱਟ_Coll(db, p_ਖਾਨਾ, pColl->zName);
    }
  }
  pTab->sz_Tab_ਪੰਕਤੀ = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
ਸਥਿਰ ਪੂਰਨ check_Subquery_Nesting_ਡੂੰਘਾਈ(Parse *pParse){
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
  ਜੇ( pParse->nNestSel >= pParse->db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ] ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "VIEWs and/or subqueries nested too deep");
    ਵਾਪਸ 1;
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
ਸਾਰਣੀ *sqlite3_ਨਤੀਜਾ_ਸੈੱਟ_ਦਾ_Select(Parse *pParse, Select *pSelect, ਅੱਖਰ aff){
  ਸਾਰਣੀ *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  ਜੇ( check_Subquery_Nesting_ਡੂੰਘਾਈ(pParse) ) ਵਾਪਸ 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_ਭਰਿਆ_ਖਾਨਾ_ਨਾਂ;
  db->flags |= SQLITE_Short_ਖਾਨਾ_ਨਾਂ;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  ਜੇ( pParse->nErr ) ਵਾਪਸ 0;
  ਜਦੋਂ( pSelect->p_ਪੂਰਵ ) pSelect = pSelect->p_ਪੂਰਵ;
  pTab = sqlite3DbMallocZero(db, ਆਕਾਰ(ਸਾਰਣੀ) );
  ਜੇ( pTab==0 ){
    ਵਾਪਸ 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->n_ਪੰਕਤੀ_Log_Est = 200; ਪ੍ਰਤਿਜ੍ਞਾ( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_ਵਿਅੰਜਕ_List(pParse, pSelect->pEList, &pTab->n_ਖਾਨਾ, &pTab->a_ਖਾਨਾ);
  sqlite3_Subquery_ਕਾਲਮ_Types(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  ਜੇ( db->mallocFailed ){
    sqlite3_Delete_ਸਾਰਣੀ(db, pTab);
    ਵਾਪਸ 0;
  }
  pParse->nNestSel--;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nNestSel>=0 );
  ਵਾਪਸ pTab;
}

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
Vdbe *sqlite3GetVdbe(Parse *pParse){
  ਜੇ( pParse->pVdbe ){
    ਵਾਪਸ pParse->pVdbe;
  }
  ਜੇ( pParse->pToplevel==0 ){
    pParse->ok_Const_ਗੁਣਕ = 1;
  }
  ਵਾਪਸ sqlite3VdbeCreate(pParse);
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
ਸਥਿਰ ਖਾਲੀ compute_ਹੱਦਬੰਦੀ_Registers(Parse *pParse, Select *p, ਪੂਰਨ iBreak){
  Vdbe *v = 0;
  ਪੂਰਨ i_ਹੱਦਬੰਦੀ = 0;
  ਪੂਰਨ i_ਵਿੱਥ;
  ਪੂਰਨ n;
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ = p->p_ਹੱਦਬੰਦੀ;

  ਜੇ( p->i_ਹੱਦਬੰਦੀ ) ਵਾਪਸ;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  ਜੇ( p_ਹੱਦਬੰਦੀ ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਹੱਦਬੰਦੀ->op==TK_ਹੱਦਬੰਦੀ );
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ!=0 );
    p->i_ਹੱਦਬੰਦੀ = i_ਹੱਦਬੰਦੀ = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Integer(p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ, &n, pParse, 0) ){
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, n, i_ਹੱਦਬੰਦੀ);
      VdbeComment((v, "LIMIT counter"));
      ਜੇ( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }ਨਹੀਂਤਾਂ ਜੇ( n>=0 && p->n_Select_ਪੰਕਤੀ>sqlite3LogEst((u64)n) ){
        p->n_Select_ਪੰਕਤੀ = sqlite3LogEst((u64)n);
        p->selFlags |= SF_Fixed_ਹੱਦਬੰਦੀ;
      }
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ, i_ਹੱਦਬੰਦੀ);
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_MustBeInt, i_ਹੱਦਬੰਦੀ); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IfNot, i_ਹੱਦਬੰਦੀ, iBreak); VdbeCoverage(v);
    }
    ਜੇ( p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ ){
      p->i_ਵਿੱਥ = i_ਵਿੱਥ = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ, i_ਵਿੱਥ);
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_MustBeInt, i_ਵਿੱਥ); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
      VdbeComment((v, "OFFSET counter"));
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਵਿੱਥ_ਹੱਦਬੰਦੀ, i_ਹੱਦਬੰਦੀ, i_ਵਿੱਥ+1, i_ਵਿੱਥ);
      VdbeComment((v, "LIMIT+OFFSET"));
    }
  }
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPOUND_SELECT
/*
** Return the appropriate collating sequence for the iCol-th column of
** the result set for the compound-select statement "p".  Return NULL if
** the column has no default collating sequence.
**
** The collating sequence for the compound select is taken from the
** left-most term of the select that has a collating sequence.
*/
ਸਥਿਰ CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, ਪੂਰਨ i_ਖਾਨਾ){
  CollSeq *pRet;
  ਜੇ( p->p_ਪੂਰਵ ){
    pRet = multiSelectCollSeq(pParse, p->p_ਪੂਰਵ, i_ਖਾਨਾ);
  }ਨਹੀਂਤਾਂ{
    pRet = 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  ਜੇ( pRet==0 && ALWAYS(i_ਖਾਨਾ<p->pEList->n_ਵਿਅੰਜਕ) ){
    pRet = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p->pEList->a[i_ਖਾਨਾ].p_ਵਿਅੰਜਕ);
  }
  ਵਾਪਸ pRet;
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
ਸਥਿਰ ਕੁੰਜੀ_Info *multi_Select_By_ਮਿਲਾਓ_ਕੁੰਜੀ_Info(Parse *pParse, Select *p, ਪੂਰਨ nExtra){
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By = p->p_ਕ੍ਰਮ_By;
  ਪੂਰਨ n_ਕ੍ਰਮ_By = (p_ਕ੍ਰਮ_By!=0) ? p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ : 0;
  sqlite3 *db = pParse->db;
  ਕੁੰਜੀ_Info *pRet = sqlite3_ਕੁੰਜੀ_Info_Alloc(db, n_ਕ੍ਰਮ_By+nExtra, 1);
  ਜੇ( pRet ){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<n_ਕ੍ਰਮ_By; i++){
      ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ = &p_ਕ੍ਰਮ_By->a[i];
      ਵਿਅੰਜਕ *p_ਪਦ = p_ਵਸਤੂ->p_ਵਿਅੰਜਕ;
      CollSeq *pColl;

      ਜੇ( p_ਪਦ->flags & EP_Collate ){
        pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p_ਪਦ);
      }ਨਹੀਂਤਾਂ{
        pColl = multiSelectCollSeq(pParse, p, p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ-1);
        ਜੇ( pColl==0 ) pColl = db->pDfltColl;
        p_ਕ੍ਰਮ_By->a[i].p_ਵਿਅੰਜਕ =
          sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(pParse, p_ਪਦ, pColl->zName);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਕੁੰਜੀ_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->a_ਛਾਂਟੋ_Flags[i] = p_ਕ੍ਰਮ_By->a[i].fg.ਛਾਂਟੋ_Flags;
    }
  }

  ਵਾਪਸ pRet;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
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
ਸਥਿਰ ਖਾਲੀ generateWithRecursiveQuery(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The recursive SELECT to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  SrcList *pSrc = p->pSrc;      /* The FROM clause of the recursive query */
  ਪੂਰਨ n_ਖਾਨਾ = p->pEList->n_ਵਿਅੰਜਕ;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *p_ਤਿਆਰੀ;               /* The setup query */
  Select *p_ਪਹਿਲਾ_Rec;            /* Left-most recursive term */
  ਪੂਰਨ addr_ਸਿਖਰ;                  /* Top of the loop */
  ਪੂਰਨ addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  ਪੂਰਨ iCurrent = 0;             /* The Current table */
  ਪੂਰਨ regCurrent;               /* Register holding Current table */
  ਪੂਰਨ i_ਕਤਾਰ;                   /* The Queue table */
  ਪੂਰਨ iDistinct = 0;            /* To ensure unique results if UNION */
  ਪੂਰਨ eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest dest_ਕਤਾਰ;         /* SelectDest targeting the Queue table */
  ਪੂਰਨ i;                        /* Loop counter */
  ਪੂਰਨ rc;                       /* Result code */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;           /* The ORDER BY clause */
  ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ;                 /* Saved LIMIT and OFFSET */
  ਪੂਰਨ reg_ਹੱਦਬੰਦੀ, reg_ਵਿੱਥ;      /* Registers used by LIMIT and OFFSET */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  ਜੇ( p->pWin ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "cannot use window functions in recursive queries");
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ

  /* Obtain authorization to do a recursive query */
  ਜੇ( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) ਵਾਪਸ;

  /* Process the LIMIT and OFFSET clauses, if they exist */
  addrBreak = sqlite3VdbeMakeLabel(pParse);
  p->n_Select_ਪੰਕਤੀ = 320;  /* 4 billion rows */
  compute_ਹੱਦਬੰਦੀ_Registers(pParse, p, addrBreak);
  p_ਹੱਦਬੰਦੀ = p->p_ਹੱਦਬੰਦੀ;
  reg_ਹੱਦਬੰਦੀ = p->i_ਹੱਦਬੰਦੀ;
  reg_ਵਿੱਥ = p->i_ਵਿੱਥ;
  p->p_ਹੱਦਬੰਦੀ = 0;
  p->i_ਹੱਦਬੰਦੀ = p->i_ਵਿੱਥ = 0;
  p_ਕ੍ਰਮ_By = p->p_ਕ੍ਰਮ_By;

  /* Locate the cursor number of the Current table */
  ਲਈ(i=0; ALWAYS(i<pSrc->nSrc); i++){
    ਜੇ( pSrc->a[i].fg.isRecursive ){
      iCurrent = pSrc->a[i].iCursor;
      ਰੋਕੋ;
    }
  }

  /* Allocate cursors numbers for Queue and Distinct.  The cursor number for
  ** the Distinct table must be exactly one greater than Queue in order
  ** for the SRT_DistFifo and SRT_DistQueue destinations to work. */
  i_ਕਤਾਰ = pParse->nTab++;
  ਜੇ( p->op==TK_UNION ){
    eDest = p_ਕ੍ਰਮ_By ? SRT_Dist_ਕਤਾਰ : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }ਨਹੀਂਤਾਂ{
    eDest = p_ਕ੍ਰਮ_By ? SRT_ਕਤਾਰ : SRT_Fifo;
  }
  sqlite3SelectDestInit(&dest_ਕਤਾਰ, eDest, i_ਕਤਾਰ);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenPseudo, iCurrent, regCurrent, n_ਖਾਨਾ);
  ਜੇ( p_ਕ੍ਰਮ_By ){
    ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info = multi_Select_By_ਮਿਲਾਓ_ਕੁੰਜੀ_Info(pParse, p, 1);
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_OpenEphemeral, i_ਕਤਾਰ, p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ+2, 0,
                      (ਅੱਖਰ*)p_ਕੁੰਜੀ_Info, P4_KEYINFO);
    dest_ਕਤਾਰ.p_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By;
  }ਨਹੀਂਤਾਂ{
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, i_ਕਤਾਰ, n_ਖਾਨਾ);
  }
  VdbeComment((v, "Queue table"));
  ਜੇ( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਅਗਲਾ==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList!=0 );
    n_ਖਾਨਾ = p->pEList->n_ਵਿਅੰਜਕ;
    p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_Alloc(pParse->db, n_ਖਾਨਾ, 1);
    ਜੇ( p_ਕੁੰਜੀ_Info ){
      ਲਈ(i=0, apColl=p_ਕੁੰਜੀ_Info->aColl; i<n_ਖਾਨਾ; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        ਜੇ( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_OpenEphemeral, iDistinct, n_ਖਾਨਾ, 0,
                           (ਖਾਲੀ*)p_ਕੁੰਜੀ_Info, P4_KEYINFO);
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nErr>0 );
    }
  }

  /* Detach the ORDER BY clause from the compound SELECT */
  p->p_ਕ੍ਰਮ_By = 0;

  /* Figure out how many elements of the compound SELECT are part of the
  ** recursive query.  Make sure no recursive elements use aggregate
  ** functions.  Mark the recursive elements as UNION ALL even if they
  ** are really UNION because the distinctness will be enforced by the
  ** iDistinct table.  pFirstRec is left pointing to the left-most
  ** recursive term of the CTE.
  */
  ਲਈ(p_ਪਹਿਲਾ_Rec=p; ALWAYS(p_ਪਹਿਲਾ_Rec!=0); p_ਪਹਿਲਾ_Rec=p_ਪਹਿਲਾ_Rec->p_ਪੂਰਵ){
    ਜੇ( p_ਪਹਿਲਾ_Rec->selFlags & SF_Aggregate ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "recursive aggregate queries not supported");
      ਜਾਓ ਸਿਰਾ_ਦਾ_recursive_query;
    }
    p_ਪਹਿਲਾ_Rec->op = TK_ALL;
    ਜੇ( (p_ਪਹਿਲਾ_Rec->p_ਪੂਰਵ->selFlags & SF_Recursive)==0 ) ਰੋਕੋ;
  }

  /* Store the results of the setup-query in Queue. */
  p_ਤਿਆਰੀ = p_ਪਹਿਲਾ_Rec->p_ਪੂਰਵ;
  p_ਤਿਆਰੀ->p_ਅਗਲਾ = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, p_ਤਿਆਰੀ, &dest_ਕਤਾਰ);
  p_ਤਿਆਰੀ->p_ਅਗਲਾ = p;
  ਜੇ( rc ) ਜਾਓ ਸਿਰਾ_ਦਾ_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Rewind, i_ਕਤਾਰ, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Null_ਪੰਕਤੀ, iCurrent); /* To reset column cache */
  ਜੇ( p_ਕ੍ਰਮ_By ){
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, i_ਕਤਾਰ, p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ+1, regCurrent);
  }ਨਹੀਂਤਾਂ{
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਪੰਕਤੀ_ਅੰਕੜੇ, i_ਕਤਾਰ, regCurrent);
  }
  sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Delete, i_ਕਤਾਰ);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  ਕੋਡ_ਵਿੱਥ(v, reg_ਵਿੱਥ, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  ਜੇ( reg_ਹੱਦਬੰਦੀ ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_DecrJumpZero, reg_ਹੱਦਬੰਦੀ, addrBreak);
    VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, addrCont);

  /* Execute the recursive SELECT taking the single row in Current as
  ** the value for the recursive-table. Store the results in the Queue.
  */
  p_ਪਹਿਲਾ_Rec->p_ਪੂਰਵ = 0;
  ExplainQueryPlan((pParse, 1, "RECURSIVE STEP"));
  sqlite3Select(pParse, p, &dest_ਕਤਾਰ);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪਹਿਲਾ_Rec->p_ਪੂਰਵ==0 );
  p_ਪਹਿਲਾ_Rec->p_ਪੂਰਵ = p_ਤਿਆਰੀ;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_ਸਿਖਰ);
  sqlite3VdbeResolveLabel(v, addrBreak);

ਸਿਰਾ_ਦਾ_recursive_query:
  sqlite3_ਵਿਅੰਜਕ_List_Delete(pParse->db, p->p_ਕ੍ਰਮ_By);
  p->p_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By;
  p->p_ਹੱਦਬੰਦੀ = p_ਹੱਦਬੰਦੀ;
  ਵਾਪਸ;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_CTE */

/* Forward references */
ਸਥਿਰ ਪੂਰਨ multi_Select_By_ਮਿਲਾਓ(
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
ਸਥਿਰ ਪੂਰਨ multi_Select_ਮੁੱਲਾਂ(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ਪੂਰਨ n_ਪੰਕਤੀ = 1;
  ਪੂਰਨ rc = 0;
  ਪੂਰਨ bShowAll = p->p_ਹੱਦਬੰਦੀ==0;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->selFlags & SF_Multi_ਮੁੱਲ );
  ਕਰੋ{
    ਪ੍ਰਤਿਜ੍ਞਾ( p->selFlags & SF_ਮੁੱਲਾਂ );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->op==TK_ALL || (p->op==TK_SELECT && p->p_ਪੂਰਵ==0) );
    ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਅਗਲਾ==0 || p->pEList->n_ਵਿਅੰਜਕ==p->p_ਅਗਲਾ->pEList->n_ਵਿਅੰਜਕ );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( p->pWin ) ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ
    ਜੇ( p->p_ਪੂਰਵ==0 ) ਰੋਕੋ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਪੂਰਵ->p_ਅਗਲਾ==p );
    p = p->p_ਪੂਰਵ;
    n_ਪੰਕਤੀ += bShowAll;
  }ਜਦੋਂ(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", n_ਪੰਕਤੀ,
                    n_ਪੰਕਤੀ==1 ? "" : "S"));
  ਜਦੋਂ( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    ਜੇ( !bShowAll ) ਰੋਕੋ;
    p->n_Select_ਪੰਕਤੀ = n_ਪੰਕਤੀ;
    p = p->p_ਅਗਲਾ;
  }
  ਵਾਪਸ rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
ਸਥਿਰ ਪੂਰਨ hasAnchor(Select *p){
  ਜਦੋਂ( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->p_ਪੂਰਵ; }
  ਵਾਪਸ p!=0;
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
ਸਥਿਰ ਪੂਰਨ union_With_ਹੱਦਬੰਦੀ_One(sqlite3 *db, Select *p){
  ਪੂਰਨ v;
  ਜੇ( p->op!=TK_UNION ) ਵਾਪਸ 0;
  ਜੇ( p->p_ਹੱਦਬੰਦੀ==0 ) ਵਾਪਸ 0;
  ਜੇ( p->p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ ) ਵਾਪਸ 0;  /* No OFFSET */
  v = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ!=0 );
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Integer(p->p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ, &v, 0, 0)==0 ) ਵਾਪਸ 0;
  ਜੇ( v!=1 ) ਵਾਪਸ 0;
  ਜੇ( OptimizationDisabled(db, SQLITE_Union_ਹੱਦਬੰਦੀ) ) ਵਾਪਸ 0;
  ਵਾਪਸ 1;
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
ਸਥਿਰ ਪੂਰਨ multiSelect(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ਪੂਰਨ rc = SQLITE_OK;   /* Success code from a subroutine */
  Select *p_ਪੂਰਵ;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( p && p->p_ਪੂਰਵ );  /* Calling function guarantees this much */
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->selFlags & SF_Compound );
  db = pParse->db;
  p_ਪੂਰਵ = p->p_ਪੂਰਵ;
  dest = *pDest;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ->p_ਕ੍ਰਮ_By==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ->p_ਹੱਦਬੰਦੀ==0 );

  v = sqlite3GetVdbe(pParse);
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  ਜੇ( dest.eDest==SRT_EphemTab ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList );
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_ਵਿਅੰਜਕ);
    dest.eDest = SRT_ਸਾਰਣੀ;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  ਜੇ( p->selFlags & SF_Multi_ਮੁੱਲ ){
    rc = multi_Select_ਮੁੱਲਾਂ(pParse, p, &dest);
    ਜੇ( rc>=0 ) ਜਾਓ multi_select_ਸਿਰਾ;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList && p_ਪੂਰਵ->pEList );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList->n_ਵਿਅੰਜਕ==p_ਪੂਰਵ->pEList->n_ਵਿਅੰਜਕ );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
  ਜੇ( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
  ਜੇ( p->p_ਕ੍ਰਮ_By ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    ਵਾਪਸ multi_Select_By_ਮਿਲਾਓ(pParse, p, pDest);
  }ਨਹੀਂਤਾਂ ਜੇ( p->op!=TK_ALL && !union_With_ਹੱਦਬੰਦੀ_One(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    ਵਿਅੰਜਕ *pOne = sqlite3_ਵਿਅੰਜਕ_Int32(db, 1);
    p->p_ਕ੍ਰਮ_By = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0, pOne);
    ਜੇ( pParse->nErr ) ਜਾਓ multi_select_ਸਿਰਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਕ੍ਰਮ_By!=0 );
    p->p_ਕ੍ਰਮ_By->a[0].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = 1;
    ਵਾਪਸ multi_Select_By_ਮਿਲਾਓ(pParse, p, pDest);
  }ਨਹੀਂਤਾਂ{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    ਪੂਰਨ addr = 0;
    ਪੂਰਨ n_ਹੱਦਬੰਦੀ = 0;  /* Initialize to suppress harmless compiler warning */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
    ਜੇ( p_ਪੂਰਵ->p_ਪੂਰਵ==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#ਜੇ_ਅੰਤ
    ਪ੍ਰਤਿਜ੍ਞਾ( !p_ਪੂਰਵ->p_ਹੱਦਬੰਦੀ );
    p_ਪੂਰਵ->i_ਹੱਦਬੰਦੀ = p->i_ਹੱਦਬੰਦੀ;
    p_ਪੂਰਵ->i_ਵਿੱਥ = p->i_ਵਿੱਥ;
    p_ਪੂਰਵ->p_ਹੱਦਬੰਦੀ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p->p_ਹੱਦਬੰਦੀ, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, p_ਪੂਰਵ, &dest);
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਪੂਰਵ->p_ਹੱਦਬੰਦੀ);
    p_ਪੂਰਵ->p_ਹੱਦਬੰਦੀ = 0;
    ਜੇ( rc ){
      ਜਾਓ multi_select_ਸਿਰਾ;
    }
    p->p_ਪੂਰਵ = 0;
    p->i_ਹੱਦਬੰਦੀ = p_ਪੂਰਵ->i_ਹੱਦਬੰਦੀ;
    p->i_ਵਿੱਥ = p_ਪੂਰਵ->i_ਵਿੱਥ;
    ਜੇ( p->i_ਹੱਦਬੰਦੀ ){
      addr = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IfNot, p->i_ਹੱਦਬੰਦੀ); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      ਜੇ( p->i_ਵਿੱਥ ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਵਿੱਥ_ਹੱਦਬੰਦੀ,
                          p->i_ਹੱਦਬੰਦੀ, p->i_ਵਿੱਥ+1, p->i_ਵਿੱਥ);
      }
    }
    ExplainQueryPlan((pParse, 1, "UNION ALL"));
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL right...\n"));
    rc = sqlite3Select(pParse, p, &dest);
    testcase( rc!=SQLITE_OK );
    pDelete = p->p_ਪੂਰਵ;
    p->p_ਪੂਰਵ = p_ਪੂਰਵ;
    p->n_Select_ਪੰਕਤੀ = sqlite3_Log_Est_ਜੋੜੋ(p->n_Select_ਪੰਕਤੀ, p_ਪੂਰਵ->n_Select_ਪੰਕਤੀ);
    ਜੇ( p->p_ਹੱਦਬੰਦੀ
     && sqlite3_ਵਿਅੰਜਕ_Is_Integer(p->p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ, &n_ਹੱਦਬੰਦੀ, pParse, 0)
     && n_ਹੱਦਬੰਦੀ>0 && p->n_Select_ਪੰਕਤੀ > sqlite3LogEst((u64)n_ਹੱਦਬੰਦੀ)
    ){
      p->n_Select_ਪੰਕਤੀ = sqlite3LogEst((u64)n_ਹੱਦਬੰਦੀ);
    }
    ਜੇ( addr ){
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr);
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
    ਜੇ( p->p_ਅਗਲਾ==0 ){
      Explain_Query_Plan_ਕੱਢੋ(pParse);
    }
#ਜੇ_ਅੰਤ
  }

multi_select_ਸਿਰਾ:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  ਜੇ( pDelete ){
    sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
ਖਾਲੀ sqlite3_Select_Wrong_ਅੰਕ_Terms_Error(Parse *pParse, Select *p){
  ਜੇ( p->selFlags & SF_ਮੁੱਲਾਂ ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "all VALUES must have the same number of terms");
  }ਨਹੀਂਤਾਂ{
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "SELECTs to the left and right of %s"
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
ਸਥਿਰ ਪੂਰਨ generateOutputSubroutine(
  Parse *pParse,          /* Parsing context */
  Select *p,              /* The SELECT statement */
  SelectDest *p_ਵਿੱਚ,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  ਪੂਰਨ regReturn,          /* The return address register */
  ਪੂਰਨ regPrev,            /* Previous result register.  No uniqueness if 0 */
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info,      /* For comparing with previous entry */
  ਪੂਰਨ iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ iContinue;
  ਪੂਰਨ addr;

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿੱਚ->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  ਜੇ( regPrev ){
    ਪੂਰਨ addr1, addr2;
    addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_IfNot, regPrev); VdbeCoverage(v);
    addr2 = sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Compare, p_ਵਿੱਚ->iSdst, regPrev+1, p_ਵਿੱਚ->nSdst,
                              (ਅੱਖਰ*)sqlite3_ਕੁੰਜੀ_Info_Ref(p_ਕੁੰਜੀ_Info), P4_KEYINFO);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);
    sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Copy, p_ਵਿੱਚ->iSdst, regPrev+1, p_ਵਿੱਚ->nSdst-1);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, regPrev);
  }
  ਜੇ( pParse->db->mallocFailed ) ਵਾਪਸ 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  ਕੋਡ_ਵਿੱਥ(v, p->i_ਵਿੱਥ, iContinue);

  ਚੋਣ( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    ਹਾਲਤ SRT_Fifo:
    ਹਾਲਤ SRT_DistFifo:
    ਹਾਲਤ SRT_ਸਾਰਣੀ:
    ਹਾਲਤ SRT_EphemTab: {
      ਪੂਰਨ r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      ਪੂਰਨ r2 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      ਪੂਰਨ iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_ਸਾਰਣੀ );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, p_ਵਿੱਚ->iSdst, p_ਵਿੱਚ->nSdst, r1);
#ਜੇ !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ਜੇ( pDest->eDest==SRT_ਸਾਰਣੀ && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
      ਜੇ( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iParm+1, r1,
                             p_ਵਿੱਚ->iSdst, p_ਵਿੱਚ->nSdst);
      }
#ਜੇ_ਅੰਤ
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_NewRowid, iParm, r2);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਪਾਓ, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r2);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
      ਰੋਕੋ;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    ਹਾਲਤ SRT_Exists: {
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      ਰੋਕੋ;
    }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    ਹਾਲਤ SRT_ਸੈੱਟ: {
      ਪੂਰਨ r1;
      testcase( p_ਵਿੱਚ->nSdst>1 );
      r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Make_ਰਿਕਾਰਡ, p_ਵਿੱਚ->iSdst, p_ਵਿੱਚ->nSdst,
          r1, pDest->zAffSdst, p_ਵਿੱਚ->nSdst);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, pDest->iSDParm, r1,
                           p_ਵਿੱਚ->iSdst, p_ਵਿੱਚ->nSdst);
      ਜੇ( pDest->iSDParm2>0 ){
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3CursorBloomable(pParse, pDest->iSDParm, p_ਵਿੱਚ->nSdst) );
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Filter_ਜੋੜੋ, pDest->iSDParm2, 0,
                             p_ਵਿੱਚ->iSdst, p_ਵਿੱਚ->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
      ਰੋਕੋ;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    ਹਾਲਤ SRT_Mem: {
      testcase( p_ਵਿੱਚ->nSdst>1 );
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Move(pParse, p_ਵਿੱਚ->iSdst, pDest->iSDParm, p_ਵਿੱਚ->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    ਹਾਲਤ SRT_Coroutine: {
      ਜੇ( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, p_ਵਿੱਚ->nSdst);
        pDest->nSdst = p_ਵਿੱਚ->nSdst;
      }
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Move(pParse, p_ਵਿੱਚ->iSdst, pDest->iSdst, p_ਵਿੱਚ->nSdst);
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Yield, pDest->iSDParm);
      ਰੋਕੋ;
    }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    ਹਾਲਤ SRT_Dist_ਕਤਾਰ:
    ਹਾਲਤ SRT_ਕਤਾਰ: {
      ਪੂਰਨ n_ਕੁੰਜੀ;
      ਪੂਰਨ r1, r2, r3, ii;
      ਵਿਅੰਜਕ_List *pSO;
      ਪੂਰਨ iParm = pDest->iSDParm;
      pSO = pDest->p_ਕ੍ਰਮ_By;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSO );
      n_ਕੁੰਜੀ = pSO->n_ਵਿਅੰਜਕ;
      r1 = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
      r2 = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_ਕੁੰਜੀ+2);
      r3 = r2+n_ਕੁੰਜੀ+1;

      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, p_ਵਿੱਚ->iSdst, p_ਵਿੱਚ->nSdst, r3);
      ਜੇ( pDest->eDest==SRT_Dist_ਕਤਾਰ ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Idx_ਪਾਓ, iParm+1, r3);
      }
      ਲਈ(ii=0; ii<n_ਕੁੰਜੀ; ii++){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_SCopy,
                          p_ਵਿੱਚ->iSdst + pSO->a[ii].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ - 1,
                          r2+ii);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Sequence, iParm, r2+n_ਕੁੰਜੀ);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, r2, n_ਕੁੰਜੀ+2, r1);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, iParm, r1, r2, n_ਕੁੰਜੀ+2);
      sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, r1);
      sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, r2, n_ਕੁੰਜੀ+2);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_CTE */

    /* Ignore the output */
    ਹਾਲਤ SRT_Discard: {
      ਰੋਕੋ;
    }

    /* If none of the above, then the result destination must be
    ** SRT_Output.
    **
    ** For SRT_Output, results are stored in a sequence of registers. 
    ** Then the OP_ResultRow opcode is used to cause sqlite3_step() to
    ** return the next row of result.
    */
    ਮੂਲ: {
      ਪ੍ਰਤਿਜ੍ਞਾ( pDest->eDest==SRT_Output );
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਨਤੀਜਾ_ਪੰਕਤੀ, p_ਵਿੱਚ->iSdst, p_ਵਿੱਚ->nSdst);
      ਰੋਕੋ;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  ਜੇ( p->i_ਹੱਦਬੰਦੀ ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_DecrJumpZero, p->i_ਹੱਦਬੰਦੀ, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Return, regReturn);

  ਵਾਪਸ addr;
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPOUND_SELECT
ਸਥਿਰ ਪੂਰਨ multi_Select_By_ਮਿਲਾਓ(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ਪੂਰਨ i, j;             /* Loop counters */
  Select *p_ਪੂਰਵ;       /* Another SELECT immediately to our left */
  Select *p_ਵੰਡੋ;       /* Left-most SELECT in the right-hand group */
  ਪੂਰਨ nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  ਪੂਰਨ regAddrA;         /* Address register for select-A coroutine */
  ਪੂਰਨ regAddrB;         /* Address register for select-B coroutine */
  ਪੂਰਨ addrSelectA;      /* Address of the select-A coroutine */
  ਪੂਰਨ addrSelectB;      /* Address of the select-B coroutine */
  ਪੂਰਨ reg_ਬਾਹਰਲਾ_A;          /* Address register for the output-A subroutine */
  ਪੂਰਨ reg_ਬਾਹਰਲਾ_B;          /* Address register for the output-B subroutine */
  ਪੂਰਨ addr_ਬਾਹਰਲਾ_A;         /* Address of the output-A subroutine */
  ਪੂਰਨ addr_ਬਾਹਰਲਾ_B = 0;     /* Address of the output-B subroutine */
  ਪੂਰਨ addrEofA;         /* Address of the select-A-exhausted subroutine */
  ਪੂਰਨ addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  ਪੂਰਨ addrEofB;         /* Address of the select-B-exhausted subroutine */
  ਪੂਰਨ addrAltB;         /* Address of the A<B subroutine */
  ਪੂਰਨ addrAeqB;         /* Address of the A==B subroutine */
  ਪੂਰਨ addrAgtB;         /* Address of the A>B subroutine */
  ਪੂਰਨ reg_ਹੱਦਬੰਦੀ_A;        /* Limit register for select-A */
  ਪੂਰਨ reg_ਹੱਦਬੰਦੀ_B;        /* Limit register for select-A */
  ਪੂਰਨ regPrev;          /* A range of registers to hold previous output */
  ਪੂਰਨ saved_ਹੱਦਬੰਦੀ;       /* Saved value of p->iLimit */
  ਪੂਰਨ saved_ਵਿੱਥ;      /* Saved value of p->iOffset */
  ਪੂਰਨ labelCmpr;        /* Label for the start of the merge algorithm */
  ਪੂਰਨ label_ਸਿਰਾ;         /* Label for the end of the overall SELECT stmt */
  ਪੂਰਨ addr1;            /* Jump instructions that get retargeted */
  ਪੂਰਨ op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Dup = 0; /* Comparison information for duplicate removal */
  ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_ਮਿਲਾਓ;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;   /* The ORDER BY clause */
  ਪੂਰਨ n_ਕ੍ਰਮ_By;         /* Number of terms in the ORDER BY clause */
  u32 *a_ਕ੍ਰਮ_ਬਦਲੋ;        /* Mapping from ORDER BY terms to result set columns */

  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਕ੍ਰਮ_By!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕੁੰਜੀ_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  ਪ੍ਰਤਿਜ੍ਞਾ( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  label_ਸਿਰਾ = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਪੂਰਵ->p_ਕ੍ਰਮ_By==0 );
  p_ਕ੍ਰਮ_By = p->p_ਕ੍ਰਮ_By;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕ੍ਰਮ_By );
  n_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  ਲਈ(p_ਪੂਰਵ=p; p_ਪੂਰਵ && p_ਪੂਰਵ->op==TK_ALL; p_ਪੂਰਵ=p_ਪੂਰਵ->p_ਪੂਰਵ){}
  ਜੇ( p_ਪੂਰਵ && p_ਪੂਰਵ->p_ਪੂਰਵ ){
    ਲਈ(i=1; db->mallocFailed==0 && i<=p->pEList->n_ਵਿਅੰਜਕ; i++){
      ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
      ਲਈ(j=0, p_ਵਸਤੂ=p_ਕ੍ਰਮ_By->a; j<n_ਕ੍ਰਮ_By; j++, p_ਵਸਤੂ++){
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ!=0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ>0 );
        ਜੇ( p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ==i ) ਰੋਕੋ;
      }
      ਜੇ( j==n_ਕ੍ਰਮ_By ){
        ਵਿਅੰਜਕ *pNew = sqlite3_ਵਿਅੰਜਕ_Int32(db, i);
        ਜੇ( pNew==0 ) ਵਾਪਸ SQLITE_NOMEM_BKPT;
        p->p_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, p_ਕ੍ਰਮ_By, pNew);
        ਜੇ( p_ਕ੍ਰਮ_By ) p_ਕ੍ਰਮ_By->a[n_ਕ੍ਰਮ_By++].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  a_ਕ੍ਰਮ_ਬਦਲੋ = sqlite3DbMallocRawNN(db, ਆਕਾਰ(u32)*(n_ਕ੍ਰਮ_By + 1));
  ਜੇ( a_ਕ੍ਰਮ_ਬਦਲੋ ){
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
    ਪੂਰਨ bKeep = 0;
    a_ਕ੍ਰਮ_ਬਦਲੋ[0] = n_ਕ੍ਰਮ_By;
    ਲਈ(i=1, p_ਵਸਤੂ=p_ਕ੍ਰਮ_By->a; i<=n_ਕ੍ਰਮ_By; i++, p_ਵਸਤੂ++){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ>0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ<=p->pEList->n_ਵਿਅੰਜਕ );
      a_ਕ੍ਰਮ_ਬਦਲੋ[i] = p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ - 1;
      ਜੇ( a_ਕ੍ਰਮ_ਬਦਲੋ[i]!=(u32)i-1 ) bKeep = 1;
    }
     ਜੇ( bKeep==0 ){
      sqlite3DbFreeNN(db, a_ਕ੍ਰਮ_ਬਦਲੋ);
      a_ਕ੍ਰਮ_ਬਦਲੋ = 0;
    }
  }
  p_ਕੁੰਜੀ_ਮਿਲਾਓ = multi_Select_By_ਮਿਲਾਓ_ਕੁੰਜੀ_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  ਜੇ( op==TK_ALL ){
    regPrev = 0;
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ n_ਵਿਅੰਜਕ = p->pEList->n_ਵਿਅੰਜਕ;
    ਪ੍ਰਤਿਜ੍ਞਾ( n_ਕ੍ਰਮ_By>=n_ਵਿਅੰਜਕ || db->mallocFailed );
    regPrev = pParse->nMem+1;
    pParse->nMem += n_ਵਿਅੰਜਕ+1;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, regPrev);
    p_ਕੁੰਜੀ_Dup = sqlite3_ਕੁੰਜੀ_Info_Alloc(db, n_ਵਿਅੰਜਕ, 1);
    ਜੇ( p_ਕੁੰਜੀ_Dup ){
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਕੁੰਜੀ_Info_Is_Writeable(p_ਕੁੰਜੀ_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      ਲਈ(i=0; ALWAYS(p_ਕੁੰਜੀ_ਮਿਲਾਓ) && i<n_ਕ੍ਰਮ_By; i++){
        p_ਕੁੰਜੀ_Dup->aColl[p_ਕ੍ਰਮ_By->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ-1] = p_ਕੁੰਜੀ_ਮਿਲਾਓ->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  ਜੇ( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_ਸੰਤੁਲਿਤ_ਮਿਲਾਓ)
  ){
    ਲਈ(p_ਵੰਡੋ=p; p_ਵੰਡੋ->p_ਪੂਰਵ!=0 && p_ਵੰਡੋ->op==op; p_ਵੰਡੋ=p_ਵੰਡੋ->p_ਪੂਰਵ){
      nSelect++;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵੰਡੋ->p_ਪੂਰਵ->p_ਅਗਲਾ==p_ਵੰਡੋ );
    }
  }
  ਜੇ( nSelect<=3 ){
    p_ਵੰਡੋ = p;
  }ਨਹੀਂਤਾਂ{
    p_ਵੰਡੋ = p;
    ਲਈ(i=2; i<nSelect; i+=2){ p_ਵੰਡੋ = p_ਵੰਡੋ->p_ਪੂਰਵ; }
  }
  p_ਪੂਰਵ = p_ਵੰਡੋ->p_ਪੂਰਵ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ!=0 );
  p_ਵੰਡੋ->p_ਪੂਰਵ = 0;
  p_ਪੂਰਵ->p_ਅਗਲਾ = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਕ੍ਰਮ_By == p_ਕ੍ਰਮ_By );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕ੍ਰਮ_By!=0 || db->mallocFailed );
  p_ਪੂਰਵ->p_ਕ੍ਰਮ_By = sqlite3_ਵਿਅੰਜਕ_List_Dup(pParse->db, p_ਕ੍ਰਮ_By, 0);
  sqlite3_Resolve_ਕ੍ਰਮ_ਸਮੂਹ_By(pParse, p, p->p_ਕ੍ਰਮ_By, "ORDER");
  sqlite3_Resolve_ਕ੍ਰਮ_ਸਮੂਹ_By(pParse, p_ਪੂਰਵ, p_ਪੂਰਵ->p_ਕ੍ਰਮ_By, "ORDER");

  /* Compute the limit registers */
  compute_ਹੱਦਬੰਦੀ_Registers(pParse, p, label_ਸਿਰਾ);
  ਜੇ( p->i_ਹੱਦਬੰਦੀ && (op==TK_ALL || union_With_ਹੱਦਬੰਦੀ_One(db,p)) ){
    reg_ਹੱਦਬੰਦੀ_A = ++pParse->nMem;
    reg_ਹੱਦਬੰਦੀ_B = ++pParse->nMem;
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Copy, p->i_ਵਿੱਥ ? p->i_ਵਿੱਥ+1 : p->i_ਹੱਦਬੰਦੀ,
                                  reg_ਹੱਦਬੰਦੀ_A);
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Copy, reg_ਹੱਦਬੰਦੀ_A, reg_ਹੱਦਬੰਦੀ_B);
  }ਨਹੀਂਤਾਂ{
    reg_ਹੱਦਬੰਦੀ_A = reg_ਹੱਦਬੰਦੀ_B = 0;
  }
  sqlite3_ਵਿਅੰਜਕ_Delete(db, p->p_ਹੱਦਬੰਦੀ);
  p->p_ਹੱਦਬੰਦੀ = 0;

  regAddrA = ++pParse->nMem;
  regAddrB = ++pParse->nMem;
  reg_ਬਾਹਰਲਾ_A = ++pParse->nMem;
  reg_ਬਾਹਰਲਾ_B = ++pParse->nMem;
  sqlite3SelectDestInit(&destA, SRT_Coroutine, regAddrA);
  sqlite3SelectDestInit(&destB, SRT_Coroutine, regAddrB);

  ExplainQueryPlan((pParse, 1, "MERGE (%s)", sqlite3SelectOpName(p->op)));

  /* Generate a coroutine to evaluate the SELECT statement to the
  ** left of the compound operator - the "A" select.
  */
  addrSelectA = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_InitCoroutine, regAddrA, 0, addrSelectA);
  VdbeComment((v, "SUBR: next-A"));
  p_ਪੂਰਵ->i_ਹੱਦਬੰਦੀ = reg_ਹੱਦਬੰਦੀ_A;
  ExplainQueryPlan((pParse, 1, "LEFT"));
  sqlite3Select(pParse, p_ਪੂਰਵ, &destA);
  sqlite3_Vdbe_ਸਿਰਾ_Coroutine(v, regAddrA);
  sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);

  /* Generate a coroutine to evaluate the SELECT statement on
  ** the right - the "B" select
  */
  addrSelectB = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_InitCoroutine, regAddrB, 0, addrSelectB);
  VdbeComment((v, "SUBR: next-B"));
  saved_ਹੱਦਬੰਦੀ = p->i_ਹੱਦਬੰਦੀ;
  saved_ਵਿੱਥ = p->i_ਵਿੱਥ;
  p->i_ਹੱਦਬੰਦੀ = reg_ਹੱਦਬੰਦੀ_B;
  p->i_ਵਿੱਥ = 0; 
  ExplainQueryPlan((pParse, 1, "RIGHT"));
  sqlite3Select(pParse, p, &destB);
  p->i_ਹੱਦਬੰਦੀ = saved_ਹੱਦਬੰਦੀ;
  p->i_ਵਿੱਥ = saved_ਵਿੱਥ;
  sqlite3_Vdbe_ਸਿਰਾ_Coroutine(v, regAddrB);

  /* Generate a subroutine that outputs the current row of the A
  ** select as the next output row of the compound select.
  */
  VdbeNoopComment((v, "SUBR: out-A"));
  addr_ਬਾਹਰਲਾ_A = generateOutputSubroutine(pParse,
                 p, &destA, pDest, reg_ਬਾਹਰਲਾ_A,
                 regPrev, p_ਕੁੰਜੀ_Dup, label_ਸਿਰਾ);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  ਜੇ( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addr_ਬਾਹਰਲਾ_B = generateOutputSubroutine(pParse,
                 p, &destB, pDest, reg_ਬਾਹਰਲਾ_B,
                 regPrev, p_ਕੁੰਜੀ_Dup, label_ਸਿਰਾ);
  }
  sqlite3_ਕੁੰਜੀ_Info_Unref(p_ਕੁੰਜੀ_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  ਜੇ( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = label_ਸਿਰਾ;
  }ਨਹੀਂਤਾਂ{ 
    VdbeNoopComment((v, "SUBR: eof-A"));
    addrEofA = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, reg_ਬਾਹਰਲਾ_B, addr_ਬਾਹਰਲਾ_B);
    VdbeComment((v, "out-B"));
    addrEofA_noB = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Yield, regAddrB, label_ਸਿਰਾ);
                                     VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, addrEofA);
    p->n_Select_ਪੰਕਤੀ = sqlite3_Log_Est_ਜੋੜੋ(p->n_Select_ਪੰਕਤੀ, p_ਪੂਰਵ->n_Select_ਪੰਕਤੀ);
  }

  /* Generate a subroutine to run when the results from select B
  ** are exhausted and only data in select A remains.
  */
  ਜੇ( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    ਜੇ( p->n_Select_ਪੰਕਤੀ > p_ਪੂਰਵ->n_Select_ਪੰਕਤੀ ) p->n_Select_ਪੰਕਤੀ = p_ਪੂਰਵ->n_Select_ਪੰਕਤੀ;
  }ਨਹੀਂਤਾਂ{ 
    VdbeNoopComment((v, "SUBR: eof-B"));
    addrEofB = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, reg_ਬਾਹਰਲਾ_A, addr_ਬਾਹਰਲਾ_A);
    VdbeComment((v, "out-A"));
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Yield, regAddrA, label_ਸਿਰਾ); VdbeCoverage(v);
    VdbeComment((v, "next-A"));
    sqlite3VdbeGoto(v, addrEofB);
  }

  /* Generate code to handle the case of A<B
  */
  addrAltB = sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, reg_ਬਾਹਰਲਾ_A, addr_ਬਾਹਰਲਾ_A);
  VdbeComment((v, "out-A"));
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Yield, regAddrA, addrEofA); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  sqlite3VdbeGoto(v, labelCmpr);

  /* Generate code to handle the case of A==B
  */
  ਜੇ( op==TK_ALL ){
    addrAeqB = addrAltB;
  }ਨਹੀਂਤਾਂ ਜੇ( op==TK_INTERSECT ){
    addrAeqB = addrAltB;
    addrAltB++;
  }ਨਹੀਂਤਾਂ{
    addrAeqB = addrAltB + 1;
  }

  /* Generate code to handle the case of A>B
  */
  addrAgtB = sqlite3VdbeCurrentAddr(v);
  ਜੇ( op==TK_ALL || op==TK_UNION ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, reg_ਬਾਹਰਲਾ_B, addr_ਬਾਹਰਲਾ_B);
    VdbeComment((v, "out-B"));
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }ਨਹੀਂਤਾਂ{
    addrAgtB++; /* Just do next-B.  Might as well use the next-B call
                ** in the next code block */
  }

  /* This code runs once to initialize everything.
  */
  sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Yield, regAddrA, addrEofA_noB); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  /* v---  Also the A>B case for EXCEPT and INTERSECT */
  sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
  VdbeComment((v, "next-B"));

  /* Implement the main merge loop
  */
  ਜੇ( a_ਕ੍ਰਮ_ਬਦਲੋ!=0 ){
    sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Permutation, 0, 0, 0, (ਅੱਖਰ*)a_ਕ੍ਰਮ_ਬਦਲੋ, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Compare, destA.iSdst, destB.iSdst, n_ਕ੍ਰਮ_By,
                         (ਅੱਖਰ*)p_ਕੁੰਜੀ_ਮਿਲਾਓ, P4_KEYINFO);
  ਜੇ( a_ਕ੍ਰਮ_ਬਦਲੋ!=0 ){
    sqlite3VdbeChangeP5(v, OPFLAG_ਕ੍ਰਮ_ਬਦਲੋ);
  }
  sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Jump, addrAltB, addrAeqB, addrAgtB);
  VdbeCoverageIf(v, op==TK_ALL);
  VdbeCoverageIf(v, op==TK_UNION);
  VdbeCoverageIf(v, op==TK_EXCEPT);
  VdbeCoverageIf(v, op==TK_INTERSECT);

  /* Jump to the this point in order to terminate the query.
  */
  sqlite3VdbeResolveLabel(v, label_ਸਿਰਾ);

  /* Make arrangements to free the 2nd and subsequent arms of the compound
  ** after the parse has finished */
  ਜੇ( p_ਵੰਡੋ->p_ਪੂਰਵ ){
    sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, sqlite3SelectDeleteGeneric, p_ਵੰਡੋ->p_ਪੂਰਵ);
  }
  p_ਵੰਡੋ->p_ਪੂਰਵ = p_ਪੂਰਵ;
  p_ਪੂਰਵ->p_ਅਗਲਾ = p_ਵੰਡੋ;
  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_ਪੂਰਵ->p_ਕ੍ਰਮ_By);
  p_ਪੂਰਵ->p_ਕ੍ਰਮ_By = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_ਕੱਢੋ(pParse);
  ਵਾਪਸ pParse->nErr!=0;
}
#ਜੇ_ਅੰਤ

#ਜੇ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)

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
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ SubstContext {
  Parse *pParse;            /* The parsing context */
  ਪੂਰਨ i_ਸਾਰਣੀ;               /* Replace references to this table */
  ਪੂਰਨ i_New_ਸਾਰਣੀ;            /* New table number */
  ਪੂਰਨ isOuterJoin;          /* Add TK_IF_NULL_ROW opcodes on each replacement */
  ਪੂਰਨ n_Sel_ਡੂੰਘਾਈ;            /* Depth of sub-query recursion.  Top==1 */
  ਵਿਅੰਜਕ_List *pEList;         /* Replacement expressions */
  ਵਿਅੰਜਕ_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
ਸਥਿਰ ਖਾਲੀ subst_ਵਿਅੰਜਕ_List(SubstContext*, ਵਿਅੰਜਕ_List*);
ਸਥਿਰ ਖਾਲੀ substSelect(SubstContext*, Select*, ਪੂਰਨ);

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
ਸਥਿਰ ਵਿਅੰਜਕ *subst_ਵਿਅੰਜਕ(
  SubstContext *pSubst,  /* Description of the substitution */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ            /* Expr in which substitution occurs */
){
  ਜੇ( p_ਵਿਅੰਜਕ==0 ) ਵਾਪਸ 0;
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON|EP_InnerON)
   && p_ਵਿਅੰਜਕ->w.iJoin==pSubst->i_ਸਾਰਣੀ
  ){
    testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_InnerON) );
    p_ਵਿਅੰਜਕ->w.iJoin = pSubst->i_New_ਸਾਰਣੀ;
  }
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ
   && p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==pSubst->i_ਸਾਰਣੀ
   && !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Fixed_ਖਾਨਾ)
  ){
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
    ਜੇ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ<0 ){
      p_ਵਿਅੰਜਕ->op = TK_NULL;
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
    {
      ਵਿਅੰਜਕ *pNew;
      ਪੂਰਨ i_ਕਾਲਮ;
      ਵਿਅੰਜਕ *pCopy;
      ਵਿਅੰਜਕ if_Null_ਪੰਕਤੀ;
      i_ਕਾਲਮ = p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
      ਪ੍ਰਤਿਜ੍ਞਾ( i_ਕਾਲਮ>=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSubst->pEList!=0 && i_ਕਾਲਮ<pSubst->pEList->n_ਵਿਅੰਜਕ );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਸੱਜਾ==0 );
      pCopy = pSubst->pEList->a[i_ਕਾਲਮ].p_ਵਿਅੰਜਕ;
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_ਸਦਿਸ਼(pCopy) ){
        sqlite3_ਸਦਿਸ਼_Error_ਸੁਨੇਹਾ(pSubst->pParse, pCopy);
      }ਨਹੀਂਤਾਂ{
        sqlite3 *db = pSubst->pParse->db;
        ਜੇ( pSubst->isOuterJoin
         && (pCopy->op!=TK_ਕਾਲਮ || pCopy->i_ਸਾਰਣੀ!=pSubst->i_New_ਸਾਰਣੀ)
        ){
          memset(&if_Null_ਪੰਕਤੀ, 0, ਆਕਾਰ(if_Null_ਪੰਕਤੀ));
          if_Null_ਪੰਕਤੀ.op = TK_IF_NULL_ਪੰਕਤੀ;
          if_Null_ਪੰਕਤੀ.p_ਖੱਬਾ = pCopy;
          if_Null_ਪੰਕਤੀ.i_ਸਾਰਣੀ = pSubst->i_New_ਸਾਰਣੀ;
          if_Null_ਪੰਕਤੀ.i_ਕਾਲਮ = -99;
          if_Null_ਪੰਕਤੀ.flags = EP_If_Null_ਪੰਕਤੀ;
          pCopy = &if_Null_ਪੰਕਤੀ;
        }
        testcase( ਵਿਅੰਜਕ_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_ਵਿਅੰਜਕ_Dup(db, pCopy, 0);
        ਜੇ( db->mallocFailed ){
          sqlite3_ਵਿਅੰਜਕ_Delete(db, pNew);
          ਵਾਪਸ p_ਵਿਅੰਜਕ;
        }
        ਜੇ( pSubst->isOuterJoin ){
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pNew, EP_CanBeNull);
        }
        ਜੇ( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_ਮੁੱਲ = sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(pNew);
          pNew->op = TK_INTEGER;
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pNew, EP_Int_ਮੁੱਲ);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[i_ਕਾਲਮ].p_ਵਿਅੰਜਕ
          );
          ਜੇ( pNat!=pColl || (pNew->op!=TK_ਕਾਲਮ && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        ਵਿਅੰਜਕ_Clear_Property(pNew, EP_Collate);
        ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_OuterON|EP_InnerON) ){
          sqlite3_ਸੈੱਟ_Join_ਵਿਅੰਜਕ(pNew, p_ਵਿਅੰਜਕ->w.iJoin,
                             p_ਵਿਅੰਜਕ->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਿਅੰਜਕ);
        p_ਵਿਅੰਜਕ = pNew;
      }
    }
  }ਨਹੀਂਤਾਂ{
    ਜੇ( p_ਵਿਅੰਜਕ->op==TK_IF_NULL_ਪੰਕਤੀ && p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==pSubst->i_ਸਾਰਣੀ ){
      p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = pSubst->i_New_ਸਾਰਣੀ;
    }
    ਜੇ( p_ਵਿਅੰਜਕ->op==TK_AGG_FUNCTION && p_ਵਿਅੰਜਕ->op2>=pSubst->n_Sel_ਡੂੰਘਾਈ ){
      p_ਵਿਅੰਜਕ->op2--;
    }
    p_ਵਿਅੰਜਕ->p_ਖੱਬਾ = subst_ਵਿਅੰਜਕ(pSubst, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
    p_ਵਿਅੰਜਕ->p_ਸੱਜਾ = subst_ਵਿਅੰਜਕ(pSubst, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
    ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
      substSelect(pSubst, p_ਵਿਅੰਜਕ->x.pSelect, 1);
    }ਨਹੀਂਤਾਂ{
      subst_ਵਿਅੰਜਕ_List(pSubst, p_ਵਿਅੰਜਕ->x.pList);
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
      ਖਿੜਕੀ *pWin = p_ਵਿਅੰਜਕ->y.pWin;
      pWin->pFilter = subst_ਵਿਅੰਜਕ(pSubst, pWin->pFilter);
      subst_ਵਿਅੰਜਕ_List(pSubst, pWin->p_ਹਿੱਸੇ_ਕਰੋ);
      subst_ਵਿਅੰਜਕ_List(pSubst, pWin->p_ਕ੍ਰਮ_By);
    }
#ਜੇ_ਅੰਤ
  }
  ਵਾਪਸ p_ਵਿਅੰਜਕ;
}
ਸਥਿਰ ਖਾਲੀ subst_ਵਿਅੰਜਕ_List(
  SubstContext *pSubst, /* Description of the substitution */
  ਵਿਅੰਜਕ_List *pList       /* List to scan and in which to make substitutes */
){
  ਪੂਰਨ i;
  ਜੇ( pList==0 ) ਵਾਪਸ;
  ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
    pList->a[i].p_ਵਿਅੰਜਕ = subst_ਵਿਅੰਜਕ(pSubst, pList->a[i].p_ਵਿਅੰਜਕ);
  }
}
ਸਥਿਰ ਖਾਲੀ substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  ਪੂਰਨ do_ਪੂਰਵ           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  Src_ਵਸਤੂ *p_ਵਸਤੂ;
  ਪੂਰਨ i;
  ਜੇ( !p ) ਵਾਪਸ;
  pSubst->n_Sel_ਡੂੰਘਾਈ++;
  ਕਰੋ{
    subst_ਵਿਅੰਜਕ_List(pSubst, p->pEList);
    subst_ਵਿਅੰਜਕ_List(pSubst, p->p_ਸਮੂਹ_By);
    subst_ਵਿਅੰਜਕ_List(pSubst, p->p_ਕ੍ਰਮ_By);
    p->pHaving = subst_ਵਿਅੰਜਕ(pSubst, p->pHaving);
    p->pWhere = subst_ਵਿਅੰਜਕ(pSubst, p->pWhere);
    pSrc = p->pSrc;
    ਪ੍ਰਤਿਜ੍ਞਾ( pSrc!=0 );
    ਲਈ(i=pSrc->nSrc, p_ਵਸਤੂ=pSrc->a; i>0; i--, p_ਵਸਤੂ++){
      ਜੇ( p_ਵਸਤੂ->fg.isSubquery ){
        substSelect(pSubst, p_ਵਸਤੂ->u4.pSubq->pSelect, 1);
      }
      ਜੇ( p_ਵਸਤੂ->fg.isTabFunc ){
        subst_ਵਿਅੰਜਕ_List(pSubst, p_ਵਸਤੂ->u1.pFuncArg);
      }
    }
  }ਜਦੋਂ( do_ਪੂਰਵ && (p = p->p_ਪੂਰਵ)!=0 );
  pSubst->n_Sel_ਡੂੰਘਾਈ--;
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ਜੇ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
ਸਥਿਰ ਪੂਰਨ recompute_Columns_ਵਰਤਿਆ_ਵਿਅੰਜਕ(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  Src_ਵਸਤੂ *p_ਵਸਤੂ;
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_ਕਾਲਮ ) ਵਾਪਸ WRC_Continue;
  p_ਵਸਤੂ = pWalker->u.p_Src_ਵਸਤੂ;
  ਜੇ( p_ਵਸਤੂ->iCursor!=p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ ) ਵਾਪਸ WRC_Continue;
  ਜੇ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ<0 ) ਵਾਪਸ WRC_Continue;
  p_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ |= sqlite3_ਵਿਅੰਜਕ_ਖਾਨਾ_ਵਰਤਿਆ(p_ਵਿਅੰਜਕ);
  ਵਾਪਸ WRC_Continue;
}
ਸਥਿਰ ਖਾਲੀ recompute_Columns_ਵਰਤਿਆ(
  Select *pSelect,                 /* The complete SELECT statement */
  Src_ਵਸਤੂ *p_Src_ਵਸਤੂ                /* Which FROM clause item to recompute */
){
  Walker w;
  ਜੇ( NEVER(p_Src_ਵਸਤੂ->pSTab==0) ) ਵਾਪਸ;
  memset(&w, 0, ਆਕਾਰ(w));
  w.x_ਵਿਅੰਜਕ_Callback = recompute_Columns_ਵਰਤਿਆ_ਵਿਅੰਜਕ;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.p_Src_ਵਸਤੂ = p_Src_ਵਸਤੂ;
  p_Src_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ਜੇ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ਸਥਿਰ ਖਾਲੀ srclistRenumberCursors(
  Parse *pParse,                  /* Parse context */
  ਪੂਰਨ *aCsrMap,                   /* Array to store cursor mappings in */
  SrcList *pSrc,                  /* FROM clause to renumber */
  ਪੂਰਨ iExcept                     /* FROM clause item to skip */
){
  ਪੂਰਨ i;
  Src_ਵਸਤੂ *p_ਵਸਤੂ;
  ਲਈ(i=0, p_ਵਸਤੂ=pSrc->a; i<pSrc->nSrc; i++, p_ਵਸਤੂ++){
    ਜੇ( i!=iExcept ){
      Select *p;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->iCursor < aCsrMap[0] );
      ਜੇ( !p_ਵਸਤੂ->fg.isRecursive || aCsrMap[p_ਵਸਤੂ->iCursor+1]==0 ){
        aCsrMap[p_ਵਸਤੂ->iCursor+1] = pParse->nTab++;
      }
      p_ਵਸਤੂ->iCursor = aCsrMap[p_ਵਸਤੂ->iCursor+1];
      ਜੇ( p_ਵਸਤੂ->fg.isSubquery ){
        ਲਈ(p=p_ਵਸਤੂ->u4.pSubq->pSelect; p; p=p->p_ਪੂਰਵ){
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

/*
** *piCursor is a cursor number.  Change it if it needs to be mapped.
*/
ਸਥਿਰ ਖਾਲੀ renumberCursorDoMapping(Walker *pWalker, ਪੂਰਨ *piCursor){
  ਪੂਰਨ *aCsrMap = pWalker->u.ai_ਖਾਨਾ;
  ਪੂਰਨ iCsr = *piCursor;
  ਜੇ( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
ਸਥਿਰ ਪੂਰਨ renumberCursorsCb(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ op = p_ਵਿਅੰਜਕ->op;
  ਜੇ( op==TK_ਕਾਲਮ || op==TK_IF_NULL_ਪੰਕਤੀ ){
    renumberCursorDoMapping(pWalker, &p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ);
  }
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, (EP_OuterON|EP_InnerON)) ){
    testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) );
    testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_ਵਿਅੰਜਕ->w.iJoin);
  }
  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਖਾਲੀ renumberCursors(
  Parse *pParse,                  /* Parse context */
  Select *p,                      /* Select to renumber cursors within */
  ਪੂਰਨ iExcept,                    /* FROM clause item to skip */
  ਪੂਰਨ *aCsrMap                    /* Working space */
){
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  memset(&w, 0, ਆਕਾਰ(w));
  w.u.ai_ਖਾਨਾ = aCsrMap;
  w.x_ਵਿਅੰਜਕ_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
ਸਥਿਰ ਵਿਅੰਜਕ_List *ਲੱਭੋ_Leftmost_Exprlist(Select *pSel){
  ਜਦੋਂ( pSel->p_ਪੂਰਵ ){
    pSel = pSel->p_ਪੂਰਵ;
  }
  ਵਾਪਸ pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
ਸਥਿਰ ਪੂਰਨ compoundHasDifferentAffinities(Select *p){
  ਪੂਰਨ ii;
  ਵਿਅੰਜਕ_List *pList;
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਪੂਰਵ!=0 );
  pList = p->pEList;
  ਲਈ(ii=0; ii<pList->n_ਵਿਅੰਜਕ; ii++){
    ਅੱਖਰ aff;
    Select *pSub1;
    ਪ੍ਰਤਿਜ੍ਞਾ( pList->a[ii].p_ਵਿਅੰਜਕ!=0 );
    aff = sqlite3_ਵਿਅੰਜਕ_Affinity(pList->a[ii].p_ਵਿਅੰਜਕ);
    ਲਈ(pSub1=p->p_ਪੂਰਵ; pSub1; pSub1=pSub1->p_ਪੂਰਵ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pSub1->pEList!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSub1->pEList->n_ਵਿਅੰਜਕ>ii );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSub1->pEList->a[ii].p_ਵਿਅੰਜਕ!=0 );
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Affinity(pSub1->pEList->a[ii].p_ਵਿਅੰਜਕ)!=aff ){
        ਵਾਪਸ 1;
      }
    }
  }
  ਵਾਪਸ 0;
}

#ਜੇ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ਸਥਿਰ ਪੂਰਨ flattenSubquery(
  Parse *pParse,       /* Parsing context */
  Select *p,           /* The parent or outer SELECT statement */
  ਪੂਰਨ iFrom,           /* Index in p->pSrc->a[] of the inner subquery */
  ਪੂਰਨ isAgg            /* True if outer SELECT uses aggregate functions */
){
  ਅਚਲ ਅੱਖਰ *zSavedAuthContext = pParse->zAuthContext;
  Select *p_ਮਾਪਾ;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  ਪੂਰਨ i_ਮਾਪਾ;        /* VDBE cursor number of the pSub result set temp table */
  ਪੂਰਨ i_New_ਮਾਪਾ = -1;/* Replacement table for iParent */
  ਪੂਰਨ isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  ਪੂਰਨ i;              /* Loop counter */
  ਵਿਅੰਜਕ *pWhere;                    /* The WHERE clause */
  Src_ਵਸਤੂ *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  ਪੂਰਨ *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਪੂਰਵ==0 );
  ਜੇ( OptimizationDisabled(db, SQLITE_QueryFlattener) ) ਵਾਪਸ 0;
  pSrc = p->pSrc;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  i_ਮਾਪਾ = pSubitem->iCursor;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSub!=0 );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  ਜੇ( p->pWin || pSub->pWin ) ਵਾਪਸ 0;                  /* Restriction (25) */
#ਜੇ_ਅੰਤ

  pSubSrc = pSub->pSrc;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  ਜੇ( pSub->p_ਹੱਦਬੰਦੀ && p->p_ਹੱਦਬੰਦੀ ) ਵਾਪਸ 0;              /* Restriction (13) */
  ਜੇ( pSub->p_ਹੱਦਬੰਦੀ && pSub->p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ ) ਵਾਪਸ 0;   /* Restriction (14) */
  ਜੇ( (p->selFlags & SF_Compound)!=0 && pSub->p_ਹੱਦਬੰਦੀ ){
    ਵਾਪਸ 0;                                            /* Restriction (15) */
  }
  ਜੇ( pSubSrc->nSrc==0 ) ਵਾਪਸ 0;                       /* Restriction (7)  */
  ਜੇ( pSub->selFlags & SF_Distinct ) ਵਾਪਸ 0;           /* Restriction (4)  */
  ਜੇ( pSub->p_ਹੱਦਬੰਦੀ && (pSrc->nSrc>1 || isAgg) ){
     ਵਾਪਸ 0;         /* Restrictions (8)(9) */
  }
  ਜੇ( p->p_ਕ੍ਰਮ_By && pSub->p_ਕ੍ਰਮ_By ){
     ਵਾਪਸ 0;                                           /* Restriction (11) */
  }
  ਜੇ( isAgg && pSub->p_ਕ੍ਰਮ_By ) ਵਾਪਸ 0;                /* Restriction (16) */
  ਜੇ( pSub->p_ਹੱਦਬੰਦੀ && p->pWhere ) ਵਾਪਸ 0;              /* Restriction (19) */
  ਜੇ( pSub->p_ਹੱਦਬੰਦੀ && (p->selFlags & SF_Distinct)!=0 ){
     ਵਾਪਸ 0;         /* Restriction (21) */
  }
  ਜੇ( pSub->selFlags & (SF_Recursive) ){
    ਵਾਪਸ 0; /* Restrictions (22) */
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
  ਜੇ( (pSubitem->fg.jointype & (JT_OUTER|JT_LTORJ))!=0 ){
    ਜੇ( pSubSrc->nSrc>1                        /* (3a) */
     /**** || IsVirtual(pSubSrc->a[0].pSTab)      (3b)-omitted */
     || (p->selFlags & SF_Distinct)!=0         /* (3d) */
     || (pSubitem->fg.jointype & JT_ਸੱਜਾ)!=0  /* (26) */
    ){
      ਵਾਪਸ 0;
    }
    isOuterJoin = 1;
  }

  ਪ੍ਰਤਿਜ੍ਞਾ( pSubSrc->nSrc>0 );  /* True by restriction (7) */
  ਜੇ( iFrom>0 && (pSubSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
    ਵਾਪਸ 0;   /* Restriction (27a) */
  }

  /* Condition (28) is blocked by the caller */
  ਪ੍ਰਤਿਜ੍ਞਾ( !pSubitem->fg.isCte || pSubitem->u2.pCteUse->eM10d!=M10d_Yes );

  /* Restriction (17): If the sub-query is a compound SELECT, then it must
  ** use only the UNION ALL operator. And none of the simple select queries
  ** that make up the compound SELECT are allowed to be aggregate or distinct
  ** queries.
  */
  ਜੇ( pSub->p_ਪੂਰਵ ){
    ਪੂਰਨ ii;
    ਜੇ( pSub->p_ਕ੍ਰਮ_By ){
      ਵਾਪਸ 0;  /* Restriction (20) */
    }
    ਜੇ( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      ਵਾਪਸ 0; /* (17d1), (17d2), or (17f) */
    }
    ਲਈ(pSub1=pSub; pSub1; pSub1=pSub1->p_ਪੂਰਵ){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSub->pSrc!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( (pSub->selFlags & SF_Recursive)==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSub->pEList->n_ਵਿਅੰਜਕ==pSub1->pEList->n_ਵਿਅੰਜਕ );
      ਜੇ( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->p_ਪੂਰਵ && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#ਜੇ_ਅੰਤ
      ){
        ਵਾਪਸ 0;
      }
      ਜੇ( iFrom>0 && (pSub1->pSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
        /* Without this restriction, the JT_LTORJ flag would end up being
        ** omitted on left-hand tables of the right join that is being
        ** flattened. */
        ਵਾਪਸ 0;   /* Restrictions (17g), (27b) */
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }

    /* Restriction (18). */
    ਜੇ( p->p_ਕ੍ਰਮ_By ){
      ਲਈ(ii=0; ii<p->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; ii++){
        ਜੇ( p->p_ਕ੍ਰਮ_By->a[ii].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ==0 ) ਵਾਪਸ 0;
      }
    }

    /* Restriction (23) */
    ਜੇ( (p->selFlags & SF_Recursive) ) ਵਾਪਸ 0;

    /* Restriction (17h) */
    ਜੇ( compoundHasDifferentAffinities(pSub) ) ਵਾਪਸ 0;

    ਜੇ( pSrc->nSrc>1 ){
      ਜੇ( pParse->nSelect>500 ) ਵਾਪਸ 0;
      ਜੇ( OptimizationDisabled(db, SQLITE_FlttnUnionAll) ) ਵਾਪਸ 0;
      aCsrMap = sqlite3DbMallocZero(db, ((i64)pParse->nTab+1)*ਆਕਾਰ(ਪੂਰਨ));
      ਜੇ( aCsrMap ) aCsrMap[0] = pParse->nTab;
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
  
  ਜੇ( ALWAYS(pSubitem->fg.isSubquery) ){
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  }ਨਹੀਂਤਾਂ{
    pSub1 = 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pSubitem->fg.isSubquery==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pSubitem->fg.fixedSchema==0 );
  sqlite3DbFree(db, pSubitem->zName);
  sqlite3DbFree(db, pSubitem->zAlias);
  pSubitem->zName = 0;
  pSubitem->zAlias = 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSubitem->fg.isUsing!=0 || pSubitem->u3.pOn==0 );

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
  ਲਈ(pSub=pSub->p_ਪੂਰਵ; pSub; pSub=pSub->p_ਪੂਰਵ){
    Select *pNew;
    ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By = p->p_ਕ੍ਰਮ_By;
    ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ = p->p_ਹੱਦਬੰਦੀ;
    Select *p_ਪੂਰਵ = p->p_ਪੂਰਵ;
    ਸਾਰਣੀ *p_ਵਸਤੂ_Tab = pSubitem->pSTab;
    pSubitem->pSTab = 0;
    p->p_ਕ੍ਰਮ_By = 0;
    p->p_ਪੂਰਵ = 0;
    p->p_ਹੱਦਬੰਦੀ = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->p_ਹੱਦਬੰਦੀ = p_ਹੱਦਬੰਦੀ;
    p->p_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By;
    p->op = TK_ALL;
    pSubitem->pSTab = p_ਵਸਤੂ_Tab;
    ਜੇ( pNew==0 ){
      p->p_ਪੂਰਵ = p_ਪੂਰਵ;
    }ਨਹੀਂਤਾਂ{
      pNew->selId = ++pParse->nSelect;
      ਜੇ( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->p_ਪੂਰਵ = p_ਪੂਰਵ;
      ਜੇ( p_ਪੂਰਵ ) p_ਪੂਰਵ->p_ਅਗਲਾ = pNew;
      pNew->p_ਅਗਲਾ = p;
      p->p_ਪੂਰਵ = pNew;
      TREETRACE(0x4,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pSubitem->fg.isSubquery==0 );
  }
  sqlite3DbFree(db, aCsrMap);
  ਜੇ( db->mallocFailed ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pSubitem->fg.fixedSchema==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pSubitem->fg.isSubquery==0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pSubitem->u4.zDatabase==0 );
    sqlite3_Src_ਵਸਤੂ_Attach_Subquery(pParse, pSubitem, pSub1, 0);
    ਵਾਪਸ 1;
  }

  /* Defer deleting the Table object associated with the
  ** subquery until code generation is
  ** complete, since there may still exist Expr.pTab entries that
  ** refer to the subquery even after flattening.  Ticket #3346.
  **
  ** pSubitem->pSTab is always non-NULL by test restrictions and tests above.
  */
  ਜੇ( ALWAYS(pSubitem->pSTab!=0) ){
    ਸਾਰਣੀ *p_Tab_ਮੰਜ਼ਿਲ_Del = pSubitem->pSTab;
    ਜੇ( p_Tab_ਮੰਜ਼ਿਲ_Del->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      sqlite3_Parser_ਜੋੜੋ_Cleanup(pToplevel, sqlite3_Delete_ਸਾਰਣੀ_Generic, p_Tab_ਮੰਜ਼ਿਲ_Del);
      testcase( pToplevel->earlyCleanup );
    }ਨਹੀਂਤਾਂ{
      p_Tab_ਮੰਜ਼ਿਲ_Del->nTabRef--;
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
  ਲਈ(p_ਮਾਪਾ=p; p_ਮਾਪਾ; p_ਮਾਪਾ=p_ਮਾਪਾ->p_ਪੂਰਵ, pSub=pSub->p_ਪੂਰਵ){
    ਪੂਰਨ nSubSrc;
    u8 jointype = p_ਮਾਪਾ->pSrc->a[iFrom].fg.jointype;
    ਪ੍ਰਤਿਜ੍ਞਾ( pSub!=0 );
    pSubSrc = pSub->pSrc;     /* FROM clause of subquery */
    nSubSrc = pSubSrc->nSrc;  /* Number of terms in subquery FROM clause */
    pSrc = p_ਮਾਪਾ->pSrc;     /* FROM clause of the outer query */

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
    ਜੇ( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      ਜੇ( pSrc==0 ) ਰੋਕੋ;
      p_ਮਾਪਾ->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    i_New_ਮਾਪਾ = pSubSrc->a[0].iCursor;
    ਲਈ(i=0; i<nSubSrc; i++){
      Src_ਵਸਤੂ *p_ਵਸਤੂ = &pSrc->a[i+iFrom];
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.isTabFunc==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.isSubquery
           || p_ਵਸਤੂ->fg.fixedSchema
           || p_ਵਸਤੂ->u4.zDatabase==0 );
      ਜੇ( p_ਵਸਤੂ->fg.isUsing ) sqlite3IdListDelete(db, p_ਵਸਤੂ->u3.pUsing);
      *p_ਵਸਤੂ = pSubSrc->a[i];
      p_ਵਸਤੂ->fg.jointype |= (jointype & JT_LTORJ);
      memset(&pSubSrc->a[i], 0, ਆਕਾਰ(pSubSrc->a[i]));
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
    ਜੇ( pSub->p_ਕ੍ਰਮ_By ){
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
      ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By = pSub->p_ਕ੍ਰਮ_By;
      ਲਈ(i=0; i<p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; i++){
        p_ਕ੍ਰਮ_By->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = 0;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮਾਪਾ->p_ਕ੍ਰਮ_By==0 );
      p_ਮਾਪਾ->p_ਕ੍ਰਮ_By = p_ਕ੍ਰਮ_By;
      pSub->p_ਕ੍ਰਮ_By = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    ਜੇ( isOuterJoin>0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pSubSrc->nSrc==1 );
      sqlite3_ਸੈੱਟ_Join_ਵਿਅੰਜਕ(pWhere, i_New_ਮਾਪਾ, EP_OuterON);
    }
    ਜੇ( pWhere ){
      ਜੇ( p_ਮਾਪਾ->pWhere ){
        p_ਮਾਪਾ->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, p_ਮਾਪਾ->pWhere);
      }ਨਹੀਂਤਾਂ{
        p_ਮਾਪਾ->pWhere = pWhere;
      }
    }
    ਜੇ( db->mallocFailed==0 ){
      SubstContext x;
      x.pParse = pParse;
      x.i_ਸਾਰਣੀ = i_ਮਾਪਾ;
      x.i_New_ਸਾਰਣੀ = i_New_ਮਾਪਾ;
      x.isOuterJoin = isOuterJoin;
      x.n_Sel_ਡੂੰਘਾਈ = 0;
      x.pEList = pSub->pEList;
      x.pCList = ਲੱਭੋ_Leftmost_Exprlist(pSub);
      substSelect(&x, p_ਮਾਪਾ, 0);
    }
 
    /* The flattened query is a compound if either the inner or the
    ** outer query is a compound. */
    p_ਮਾਪਾ->selFlags |= pSub->selFlags & SF_Compound;
    ਪ੍ਰਤਿਜ੍ਞਾ( (pSub->selFlags & SF_Distinct)==0 ); /* restriction (17b) */
 
    /*
    ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y;
    **
    ** One is tempted to try to add a and b to combine the limits.  But this
    ** does not work if either limit is negative.
    */
    ਜੇ( pSub->p_ਹੱਦਬੰਦੀ ){
      p_ਮਾਪਾ->p_ਹੱਦਬੰਦੀ = pSub->p_ਹੱਦਬੰਦੀ;
      pSub->p_ਹੱਦਬੰਦੀ = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    ਲਈ(i=0; i<nSubSrc; i++){
      recompute_Columns_ਵਰਤਿਆ(p_ਮਾਪਾ, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#ਜੇ TREETRACE_ENABLED
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
  }
#ਜੇ_ਅੰਤ

  ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** A structure to keep track of all of the column values that are fixed to
** a known value due to WHERE clause constraints of the form COLUMN=VALUE.
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ WhereConst WhereConst;
ਬਣਤਰ WhereConst {
  Parse *pParse;   /* Parsing context */
  u8 *pOomFault;   /* Pointer to pParse->db->mallocFailed */
  ਪੂਰਨ nConst;      /* Number for COLUMN=CONSTANT terms */
  ਪੂਰਨ nChng;       /* Number of times a constant is propagated */
  ਪੂਰਨ bHasAffBlob; /* At least one column in apExpr[] as affinity BLOB */
  u32 mExcludeOn;  /* Which ON expressions to exclude from considertion.
                   ** Either EP_OuterON or EP_InnerON|EP_OuterON */
  ਵਿਅੰਜਕ **ap_ਵਿਅੰਜਕ;   /* [i*2] is COLUMN and [i*2+1] is VALUE */
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
ਸਥਿਰ ਖਾਲੀ const_ਪਾਓ(
  WhereConst *pConst,  /* The WhereConst into which we are inserting */
  ਵਿਅੰਜਕ *p_ਕਾਲਮ,       /* The COLUMN part of the constraint */
  ਵਿਅੰਜਕ *p_ਮੁੱਲ,        /* The VALUE part of the constraint */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕਾਲਮ->op==TK_ਕਾਲਮ );
  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਵਿਅੰਜਕ_Is_Constant(pConst->pParse, p_ਮੁੱਲ) );
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਕਾਲਮ, EP_Fixed_ਖਾਨਾ) ) ਵਾਪਸ;
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਮੁੱਲ)!=0 ) ਵਾਪਸ;
  ਜੇ( !sqlite3_Is_ਦੁਆਧਾਰੀ(sqlite3_ਵਿਅੰਜਕ_Compare_Coll_Seq(pConst->pParse,p_ਵਿਅੰਜਕ)) ){
    ਵਾਪਸ;
  }
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Can_Return_Subtype(pConst->pParse, p_ਮੁੱਲ) ) ਵਾਪਸ;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  ਲਈ(i=0; i<pConst->nConst; i++){
    ਅਚਲ ਵਿਅੰਜਕ *pE2 = pConst->ap_ਵਿਅੰਜਕ[i*2];
    ਪ੍ਰਤਿਜ੍ਞਾ( pE2->op==TK_ਕਾਲਮ );
    ਜੇ( pE2->i_ਸਾਰਣੀ==p_ਕਾਲਮ->i_ਸਾਰਣੀ
     && pE2->i_ਕਾਲਮ==p_ਕਾਲਮ->i_ਕਾਲਮ
    ){
      ਵਾਪਸ;  /* Already present.  Return without doing anything. */
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਕਾਲਮ)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_ਵਿਅੰਜਕ = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_ਵਿਅੰਜਕ,
                         pConst->nConst*2*ਆਕਾਰ(ਵਿਅੰਜਕ*));
  ਜੇ( pConst->ap_ਵਿਅੰਜਕ==0 ){
    pConst->nConst = 0;
  }ਨਹੀਂਤਾਂ{
    pConst->ap_ਵਿਅੰਜਕ[pConst->nConst*2-2] = p_ਕਾਲਮ;
    pConst->ap_ਵਿਅੰਜਕ[pConst->nConst*2-1] = p_ਮੁੱਲ;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
ਸਥਿਰ ਖਾਲੀ ਲੱਭੋ_Const_ਵਿੱਚ_Where(WhereConst *pConst, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਵਿਅੰਜਕ *p_ਸੱਜਾ, *p_ਖੱਬਾ;
  ਜੇ( NEVER(p_ਵਿਅੰਜਕ==0) ) ਵਾਪਸ;
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, pConst->mExcludeOn) ){
    testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) );
    testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_InnerON) );
    ਵਾਪਸ;
  }
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_AND ){
    ਲੱਭੋ_Const_ਵਿੱਚ_Where(pConst, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
    ਲੱਭੋ_Const_ਵਿੱਚ_Where(pConst, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
    ਵਾਪਸ;
  }
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_EQ ) ਵਾਪਸ;
  p_ਸੱਜਾ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
  p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸੱਜਾ!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਖੱਬਾ!=0 );
  ਜੇ( p_ਸੱਜਾ->op==TK_ਕਾਲਮ && sqlite3_ਵਿਅੰਜਕ_Is_Constant(pConst->pParse, p_ਖੱਬਾ) ){
    const_ਪਾਓ(pConst,p_ਸੱਜਾ,p_ਖੱਬਾ,p_ਵਿਅੰਜਕ);
  }
  ਜੇ( p_ਖੱਬਾ->op==TK_ਕਾਲਮ && sqlite3_ਵਿਅੰਜਕ_Is_Constant(pConst->pParse, p_ਸੱਜਾ) ){
    const_ਪਾਓ(pConst,p_ਖੱਬਾ,p_ਸੱਜਾ,p_ਵਿਅੰਜਕ);
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
ਸਥਿਰ ਪੂਰਨ propagate_Constant_ਵਿਅੰਜਕ_Rewrite_One(
  WhereConst *pConst,
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,
  ਪੂਰਨ bIgnoreAffBlob
){
  ਪੂਰਨ i;
  ਜੇ( pConst->pOomFault[0] ) ਵਾਪਸ WRC_Prune;
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_ਕਾਲਮ ) ਵਾਪਸ WRC_Continue;
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Fixed_ਖਾਨਾ|pConst->mExcludeOn) ){
    testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Fixed_ਖਾਨਾ) );
    testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) );
    testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_InnerON) );
    ਵਾਪਸ WRC_Continue;
  }
  ਲਈ(i=0; i<pConst->nConst; i++){
    ਵਿਅੰਜਕ *p_ਕਾਲਮ = pConst->ap_ਵਿਅੰਜਕ[i*2];
    ਜੇ( p_ਕਾਲਮ==p_ਵਿਅੰਜਕ ) ਜਾਰੀ;
    ਜੇ( p_ਕਾਲਮ->i_ਸਾਰਣੀ!=p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ ) ਜਾਰੀ;
    ਜੇ( p_ਕਾਲਮ->i_ਕਾਲਮ!=p_ਵਿਅੰਜਕ->i_ਕਾਲਮ ) ਜਾਰੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    ਜੇ( bIgnoreAffBlob && sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਕਾਲਮ)<=SQLITE_AFF_BLOB ){
      ਰੋਕੋ;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    ਵਿਅੰਜਕ_Clear_Property(p_ਵਿਅੰਜਕ, EP_ਪੱਤੀ);
    ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_Fixed_ਖਾਨਾ);
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ==0 );
    p_ਵਿਅੰਜਕ->p_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ_Dup(pConst->pParse->db, pConst->ap_ਵਿਅੰਜਕ[i*2+1], 0);
    ਜੇ( pConst->pParse->db->mallocFailed ) ਵਾਪਸ WRC_Prune;
    ਰੋਕੋ;
  }
  ਵਾਪਸ WRC_Prune;
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
ਸਥਿਰ ਪੂਰਨ propagate_Constant_ਵਿਅੰਜਕ_Rewrite(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  WhereConst *pConst = pWalker->u.pConst;
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_GT==TK_EQ+1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_LE==TK_EQ+2 );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_LT==TK_EQ+3 );
  ਪ੍ਰਤਿਜ੍ਞਾ( TK_GE==TK_EQ+4 );
  ਜੇ( pConst->bHasAffBlob ){
    ਜੇ( (p_ਵਿਅੰਜਕ->op>=TK_EQ && p_ਵਿਅੰਜਕ->op<=TK_GE)
     || p_ਵਿਅੰਜਕ->op==TK_IS
    ){
      propagate_Constant_ਵਿਅੰਜਕ_Rewrite_One(pConst, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
      ਜੇ( pConst->pOomFault[0] ) ਵਾਪਸ WRC_Prune;
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Affinity(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ)!=SQLITE_AFF_ਪਾਠ ){
        propagate_Constant_ਵਿਅੰਜਕ_Rewrite_One(pConst, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, 0);
      }
    }
  }
  ਵਾਪਸ propagate_Constant_ਵਿਅੰਜਕ_Rewrite_One(pConst, p_ਵਿਅੰਜਕ, pConst->bHasAffBlob);
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
ਸਥਿਰ ਪੂਰਨ propagateConstants(
  Parse *pParse,   /* The parsing context */
  Select *p        /* The query in which to propagate constants */
){
  WhereConst x;
  Walker w;
  ਪੂਰਨ nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  ਕਰੋ{
    x.nConst = 0;
    x.nChng = 0;
    x.ap_ਵਿਅੰਜਕ = 0;
    x.bHasAffBlob = 0;
    ਜੇ( ALWAYS(p->pSrc!=0)
     && p->pSrc->nSrc>0
     && (p->pSrc->a[0].fg.jointype & JT_LTORJ)!=0
    ){
      /* Do not propagate constants on any ON clause if there is a
      ** RIGHT JOIN anywhere in the query */
      x.mExcludeOn = EP_InnerON | EP_OuterON;
    }ਨਹੀਂਤਾਂ{
      /* Do not propagate constants through the ON clause of a LEFT JOIN */
      x.mExcludeOn = EP_OuterON;
    }
    ਲੱਭੋ_Const_ਵਿੱਚ_Where(&x, p->pWhere);
    ਜੇ( x.nConst ){
      memset(&w, 0, ਆਕਾਰ(w));
      w.pParse = pParse;
      w.x_ਵਿਅੰਜਕ_Callback = propagate_Constant_ਵਿਅੰਜਕ_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walker_ਡੂੰਘਾਈ = 0;
      w.u.pConst = &x;
      sqlite3_Walk_ਵਿਅੰਜਕ(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_ਵਿਅੰਜਕ);
      nChng += x.nChng;
    }
  }ਜਦੋਂ( x.nChng ); 
  ਵਾਪਸ nChng;
}

#ਜੇ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
# ਜੇ !defined(SQLITE_OMIT_WINDOWFUNC)
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
ਸਥਿਰ ਪੂਰਨ ਧੱਕੋ_ਥੱਲੇ_ਖਿੜਕੀ_Check(Parse *pParse, Select *pSubq, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪ੍ਰਤਿਜ੍ਞਾ( pSubq->pWin->p_ਹਿੱਸੇ_ਕਰੋ );
  ਪ੍ਰਤਿਜ੍ਞਾ( (pSubq->selFlags & SF_MultiPart)==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pSubq->p_ਪੂਰਵ==0 );
  ਵਾਪਸ sqlite3_ਵਿਅੰਜਕ_Is_Constant_Or_ਸਮੂਹ_By(pParse, p_ਵਿਅੰਜਕ, pSubq->pWin->p_ਹਿੱਸੇ_ਕਰੋ);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ਜੇ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ਸਥਿਰ ਪੂਰਨ ਧੱਕੋ_ਥੱਲੇ_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  ਵਿਅੰਜਕ *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  ਪੂਰਨ iSrc              /* Which FROM clause term to try to push into  */
){
  ਵਿਅੰਜਕ *pNew;
  Src_ਵਸਤੂ *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  ਪੂਰਨ nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  ਜੇ( pWhere==0 ) ਵਾਪਸ 0;
  ਜੇ( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    ਵਾਪਸ 0;           /* restrictions (2) and (11) */
  }
  ਜੇ( pSrc->fg.jointype & (JT_LTORJ|JT_ਸੱਜਾ) ){
    ਵਾਪਸ 0;           /* restrictions (10) */
  }

  ਜੇ( pSubq->p_ਪੂਰਵ ){
    Select *pSel;
    ਪੂਰਨ notUnionAll = 0;
    ਲਈ(pSel=pSubq; pSel; pSel=pSel->p_ਪੂਰਵ){
      u8 op = pSel->op;
      ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      ਜੇ( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਜੇ( pSel->pWin ) ਵਾਪਸ 0;    /* restriction (6b) */
#ਜੇ_ਅੰਤ
    }
    ਜੇ( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      ਲਈ(pSel=pSubq; pSel; pSel=pSel->p_ਪੂਰਵ){
        ਪੂਰਨ ii;
        ਅਚਲ ਵਿਅੰਜਕ_List *pList = pSel->pEList;
        ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );
        ਲਈ(ii=0; ii<pList->n_ਵਿਅੰਜਕ; ii++){
          CollSeq *pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, pList->a[ii].p_ਵਿਅੰਜਕ);
          ਜੇ( !sqlite3_Is_ਦੁਆਧਾਰੀ(pColl) ){
            ਵਾਪਸ 0;  /* Restriction (8) */
          }
        }
      }
    }
  }ਨਹੀਂਤਾਂ{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( pSubq->pWin && pSubq->pWin->p_ਹਿੱਸੇ_ਕਰੋ==0 ) ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    ਲਈ(pX=pSubq; pX; pX=pX->p_ਪੂਰਵ){
      ਪ੍ਰਤਿਜ੍ਞਾ( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#ਜੇ_ਅੰਤ

  ਜੇ( pSubq->p_ਹੱਦਬੰਦੀ!=0 ){
    ਵਾਪਸ 0; /* restriction (3) */
  }
  ਜਦੋਂ( pWhere->op==TK_AND ){
    nChng += ਧੱਕੋ_ਥੱਲੇ_Where_Terms(pParse, pSubq, pWhere->p_ਸੱਜਾ, pSrcList, iSrc);
    pWhere = pWhere->p_ਖੱਬਾ;
  }

#ਜੇ 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    ਪੂਰਨ jj;
    ਲਈ(jj=0; jj<iSrc; jj++){
      ਜੇ( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        ਲਈ(jj++; jj<iSrc; jj++){
          ਜੇ( (pSrcList->a[jj].fg.jointype & JT_ਸੱਜਾ)!=0 ){
            ਵਾਪਸ 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  ਜੇ( is_ਖੱਬਾ_Join
   && (ਵਿਅੰਜਕ_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    ਵਾਪਸ 0; /* restriction (4) */
  }
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    ਵਾਪਸ 0; /* restriction (5) */
  }
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
  ਜੇ( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    ਵਿਅੰਜਕ *p_ਖੱਬਾ = pWhere->p_ਖੱਬਾ;
    ਜੇ( ALWAYS(p_ਖੱਬਾ) 
     && p_ਖੱਬਾ->op==TK_ਕਾਲਮ
     && p_ਖੱਬਾ->i_ਕਾਲਮ < 0
    ){
      ਵਾਪਸ 0;  /* Restriction (12) */
    }
  }
#ਜੇ_ਅੰਤ

  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Single_ਸਾਰਣੀ_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_ਧੱਕੋ_ਥੱਲੇ;
    ਜਦੋਂ( pSubq ){
      SubstContext x;
      pNew = sqlite3_ਵਿਅੰਜਕ_Dup(pParse->db, pWhere, 0);
      unset_Join_ਵਿਅੰਜਕ(pNew, -1, 1);
      x.pParse = pParse;
      x.i_ਸਾਰਣੀ = pSrc->iCursor;
      x.i_New_ਸਾਰਣੀ = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.n_Sel_ਡੂੰਘਾਈ = 0;
      x.pEList = pSubq->pEList;
      x.pCList = ਲੱਭੋ_Leftmost_Exprlist(pSubq);
      pNew = subst_ਵਿਅੰਜਕ(&x, pNew);
      ਪ੍ਰਤਿਜ੍ਞਾ( pNew!=0 || pParse->nErr!=0 );
      ਜੇ( pParse->nErr==0 && pNew->op==TK_ਵਿੱਚ && ਵਿਅੰਜਕ_Use_XSelect(pNew) ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_Cloned_Rhs_ਵਿੱਚ;
        ਪ੍ਰਤਿਜ੍ਞਾ( pWhere!=0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( pWhere->op==TK_ਵਿੱਚ );
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(pWhere) );
        ਪ੍ਰਤਿਜ੍ਞਾ( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_Cloned_Rhs_ਵਿੱਚ;
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਜੇ( pSubq->pWin && 0==ਧੱਕੋ_ਥੱਲੇ_ਖਿੜਕੀ_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, pNew);
        nChng--;
        ਰੋਕੋ;
      }
#ਜੇ_ਅੰਤ
      ਜੇ( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_ਵਿਅੰਜਕ_And(pParse, pSubq->pHaving, pNew);
      }ਨਹੀਂਤਾਂ{
        pSubq->pWhere = sqlite3_ਵਿਅੰਜਕ_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->p_ਪੂਰਵ;
    }
  }
  ਵਾਪਸ nChng;
}
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
ਸਥਿਰ ਪੂਰਨ disable_ਅਣਵਰਤਿਆ_Subquery_ਨਤੀਜਾ_Columns(Src_ਵਸਤੂ *p_ਵਸਤੂ){
  ਪੂਰਨ n_ਖਾਨਾ;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  ਸਾਰਣੀ *pTab;       /* The table that describes the subquery */
  ਪੂਰਨ j;             /* Column number */
  ਪੂਰਨ nChng = 0;     /* Number of columns converted to NULL */
  Bitmask ਖਾਨਾ_ਵਰਤਿਆ;   /* Columns that may not be NULLed out */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ!=0 );
  ਜੇ( p_ਵਸਤੂ->fg.isCorrelated || p_ਵਸਤੂ->fg.isCte ){
    ਵਾਪਸ 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->pSTab!=0 );
  pTab = p_ਵਸਤੂ->pSTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.isSubquery );
  pSub = p_ਵਸਤੂ->u4.pSubq->pSelect;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSub->pEList->n_ਵਿਅੰਜਕ==pTab->n_ਖਾਨਾ );
  ਲਈ(pX=pSub; pX; pX=pX->p_ਪੂਰਵ){
    ਜੇ( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      ਵਾਪਸ 0;
    }
    ਜੇ( pX->p_ਪੂਰਵ && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      ਵਾਪਸ 0;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      ਵਾਪਸ 0;
    }
#ਜੇ_ਅੰਤ
  }
  ਖਾਨਾ_ਵਰਤਿਆ = p_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ;
  ਜੇ( pSub->p_ਕ੍ਰਮ_By ){
    ਵਿਅੰਜਕ_List *pList = pSub->p_ਕ੍ਰਮ_By;
    ਲਈ(j=0; j<pList->n_ਵਿਅੰਜਕ; j++){
      u16 i_ਖਾਨਾ = pList->a[j].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ;
      ਜੇ( i_ਖਾਨਾ>0 ){
        i_ਖਾਨਾ--;
        ਖਾਨਾ_ਵਰਤਿਆ |= ((Bitmask)1)<<(i_ਖਾਨਾ>=BMS ? BMS-1 : i_ਖਾਨਾ);
      }
    }
  }
  n_ਖਾਨਾ = pTab->n_ਖਾਨਾ;
  ਲਈ(j=0; j<n_ਖਾਨਾ; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    ਜੇ( (m & ਖਾਨਾ_ਵਰਤਿਆ)!=0 ) ਜਾਰੀ;
    ਲਈ(pX=pSub; pX; pX=pX->p_ਪੂਰਵ) {
      ਵਿਅੰਜਕ *pY = pX->pEList->a[j].p_ਵਿਅੰਜਕ;
      ਜੇ( pY->op==TK_NULL ) ਜਾਰੀ;
      pY->op = TK_NULL;
      ਵਿਅੰਜਕ_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_ਧੱਕੋ_ਥੱਲੇ;
      nChng++;
    }
  }
  ਵਾਪਸ nChng;
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
ਸਥਿਰ u8 minMaxQuery(sqlite3 *db, ਵਿਅੰਜਕ *pFunc, ਵਿਅੰਜਕ_List **ppMinMax){
  ਪੂਰਨ eRet = WHERE_ORDERBY_ਸਾਧਾਰਨ;      /* Return value */
  ਵਿਅੰਜਕ_List *pEList;                     /* Arguments to agg function */
  ਅਚਲ ਅੱਖਰ *zFunc;                    /* Name of aggregate function pFunc */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;
  u8 ਛਾਂਟੋ_Flags = 0;

  ਪ੍ਰਤਿਜ੍ਞਾ( *ppMinMax==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFunc->op==TK_AGG_FUNCTION );
  ਪ੍ਰਤਿਜ੍ਞਾ( !Is_ਖਿੜਕੀ_Func(pFunc) );
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  ਜੇ( pEList==0
   || pEList->n_ਵਿਅੰਜਕ!=1
   || ਵਿਅੰਜਕ_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    ਵਾਪਸ eRet;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pFunc, EP_Int_ਮੁੱਲ) );
  zFunc = pFunc->u.zToken;
  ਜੇ( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(pEList->a[0].p_ਵਿਅੰਜਕ) ){
      ਛਾਂਟੋ_Flags = KEYINFO_ਕ੍ਰਮ_BIGNULL;
    }
  }ਨਹੀਂਤਾਂ ਜੇ( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    ਛਾਂਟੋ_Flags = KEYINFO_ਕ੍ਰਮ_DESC;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ eRet;
  }
  *ppMinMax = p_ਕ੍ਰਮ_By = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, pEList, 0);
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕ੍ਰਮ_By!=0 || db->mallocFailed );
  ਜੇ( p_ਕ੍ਰਮ_By ) p_ਕ੍ਰਮ_By->a[0].fg.ਛਾਂਟੋ_Flags = ਛਾਂਟੋ_Flags;
  ਵਾਪਸ eRet;
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
ਸਥਿਰ ਸਾਰਣੀ *is_Simple_ਗਿਣਤੀ(Select *p, AggInfo *pAggInfo){
  ਸਾਰਣੀ *pTab;
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;

  ਪ੍ਰਤਿਜ੍ਞਾ( !p->p_ਸਮੂਹ_By );

  ਜੇ( p->pWhere
   || p->pEList->n_ਵਿਅੰਜਕ!=1
   || p->pSrc->nSrc!=1
   || p->pSrc->a[0].fg.isSubquery
   || pAggInfo->nFunc!=1
   || p->pHaving
  ){
    ਵਾਪਸ 0;
  }
  pTab = p->pSrc->a[0].pSTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !IsView(pTab) );
  ਜੇ( !Is_Ordinary_ਸਾਰਣੀ(pTab) ) ਵਾਪਸ 0;
  p_ਵਿਅੰਜਕ = p->pEList->a[0].p_ਵਿਅੰਜਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_AGG_FUNCTION ) ਵਾਪਸ 0;
  ਜੇ( p_ਵਿਅੰਜਕ->pAggInfo!=pAggInfo ) ਵਾਪਸ 0;
  ਜੇ( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_ਗਿਣਤੀ)==0 ) ਵਾਪਸ 0;
  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo->aFunc[0].pFExpr==p_ਵਿਅੰਜਕ );
  testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Distinct) );
  testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) );
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Distinct|EP_WinFunc) ) ਵਾਪਸ 0;

  ਵਾਪਸ pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
ਪੂਰਨ sqlite3_ਸੂਚਕਾਂਕਿਤ_By_Lookup(Parse *pParse, Src_ਵਸਤੂ *pFrom){
  ਸਾਰਣੀ *pTab = pFrom->pSTab;
  ਅੱਖਰ *z_ਸੂਚਕਾਂਕਿਤ_By = pFrom->u1.z_ਸੂਚਕਾਂਕਿਤ_By;
  ਸੂਚਕਾਂਕ *pIdx;
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.is_ਸੂਚਕਾਂਕਿਤ_By!=0 );

  ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ;
      pIdx && sqlite3StrICmp(pIdx->zName, z_ਸੂਚਕਾਂਕਿਤ_By);
      pIdx=pIdx->p_ਅਗਲਾ
  );
  ਜੇ( !pIdx ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such index: %s", z_ਸੂਚਕਾਂਕਿਤ_By, 0);
    pParse->checkSchema = 1;
    ਵਾਪਸ SQLITE_ERROR;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.isCte==0 );
  pFrom->u2.pIBIndex = pIdx;
  ਵਾਪਸ SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ convert_Compound_Select_ਮੰਜ਼ਿਲ_Subquery(Walker *pWalker, Select *p){
  ਪੂਰਨ i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  ਜੇ( p->p_ਪੂਰਵ==0 ) ਵਾਪਸ WRC_Continue;
  ਜੇ( p->p_ਕ੍ਰਮ_By==0 ) ਵਾਪਸ WRC_Continue;
  ਲਈ(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->p_ਪੂਰਵ){}
  ਜੇ( pX==0 ) ਵਾਪਸ WRC_Continue;
  a = p->p_ਕ੍ਰਮ_By->a;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  ਜੇ( a[0].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ ) ਵਾਪਸ WRC_Continue;
#ਜੇ_ਅੰਤ
  ਲਈ(i=p->p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ-1; i>=0; i--){
    ਜੇ( a[i].p_ਵਿਅੰਜਕ->flags & EP_Collate ) ਰੋਕੋ;
  }
  ਜੇ( i<0 ) ਵਾਪਸ WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, ਆਕਾਰ(*pNew) );
  ਜੇ( pNew==0 ) ਵਾਪਸ WRC_Abort;
  memset(&dummy, 0, ਆਕਾਰ(dummy));
  pNewSrc = sqlite3_Src_List_Append_From_ਪਦ(pParse,0,0,0,&dummy,pNew,0);
  ਪ੍ਰਤਿਜ੍ਞਾ( pNewSrc!=0 || pParse->nErr );
  ਜੇ( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    ਵਾਪਸ WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0, sqlite3_ਵਿਅੰਜਕ(db, TK_ASTERISK, 0));
  p->op = TK_SELECT;
  p->pWhere = 0;
  p->p_ਸਮੂਹ_By = 0;
  p->pHaving = 0;
  pNew->p_ਕ੍ਰਮ_By = 0;
  p->p_ਪੂਰਵ = 0;
  p->p_ਅਗਲਾ = 0;
  p->pWith = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  p->pWinDefn = 0;
#ਜੇ_ਅੰਤ
  p->selFlags &= ~(u32)(SF_Compound|SF_Distinct);
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNew->p_ਪੂਰਵ!=0 );
  pNew->p_ਪੂਰਵ->p_ਅਗਲਾ = pNew;
  pNew->p_ਹੱਦਬੰਦੀ = 0;
  ਵਾਪਸ WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
ਸਥਿਰ ਪੂਰਨ cannotBeFunction(Parse *pParse, Src_ਵਸਤੂ *pFrom){
  ਜੇ( pFrom->fg.isTabFunc ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "'%s' is not a function", pFrom->zName);
    ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
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
ਸਥਿਰ ਬਣਤਰ Cte *ਖੋਜ_With(
  With *pWith,                    /* Current innermost WITH clause */
  Src_ਵਸਤੂ *p_ਵਸਤੂ,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  ਅਚਲ ਅੱਖਰ *zName = p_ਵਸਤੂ->zName;
  With *p;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.fixedSchema || p_ਵਸਤੂ->u4.zDatabase==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( zName!=0 );
  ਲਈ(p=pWith; p; p=p->pOuter){
    ਪੂਰਨ i;
    ਲਈ(i=0; i<p->nCte; i++){
      ਜੇ( sqlite3StrICmp(zName, p->a[i].zName)==0 ){
        *ppContext = p;
        ਵਾਪਸ &p->a[i];
      }
    }
    ਜੇ( p->bView ) ਰੋਕੋ;
  }
  ਵਾਪਸ 0;
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
With *sqlite3_With_ਧੱਕੋ(Parse *pParse, With *pWith, u8 bFree){
  ਜੇ( pWith ){
    ਜੇ( bFree ){
      pWith = (With*)sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, sqlite3WithDeleteGeneric,
                      pWith);
      ਜੇ( pWith==0 ) ਵਾਪਸ 0;
    }
    ਜੇ( pParse->nErr==0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pWith!=pWith );
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  ਵਾਪਸ pWith;
}

ਸਥਿਰ ਪੂਰਨ selectExpanderWalk(Walker*, Select*);

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
ਸਥਿਰ ਪੂਰਨ resolve_From_ਪਦ_ਮੰਜ਼ਿਲ_Cte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  Src_ਵਸਤੂ *pFrom                  /* The FROM clause term to check */
){
  Cte *pCte;               /* Matched CTE (or NULL if no match) */
  With *pWith;             /* The matching WITH */

  ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->pSTab==0 );
  ਜੇ( pParse->pWith==0 ){
    /* There are no WITH clauses in the stack.  No match is possible */
    ਵਾਪਸ 0;
  }
  ਜੇ( pParse->nErr ){
    /* Prior errors might have left pParse->pWith in a goofy state, so
    ** go no further. */
    ਵਾਪਸ 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.hadSchema==0 || pFrom->fg.notCte!=0 );
  ਜੇ( pFrom->fg.fixedSchema==0 && pFrom->u4.zDatabase!=0 ){
    /* The FROM term contains a schema qualifier (ex: main.t1) and so
    ** it cannot possibly be a CTE reference. */
    ਵਾਪਸ 0;
  }
  ਜੇ( pFrom->fg.notCte ){
    /* The FROM term is specifically excluded from matching a CTE.
    **   (1)  It is part of a trigger that used to have zDatabase but had
    **        zDatabase removed by sqlite3FixTriggerStep().
    **   (2)  This is the first term in the FROM clause of an UPDATE.
    */
    ਵਾਪਸ 0;
  }
  pCte = ਖੋਜ_With(pParse->pWith, pFrom, &pWith);
  ਜੇ( pCte ){
    sqlite3 *db = pParse->db;
    ਸਾਰਣੀ *pTab;
    ਵਿਅੰਜਕ_List *pEList;
    Select *pSel;
    Select *p_ਖੱਬਾ;                /* Left-most SELECT statement */
    Select *p_Rec_ਪਦ;             /* Left-most recursive term */
    ਪੂਰਨ bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    ਪੂਰਨ iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    ਜੇ( pCte->zCteErr ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, pCte->zCteErr, pCte->zName);
      ਵਾਪਸ 2;
    }
    ਜੇ( cannotBeFunction(pParse, pFrom) ) ਵਾਪਸ 2;

    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->pSTab==0 );
    pTab = sqlite3DbMallocZero(db, ਆਕਾਰ(ਸਾਰਣੀ));
    ਜੇ( pTab==0 ) ਵਾਪਸ 2;
    pCteUse = pCte->pUse;
    ਜੇ( pCteUse==0 ){
      pCte->pUse = pCteUse = sqlite3DbMallocZero(db, ਆਕਾਰ(pCteUse[0]));
      ਜੇ( pCteUse==0
       || sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse,sqlite3DbFree,pCteUse)==0
      ){
        sqlite3DbFree(db, pTab);
        ਵਾਪਸ 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->n_ਪੰਕਤੀ_Log_Est = 200; ਪ੍ਰਤਿਜ੍ਞਾ( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3_Src_ਵਸਤੂ_Attach_Subquery(pParse, pFrom, pCte->pSelect, 1);
    ਜੇ( db->mallocFailed ) ਵਾਪਸ 2;
    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    ਪ੍ਰਤਿਜ੍ਞਾ( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    ਜੇ( pFrom->fg.is_ਸੂਚਕਾਂਕਿਤ_By ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such index: \"%s\"", pFrom->u1.z_ਸੂਚਕਾਂਕਿਤ_By);
      ਵਾਪਸ 2;
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( !pFrom->fg.is_ਸੂਚਕਾਂਕਿਤ_By );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    p_Rec_ਪਦ = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    ਜਦੋਂ( bMayRecursive && p_Rec_ਪਦ->op==pSel->op ){
      ਪੂਰਨ i;
      SrcList *pSrc = p_Rec_ਪਦ->pSrc;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Rec_ਪਦ->p_ਪੂਰਵ!=0 );
      ਲਈ(i=0; i<pSrc->nSrc; i++){
        Src_ਵਸਤੂ *p_ਵਸਤੂ = &pSrc->a[i];
        ਜੇ( p_ਵਸਤੂ->zName!=0
         && !p_ਵਸਤੂ->fg.hadSchema
         && ALWAYS( !p_ਵਸਤੂ->fg.isSubquery )
         && (p_ਵਸਤੂ->fg.fixedSchema || p_ਵਸਤੂ->u4.zDatabase==0)
         && 0==sqlite3StrICmp(p_ਵਸਤੂ->zName, pCte->zName)
        ){
          p_ਵਸਤੂ->pSTab = pTab;
          pTab->nTabRef++;
          p_ਵਸਤੂ->fg.isRecursive = 1;
          ਜੇ( p_Rec_ਪਦ->selFlags & SF_Recursive ){
            sqlite3_Error_ਸੁਨੇਹਾ(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            ਵਾਪਸ 2;
          }
          p_Rec_ਪਦ->selFlags |= SF_Recursive;
          ਜੇ( iRecTab<0 ) iRecTab = pParse->nTab++;
          p_ਵਸਤੂ->iCursor = iRecTab;
        }
      }
      ਜੇ( (p_Rec_ਪਦ->selFlags & SF_Recursive)==0 ) ਰੋਕੋ;
      p_Rec_ਪਦ = p_Rec_ਪਦ->p_ਪੂਰਵ;
    }

    ਜੇ( pParse->nTab >= db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_CTES]
     && db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_CTES]>0
    ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many CTEs");
      ਵਾਪਸ 2;
    }
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
    pParse->nNestSel++;
    ਜੇ( check_Subquery_Nesting_ਡੂੰਘਾਈ(pParse) ) ਵਾਪਸ 2;
#ਜੇ_ਅੰਤ
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    ਜੇ( pSel->selFlags & SF_Recursive ){
      ਪੂਰਨ rc;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Rec_ਪਦ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_Rec_ਪਦ->selFlags & SF_Recursive)==0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Rec_ਪਦ->p_ਅਗਲਾ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( (p_Rec_ਪਦ->p_ਅਗਲਾ->selFlags & SF_Recursive)!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_Rec_ਪਦ->pWith==0 );
      p_Rec_ਪਦ->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, p_Rec_ਪਦ);
      p_Rec_ਪਦ->pWith = 0;
      ਜੇ( rc ){
        pParse->pWith = pSavedWith;
        ਵਾਪਸ 2;
      }
    }ਨਹੀਂਤਾਂ{
      ਜੇ( selectExpanderWalk(pWalker, pSel) ){
        pParse->pWith = pSavedWith;
        ਵਾਪਸ 2;
      }
    }
    pParse->pWith = pWith;
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
    pParse->nNestSel--;
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nNestSel>=0 );
#ਜੇ_ਅੰਤ

    ਲਈ(p_ਖੱਬਾ=pSel; p_ਖੱਬਾ->p_ਪੂਰਵ; p_ਖੱਬਾ=p_ਖੱਬਾ->p_ਪੂਰਵ);
    pEList = p_ਖੱਬਾ->pEList;
    ਜੇ( pCte->p_ਖਾਨੇ_ਗਿਣਤੀ ){
      ਜੇ( pEList && pEList->n_ਵਿਅੰਜਕ!=pCte->p_ਖਾਨੇ_ਗਿਣਤੀ->n_ਵਿਅੰਜਕ ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_ਵਿਅੰਜਕ, pCte->p_ਖਾਨੇ_ਗਿਣਤੀ->n_ਵਿਅੰਜਕ
        );
        pParse->pWith = pSavedWith;
        ਵਾਪਸ 2;
      }
      pEList = pCte->p_ਖਾਨੇ_ਗਿਣਤੀ;
    }

    sqlite3_Columns_From_ਵਿਅੰਜਕ_List(pParse, pEList, &pTab->n_ਖਾਨਾ, &pTab->a_ਖਾਨਾ);
    ਜੇ( bMayRecursive ){
      ਜੇ( pSel->selFlags & SF_Recursive ){
        pCte->zCteErr = "multiple recursive references: %s";
      }ਨਹੀਂਤਾਂ{
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    ਵਾਪਸ 1;  /* Success */
  }
  ਵਾਪਸ 0;  /* No match */
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
/*
** If the SELECT passed as the second argument has an associated WITH
** clause, pop it from the stack stored as part of the Parse object.
**
** This function is used as the xSelectCallback2() callback by
** sqlite3SelectExpand() when walking a SELECT tree to resolve table
** names and other FROM clause elements.
*/
ਖਾਲੀ sqlite3_Select_ਕੱਢੋ_With(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ਜੇ( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->p_ਪੂਰਵ==0 ){
    With *pWith = ਲੱਭੋ_Rightmost(p)->pWith;
    ਜੇ( pWith!=0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( pParse->pWith==pWith || pParse->nErr );
      pParse->pWith = pWith->pOuter;
    }
  }
}
#ਜੇ_ਅੰਤ

/*
** The SrcItem structure passed as the second argument represents a
** sub-query in the FROM clause of a SELECT statement. This function
** allocates and populates the SrcItem.pTab object. If successful,
** SQLITE_OK is returned. Otherwise, if an OOM error is encountered,
** SQLITE_NOMEM.
*/
ਪੂਰਨ sqlite3ExpandSubquery(Parse *pParse, Src_ਵਸਤੂ *pFrom){
  Select *pSel;
  ਸਾਰਣੀ *pTab;

  ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.isSubquery );
  ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->u4.pSubq!=0 );
  pSel = pFrom->u4.pSubq->pSelect;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSel );
  pFrom->pSTab = pTab = sqlite3DbMallocZero(pParse->db, ਆਕਾਰ(ਸਾਰਣੀ));
  ਜੇ( pTab==0 ) ਵਾਪਸ SQLITE_NOMEM;
  pTab->nTabRef = 1;
  ਜੇ( pFrom->zAlias ){
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  }ਨਹੀਂਤਾਂ{
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  ਜਦੋਂ( pSel->p_ਪੂਰਵ ){ pSel = pSel->p_ਪੂਰਵ; }
  sqlite3_Columns_From_ਵਿਅੰਜਕ_List(pParse, pSel->pEList,&pTab->n_ਖਾਨਾ,&pTab->a_ਖਾਨਾ);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->n_ਪੰਕਤੀ_Log_Est = 200; ਪ੍ਰਤਿਜ੍ਞਾ( 200==sqlite3LogEst(1048576) );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#ਨਹੀਂਤਾਂ
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
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
ਸਥਿਰ ਪੂਰਨ ਵਿੱਚ_Any_Using_Clause(
  ਅਚਲ ਅੱਖਰ *zName, /* Name we are looking for */
  Src_ਵਸਤੂ *p_ਅਧਾਰ,    /* The base SrcItem.  Looking at pBase[1] and following */
  ਪੂਰਨ N              /* How many SrcItems to check */
){
  ਜਦੋਂ( N>0 ){
    N--;
    p_ਅਧਾਰ++;
    ਜੇ( p_ਅਧਾਰ->fg.isUsing==0 ) ਜਾਰੀ;
    ਜੇ( NEVER(p_ਅਧਾਰ->u3.pUsing==0) ) ਜਾਰੀ;
    ਜੇ( sqlite3_Id_List_ਸੂਚਕਾਂਕ(p_ਅਧਾਰ->u3.pUsing, zName)>=0 ) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ selectExpander(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ਪੂਰਨ i, j, k, rc;
  SrcList *pTabList;
  ਵਿਅੰਜਕ_List *pEList;
  Src_ਵਸਤੂ *pFrom;
  sqlite3 *db = pParse->db;
  ਵਿਅੰਜਕ *pE, *p_ਸੱਜਾ, *p_ਵਿਅੰਜਕ;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= SF_Expanded;
  ਜੇ( db->mallocFailed  ){
    ਵਾਪਸ WRC_Abort;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pSrc!=0 );
  ਜੇ( (selFlags & SF_Expanded)!=0 ){
    ਵਾਪਸ WRC_Prune;
  }
  ਜੇ( pWalker->e_ਕੋਡ ){
    /* Renumber selId because it has been copied from a view */
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  ਜੇ( pParse->pWith && (p->selFlags & SF_View) ){
    ਜੇ( p->pWith==0 ){
      p->pWith = (With*)sqlite3DbMallocZero(db, SZ_WITH(1) );
      ਜੇ( p->pWith==0 ){
        ਵਾਪਸ WRC_Abort;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3_With_ਧੱਕੋ(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3_Src_List_ਸੌਂਪੋ_Cursors(pParse, pTabList);

  /* Look up every table named in the FROM clause of the select.  If
  ** an entry of the FROM clause is a subquery instead of a table or view,
  ** then create a transient table structure to describe the subquery.
  */
  ਲਈ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    ਸਾਰਣੀ *pTab;
    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.isRecursive==0 || pFrom->pSTab!=0 );
    ਜੇ( pFrom->pSTab ) ਜਾਰੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.isRecursive==0 );
    ਜੇ( pFrom->zName==0 ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
      Select *pSel;
      ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      ਪ੍ਰਤਿਜ੍ਞਾ( pSel!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->pSTab==0 );
      ਜੇ( sqlite3WalkSelect(pWalker, pSel) ) ਵਾਪਸ WRC_Abort;
      ਜੇ( sqlite3ExpandSubquery(pParse, pFrom) ) ਵਾਪਸ WRC_Abort;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
    }ਨਹੀਂਤਾਂ ਜੇ( (rc = resolve_From_ਪਦ_ਮੰਜ਼ਿਲ_Cte(pParse, pWalker, pFrom))!=0 ){
      ਜੇ( rc>1 ) ਵਾਪਸ WRC_Abort;
      pTab = pFrom->pSTab;
      ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
#ਜੇ_ਅੰਤ
    }ਨਹੀਂਤਾਂ{
      /* An ordinary table or view name in the FROM clause */
      ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3_Locate_ਸਾਰਣੀ_ਵਸਤੂ(pParse, 0, pFrom);
      ਜੇ( pTab==0 ) ਵਾਪਸ WRC_Abort;
      ਜੇ( pTab->nTabRef>=0xffff ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many references to \"%s\": max 65535",
           pTab->zName);
        pFrom->pSTab = 0;
        ਵਾਪਸ WRC_Abort;
      }
      pTab->nTabRef++;
      ਜੇ( !IsVirtual(pTab) && cannotBeFunction(pParse, pFrom) ){
        ਵਾਪਸ WRC_Abort;
      }
#ਜੇ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
      ਜੇ( !Is_Ordinary_ਸਾਰਣੀ(pTab) ){
        i16 n_ਖਾਨਾ;
        u8 e_ਕੋਡ_Orig = pWalker->e_ਕੋਡ;
        ਜੇ( sqlite3_View_Get_ਕਾਲਮ_ਨਾਂ(pParse, pTab) ) ਵਾਪਸ WRC_Abort;
        ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.isSubquery==0 );
        ਜੇ( IsView(pTab) ){
          ਜੇ( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_ਸੁਨੇਹਾ(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3_Src_ਵਸਤੂ_Attach_Subquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_VIRTUALTABLE
        ਨਹੀਂਤਾਂ ਜੇ( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_VTABRISK_ਸਾਧਾਰਨ==1 && SQLITE_VTABRISK_ਉੱਚਾ==2 );
#ਜੇ_ਅੰਤ
        n_ਖਾਨਾ = pTab->n_ਖਾਨਾ;
        pTab->n_ਖਾਨਾ = -1;
        pWalker->e_ਕੋਡ = 1;  /* Turn on Select.selId renumbering */
        ਜੇ( pFrom->fg.isSubquery ){
          bft hasCompound = pParse->hasCompound;
          pParse->hasCompound = 1;
          selectExpanderWalk(pWalker, pFrom->u4.pSubq->pSelect);
          pParse->hasCompound = hasCompound;
        }
        pWalker->e_ਕੋਡ = e_ਕੋਡ_Orig;
        pTab->n_ਖਾਨਾ = n_ਖਾਨਾ;
      }
#ਜੇ_ਅੰਤ
    }

    /* Locate the index named by the INDEXED BY clause, if any. */
    ਜੇ( pFrom->fg.is_ਸੂਚਕਾਂਕਿਤ_By && sqlite3_ਸੂਚਕਾਂਕਿਤ_By_Lookup(pParse, pFrom) ){
      ਵਾਪਸ WRC_Abort;
    }
  }

  /* Process NATURAL keywords, and ON and USING clauses of joins.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 || pParse->nErr!=0 );
  ਜੇ( pParse->nErr || sqlite3ProcessJoin(pParse, p) ){
    ਵਾਪਸ WRC_Abort;
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
  ਲਈ(k=0; k<pEList->n_ਵਿਅੰਜਕ; k++){
    pE = pEList->a[k].p_ਵਿਅੰਜਕ;
    ਜੇ( pE->op==TK_ASTERISK ) ਰੋਕੋ;
    ਪ੍ਰਤਿਜ੍ਞਾ( pE->op!=TK_DOT || pE->p_ਸੱਜਾ!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pE->op!=TK_DOT || (pE->p_ਖੱਬਾ!=0 && pE->p_ਖੱਬਾ->op==TK_ID) );
    ਜੇ( pE->op==TK_DOT && pE->p_ਸੱਜਾ->op==TK_ASTERISK ) ਰੋਕੋ;
    elistFlags |= pE->flags;
  }
  ਜੇ( k<pEList->n_ਵਿਅੰਜਕ ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *a = pEList->a;
    ਵਿਅੰਜਕ_List *pNew = 0;
    ਪੂਰਨ flags = pParse->db->flags;
    ਪੂਰਨ long_ਨਾਂ = (flags & SQLITE_ਭਰਿਆ_ਖਾਨਾ_ਨਾਂ)!=0
                      && (flags & SQLITE_Short_ਖਾਨਾ_ਨਾਂ)==0;

    ਲਈ(k=0; k<pEList->n_ਵਿਅੰਜਕ; k++){
      pE = a[k].p_ਵਿਅੰਜਕ;
      elistFlags |= pE->flags;
      p_ਸੱਜਾ = pE->p_ਸੱਜਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pE->op!=TK_DOT || p_ਸੱਜਾ!=0 );
      ਜੇ( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_ਸੱਜਾ->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pNew, a[k].p_ਵਿਅੰਜਕ);
        ਜੇ( pNew ){
          pNew->a[pNew->n_ਵਿਅੰਜਕ-1].zEName = a[k].zEName;
          pNew->a[pNew->n_ਵਿਅੰਜਕ-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_ਵਿਅੰਜਕ = 0;
      }ਨਹੀਂਤਾਂ{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        ਪੂਰਨ ਸਾਰਣੀ_ਵੇਖਿਆ = 0;      /* Set to 1 when TABLE matches */
        ਅੱਖਰ *zTName = 0;       /* text of name of TABLE */
        ਪੂਰਨ iErrOfst;
        ਜੇ( pE->op==TK_DOT ){
          ਪ੍ਰਤਿਜ੍ਞਾ( (selFlags & SF_NestedFrom)==0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( pE->p_ਖੱਬਾ!=0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pE->p_ਖੱਬਾ, EP_Int_ਮੁੱਲ) );
          zTName = pE->p_ਖੱਬਾ->u.zToken;
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_WOfst(pE->p_ਖੱਬਾ) );
          iErrOfst = pE->p_ਸੱਜਾ->w.iOfst;
        }ਨਹੀਂਤਾਂ{
          ਪ੍ਰਤਿਜ੍ਞਾ( pE->p_ਖੱਬਾ==0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        ਲਈ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          ਪੂਰਨ n_ਜੋੜੋ;                    /* Number of cols including rowid */
          ਸਾਰਣੀ *pTab = pFrom->pSTab;  /* Table for this data source */
          ਵਿਅੰਜਕ_List *pNestedFrom;       /* Result-set of a nested FROM clause */
          ਅੱਖਰ *zTabName;              /* AS name for this data source */
          ਅਚਲ ਅੱਖਰ *zSchemaName = 0; /* Schema name for this data source */
          ਪੂਰਨ iDb;                     /* Schema index for this data src */
          IdList *pUsing;              /* USING clause for pFrom[1] */

          ਜੇ( (zTabName = pFrom->zAlias)==0 ){
            zTabName = pTab->zName;
          }
          ਜੇ( db->mallocFailed ) ਰੋਕੋ;
          ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੂਰਨ)pFrom->fg.isNestedFrom == IsNestedFrom(pFrom) );
          ਜੇ( pFrom->fg.isNestedFrom ){
            ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->fg.isSubquery && pFrom->u4.pSubq );
            ਪ੍ਰਤਿਜ੍ਞਾ( pFrom->u4.pSubq->pSelect!=0 );
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;
            ਪ੍ਰਤਿਜ੍ਞਾ( pNestedFrom!=0 );
            ਪ੍ਰਤਿਜ੍ਞਾ( pNestedFrom->n_ਵਿਅੰਜਕ==pTab->n_ਖਾਨਾ );
            ਪ੍ਰਤਿਜ੍ਞਾ( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }ਨਹੀਂਤਾਂ{
            ਜੇ( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              ਜਾਰੀ;
            }
            pNestedFrom = 0;
            iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(db, pTab->pSchema);
            zSchemaName = iDb>=0 ? db->aDb[iDb].zDbSName : "*";
          }
          ਜੇ( i+1<pTabList->nSrc
           && pFrom[1].fg.isUsing
           && (selFlags & SF_NestedFrom)!=0
          ){
            ਪੂਰਨ ii;
            pUsing = pFrom[1].u3.pUsing;
            ਲਈ(ii=0; ii<pUsing->nId; ii++){
              ਅਚਲ ਅੱਖਰ *zUName = pUsing->a[ii].zName;
              p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ(db, TK_ID, zUName);
              sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_Error_ਵਿੱਥ(p_ਸੱਜਾ, iErrOfst);
              pNew = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pNew, p_ਸੱਜਾ);
              ਜੇ( pNew ){
                ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *pX = &pNew->a[pNew->n_ਵਿਅੰਜਕ-1];
                ਪ੍ਰਤਿਜ੍ਞਾ( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.b_Using_ਪਦ = 1;
              }
            }
          }ਨਹੀਂਤਾਂ{
            pUsing = 0;
          }

          n_ਜੋੜੋ = pTab->n_ਖਾਨਾ;
          ਜੇ( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) n_ਜੋੜੋ++;
          ਲਈ(j=0; j<n_ਜੋੜੋ; j++){
            ਅਚਲ ਅੱਖਰ *zName; 
            ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *pX; /* Newly added ExprList term */

            ਜੇ( j==pTab->n_ਖਾਨਾ ){
              zName = sqlite3RowidAlias(pTab);
              ਜੇ( zName==0 ) ਜਾਰੀ;
            }ਨਹੀਂਤਾਂ{
              zName = pTab->a_ਖਾਨਾ[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              ਜੇ( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                ਜਾਰੀ;
              }

              ਜੇ( zTName
               && pNestedFrom
               && sqlite3_ਮੇਲ_EName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                ਜਾਰੀ;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              ਜੇ( (p->selFlags & SF_IncludeHidden)==0
                && Is_Hidden_ਕਾਲਮ(&pTab->a_ਖਾਨਾ[j])
              ){
                ਜਾਰੀ;
              }
              ਜੇ( (pTab->a_ਖਾਨਾ[j].ਖਾਨਾ_Flags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                ਜਾਰੀ;
              }
            }
            ਪ੍ਰਤਿਜ੍ਞਾ( zName );
            ਸਾਰਣੀ_ਵੇਖਿਆ = 1;

            ਜੇ( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              ਜੇ( pFrom->fg.isUsing
               && sqlite3_Id_List_ਸੂਚਕਾਂਕ(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                ਜਾਰੀ;
              }
            }
            p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ(db, TK_ID, zName);
            ਜੇ( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !ਵਿੱਚ_Any_Using_Clause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || ਵਿੱਚ_RENAME_OBJECT
            ){
              ਵਿਅੰਜਕ *p_ਖੱਬਾ;
              p_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ(db, TK_ID, zTabName);
              p_ਵਿਅੰਜਕ = sqlite3PExpr(pParse, TK_DOT, p_ਖੱਬਾ, p_ਸੱਜਾ);
              ਜੇ( ਵਿੱਚ_RENAME_OBJECT && pE->p_ਖੱਬਾ ){
                ਪ੍ਰਤਿਜ੍ਞਾ( pE->op==TK_DOT && pE->p_ਸੱਜਾ->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_ਖੱਬਾ, pE->p_ਖੱਬਾ);
              }
              ਜੇ( zSchemaName ){
                p_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ(db, TK_ID, zSchemaName);
                p_ਵਿਅੰਜਕ = sqlite3PExpr(pParse, TK_DOT, p_ਖੱਬਾ, p_ਵਿਅੰਜਕ);
              }
            }ਨਹੀਂਤਾਂ{
              p_ਵਿਅੰਜਕ = p_ਸੱਜਾ;
            }
            sqlite3_ਵਿਅੰਜਕ_ਸੈੱਟ_Error_ਵਿੱਥ(p_ਵਿਅੰਜਕ, iErrOfst);
            pNew = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pNew, p_ਵਿਅੰਜਕ);
            ਜੇ( pNew==0 ){
              ਰੋਕੋ;  /* OOM */
            }
            pX = &pNew->a[pNew->n_ਵਿਅੰਜਕ-1];
            ਪ੍ਰਤਿਜ੍ਞਾ( pX->zEName==0 );
            ਜੇ( (selFlags & SF_NestedFrom)!=0 && !ਵਿੱਚ_RENAME_OBJECT ){
              ਜੇ( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_ਵਿਅੰਜਕ) ){
                ਪ੍ਰਤਿਜ੍ਞਾ( j<pNestedFrom->n_ਵਿਅੰਜਕ );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }ਨਹੀਂਤਾਂ{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->n_ਖਾਨਾ ? ENAME_ROWID : ENAME_TAB);
              ਜੇ( (pFrom->fg.isUsing
                   && sqlite3_Id_List_ਸੂਚਕਾਂਕ(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3_Id_List_ਸੂਚਕਾਂਕ(pUsing, zName)>=0)
               || (j<pTab->n_ਖਾਨਾ && (pTab->a_ਖਾਨਾ[j].ਖਾਨਾ_Flags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }ਨਹੀਂਤਾਂ ਜੇ( long_ਨਾਂ ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }ਨਹੀਂਤਾਂ{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        ਜੇ( !ਸਾਰਣੀ_ਵੇਖਿਆ ){
          ਜੇ( zTName ){
            sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such table: %s", zTName);
          }ਨਹੀਂਤਾਂ{
            sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  ਜੇ( p->pEList ){
    ਜੇ( p->pEList->n_ਵਿਅੰਜਕ>db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਲਮ] ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many columns in result set");
      ਵਾਪਸ WRC_Abort;
    }
    ਜੇ( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_Complex_ਨਤੀਜਾ;
    }
  }
#ਜੇ TREETRACE_ENABLED
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ WRC_Continue;
}

#ਜੇ SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
ਖਾਲੀ sqlite3SelectWalkAssert2(Walker *Not_ਵਰਤਿਆ, Select *NotUsed2){
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  ਪ੍ਰਤਿਜ੍ਞਾ( 0 );
}
#ਜੇ_ਅੰਤ

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
ਸਥਿਰ ਪੂਰਨ selectExpanderWalk(Walker *pWalker, Select *pSelect){
  ਪ੍ਰਤਿਜ੍ਞਾ( pWalker->x_ਵਿਅੰਜਕ_Callback==sqlite3_ਵਿਅੰਜਕ_Walk_Noop );
  ਪ੍ਰਤਿਜ੍ਞਾ( pWalker->pParse!=0 );
  ਜੇ( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convert_Compound_Select_ਮੰਜ਼ਿਲ_Subquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_ਕੱਢੋ_With;
  ਵਾਪਸ sqlite3WalkSelect(pWalker, pSelect);
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
ਸਥਿਰ ਖਾਲੀ sqlite3SelectExpand(Parse *pParse, Select *pSelect){
  Walker w;
  w.x_ਵਿਅੰਜਕ_Callback = sqlite3_ਵਿਅੰਜਕ_Walk_Noop;
  w.pParse = pParse;
  w.e_ਕੋਡ = 0;
  selectExpanderWalk(&w, pSelect);
}


#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
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
ਸਥਿਰ ਖਾਲੀ select_ਜੋੜੋ_Subquery_Type_Info(Walker *pWalker, Select *p){
  Parse *pParse;
  ਪੂਰਨ i;
  SrcList *pTabList;
  Src_ਵਸਤੂ *pFrom;

  ਜੇ( p->selFlags & SF_HasTypeInfo ) ਵਾਪਸ;
  p->selFlags |= SF_HasTypeInfo;
  pParse = pWalker->pParse;
  ਪ੍ਰਤਿਜ੍ਞਾ( (p->selFlags & SF_Resolved) );
  pTabList = p->pSrc;
  ਲਈ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    ਸਾਰਣੀ *pTab = pFrom->pSTab;
    ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
    ਜੇ( (pTab->tabFlags & TF_Ephemeral)!=0 && pFrom->fg.isSubquery ){
      /* A sub-query in the FROM clause of a SELECT */
      Select *pSel = pFrom->u4.pSubq->pSelect;
      sqlite3_Subquery_ਕਾਲਮ_Types(pParse, pTab, pSel, SQLITE_AFF_NONE);
    }
  }
}
#ਜੇ_ਅੰਤ


/*
** This routine adds datatype and collating sequence information to
** the Table structures of all FROM-clause subqueries in a
** SELECT statement.
**
** Use this routine after name resolution.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3_Select_ਜੋੜੋ_Type_Info(Parse *pParse, Select *pSelect){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = select_ਜੋੜੋ_Subquery_Type_Info;
  w.x_ਵਿਅੰਜਕ_Callback = sqlite3_ਵਿਅੰਜਕ_Walk_Noop;
  w.pParse = pParse;
  sqlite3WalkSelect(&w, pSelect);
#ਜੇ_ਅੰਤ
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
ਖਾਲੀ sqlite3SelectPrep(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for container */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 || pParse->db->mallocFailed );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->pParse==pParse );
  ਜੇ( pParse->db->mallocFailed ) ਵਾਪਸ;
  ਜੇ( p->selFlags & SF_HasTypeInfo ) ਵਾਪਸ;
  sqlite3SelectExpand(pParse, p);
  ਜੇ( pParse->nErr ) ਵਾਪਸ;
  sqlite3_Resolve_Select_ਨਾਂ(pParse, p, pOuterNC);
  ਜੇ( pParse->nErr ) ਵਾਪਸ;
  sqlite3_Select_ਜੋੜੋ_Type_Info(pParse, p);
}

#ਜੇ TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
ਸਥਿਰ ਖਾਲੀ printAggInfo(AggInfo *pAggInfo){
  ਪੂਰਨ ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  ਲਈ(ii=0; ii<pAggInfo->n_ਕਾਲਮ; ii++){
    ਬਣਤਰ Agg_Info_ਖਾਨਾ *p_ਖਾਨਾ = &pAggInfo->a_ਖਾਨਾ[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, p_ਖਾਨਾ->pTab ? p_ਖਾਨਾ->pTab->zName : "NULL",
       p_ਖਾਨਾ->i_ਸਾਰਣੀ, p_ਖਾਨਾ->i_ਕਾਲਮ, pAggInfo->i_ਪਹਿਲਾ_Reg+ii,
       p_ਖਾਨਾ->i_Sorter_ਕਾਲਮ,
       ii>=pAggInfo->n_ਸੰਚਾਇਕ ? "" : " Accumulator");
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(0, pAggInfo->a_ਖਾਨਾ[ii].pCExpr, 0);
  }
  ਲਈ(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->i_ਪਹਿਲਾ_Reg+pAggInfo->n_ਕਾਲਮ+ii);
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(0, pAggInfo->aFunc[ii].pFExpr, 0);
  }
}
#ਜੇ_ਅੰਤ /* TREETRACE_ENABLED */

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
ਸਥਿਰ ਖਾਲੀ analyze_Agg_Func_ਦਲੀਲਾਂ(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  ਪੂਰਨ i;
  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo->i_ਪਹਿਲਾ_Reg==0 );
  pNC->ncFlags |= NC_ਵਿੱਚ_Agg_Func;
  ਲਈ(i=0; i<pAggInfo->nFunc; i++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pAggInfo->aFunc[i].pFExpr;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION || p_ਵਿਅੰਜਕ->op==TK_AGG_FUNCTION );
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
    sqlite3_ਵਿਅੰਜਕ_Analyze_Agg_List(pNC, p_ਵਿਅੰਜਕ->x.pList);
    ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->op==TK_ਕ੍ਰਮ );
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) );
      sqlite3_ਵਿਅੰਜਕ_Analyze_Agg_List(pNC, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->x.pList);
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਪ੍ਰਤਿਜ੍ਞਾ( !Is_ਖਿੜਕੀ_Func(p_ਵਿਅੰਜਕ) );
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
      sqlite3_ਵਿਅੰਜਕ_Analyze_Aggregates(pNC, p_ਵਿਅੰਜਕ->y.pWin->pFilter);
    }
#ਜੇ_ਅੰਤ
  }
  pNC->ncFlags &= ~NC_ਵਿੱਚ_Agg_Func;
}

/*
** An index on expressions is being used in the inner loop of an
** aggregate query with a GROUP BY clause.  This routine attempts
** to adjust the AggInfo object to take advantage of index and to
** perhaps use the index as a covering index.
**
*/
ਸਥਿਰ ਖਾਲੀ optimize_Aggregate_Use_ਦਾ_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo->i_ਪਹਿਲਾ_Reg==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pSelect!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pSelect->p_ਸਮੂਹ_By!=0 );
  pAggInfo->n_ਕਾਲਮ = pAggInfo->n_ਸੰਚਾਇਕ;
  ਜੇ( ALWAYS(pAggInfo->n_ਛਾਂਟੀ_ਕਾਲਮ>0) ){
    ਪੂਰਨ mx = pSelect->p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ - 1;
    ਪੂਰਨ j, k;
    ਲਈ(j=0; j<pAggInfo->n_ਕਾਲਮ; j++){
      k = pAggInfo->a_ਖਾਨਾ[j].i_Sorter_ਕਾਲਮ;
      ਜੇ( k>mx ) mx = k;
    }
    pAggInfo->n_ਛਾਂਟੀ_ਕਾਲਮ = mx+1;
  }
  analyze_Agg_Func_ਦਲੀਲਾਂ(pAggInfo, pNC);
#ਜੇ TREETRACE_ENABLED
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x20 ){
    ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_ਰੁੱਖ_View_Select(0, pSelect, 0);
    ਲਈ(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      ਲਿਖੋ("data-cursor=%d index={%d,%d}\n",
          pIEpr->i_ਅੰਕੜੇ_Cur, pIEpr->iIdxCur, pIEpr->i_Idx_ਖਾਨਾ);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(0, pIEpr->p_ਵਿਅੰਜਕ, 0);
    }
    printAggInfo(pAggInfo);
  }
#ਨਹੀਂਤਾਂ
  ਅਣਵਰਤਿਆ_PARAMETER(pSelect);
  ਅਣਵਰਤਿਆ_PARAMETER(pParse);
#ਜੇ_ਅੰਤ
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
ਸਥਿਰ ਪੂਰਨ aggregate_Idx_Epr_Ref_ਮੰਜ਼ਿਲ_ਖਾਨਾ_Callback(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  AggInfo *pAggInfo;
  ਬਣਤਰ Agg_Info_ਖਾਨਾ *p_ਖਾਨਾ;
  ਅਣਵਰਤਿਆ_PARAMETER(pWalker);
  ਜੇ( p_ਵਿਅੰਜਕ->pAggInfo==0 ) ਵਾਪਸ WRC_Continue;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_AGG_ਕਾਲਮ ) ਵਾਪਸ WRC_Continue;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_AGG_FUNCTION ) ਵਾਪਸ WRC_Continue;
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_IF_NULL_ਪੰਕਤੀ ) ਵਾਪਸ WRC_Continue;
  pAggInfo = p_ਵਿਅੰਜਕ->pAggInfo;
  ਜੇ( NEVER(p_ਵਿਅੰਜਕ->iAgg>=pAggInfo->n_ਕਾਲਮ) ) ਵਾਪਸ WRC_Continue;
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->iAgg>=0 );
  p_ਖਾਨਾ = &pAggInfo->a_ਖਾਨਾ[p_ਵਿਅੰਜਕ->iAgg];
  p_ਵਿਅੰਜਕ->op = TK_AGG_ਕਾਲਮ;
  p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = p_ਖਾਨਾ->i_ਸਾਰਣੀ;
  p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = p_ਖਾਨਾ->i_ਕਾਲਮ;
  ਵਿਅੰਜਕ_Clear_Property(p_ਵਿਅੰਜਕ, EP_Skip|EP_Collate|EP_Unlikely);
  ਵਾਪਸ WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
ਸਥਿਰ ਖਾਲੀ aggregate_Convert_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ_Ref_ਮੰਜ਼ਿਲ_ਕਾਲਮ(AggInfo *pAggInfo){
  ਪੂਰਨ i;
  Walker w;
  memset(&w, 0, ਆਕਾਰ(w));
  w.x_ਵਿਅੰਜਕ_Callback = aggregate_Idx_Epr_Ref_ਮੰਜ਼ਿਲ_ਖਾਨਾ_Callback;
  ਲਈ(i=0; i<pAggInfo->nFunc; i++){
    sqlite3_Walk_ਵਿਅੰਜਕ(&w, pAggInfo->aFunc[i].pFExpr);
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
ਸਥਿਰ ਖਾਲੀ ਸੌਂਪੋ_Aggregate_Registers(Parse *pParse, AggInfo *pAggInfo){
  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo->i_ਪਹਿਲਾ_Reg==0 );
  pAggInfo->i_ਪਹਿਲਾ_Reg = pParse->nMem + 1;
  pParse->nMem += pAggInfo->n_ਕਾਲਮ + pAggInfo->nFunc;
}

/*
** Reset the aggregate accumulator.
**
** The aggregate accumulator is a set of memory cells that hold
** intermediate results while calculating an aggregate.  This
** routine generates code that stores NULLs in all of those memory
** cells.
*/
ਸਥਿਰ ਖਾਲੀ reset_ਸੰਚਾਇਕ(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ i;
  ਬਣਤਰ AggInfo_func *pFunc;
  ਪੂਰਨ nReg = pAggInfo->nFunc + pAggInfo->n_ਕਾਲਮ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo->i_ਪਹਿਲਾ_Reg>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->pParse==pParse );
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ਜੇ( nReg==0 ) ਵਾਪਸ;
  ਜੇ( pParse->nErr ) ਵਾਪਸ;
  sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 0, pAggInfo->i_ਪਹਿਲਾ_Reg,
                    pAggInfo->i_ਪਹਿਲਾ_Reg+nReg-1);
  ਲਈ(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    ਜੇ( pFunc->iDistinct>=0 ){
      ਵਿਅੰਜਕ *pE = pFunc->pFExpr;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pE) );
      ਜੇ( pE->x.pList==0 || pE->x.pList->n_ਵਿਅੰਜਕ!=1 ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }ਨਹੀਂਤਾਂ{
        ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_From_ਵਿਅੰਜਕ_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (ਅੱਖਰ*)p_ਕੁੰਜੀ_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    ਜੇ( pFunc->iOBTab>=0 ){
      ਵਿਅੰਜਕ_List *pOBList;
      ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;
      ਪੂਰਨ nExtra = 0;
      ਪ੍ਰਤਿਜ੍ਞਾ( pFunc->pFExpr->p_ਖੱਬਾ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pFunc->pFExpr->p_ਖੱਬਾ->op==TK_ਕ੍ਰਮ );
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pFunc->pFExpr->p_ਖੱਬਾ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_ਖੱਬਾ->x.pList;
      ਜੇ( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      ਜੇ( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pFunc->pFExpr) );
        ਪ੍ਰਤਿਜ੍ਞਾ( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_ਵਿਅੰਜਕ;
      }
      ਜੇ( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_ਵਿਅੰਜਕ;
      }
      p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_From_ਵਿਅੰਜਕ_List(pParse, pOBList, 0, nExtra);
      ਜੇ( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_ਕੁੰਜੀ_Info->n_ਕੁੰਜੀ_ਖੇਤਰ++;
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_ਵਿਅੰਜਕ+nExtra, 0,
            (ਅੱਖਰ*)p_ਕੁੰਜੀ_Info, P4_KEYINFO);
      ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(ORDER BY)",
                          pFunc->pFunc->zName));
    }
  }
}

/*
** Invoke the OP_AggFinalize opcode for every aggregate function
** in the AggInfo structure.
*/
ਸਥਿਰ ਖਾਲੀ finalizeAggFunctions(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ i;
  ਬਣਤਰ AggInfo_func *pF;
  ਲਈ(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    ਵਿਅੰਜਕ_List *pList;
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pF->pFExpr) );
    ਜੇ( pParse->nErr ) ਵਾਪਸ;
    pList = pF->pFExpr->x.pList;
    ਜੇ( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      ਪੂਰਨ i_ਸਿਖਰ;        /* Start of loop for extracting columns */
      ਪੂਰਨ nArg;        /* Number of columns to extract */
      ਪੂਰਨ n_ਕੁੰਜੀ;        /* Key columns to be skipped */
      ਪੂਰਨ regAgg;      /* Extract into this array */
      ਪੂਰਨ j;           /* Loop counter */
     
      ਪ੍ਰਤਿਜ੍ਞਾ( pF->pFunc!=0 );
      nArg = pList->n_ਵਿਅੰਜਕ;
      regAgg = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, nArg);

      ਜੇ( pF->bOBPayload==0 ){
        n_ਕੁੰਜੀ = 0;
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( pF->pFExpr->p_ਖੱਬਾ!=0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pF->pFExpr->p_ਖੱਬਾ) );
        ਪ੍ਰਤਿਜ੍ਞਾ( pF->pFExpr->p_ਖੱਬਾ->x.pList!=0 );
        n_ਕੁੰਜੀ = pF->pFExpr->p_ਖੱਬਾ->x.pList->n_ਵਿਅੰਜਕ;
        ਜੇ( ALWAYS(!pF->bOBUnique) ) n_ਕੁੰਜੀ++;
      }
      i_ਸਿਖਰ = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      ਲਈ(j=nArg-1; j>=0; j--){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, pF->iOBTab, n_ਕੁੰਜੀ+j, regAgg+j);
      }
      ਜੇ( pF->bUseSubtype ){
        ਪੂਰਨ regSubtype = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
        ਪੂਰਨ i_ਅਧਾਰ_ਖਾਨਾ = n_ਕੁੰਜੀ + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        ਲਈ(j=nArg-1; j>=0; j--){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, pF->iOBTab, i_ਅਧਾਰ_ਖਾਨਾ+j, regSubtype);
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਸੈੱਟ_Subtype, regSubtype, regAgg+j);
        }
        sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, regSubtype);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Agg_ਕਦਮ, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਅਗਲਾ, pF->iOBTab, i_ਸਿਖਰ+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, i_ਸਿਖਰ);
      sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, regAgg, nArg);
    }
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
                      pList ? pList->n_ਵਿਅੰਜਕ : 0);
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
ਸਥਿਰ ਖਾਲੀ update_ਸੰਚਾਇਕ(
  Parse *pParse,
  ਪੂਰਨ regAcc,
  AggInfo *pAggInfo,
  ਪੂਰਨ eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  ਪੂਰਨ i;
  ਪੂਰਨ reg_ਪ੍ਰਾਪਤੀ = 0;
  ਪੂਰਨ addr_ਪ੍ਰਾਪਤੀ_Test = 0;
  ਬਣਤਰ AggInfo_func *pF;
  ਬਣਤਰ Agg_Info_ਖਾਨਾ *pC;

  ਪ੍ਰਤਿਜ੍ਞਾ( pAggInfo->i_ਪਹਿਲਾ_Reg>0 );
  ਜੇ( pParse->nErr ) ਵਾਪਸ;
  pAggInfo->ਸਿੱਧਾ_ਬਹੁਲਕ = 1;
  ਲਈ(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    ਪੂਰਨ nArg;
    ਪੂਰਨ addr_ਅਗਲਾ = 0;
    ਪੂਰਨ regAgg;
    ਪੂਰਨ regAggSz = 0;
    ਪੂਰਨ regDistinct = 0;
    ਵਿਅੰਜਕ_List *pList;
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pF->pFExpr) );
    ਪ੍ਰਤਿਜ੍ਞਾ( !Is_ਖਿੜਕੀ_Func(pF->pFExpr) );
    ਪ੍ਰਤਿਜ੍ਞਾ( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(pF->pFExpr, EP_WinFunc) ){
      ਵਿਅੰਜਕ *pFilter = pF->pFExpr->y.pWin->pFilter;
      ਜੇ( pAggInfo->n_ਸੰਚਾਇਕ
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        ਜੇ( reg_ਪ੍ਰਾਪਤੀ==0 ) reg_ਪ੍ਰਾਪਤੀ = ++pParse->nMem;
        /* If this is the first row of the group (regAcc contains 0), clear the
        ** "magnet" register regHit so that the accumulator registers
        ** are populated if the FILTER clause jumps over the the
        ** invocation of min() or max() altogether. Or, if this is not
        ** the first row (regAcc contains 1), set the magnet register so that
        ** the accumulators are not populated unless the min()/max() is invoked
        ** and indicates that they should be.  */
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Copy, regAcc, reg_ਪ੍ਰਾਪਤੀ);
      }
      addr_ਅਗਲਾ = sqlite3VdbeMakeLabel(pParse);
      sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pFilter, addr_ਅਗਲਾ, SQLITE_JUMPIFNULL);
    }
    ਜੇ( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      ਪੂਰਨ jj;                /* Registered used so far in building the record */
      ਵਿਅੰਜਕ_List *pOBList;     /* The ORDER BY clause */
      ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );
      nArg = pList->n_ਵਿਅੰਜਕ;
      ਪ੍ਰਤਿਜ੍ਞਾ( nArg>0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pF->pFExpr->p_ਖੱਬਾ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pF->pFExpr->p_ਖੱਬਾ->op==TK_ਕ੍ਰਮ );
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pF->pFExpr->p_ਖੱਬਾ) );
      pOBList = pF->pFExpr->p_ਖੱਬਾ->x.pList;
      ਪ੍ਰਤਿਜ੍ਞਾ( pOBList!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( pOBList->n_ਵਿਅੰਜਕ>0 );
      regAggSz = pOBList->n_ਵਿਅੰਜਕ;
      ਜੇ( !pF->bOBUnique ){
        regAggSz++;   /* One register for OP_Sequence */
      }
      ਜੇ( pF->bOBPayload ){
        regAggSz += nArg;
      }
      ਜੇ( pF->bUseSubtype ){
        regAggSz += nArg;
      }
      regAggSz++;  /* One extra register to hold result of MakeRecord */
      regAgg = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_ਵਿਅੰਜਕ;
      ਜੇ( !pF->bOBUnique ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      ਜੇ( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      ਜੇ( pF->bUseSubtype ){
        ਪੂਰਨ kk;
        ਪੂਰਨ reg_ਅਧਾਰ = pF->bOBPayload ? regDistinct : regAgg;
        ਲਈ(kk=0; kk<nArg; kk++, jj++){
          sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_GetSubtype, reg_ਅਧਾਰ+kk, regAgg+jj);
        }
      }
    }ਨਹੀਂਤਾਂ ਜੇ( pList ){
      nArg = pList->n_ਵਿਅੰਜਕ;
      regAgg = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }ਨਹੀਂਤਾਂ{
      nArg = 0;
      regAgg = 0;
    }
    ਜੇ( pF->iDistinct>=0 && pList ){
      ਜੇ( addr_ਅਗਲਾ==0 ){
        addr_ਅਗਲਾ = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = ਕੋਡ_Distinct(pParse, eDistinctType,
          pF->iDistinct, addr_ਅਗਲਾ, pList, regDistinct);
    }
    ਜੇ( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_Idx_ਪਾਓ, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, regAgg, regAggSz);
    }ਨਹੀਂਤਾਂ{
      /* Invoke the AggStep function */
      ਜੇ( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
        ਪੂਰਨ j;
        ਪ੍ਰਤਿਜ੍ਞਾ( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        ਲਈ(j=0, p_ਵਸਤੂ=pList->a; !pColl && j<nArg; j++, p_ਵਸਤੂ++){
          pColl = sqlite3_ਵਿਅੰਜਕ_Coll_Seq(pParse, p_ਵਸਤੂ->p_ਵਿਅੰਜਕ);
        }
        ਜੇ( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        ਜੇ( reg_ਪ੍ਰਾਪਤੀ==0 && pAggInfo->n_ਸੰਚਾਇਕ ) reg_ਪ੍ਰਾਪਤੀ = ++pParse->nMem;
        sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_CollSeq, reg_ਪ੍ਰਾਪਤੀ, 0, 0,
                         (ਅੱਖਰ *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Agg_ਕਦਮ, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, regAgg, nArg);
    }
    ਜੇ( addr_ਅਗਲਾ ){
      sqlite3VdbeResolveLabel(v, addr_ਅਗਲਾ);
    }
    ਜੇ( pParse->nErr ) ਵਾਪਸ;
  }
  ਜੇ( reg_ਪ੍ਰਾਪਤੀ==0 && pAggInfo->n_ਸੰਚਾਇਕ ){
    reg_ਪ੍ਰਾਪਤੀ = regAcc;
  }
  ਜੇ( reg_ਪ੍ਰਾਪਤੀ ){
    addr_ਪ੍ਰਾਪਤੀ_Test = sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_If, reg_ਪ੍ਰਾਪਤੀ); VdbeCoverage(v);
  }
  ਲਈ(i=0, pC=pAggInfo->a_ਖਾਨਾ; i<pAggInfo->n_ਸੰਚਾਇਕ; i++, pC++){
    sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, pC->pCExpr, Agg_Info_ਕਾਲਮ_Reg(pAggInfo,i));
    ਜੇ( pParse->nErr ) ਵਾਪਸ;
  }

  pAggInfo->ਸਿੱਧਾ_ਬਹੁਲਕ = 0;
  ਜੇ( addr_ਪ੍ਰਾਪਤੀ_Test ){
    sqlite3_Vdbe_Jump_ਇੱਥੇ_Or_ਕੱਢੋ_Inst(v, addr_ਪ੍ਰਾਪਤੀ_Test);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_EXPLAIN
ਸਥਿਰ ਖਾਲੀ explain_Simple_ਗਿਣਤੀ(
  Parse *pParse,                  /* Parse context */
  ਸਾਰਣੀ *pTab,                    /* Table being queried */
  ਸੂਚਕਾਂਕ *pIdx                     /* Index used to optimize scan, or NULL */
){
  ਜੇ( pParse->explain==2 ){
    ਪੂਰਨ bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#ਨਹੀਂਤਾਂ
# define explain_Simple_ਗਿਣਤੀ(a,b,c)
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ having_ਮੰਜ਼ਿਲ_Where_ਵਿਅੰਜਕ_Cb(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Constant_Or_ਸਮੂਹ_By(pWalker->pParse, p_ਵਿਅੰਜਕ, pS->p_ਸਮੂਹ_By)
     && ਵਿਅੰਜਕ_Always_False(p_ਵਿਅੰਜਕ)==0
     && p_ਵਿਅੰਜਕ->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      ਵਿਅੰਜਕ *pNew = sqlite3_ਵਿਅੰਜਕ_Int32(db, 1);
      ਜੇ( pNew ){
        ਵਿਅੰਜਕ *pWhere = pS->pWhere;
        ਅਦਲਾ_ਬਦਲੀ(ਵਿਅੰਜਕ, *pNew, *p_ਵਿਅੰਜਕ);
        pNew = sqlite3_ਵਿਅੰਜਕ_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->e_ਕੋਡ = 1;
      }
    }
    ਵਾਪਸ WRC_Prune;
  }
  ਵਾਪਸ WRC_Continue;
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
ਸਥਿਰ ਖਾਲੀ having_ਮੰਜ਼ਿਲ_Where(Parse *pParse, Select *p){
  Walker sWalker;
  memset(&sWalker, 0, ਆਕਾਰ(sWalker));
  sWalker.pParse = pParse;
  sWalker.x_ਵਿਅੰਜਕ_Callback = having_ਮੰਜ਼ਿਲ_Where_ਵਿਅੰਜਕ_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, p->pHaving);
#ਜੇ TREETRACE_ENABLED
  ਜੇ( sWalker.e_ਕੋਡ && (sqlite3_ਰੁੱਖ_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
  }
#ਜੇ_ਅੰਤ
}

/*
** Check to see if the pThis entry of pTabList is a self-join of another view.
** Search FROM-clause entries in the range of iFirst..iEnd, including iFirst
** but stopping before iEnd.
**
** If pThis is a self-join, then return the SrcItem for the first other
** instance of that view found.  If pThis is not a self-join then return 0.
*/
ਸਥਿਰ Src_ਵਸਤੂ *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  Src_ਵਸਤੂ *pThis,              /* Search for prior reference to this subquery */
  ਪੂਰਨ i_ਪਹਿਲਾ, ਪੂਰਨ i_ਸਿਰਾ        /* Range of FROM-clause entries to search. */
){
  Src_ਵਸਤੂ *p_ਵਸਤੂ;
  Select *pSel;
  ਪ੍ਰਤਿਜ੍ਞਾ( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSel!=0 );
  ਜੇ( pSel->selFlags & SF_ਧੱਕੋ_ਥੱਲੇ ) ਵਾਪਸ 0;
  ਜਦੋਂ( i_ਪਹਿਲਾ<i_ਸਿਰਾ ){
    Select *pS1;
    p_ਵਸਤੂ = &pTabList->a[i_ਪਹਿਲਾ++];
    ਜੇ( !p_ਵਸਤੂ->fg.isSubquery ) ਜਾਰੀ;
    ਜੇ( p_ਵਸਤੂ->fg.ਰਾਹੀਂ_Coroutine ) ਜਾਰੀ;
    ਜੇ( p_ਵਸਤੂ->zName==0 ) ਜਾਰੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->pSTab!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( pThis->pSTab!=0 );
    ਜੇ( p_ਵਸਤੂ->pSTab->pSchema!=pThis->pSTab->pSchema ) ਜਾਰੀ;
    ਜੇ( sqlite3_stricmp(p_ਵਸਤੂ->zName, pThis->zName)!=0 ) ਜਾਰੀ;
    pS1 = p_ਵਸਤੂ->u4.pSubq->pSelect;
    ਜੇ( p_ਵਸਤੂ->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      ਜਾਰੀ;
    }
    ਜੇ( pS1->selFlags & SF_ਧੱਕੋ_ਥੱਲੇ ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      ਜਾਰੀ;
    }
    ਵਾਪਸ p_ਵਸਤੂ;
  }
  ਵਾਪਸ 0;
}

/*
** Deallocate a single AggInfo object
*/
ਸਥਿਰ ਖਾਲੀ agginfoFree(sqlite3 *db, ਖਾਲੀ *pArg){
  AggInfo *p = (AggInfo*)pArg;
  sqlite3DbFree(db, p->a_ਖਾਨਾ);
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
ਸਥਿਰ ਪੂਰਨ ਗਿਣਤੀ_ਦਾ_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *p_ਪੂਰਵ;
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ;
  ਵਿਅੰਜਕ *p_ਗਿਣਤੀ;
  sqlite3 *db;
  Src_ਵਸਤੂ *pFrom;
  ਜੇ( (p->selFlags & SF_Aggregate)==0 ) ਵਾਪਸ 0;   /* This is an aggregate */
  ਜੇ( p->pEList->n_ਵਿਅੰਜਕ!=1 ) ਵਾਪਸ 0;               /* Single result column */
  ਜੇ( p->pWhere ) ਵਾਪਸ 0;
  ਜੇ( p->pHaving ) ਵਾਪਸ 0;
  ਜੇ( p->p_ਸਮੂਹ_By ) ਵਾਪਸ 0;
  ਜੇ( p->p_ਕ੍ਰਮ_By ) ਵਾਪਸ 0;
  p_ਵਿਅੰਜਕ = p->pEList->a[0].p_ਵਿਅੰਜਕ;
  ਜੇ( p_ਵਿਅੰਜਕ->op!=TK_AGG_FUNCTION ) ਵਾਪਸ 0;        /* Result is an aggregate */
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_UToken(p_ਵਿਅੰਜਕ) );
  ਜੇ( sqlite3_stricmp(p_ਵਿਅੰਜਕ->u.zToken,"count") ) ਵਾਪਸ 0;  /* Is count() */
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
  ਜੇ( p_ਵਿਅੰਜਕ->x.pList!=0 ) ਵਾਪਸ 0;                 /* Must be count(*) */
  ਜੇ( p->pSrc->nSrc!=1 ) ਵਾਪਸ 0;                  /* One table in FROM  */
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ) ਵਾਪਸ 0;/* Not a window function */
  pFrom = p->pSrc->a;
  ਜੇ( pFrom->fg.isSubquery==0 ) ਵਾਪਸ 0;           /* FROM is a subquery */
  ਜੇ( (p->selFlags & SF_Correlated)!=0 ) ਵਾਪਸ 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  ਜੇ( pSub->p_ਪੂਰਵ==0 ) ਵਾਪਸ 0;                   /* Must be a compound */
  ਜੇ( pSub->selFlags & SF_CopyCte ) ਵਾਪਸ 0;       /* Not a CTE */
  ਕਰੋ{
    ਜੇ( pSub->op!=TK_ALL && pSub->p_ਪੂਰਵ ) ਵਾਪਸ 0;  /* Must be UNION ALL */
    ਜੇ( pSub->pWhere ) ਵਾਪਸ 0;                      /* No WHERE clause */
    ਜੇ( pSub->p_ਹੱਦਬੰਦੀ ) ਵਾਪਸ 0;                      /* No LIMIT clause */
    ਜੇ( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       ਵਾਪਸ 0;     /* Not an aggregate nor DISTINCT */
    }
    ਪ੍ਰਤਿਜ੍ਞਾ( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->p_ਪੂਰਵ;                              /* Repeat over compound */
  }ਜਦੋਂ( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_ਗਿਣਤੀ = p_ਵਿਅੰਜਕ;
  p_ਵਿਅੰਜਕ = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  ਜਦੋਂ( pSub ){
    ਵਿਅੰਜਕ *p_ਪਦ;
    p_ਪੂਰਵ = pSub->p_ਪੂਰਵ;
    pSub->p_ਪੂਰਵ = 0;
    pSub->p_ਅਗਲਾ = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->n_Select_ਪੰਕਤੀ = 0;
    sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, sqlite3_ਵਿਅੰਜਕ_List_Delete_Generic, pSub->pEList);
    p_ਪਦ = p_ਪੂਰਵ ? sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਗਿਣਤੀ, 0) : p_ਗਿਣਤੀ;
    pSub->pEList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, 0, p_ਪਦ);
    p_ਪਦ = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3_PExpr_ਜੋੜੋ_Select(pParse, p_ਪਦ, pSub);
    ਜੇ( p_ਵਿਅੰਜਕ==0 ){
      p_ਵਿਅੰਜਕ = p_ਪਦ;
    }ਨਹੀਂਤਾਂ{
      p_ਵਿਅੰਜਕ = sqlite3PExpr(pParse, TK_PLUS, p_ਪਦ, p_ਵਿਅੰਜਕ);
    }
    ਜੇ( pParse->nErr ){
      sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਿਅੰਜਕ);
      p_ਵਿਅੰਜਕ = 0;
    }
    pSub = p_ਪੂਰਵ;
  }
  p->pEList->a[0].p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ;
  p->selFlags &= ~(u32)SF_Aggregate;

#ਜੇ TREETRACE_ENABLED
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
ਸਥਿਰ ਪੂਰਨ sameSrcAlias(Src_ਵਸਤੂ *p0, SrcList *pSrc){
  ਪੂਰਨ i;
  ਲਈ(i=0; i<pSrc->nSrc; i++){
    Src_ਵਸਤੂ *p1 = &pSrc->a[i];
    ਜੇ( p1==p0 ) ਜਾਰੀ;
    ਜੇ( p0->pSTab==p1->pSTab && 0==sqlite3_stricmp(p0->zAlias, p1->zAlias) ){
      ਵਾਪਸ 1;
    }
    ਜੇ( p1->fg.isSubquery
     && (p1->u4.pSubq->pSelect->selFlags & SF_NestedFrom)!=0
     && sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)
    ){
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ from_Clause_ਪਦ_Can_Be_Coroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  ਪੂਰਨ i,                  /* Which term of the FROM clause holds the subquery */
  ਪੂਰਨ selFlags            /* Flags on the SELECT statement */
){
  Src_ਵਸਤੂ *p_ਵਸਤੂ = &pTabList->a[i];
  ਜੇ( p_ਵਸਤੂ->fg.isCte ){
    ਅਚਲ CteUse *pCteUse = p_ਵਸਤੂ->u2.pCteUse;
    ਜੇ( pCteUse->eM10d==M10d_Yes ) ਵਾਪਸ 0;                          /* (2a) */
    ਜੇ( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) ਵਾਪਸ 0;       /* (2b) */
  }
  ਜੇ( pTabList->a[0].fg.jointype & JT_LTORJ ) ਵਾਪਸ 0;               /* (3)  */
  ਜੇ( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) ਵਾਪਸ 0; /* (4)  */
  ਜੇ( isSelfJoinView(pTabList, p_ਵਸਤੂ, i+1, pTabList->nSrc)!=0 ){
    ਵਾਪਸ 0;                                                          /* (5) */
  }
  ਜੇ( i==0 ){
    ਜੇ( pTabList->nSrc==1 ) ਵਾਪਸ 1;                             /* (1a) */
    ਜੇ( pTabList->a[1].fg.jointype & JT_CROSS ) ਵਾਪਸ 1;         /* (1b) */
    ਜੇ( selFlags & SF_UpdateFrom )              ਵਾਪਸ 0;         /* (1c-iii) */
    ਵਾਪਸ 1;
  }
  ਜੇ( selFlags & SF_UpdateFrom ) ਵਾਪਸ 0;                        /* (1c-iii) */
  ਜਦੋਂ( 1 /*exit-by-break*/ ){
    ਜੇ( p_ਵਸਤੂ->fg.jointype & (JT_OUTER|JT_CROSS)  ) ਵਾਪਸ 0;     /* (1c-ii) */
    ਜੇ( i==0 ) ਰੋਕੋ;
    i--;
    p_ਵਸਤੂ--;
    ਜੇ( p_ਵਸਤੂ->fg.isSubquery ) ਵਾਪਸ 0;                          /* (1c-i) */
  }
  ਵਾਪਸ 1;
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
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ exists_ਮੰਜ਼ਿਲ_Join(
  Parse *pParse,  /* Parsing context */
  Select *p,      /* The SELECT statement being optimized */
  ਵਿਅੰਜਕ *pWhere    /* part of the WHERE clause currently being examined */
){
  ਜੇ( pParse->nErr==0
   && pWhere!=0
   && !ਵਿਅੰਜਕ_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->p_ਹੱਦਬੰਦੀ==0 || p->p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ==0)
  ){
    ਜੇ( pWhere->op==TK_AND ){
      ਵਿਅੰਜਕ *p_ਸੱਜਾ = pWhere->p_ਸੱਜਾ;
      exists_ਮੰਜ਼ਿਲ_Join(pParse, p, pWhere->p_ਖੱਬਾ);
      exists_ਮੰਜ਼ਿਲ_Join(pParse, p, p_ਸੱਜਾ);
    }
    ਨਹੀਂਤਾਂ ਜੇ( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      ਵਿਅੰਜਕ *pSubWhere = pSub->pWhere;
      ਜੇ( pSub->pSrc->nSrc==1
       && (pSub->selFlags & SF_Aggregate)==0
       && !pSub->pSrc->a[0].fg.isSubquery
       && pSub->p_ਹੱਦਬੰਦੀ==0
       && pSub->p_ਪੂਰਵ==0
      ){
        /* Before combining the sub-select with the parent, renumber the 
        ** cursor used by the subselect. This is because the EXISTS expression
        ** might be a copy of another EXISTS expression from somewhere
        ** else in the tree, and in this case it is important that it use
        ** a unique cursor number.  */
        sqlite3 *db = pParse->db;
        ਪੂਰਨ *aCsrMap = sqlite3DbMallocZero(db, (pParse->nTab+2)*ਆਕਾਰ(ਪੂਰਨ));
        ਜੇ( aCsrMap==0 ) ਵਾਪਸ;
        aCsrMap[0] = (pParse->nTab+1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        memset(pWhere, 0, ਆਕਾਰ(*pWhere));
        pWhere->op = TK_INTEGER;
        pWhere->u.i_ਮੁੱਲ = 1;
        ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pWhere, EP_Int_ਮੁੱਲ);
        ਪ੍ਰਤਿਜ੍ਞਾ( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        ਜੇ( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recompute_Columns_ਵਰਤਿਆ(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#ਜੇ TREETRACE_ENABLED
        ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
        }
#ਜੇ_ਅੰਤ
      }
    }
  }
}

/*
** Type used for Walker callbacks by selectCheckOnClauses().
*/
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ CheckOnCtx CheckOnCtx;
ਬਣਤਰ CheckOnCtx {
  SrcList *pSrc;       /* SrcList for this context */
  ਪੂਰਨ iJoin;           /* Cursors must be left of this one, if not negative */
  ਪੂਰਨ bFuncArg;        /* True for table-function arg */
  CheckOnCtx *p_ਮਾਪਾ; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ has_ਸੱਜਾ_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
ਸਥਿਰ ਪੂਰਨ select_Check_On_Clauses_ਵਿਅੰਜਕ(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  ਜੇ( (ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON))
   || (ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_InnerON) && has_ਸੱਜਾ_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    ਪ੍ਰਤਿਜ੍ਞਾ( pCtx->iJoin<0 || pCtx->iJoin==p_ਵਿਅੰਜਕ->w.iJoin );
    ਜੇ( pCtx->iJoin<0 ){
      pCtx->iJoin = p_ਵਿਅੰਜਕ->w.iJoin;
      sqlite3_Walk_ਵਿਅੰਜਕ_NN(pWalker, p_ਵਿਅੰਜਕ);
      pCtx->iJoin = -1;
      ਵਾਪਸ WRC_Prune;
    }
  }

  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    ਪੂਰਨ iTab = p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ;
    ਕਰੋ {
      SrcList *pSrc = pCtx->pSrc;
      ਪੂਰਨ nSrc = pSrc->nSrc;
      ਪੂਰਨ ii;
      ਲਈ(ii=0; ii<nSrc && pSrc->a[ii].iCursor!=iTab; ii++){}
      ਜੇ( ii<nSrc ){
        /* pSrc is the FROM clause that contains iTab */
        ਜੇ( pCtx->iJoin>=0 ){
          ਲਈ(ii--; ii>=0 && pSrc->a[ii].iCursor!=pCtx->iJoin; ii--){}
          ਜੇ( ii>=0 ){
            /* Table iJoin appears to the left of table iTab in the SrcList.
            ** Therefore the expression refers to a table to its right. */
            sqlite3_Error_ਸੁਨੇਹਾ(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            ਵਾਪਸ WRC_Abort;
          }
        }
        ਰੋਕੋ;
      }
      pCtx = pCtx->p_ਮਾਪਾ;
    }ਜਦੋਂ( pCtx );
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** The xSelect callback for the search of invalid ON clause terms.
*/
ਸਥਿਰ ਪੂਰਨ selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  ਜੇ( pSelect->pSrc==pCtx->pSrc || pSelect->pSrc->nSrc==0 ){
    ਵਾਪਸ WRC_Continue;
  }ਨਹੀਂਤਾਂ{
    CheckOnCtx sCtx;
    memset(&sCtx, 0, ਆਕਾਰ(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.p_ਮਾਪਾ = pCtx;
    sCtx.iJoin = -1;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~SF_On_ਮੰਜ਼ਿਲ_Where;
    ਵਾਪਸ WRC_Prune;
  }
}

/*
** Check all ON clauses in pSelect to verify that they do not reference
** columns to the right.
*/
ਖਾਲੀ sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect){
  Walker w;
  CheckOnCtx sCtx;
  ਪੂਰਨ ii;
  ਪ੍ਰਤਿਜ੍ਞਾ( pSelect->selFlags & SF_On_ਮੰਜ਼ਿਲ_Where );
  ਪ੍ਰਤਿਜ੍ਞਾ( pSelect->pSrc!=0 && pSelect->pSrc->nSrc>=2 );
  memset(&w, 0, ਆਕਾਰ(w));
  w.pParse = pParse;
  w.x_ਵਿਅੰਜਕ_Callback = select_Check_On_Clauses_ਵਿਅੰਜਕ;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, ਆਕਾਰ(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_ਵਿਅੰਜਕ(&w, pSelect->pWhere);
  pSelect->selFlags &= ~SF_On_ਮੰਜ਼ਿਲ_Where;

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
  ਲਈ(ii=0; ii<pSelect->pSrc->nSrc; ii++){
    Src_ਵਸਤੂ *p_ਵਸਤੂ = &pSelect->pSrc->a[ii];
    ਜੇ( p_ਵਸਤੂ->fg.isTabFunc ){
      sCtx.iJoin = -1;
      ਜੇ( (p_ਵਸਤੂ->fg.jointype & (JT_LTORJ|JT_ਸੱਜਾ)) ){
        sCtx.iJoin = p_ਵਸਤੂ->iCursor;
      }ਨਹੀਂਤਾਂ{
        ਪੂਰਨ jj;
        ਲਈ(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          Src_ਵਸਤੂ *pItem2 = &pSelect->pSrc->a[jj];
          ਜੇ( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            ਰੋਕੋ;
          }
        }
      }
      ਜੇ( sCtx.iJoin>=0 ){
        sqlite3_Walk_ਵਿਅੰਜਕ_List(&w, p_ਵਸਤੂ->u1.pFuncArg);
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
ਸਥਿਰ ਪੂਰਨ sqlite3_Copy_ਛਾਂਟੋ_ਕ੍ਰਮ(ਵਿਅੰਜਕ_List *p1, ਵਿਅੰਜਕ_List *p2){
  ਪ੍ਰਤਿਜ੍ਞਾ( p1 );
  ਜੇ( p2 && p1->n_ਵਿਅੰਜਕ==p2->n_ਵਿਅੰਜਕ ){
    ਪੂਰਨ ii;
    ਲਈ(ii=0; ii<p1->n_ਵਿਅੰਜਕ; ii++){
      u8 ਛਾਂਟੋ_Flags;
      ਛਾਂਟੋ_Flags = p2->a[ii].fg.ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_DESC;
      p1->a[ii].fg.ਛਾਂਟੋ_Flags = ਛਾਂਟੋ_Flags;
    }
    ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਵਾਪਸ 0;
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
ਪੂਰਨ sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  SelectDest *pDest      /* What to do with the query results */
){
  ਪੂਰਨ i, j;              /* Loop counters */
  WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
  Vdbe *v;               /* The virtual machine under construction */
  ਪੂਰਨ isAgg;             /* True for select lists like "count(*)" */
  ਵਿਅੰਜਕ_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  ਵਿਅੰਜਕ *pWhere;          /* The WHERE clause.  May be NULL */
  ਵਿਅੰਜਕ_List *p_ਸਮੂਹ_By;    /* The GROUP BY clause.  May be NULL */
  ਵਿਅੰਜਕ *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  ਪੂਰਨ rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  ਛਾਂਟੋ_Ctx s_ਛਾਂਟੋ;         /* Info on how to code the ORDER BY clause */
  ਪੂਰਨ i_ਸਿਰਾ;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  ਵਿਅੰਜਕ_List *p_Min_Max_ਕ੍ਰਮ_By = 0;  /* Added ORDER BY for min/max queries */
  u8 minMaxFlag;                 /* Flag for min/max queries */

  db = pParse->db;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse==db->pParse );
  v = sqlite3GetVdbe(pParse);
  ਜੇ( p==0 || pParse->nErr ){
    ਵਾਪਸ 1;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
  ਜੇ( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) ਵਾਪਸ 1;
#ਜੇ TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p, ("begin processing:\n", pParse->addrExplain));
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x10000 ){
    ਜੇ( (sqlite3_ਰੁੱਖ_Trace & 0x10001)==0x10000 ){
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#ਜੇ_ਅੰਤ

  /* tag-select-0100 */
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਕ੍ਰਮ_By==0 || pDest->eDest!=SRT_DistFifo );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਕ੍ਰਮ_By==0 || pDest->eDest!=SRT_Fifo );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਕ੍ਰਮ_By==0 || pDest->eDest!=SRT_Dist_ਕਤਾਰ );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਕ੍ਰਮ_By==0 || pDest->eDest!=SRT_ਕਤਾਰ );
  ਜੇ( IgnorableDistinct(pDest) ){
    ਪ੍ਰਤਿਜ੍ਞਾ(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_Dist_ਕਤਾਰ  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    ਜੇ( p->p_ਕ੍ਰਮ_By ){
#ਜੇ TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x800 ){
        sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(0, p->p_ਕ੍ਰਮ_By, 0, "ORDERBY");
      }
#ਜੇ_ਅੰਤ   
      sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, sqlite3_ਵਿਅੰਜਕ_List_Delete_Generic,
                              p->p_ਕ੍ਰਮ_By);
      testcase( pParse->earlyCleanup );
      p->p_ਕ੍ਰਮ_By = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  ਜੇ( pParse->nErr ){
    ਜਾਓ select_ਸਿਰਾ;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList!=0 );
#ਜੇ TREETRACE_ENABLED
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
  }
#ਜੇ_ਅੰਤ

  /* If the SF_UFSrcCheck flag is set, then this function is being called
  ** as part of populating the temp table for an UPDATE...FROM statement.
  ** In this case, it is an error if the target object (pSrc->a[0]) name
  ** or alias is duplicated within FROM clause (pSrc->a[1..n]). 
  **
  ** Postgres disallows this case too. The reason is that some other
  ** systems handle this case differently, and not all the same way,
  ** which is just confusing. To avoid this, we follow PG's lead and
  ** disallow it altogether.  */
  ਜੇ( p->selFlags & SF_UFSrcCheck ){
    Src_ਵਸਤੂ *p0 = &p->pSrc->a[0];
    ਜੇ( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      ਜਾਓ select_ਸਿਰਾ;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  ਜੇ( pDest->eDest==SRT_Output ){
    sqlite3_Generate_ਕਾਲਮ_ਨਾਂ(pParse, p);
  }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
  ਜੇ( sqlite3_ਖਿੜਕੀ_Rewrite(pParse, p) ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pParse->nErr );
    ਜਾਓ select_ਸਿਰਾ;
  }
#ਜੇ TREETRACE_ENABLED
  ਜੇ( p->pWin && (sqlite3_ਰੁੱਖ_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
  }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  memset(&s_ਛਾਂਟੋ, 0, ਆਕਾਰ(s_ਛਾਂਟੋ));
  s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By = p->p_ਕ੍ਰਮ_By;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#ਜੇ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  ਲਈ(i=0; !p->p_ਪੂਰਵ && i<pTabList->nSrc; i++){
    Src_ਵਸਤੂ *p_ਵਸਤੂ = &pTabList->a[i];
    Select *pSub = p_ਵਸਤੂ->fg.isSubquery ? p_ਵਸਤੂ->u4.pSubq->pSelect : 0;
    ਸਾਰਣੀ *pTab = p_ਵਸਤੂ->pSTab;

    /* The expander should have already created transient Table objects
    ** even for FROM clause elements such as subqueries that do not correspond
    ** to a real table */
    ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );

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
    ਜੇ( (p_ਵਸਤੂ->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ))!=0
     && sqlite3_ਵਿਅੰਜਕ_ਸਿੱਟਾ_ਦਿੰਦਾ_Non_Null_ਪੰਕਤੀ(p->pWhere, p_ਵਸਤੂ->iCursor,
                                     p_ਵਸਤੂ->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      ਜੇ( p_ਵਸਤੂ->fg.jointype & JT_ਖੱਬਾ ){
        ਜੇ( p_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          p_ਵਸਤੂ->fg.jointype &= ~JT_ਖੱਬਾ;
        }ਨਹੀਂਤਾਂ{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          p_ਵਸਤੂ->fg.jointype &= ~(JT_ਖੱਬਾ|JT_OUTER);
          unset_Join_ਵਿਅੰਜਕ(p->pWhere, p_ਵਸਤੂ->iCursor, 0);
        }
      }
      ਜੇ( p_ਵਸਤੂ->fg.jointype & JT_LTORJ ){
        ਲਈ(j=i+1; j<pTabList->nSrc; j++){
          Src_ਵਸਤੂ *pI2 = &pTabList->a[j];
          ਜੇ( pI2->fg.jointype & JT_ਸੱਜਾ ){
            ਜੇ( pI2->fg.jointype & JT_ਖੱਬਾ ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_ਸੱਜਾ;
            }ਨਹੀਂਤਾਂ{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_ਸੱਜਾ|JT_OUTER);
              unset_Join_ਵਿਅੰਜਕ(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        ਲਈ(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          ਜੇ( pTabList->a[j].fg.jointype & JT_ਸੱਜਾ ) ਰੋਕੋ;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    ਜੇ( pSub==0 ) ਜਾਰੀ;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    ਜੇ( pTab->n_ਖਾਨਾ!=pSub->pEList->n_ਵਿਅੰਜਕ ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "expected %d columns for '%s' but got %d",
                      pTab->n_ਖਾਨਾ, pTab->zName, pSub->pEList->n_ਵਿਅੰਜਕ);
      ਜਾਓ select_ਸਿਰਾ;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    ਜੇ( p_ਵਸਤੂ->fg.isCte && p_ਵਸਤੂ->u2.pCteUse->eM10d==M10d_Yes ){
      ਜਾਰੀ;
    }

    /* Do not try to flatten an aggregate subquery.
    **
    ** Flattening an aggregate subquery is only possible if the outer query
    ** is not a join.  But if the outer query is not a join, then the subquery
    ** will be implemented as a co-routine and there is no advantage to
    ** flattening in that case.
    */
    ਜੇ( (pSub->selFlags & SF_Aggregate)!=0 ) ਜਾਰੀ;
    ਪ੍ਰਤਿਜ੍ਞਾ( pSub->p_ਸਮੂਹ_By==0 );

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
    ਜੇ( pSub->p_ਕ੍ਰਮ_By!=0
     && (p->p_ਕ੍ਰਮ_By!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->p_ਹੱਦਬੰਦੀ==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_ਕ੍ਰਮ_By_Reqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_ਕ੍ਰਮ_By_Reqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_Omit_ਕ੍ਰਮ_By)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, sqlite3_ਵਿਅੰਜਕ_List_Delete_Generic,
                              pSub->p_ਕ੍ਰਮ_By);
      pSub->p_ਕ੍ਰਮ_By = 0;
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
    ਜੇ( pSub->p_ਕ੍ਰਮ_By!=0
     && i==0
     && (p->selFlags & SF_Complex_ਨਤੀਜਾ)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      ਜਾਰੀ;
    }

    /* tag-select-0240 */
    ਜੇ( flattenSubquery(pParse, p, i, isAgg) ){
      ਜੇ( pParse->nErr ) ਜਾਓ select_ਸਿਰਾ;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    ਜੇ( db->mallocFailed ) ਜਾਓ select_ਸਿਰਾ;
    ਜੇ( !IgnorableOrderby(pDest) ){
      s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By = p->p_ਕ੍ਰਮ_By;
    }
  }
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  ਜੇ( p->p_ਪੂਰਵ ){
    rc = multiSelect(pParse, p, pDest);
#ਜੇ TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    ਜੇ( (sqlite3_ਰੁੱਖ_Trace & 0x400)!=0 && Explain_Query_Plan_ਮਾਪਾ(pParse)==0 ){
      sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( p->p_ਅਗਲਾ==0 ) Explain_Query_Plan_ਕੱਢੋ(pParse);
    ਵਾਪਸ rc;
  }
#ਜੇ_ਅੰਤ

  /* If there may be an "EXISTS (SELECT ...)" in the WHERE clause, attempt
  ** to change it into a join.  */
  ਜੇ( pParse->bHasExists && OptimizationEnabled(db,SQLITE_Exists_ਮੰਜ਼ਿਲ_Join) ){
    exists_ਮੰਜ਼ਿਲ_Join(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  /* Do the WHERE-clause constant propagation optimization if this is
  ** a join.  No need to spend time on this operation for non-join queries
  ** as the equivalent optimization will be handled by query planner in
  ** sqlite3WhereBegin().  tag-select-0330
  */
  ਜੇ( p->pWhere!=0
   && p->pWhere->op==TK_AND
   && OptimizationEnabled(db, SQLITE_PropagateConst)
   && propagateConstants(pParse, p)
  ){
#ਜੇ TREETRACE_ENABLED
    ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
    }
#ਜੇ_ਅੰਤ
  }ਨਹੀਂਤਾਂ{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  ਜੇ( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_ਗਿਣਤੀ_ਦਾ_View)
   && ਗਿਣਤੀ_ਦਾ_View_Optimization(pParse, p)
  ){
    ਜੇ( db->mallocFailed ) ਜਾਓ select_ਸਿਰਾ;
    pTabList = p->pSrc;
  }

  /* Loop over all terms in the FROM clause and do two things for each term:
  **
  **   (1) Authorize unreferenced tables
  **   (2) Generate code for all sub-queries
  **
  ** tag-select-0400
  */
  ਲਈ(i=0; i<pTabList->nSrc; i++){
    Src_ਵਸਤੂ *p_ਵਸਤੂ = &pTabList->a[i];
    Src_ਵਸਤੂ *p_ਪੂਰਵ;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#ਜੇ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    ਅਚਲ ਅੱਖਰ *zSavedAuthContext;
#ਜੇ_ਅੰਤ

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
    ਜੇ( p_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ==0 && p_ਵਸਤੂ->zName!=0 ){
      ਅਚਲ ਅੱਖਰ *zDb;
      ਜੇ( p_ਵਸਤੂ->fg.fixedSchema ){
        ਪੂਰਨ iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, p_ਵਸਤੂ->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਸਤੂ->fg.isSubquery ){
        zDb = 0;
      }ਨਹੀਂਤਾਂ{
        zDb = p_ਵਸਤੂ->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, p_ਵਸਤੂ->zName, "", zDb);
    }

#ਜੇ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    ਜੇ( p_ਵਸਤੂ->fg.isSubquery==0 ) ਜਾਰੀ;
    pSubq = p_ਵਸਤੂ->u4.pSubq;
    ਪ੍ਰਤਿਜ੍ਞਾ( pSubq!=0 );
    pSub = pSubq->pSelect;

    /* The code for a subquery should only be generated once. */
    ਜੇ( pSubq->addrFillSub!=0 ) ਜਾਰੀ;

    /* Increment Parse.nHeight by the height of the largest expression
    ** tree referred to by this, the parent select. The child select
    ** may contain expression trees of at most
    ** (SQLITE_MAX_EXPR_DEPTH-Parse.nHeight) height. This is a bit
    ** more conservative than necessary, but much easier than enforcing
    ** an exact limit.
    */
    pParse->n_ਉਚਾਈ += sqlite3_Select_ਵਿਅੰਜਕ_ਉਚਾਈ(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    ਜੇ( OptimizationEnabled(db, SQLITE_ਧੱਕੋ_ਥੱਲੇ)
     && (p_ਵਸਤੂ->fg.isCte==0
         || (p_ਵਸਤੂ->u2.pCteUse->eM10d!=M10d_Yes && p_ਵਸਤੂ->u2.pCteUse->nUse<2))
     && ਧੱਕੋ_ਥੱਲੇ_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#ਜੇ TREETRACE_ENABLED
      ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
      }
#ਜੇ_ਅੰਤ
      ਪ੍ਰਤਿਜ੍ਞਾ( pSubq->pSelect && (pSub->selFlags & SF_ਧੱਕੋ_ਥੱਲੇ)!=0 );
    }ਨਹੀਂਤਾਂ{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    ਜੇ( OptimizationEnabled(db, SQLITE_Null_ਅਣਵਰਤਿਆ_ਖਾਨੇ_ਗਿਣਤੀ)
     && disable_ਅਣਵਰਤਿਆ_Subquery_ਨਤੀਜਾ_Columns(p_ਵਸਤੂ)
    ){
#ਜੇ TREETRACE_ENABLED
      ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
      }
#ਜੇ_ਅੰਤ
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = p_ਵਸਤੂ->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    ਜੇ( from_Clause_ਪਦ_Can_Be_Coroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      ਪੂਰਨ addr_ਸਿਖਰ = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_ਸਿਖਰ);
      VdbeComment((v, "%!S", p_ਵਸਤੂ));
      pSubq->addrFillSub = addr_ਸਿਖਰ;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", p_ਵਸਤੂ));
      sqlite3Select(pParse, pSub, &dest);
      p_ਵਸਤੂ->pSTab->n_ਪੰਕਤੀ_Log_Est = pSub->n_Select_ਪੰਕਤੀ;
      p_ਵਸਤੂ->fg.ਰਾਹੀਂ_Coroutine = 1;
      pSubq->reg_ਨਤੀਜਾ = dest.iSdst;
      sqlite3_Vdbe_ਸਿਰਾ_Coroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", p_ਵਸਤੂ));
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr_ਸਿਖਰ-1);
      sqlite3_Clear_ਆਰਜ਼ੀ_Reg_Cache(pParse);
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਸਤੂ->fg.isCte && p_ਵਸਤੂ->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = p_ਵਸਤੂ->u2.pCteUse;
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      ਜੇ( p_ਵਸਤੂ->iCursor!=pCteUse->iCur ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenDup, p_ਵਸਤੂ->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", p_ਵਸਤੂ));
      }
      pSub->n_Select_ਪੰਕਤੀ = pCteUse->n_ਪੰਕਤੀ_Est;
    }ਨਹੀਂਤਾਂ ਜੇ( (p_ਪੂਰਵ = isSelfJoinView(pTabList, p_ਵਸਤੂ, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *p_ਪੂਰਵ_Subq;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ->fg.isSubquery );
      p_ਪੂਰਵ_Subq = p_ਪੂਰਵ->u4.pSubq;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਪੂਰਵ_Subq!=0 );
      ਜੇ( p_ਪੂਰਵ_Subq->addrFillSub ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, p_ਪੂਰਵ_Subq->regReturn,
                                       p_ਪੂਰਵ_Subq->addrFillSub);
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenDup, p_ਵਸਤੂ->iCursor, p_ਪੂਰਵ->iCursor);
      pSub->n_Select_ਪੰਕਤੀ = p_ਪੂਰਵ_Subq->pSelect->n_Select_ਪੰਕਤੀ;
    }ਨਹੀਂਤਾਂ{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      ਪੂਰਨ ਸਿਖਰ_Addr;
      ਪੂਰਨ onceAddr = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
      ਪੂਰਨ addrExplain;
#ਜੇ_ਅੰਤ

      pSubq->regReturn = ++pParse->nMem;
      ਸਿਖਰ_Addr = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Goto);
      pSubq->addrFillSub = ਸਿਖਰ_Addr+1;
      p_ਵਸਤੂ->fg.isMaterialized = 1;
      ਜੇ( p_ਵਸਤੂ->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3_Vdbe_ਜੋੜੋ_Op0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", p_ਵਸਤੂ));
      }ਨਹੀਂਤਾਂ{
        VdbeNoopComment((v, "materialize %!S", p_ਵਸਤੂ));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, p_ਵਸਤੂ->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", p_ਵਸਤੂ));
      sqlite3Select(pParse, pSub, &dest);
      p_ਵਸਤੂ->pSTab->n_ਪੰਕਤੀ_Log_Est = pSub->n_Select_ਪੰਕਤੀ;
      ਜੇ( onceAddr ) sqlite3_Vdbe_Jump_ਇੱਥੇ(v, onceAddr);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Return, pSubq->regReturn, ਸਿਖਰ_Addr+1);
      VdbeComment((v, "end %!S", p_ਵਸਤੂ));
      sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExplain, addrExplain, -1);
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, ਸਿਖਰ_Addr);
      sqlite3_Clear_ਆਰਜ਼ੀ_Reg_Cache(pParse);
      ਜੇ( p_ਵਸਤੂ->fg.isCte && p_ਵਸਤੂ->fg.isCorrelated==0 ){
        CteUse *pCteUse = p_ਵਸਤੂ->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = p_ਵਸਤੂ->iCursor;
        pCteUse->n_ਪੰਕਤੀ_Est = pSub->n_Select_ਪੰਕਤੀ;
      }
    }
    ਜੇ( db->mallocFailed ) ਜਾਓ select_ਸਿਰਾ;
    pParse->n_ਉਚਾਈ -= sqlite3_Select_ਵਿਅੰਜਕ_ਉਚਾਈ(p);
    pParse->zAuthContext = zSavedAuthContext;
#ਜੇ_ਅੰਤ
  }

  /* Various elements of the SELECT copied into local variables for
  ** convenience */
  pEList = p->pEList;
  pWhere = p->pWhere;
  p_ਸਮੂਹ_By = p->p_ਸਮੂਹ_By;
  pHaving = p->pHaving;
  sDistinct.isTnct = (p->selFlags & SF_Distinct)!=0;

#ਜੇ TREETRACE_ENABLED
  ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
  }
#ਜੇ_ਅੰਤ

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
  ਜੇ( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct
   && sqlite3_Copy_ਛਾਂਟੋ_ਕ੍ਰਮ(pEList, s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By)
   && sqlite3_ਵਿਅੰਜਕ_List_Compare(pEList, s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By, -1)==0
   && OptimizationEnabled(db, SQLITE_ਸਮੂਹ_By_ਕ੍ਰਮ)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#ਜੇ_ਅੰਤ
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    p_ਸਮੂਹ_By = p->p_ਸਮੂਹ_By = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, pEList, 0);
    ਜੇ( p_ਸਮੂਹ_By ){
      ਲਈ(i=0; i<p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ; i++){
        p_ਸਮੂਹ_By->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    ਪ੍ਰਤਿਜ੍ਞਾ( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#ਜੇ TREETRACE_ENABLED
    ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
    }
#ਜੇ_ਅੰਤ
  }

  /* If there is an ORDER BY clause, then create an ephemeral index to
  ** do the sorting.  But this sorting ephemeral index might end up
  ** being unused if the data can be extracted in pre-sorted order.
  ** If that is the case, then the OP_OpenEphemeral instruction will be
  ** changed to an OP_Noop once we figure out that the sorting index is
  ** not needed.  The sSort.addrSortIndex variable is used to facilitate
  ** that change.  tag-select-0600
  */
  ਜੇ( s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By ){
    ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;
    p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_From_ਵਿਅੰਜਕ_List(
        pParse, s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By, 0, pEList->n_ਵਿਅੰਜਕ);
    s_ਛਾਂਟੋ.iECursor = pParse->nTab++;
    s_ਛਾਂਟੋ.addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ =
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_OpenEphemeral,
          s_ਛਾਂਟੋ.iECursor, s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ+1+pEList->n_ਵਿਅੰਜਕ, 0,
          (ਅੱਖਰ*)p_ਕੁੰਜੀ_Info, P4_KEYINFO
      );
  }ਨਹੀਂਤਾਂ{
    s_ਛਾਂਟੋ.addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  ਜੇ( pDest->eDest==SRT_EphemTab ){
    sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_ਵਿਅੰਜਕ);
    ਜੇ( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      ਪੂਰਨ ii;
      ਲਈ(ii=pEList->n_ਵਿਅੰਜਕ-1; ii>0 && pEList->a[ii].fg.b_ਵਰਤਿਆ==0; ii--){
        sqlite3_ਵਿਅੰਜਕ_Delete(db, pEList->a[ii].p_ਵਿਅੰਜਕ);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_ਵਿਅੰਜਕ--;
      }
      ਲਈ(ii=0; ii<pEList->n_ਵਿਅੰਜਕ; ii++){
        ਜੇ( pEList->a[ii].fg.b_ਵਰਤਿਆ==0 ) pEList->a[ii].p_ਵਿਅੰਜਕ->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  i_ਸਿਰਾ = sqlite3VdbeMakeLabel(pParse);
  ਜੇ( (p->selFlags & SF_Fixed_ਹੱਦਬੰਦੀ)==0 ){
    p->n_Select_ਪੰਕਤੀ = 320;  /* 4 billion rows */
  }
  ਜੇ( p->p_ਹੱਦਬੰਦੀ ) compute_ਹੱਦਬੰਦੀ_Registers(pParse, p, i_ਸਿਰਾ);
  ਜੇ( p->i_ਹੱਦਬੰਦੀ==0 && s_ਛਾਂਟੋ.addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ>=0 ){
    sqlite3VdbeChangeOpcode(v, s_ਛਾਂਟੋ.addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ, OP_SorterOpen);
    s_ਛਾਂਟੋ.ਛਾਂਟੋ_Flags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  ਜੇ( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (ਅੱਖਰ*)sqlite3_ਕੁੰਜੀ_Info_From_ਵਿਅੰਜਕ_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }ਨਹੀਂਤਾਂ{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  ਜੇ( !isAgg && p_ਸਮੂਹ_By==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_Fixed_ਹੱਦਬੰਦੀ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਖਿੜਕੀ *pWin = p->pWin;      /* Main window object (or NULL) */
    ਜੇ( pWin ){
      sqlite3_ਖਿੜਕੀ_ਕੋਡ_Init(pParse, p);
    }
#ਜੇ_ਅੰਤ
    ਪ੍ਰਤਿਜ੍ਞਾ( WHERE_USE_ਹੱਦਬੰਦੀ==SF_Fixed_ਹੱਦਬੰਦੀ );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By,
                               p->pEList, p, wctrlFlags, p->n_Select_ਪੰਕਤੀ);
    ਜੇ( pWInfo==0 ) ਜਾਓ select_ਸਿਰਾ;
    ਜੇ( sqlite3_Where_Output_ਪੰਕਤੀ_ਗਿਣਤੀ(pWInfo) < p->n_Select_ਪੰਕਤੀ ){
      p->n_Select_ਪੰਕਤੀ = sqlite3_Where_Output_ਪੰਕਤੀ_ਗਿਣਤੀ(pWInfo);
      ਜੇ( pDest->eDest<=SRT_Dist_ਕਤਾਰ && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->n_Select_ਪੰਕਤੀ -= 30;
      }
    }
    ਜੇ( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    ਜੇ( s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By ){
      s_ਛਾਂਟੋ.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      s_ਛਾਂਟੋ.labelOBLopt = sqlite3_Where_ਕ੍ਰਮ_By_ਹੱਦਬੰਦੀ_Opt_Label(pWInfo);
      ਜੇ( s_ਛਾਂਟੋ.nOBSat==s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ ){
        s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    ਜੇ( s_ਛਾਂਟੋ.addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ>=0 && s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By==0 ){
      sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, s_ਛਾਂਟੋ.addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ);
    }

    ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList==pEList );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( pWin ){
      ਪੂਰਨ addrGosub = sqlite3VdbeMakeLabel(pParse);
      ਪੂਰਨ iCont = sqlite3VdbeMakeLabel(pParse);
      ਪੂਰਨ iBreak = sqlite3VdbeMakeLabel(pParse);
      ਪੂਰਨ regGosub = ++pParse->nMem;

      sqlite3_ਖਿੜਕੀ_ਕੋਡ_ਕਦਮ(pParse, p, pWInfo, regGosub, addrGosub);

      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Goto, 0, iBreak);
      sqlite3VdbeResolveLabel(v, addrGosub);
      VdbeNoopComment((v, "inner-loop subroutine"));
      s_ਛਾਂਟੋ.labelOBLopt = 0;
      selectInnerLoop(pParse, p, -1, &s_ਛਾਂਟੋ, &sDistinct, pDest, iCont, iBreak);
      sqlite3VdbeResolveLabel(v, iCont);
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Return, regGosub);
      VdbeComment((v, "end inner-loop subroutine"));
      sqlite3VdbeResolveLabel(v, iBreak);
    }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */
    {
      /* Use the standard inner loop. */
      selectInnerLoop(pParse, p, -1, &s_ਛਾਂਟੋ, &sDistinct, pDest,
          sqlite3WhereContinueLabel(pWInfo),
          sqlite3WhereBreakLabel(pWInfo));

      /* End the database scan loop.
      */
      TREETRACE(0x2,pParse,p,("WhereEnd\n"));
      sqlite3_Where_ਸਿਰਾ(pWInfo);
    }
  }ਨਹੀਂਤਾਂ{
    /* This case is for when there exist aggregate functions or a GROUP BY
    ** clause or both.  tag-select-0800 */
    NameContext sNC;    /* Name context for processing aggregate information */
    ਪੂਰਨ iAMem;          /* First Mem address for storing current GROUP BY */
    ਪੂਰਨ iBMem;          /* First Mem address for previous GROUP BY */
    ਪੂਰਨ iUseFlag;       /* Mem address holding flag indicating that at least
                        ** one row of the input to the aggregator has been
                        ** processed */
    ਪੂਰਨ iAbortFlag;     /* Mem address which causes query abort if positive */
    ਪੂਰਨ ਸਮੂਹ_By_ਛਾਂਟੋ;    /* Rows come from source in GROUP BY order */
    ਪੂਰਨ addr_ਸਿਰਾ;        /* End of processing for this SELECT */
    ਪੂਰਨ ਛਾਂਟੋ_PTab = 0;   /* Pseudotable used to decode sorting results */
    ਪੂਰਨ ਛਾਂਟੋ_ਬਾਹਰਲਾ = 0;    /* Output register from the sorter */
    ਪੂਰਨ ਕ੍ਰਮ_By_Grp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    ਜੇ( p_ਸਮੂਹ_By ){
      ਪੂਰਨ k;                        /* Loop counter */
      ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;  /* For looping over expression in a list */

      ਲਈ(k=p->pEList->n_ਵਿਅੰਜਕ, p_ਵਸਤੂ=p->pEList->a; k>0; k--, p_ਵਸਤੂ++){
        p_ਵਸਤੂ->u.x.iAlias = 0;
      }
      ਲਈ(k=p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ, p_ਵਸਤੂ=p_ਸਮੂਹ_By->a; k>0; k--, p_ਵਸਤੂ++){
        p_ਵਸਤੂ->u.x.iAlias = 0;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( 66==sqlite3LogEst(100) );
      ਜੇ( p->n_Select_ਪੰਕਤੀ>66 ) p->n_Select_ਪੰਕਤੀ = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      ਜੇ( sqlite3_Copy_ਛਾਂਟੋ_ਕ੍ਰਮ(p_ਸਮੂਹ_By, s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By)
       && sqlite3_ਵਿਅੰਜਕ_List_Compare(p_ਸਮੂਹ_By, s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By, -1)==0
      ){
        ਕ੍ਰਮ_By_Grp = 1;
      }
    }ਨਹੀਂਤਾਂ{
      ਪ੍ਰਤਿਜ੍ਞਾ( 0==sqlite3LogEst(1) );
      p->n_Select_ਪੰਕਤੀ = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addr_ਸਿਰਾ = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, ਆਕਾਰ(*pAggInfo) );
    ਜੇ( pAggInfo ){
      sqlite3_Parser_ਜੋੜੋ_Cleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    ਜੇ( db->mallocFailed ){
      ਜਾਓ select_ਸਿਰਾ;
    }
    pAggInfo->selId = p->selId;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
    pAggInfo->pSelect = p;
#ਜੇ_ਅੰਤ
    memset(&sNC, 0, ਆਕਾਰ(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->n_ਛਾਂਟੀ_ਕਾਲਮ = p_ਸਮੂਹ_By ? p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ : 0;
    pAggInfo->p_ਸਮੂਹ_By = p_ਸਮੂਹ_By;
    sqlite3_ਵਿਅੰਜਕ_Analyze_Agg_List(&sNC, pEList);
    sqlite3_ਵਿਅੰਜਕ_Analyze_Agg_List(&sNC, s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By);
    ਜੇ( pHaving ){
      ਜੇ( p_ਸਮੂਹ_By ){
        ਪ੍ਰਤਿਜ੍ਞਾ( pWhere==p->pWhere );
        ਪ੍ਰਤਿਜ੍ਞਾ( pHaving==p->pHaving );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਮੂਹ_By==p->p_ਸਮੂਹ_By );
        having_ਮੰਜ਼ਿਲ_Where(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_ਵਿਅੰਜਕ_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->n_ਸੰਚਾਇਕ = pAggInfo->n_ਕਾਲਮ;
    ਜੇ( p->p_ਸਮੂਹ_By==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &p_Min_Max_ਕ੍ਰਮ_By);
    }ਨਹੀਂਤਾਂ{
      minMaxFlag = WHERE_ORDERBY_ਸਾਧਾਰਨ;
    }
    analyze_Agg_Func_ਦਲੀਲਾਂ(pAggInfo, &sNC);
    ਜੇ( db->mallocFailed ) ਜਾਓ select_ਸਿਰਾ;
#ਜੇ TREETRACE_ENABLED
    ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
      ਜੇ( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(0, p_Min_Max_ਕ੍ਰਮ_By, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#ਜੇ_ਅੰਤ


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    ਜੇ( p_ਸਮੂਹ_By ){
      ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info;  /* Keying information for the group by clause */
      ਪੂਰਨ addr1;          /* A-vs-B comparison jump */
      ਪੂਰਨ addr_Output_ਪੰਕਤੀ;  /* Start of subroutine that outputs a result row */
      ਪੂਰਨ reg_Output_ਪੰਕਤੀ;   /* Return address register for output subroutine */
      ਪੂਰਨ addr_ਸੈੱਟ_Abort;   /* Set the abort flag and return */
      ਪੂਰਨ addr_ਸਿਖਰ_ਦਾ_Loop;  /* Top of the input loop */
      ਪੂਰਨ addr_ਛਾਂਟੀ_Idx; /* The OP_OpenEphemeral for the sorting index */
      ਪੂਰਨ addrReset;      /* Subroutine for resetting the accumulator */
      ਪੂਰਨ regReset;       /* Return address register for reset subroutine */
      ਵਿਅੰਜਕ_List *pDistinct = 0;
      u16 distFlag = 0;
      ਪੂਰਨ eDist = WHERE_DISTINCT_NOOP;

      ਜੇ( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(ਵਿਅੰਜਕ_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_ਵਿਅੰਜਕ;
        p_ਵਿਅੰਜਕ = sqlite3_ਵਿਅੰਜਕ_Dup(db, p_ਵਿਅੰਜਕ, 0);
        pDistinct = sqlite3_ਵਿਅੰਜਕ_List_Dup(db, p_ਸਮੂਹ_By, 0);
        pDistinct = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, pDistinct, p_ਵਿਅੰਜਕ);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->ਛਾਂਟੀ_Idx = pParse->nTab++;
      p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_From_ਵਿਅੰਜਕ_List(pParse, p_ਸਮੂਹ_By,
                                            0, pAggInfo->n_ਕਾਲਮ);
      addr_ਛਾਂਟੀ_Idx = sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_SorterOpen,
          pAggInfo->ਛਾਂਟੀ_Idx, pAggInfo->n_ਛਾਂਟੀ_ਕਾਲਮ,
          0, (ਅੱਖਰ*)p_ਕੁੰਜੀ_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      reg_Output_ਪੰਕਤੀ = ++pParse->nMem;
      addr_Output_ਪੰਕਤੀ = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ;
      iBMem = pParse->nMem + 1;
      pParse->nMem += p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ;
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Null, 0, iAMem, iAMem+p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ-1);
      sqlite3_ਵਿਅੰਜਕ_Null_ਰਜਿਸਟਰ_Range(pParse, iAMem, p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ);

      /* Begin a loop that will extract all source rows in GROUP BY order.
      ** This might involve two separate loops with an OP_Sort in between, or
      ** it might be a single loop that uses an index to extract information
      ** in the right order to begin with.
      */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, regReset, addrReset);
      TREETRACE(0x2,pParse,p,("WhereBegin\n"));
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_ਸਮੂਹ_By, pDistinct,
          p, (sDistinct.isTnct==2 ? WHERE_DISTINCTBY : WHERE_GROUPBY)
          |  (ਕ੍ਰਮ_By_Grp ? WHERE_SORTBYGROUP : 0) | distFlag, 0
      );
      ਜੇ( pWInfo==0 ){
        sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pDistinct);
        ਜਾਓ select_ਸਿਰਾ;
      }
      ਜੇ( pParse->pIdxEpr ){
        optimize_Aggregate_Use_ਦਾ_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ(pParse, p, pAggInfo, &sNC);
      }
      ਸੌਂਪੋ_Aggregate_Registers(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      ਜੇ( sqlite3WhereIsOrdered(pWInfo)==p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        ਸਮੂਹ_By_ਛਾਂਟੋ = 0;
      }ਨਹੀਂਤਾਂ{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        ਪੂਰਨ reg_ਅਧਾਰ;
        ਪੂਰਨ reg_ਰਿਕਾਰਡ;
        ਪੂਰਨ n_ਖਾਨਾ;
        ਪੂਰਨ n_ਸਮੂਹ_By;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_STMT_SCANSTATUS
        ਪੂਰਨ addrExp;              /* Address of OP_Explain instruction */
#ਜੇ_ਅੰਤ
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        ਸਮੂਹ_By_ਛਾਂਟੋ = 1;
        n_ਸਮੂਹ_By = p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ;
        n_ਖਾਨਾ = n_ਸਮੂਹ_By;
        j = n_ਸਮੂਹ_By;
        ਲਈ(i=0; i<pAggInfo->n_ਕਾਲਮ; i++){
          ਜੇ( pAggInfo->a_ਖਾਨਾ[i].i_Sorter_ਕਾਲਮ>=j ){
            n_ਖਾਨਾ++;
            j++;
          }
        }
        reg_ਅਧਾਰ = sqlite3_Get_ਆਰਜ਼ੀ_Range(pParse, n_ਖਾਨਾ);
        sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_ਵਿਅੰਜਕ_List(pParse, p_ਸਮੂਹ_By, reg_ਅਧਾਰ, 0, 0);
        j = n_ਸਮੂਹ_By;
        pAggInfo->ਸਿੱਧਾ_ਬਹੁਲਕ = 1;
        ਲਈ(i=0; i<pAggInfo->n_ਕਾਲਮ; i++){
          ਬਣਤਰ Agg_Info_ਖਾਨਾ *p_ਖਾਨਾ = &pAggInfo->a_ਖਾਨਾ[i];
          ਜੇ( p_ਖਾਨਾ->i_Sorter_ਕਾਲਮ>=j ){
            sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਖਾਨਾ->pCExpr, j + reg_ਅਧਾਰ);
            j++;
          }
        }
        pAggInfo->ਸਿੱਧਾ_ਬਹੁਲਕ = 0;
        reg_ਰਿਕਾਰਡ = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
        sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Counters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Make_ਰਿਕਾਰਡ, reg_ਅਧਾਰ, n_ਖਾਨਾ, reg_ਰਿਕਾਰਡ);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Sorter_ਪਾਓ, pAggInfo->ਛਾਂਟੀ_Idx, reg_ਰਿਕਾਰਡ);
        sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExp, sqlite3VdbeCurrentAddr(v)-2, -1);
        sqlite3_Release_ਆਰਜ਼ੀ_Reg(pParse, reg_ਰਿਕਾਰਡ);
        sqlite3_Release_ਆਰਜ਼ੀ_Range(pParse, reg_ਅਧਾਰ, n_ਖਾਨਾ);
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_ਸਿਰਾ(pWInfo);
        pAggInfo->ਛਾਂਟੀ_Idx_PTab = ਛਾਂਟੋ_PTab = pParse->nTab++;
        ਛਾਂਟੋ_ਬਾਹਰਲਾ = sqlite3_Get_ਆਰਜ਼ੀ_Reg(pParse);
        sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Counters(v, addrExp, sqlite3VdbeCurrentAddr(v), 0);
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_OpenPseudo, ਛਾਂਟੋ_PTab, ਛਾਂਟੋ_ਬਾਹਰਲਾ, n_ਖਾਨਾ);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Sorter_ਛਾਂਟੋ, pAggInfo->ਛਾਂਟੀ_Idx, addr_ਸਿਰਾ);
        VdbeComment((v, "GROUP BY sort")); VdbeCoverage(v);
        pAggInfo->use_ਛਾਂਟੀ_Idx = 1;
        sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExp, -1, ਛਾਂਟੋ_PTab);
        sqlite3_Vdbe_ਜਾਂਚ_ਸਥਿਤੀ_Range(v, addrExp, -1, pAggInfo->ਛਾਂਟੀ_Idx);
      }

      /* If there are entries in pAgggInfo->aFunc[] that contain subexpressions
      ** that are indexed (and that were previously identified and tagged
      ** in optimizeAggregateUseOfIndexedExpr()) then those subexpressions
      ** must now be converted into a TK_AGG_COLUMN node so that the value
      ** is correctly pulled from the index rather than being recomputed. */
      ਜੇ( pParse->pIdxEpr ){
        aggregate_Convert_ਸੂਚਕਾਂਕਿਤ_ਵਿਅੰਜਕ_Ref_ਮੰਜ਼ਿਲ_ਕਾਲਮ(pAggInfo);
#ਜੇ TREETRACE_ENABLED
        ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
          printAggInfo(pAggInfo);
        }
#ਜੇ_ਅੰਤ
      }

      /* If the index or temporary table used by the GROUP BY sort
      ** will naturally deliver rows in the order required by the ORDER BY
      ** clause, cancel the ephemeral table open coded earlier.
      **
      ** This is an optimization - the correct answer should result regardless.
      ** Use the SQLITE_GroupByOrder flag with SQLITE_TESTCTRL_OPTIMIZER to
      ** disable this optimization for testing purposes.  */
      ਜੇ( ਕ੍ਰਮ_By_Grp && OptimizationEnabled(db, SQLITE_ਸਮੂਹ_By_ਕ੍ਰਮ)
       && (ਸਮੂਹ_By_ਛਾਂਟੋ || sqlite3WhereIsSorted(pWInfo))
      ){
        s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By = 0;
        sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, s_ਛਾਂਟੋ.addr_ਛਾਂਟੋ_ਸੂਚਕਾਂਕ);
      }

      /* Evaluate the current GROUP BY terms and store in b0, b1, b2...
      ** (b0 is memory location iBMem+0, b1 is iBMem+1, and so forth)
      ** Then compare the current GROUP BY terms against the GROUP BY terms
      ** from the previous row currently stored in a0, a1, a2...
      */
      addr_ਸਿਖਰ_ਦਾ_Loop = sqlite3VdbeCurrentAddr(v);
      ਜੇ( ਸਮੂਹ_By_ਛਾਂਟੋ ){
        sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Sorter_ਅੰਕੜੇ, pAggInfo->ਛਾਂਟੀ_Idx,
                          ਛਾਂਟੋ_ਬਾਹਰਲਾ, ਛਾਂਟੋ_PTab);
      }
      ਲਈ(j=0; j<p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ; j++){
        ਪੂਰਨ i_ਕ੍ਰਮ_By_ਖਾਨਾ = p_ਸਮੂਹ_By->a[j].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ;
        
        ਜੇ( ਸਮੂਹ_By_ਛਾਂਟੋ ){
          sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_ਕਾਲਮ, ਛਾਂਟੋ_PTab, j, iBMem+j);
        }ਨਹੀਂਤਾਂ{
          pAggInfo->ਸਿੱਧਾ_ਬਹੁਲਕ = 1;
          sqlite3_ਵਿਅੰਜਕ_ਕੋਡ(pParse, p_ਸਮੂਹ_By->a[j].p_ਵਿਅੰਜਕ, iBMem+j);
        }

        ਜੇ( i_ਕ੍ਰਮ_By_ਖਾਨਾ ){
          ਵਿਅੰਜਕ *pX = p->pEList->a[i_ਕ੍ਰਮ_By_ਖਾਨਾ-1].p_ਵਿਅੰਜਕ;
          ਵਿਅੰਜਕ *p_ਅਧਾਰ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(pX);
          ਜਦੋਂ( ALWAYS(p_ਅਧਾਰ!=0) && p_ਅਧਾਰ->op==TK_IF_NULL_ਪੰਕਤੀ ){
            pX = p_ਅਧਾਰ->p_ਖੱਬਾ;
            p_ਅਧਾਰ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(pX);
          }
          ਜੇ( ALWAYS(p_ਅਧਾਰ!=0)
           && p_ਅਧਾਰ->op!=TK_AGG_ਕਾਲਮ
           && p_ਅਧਾਰ->op!=TK_ਰਜਿਸਟਰ
          ){
            sqlite3_ਵਿਅੰਜਕ_ਮੰਜ਼ਿਲ_ਰਜਿਸਟਰ(pX, iAMem+j);
          }
        }
      }
      sqlite3_Vdbe_ਜੋੜੋ_Op4(v, OP_Compare, iAMem, iBMem, p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ,
                          (ਅੱਖਰ*)sqlite3_ਕੁੰਜੀ_Info_Ref(p_ਕੁੰਜੀ_Info), P4_KEYINFO);
      addr1 = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op3(v, OP_Jump, addr1+1, 0, addr1+1); VdbeCoverage(v);

      /* Generate code that runs whenever the GROUP BY changes.
      ** Changes in the GROUP BY are detected by the previous code
      ** block.  If there were no changes, this block is skipped.
      **
      ** This code copies current group by terms in b0,b1,b2,...
      ** over to a0,a1,a2.  It then calls the output subroutine
      ** and resets the aggregate accumulator registers in preparation
      ** for the next GROUP BY batch.
      */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, reg_Output_ਪੰਕਤੀ, addr_Output_ਪੰਕਤੀ);
      VdbeComment((v, "output one row of %d", p->selId));
      sqlite3_ਵਿਅੰਜਕ_ਕੋਡ_Move(pParse, iBMem, iAMem, p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IfPos, iAbortFlag, addr_ਸਿਰਾ); VdbeCoverage(v);
      VdbeComment((v, "check abort flag"));
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, regReset, addrReset);
      VdbeComment((v, "reset accumulator %d", p->selId));

      /* Update the aggregate accumulators based on the content of
      ** the current row
      */
      sqlite3_Vdbe_Jump_ਇੱਥੇ(v, addr1);
      update_ਸੰਚਾਇਕ(pParse, iUseFlag, pAggInfo, eDist);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, iUseFlag);
      VdbeComment((v, "indicate data in accumulator %d", p->selId));

      /* End of the loop
      */
      ਜੇ( ਸਮੂਹ_By_ਛਾਂਟੋ ){
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Sorter_ਅਗਲਾ, pAggInfo->ਛਾਂਟੀ_Idx,addr_ਸਿਖਰ_ਦਾ_Loop);
        VdbeCoverage(v);
      }ਨਹੀਂਤਾਂ{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_ਸਿਰਾ(pWInfo);
        sqlite3_Vdbe_Change_ਮੰਜ਼ਿਲ_Noop(v, addr_ਛਾਂਟੀ_Idx);
      }
      sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pDistinct);

      /* Output the final row of result
      */
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Gosub, reg_Output_ਪੰਕਤੀ, addr_Output_ਪੰਕਤੀ);
      VdbeComment((v, "output final row of %d", p->selId));

      /* Jump over the subroutines
      */
      sqlite3VdbeGoto(v, addr_ਸਿਰਾ);

      /* Generate a subroutine that outputs a single row of the result
      ** set.  This subroutine first looks at the iUseFlag.  If iUseFlag
      ** is less than or equal to zero, the subroutine is a no-op.  If
      ** the processing calls for the query to abort, this subroutine
      ** increments the iAbortFlag memory location before returning in
      ** order to signal the caller to abort.
      */
      addr_ਸੈੱਟ_Abort = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, iAbortFlag);
      VdbeComment((v, "set abort flag"));
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Return, reg_Output_ਪੰਕਤੀ);
      sqlite3VdbeResolveLabel(v, addr_Output_ਪੰਕਤੀ);
      addr_Output_ਪੰਕਤੀ = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_IfPos, iUseFlag, addr_Output_ਪੰਕਤੀ+2);
      VdbeCoverage(v);
      VdbeComment((v, "Groupby result generator entry point %d", p->selId));
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Return, reg_Output_ਪੰਕਤੀ);
      finalizeAggFunctions(pParse, pAggInfo);
      sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pHaving, addr_Output_ਪੰਕਤੀ+1, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, &s_ਛਾਂਟੋ,
                      &sDistinct, pDest,
                      addr_Output_ਪੰਕਤੀ+1, addr_ਸੈੱਟ_Abort);
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Return, reg_Output_ਪੰਕਤੀ);
      VdbeComment((v, "end groupby result generator %d", p->selId));

      /* Generate a subroutine that will reset the group-by accumulator
      */
      sqlite3VdbeResolveLabel(v, addrReset);
      reset_ਸੰਚਾਇਕ(pParse, pAggInfo);
      sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, iUseFlag);
      VdbeComment((v, "indicate accumulator %d empty", p->selId));
      sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Return, regReset);

      ਜੇ( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        ਬਣਤਰ AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    ਨਹੀਂਤਾਂ {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      ਸਾਰਣੀ *pTab;
      ਜੇ( (pTab = is_Simple_ਗਿਣਤੀ(p, pAggInfo))!=0 ){
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
        ਅਚਲ ਪੂਰਨ iDb = sqlite3_Schema_ਮੰਜ਼ਿਲ_ਸੂਚਕਾਂਕ(pParse->db, pTab->pSchema);
        ਅਚਲ ਪੂਰਨ iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        ਸੂਚਕਾਂਕ *pIdx;                         /* Iterator variable */
        ਕੁੰਜੀ_Info *p_ਕੁੰਜੀ_Info = 0;               /* Keyinfo for scanned index */
        ਸੂਚਕਾਂਕ *p_ਉੱਤਮ = 0;                    /* Best index found so far */
        Pgno i_ਜੜ੍ਹ = pTab->tnum;             /* Root page of scanned b-tree */

        sqlite3_ਕੋਡ_Verify_Schema(pParse, iDb);
        sqlite3_ਸਾਰਣੀ_ਤਾਲਾ(pParse, iDb, pTab->tnum, 0, pTab->zName);

        /* Search for the index that has the lowest scan cost.
        **
        ** (2011-04-15) Do not do a full scan of an unordered index.
        **
        ** (2013-10-03) Do not count the entries in a partial index.
        **
        ** In practice the KeyInfo structure will not be used. It is only
        ** passed to keep OP_OpenRead happy.
        */
        ਜੇ( !HasRowid(pTab) ) p_ਉੱਤਮ = sqlite3_Primary_ਕੁੰਜੀ_ਸੂਚਕਾਂਕ(pTab);
        ਜੇ( !p->pSrc->a[0].fg.not_ਸੂਚਕਾਂਕਿਤ ){
          ਲਈ(pIdx=pTab->p_ਸੂਚਕਾਂਕ; pIdx; pIdx=pIdx->p_ਅਗਲਾ){
            ਜੇ( pIdx->bUnordered==0
             && pIdx->sz_Idx_ਪੰਕਤੀ<pTab->sz_Tab_ਪੰਕਤੀ
             && pIdx->pPartIdxWhere==0
             && (!p_ਉੱਤਮ || pIdx->sz_Idx_ਪੰਕਤੀ<p_ਉੱਤਮ->sz_Idx_ਪੰਕਤੀ)
            ){
              p_ਉੱਤਮ = pIdx;
            }
          }
        }
        ਜੇ( p_ਉੱਤਮ ){
          i_ਜੜ੍ਹ = p_ਉੱਤਮ->tnum;
          p_ਕੁੰਜੀ_Info = sqlite3_ਕੁੰਜੀ_Info_ਦਾ_ਸੂਚਕਾਂਕ(pParse, p_ਉੱਤਮ);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3_Vdbe_ਜੋੜੋ_Op4_Int(v, OP_OpenRead, iCsr, (ਪੂਰਨ)i_ਜੜ੍ਹ, iDb, 1);
        ਜੇ( p_ਕੁੰਜੀ_Info ){
          sqlite3VdbeChangeP4(v, -1, (ਅੱਖਰ *)p_ਕੁੰਜੀ_Info, P4_KEYINFO);
        }
        ਸੌਂਪੋ_Aggregate_Registers(pParse, pAggInfo);
        sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_ਗਿਣਤੀ, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3_Vdbe_ਜੋੜੋ_Op1(v, OP_Close, iCsr);
        explain_Simple_ਗਿਣਤੀ(pParse, pTab, p_ਉੱਤਮ);
      }ਨਹੀਂਤਾਂ{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        ਪੂਰਨ regAcc = 0;           /* "populate accumulators" flag */
        ਵਿਅੰਜਕ_List *pDistinct = 0;
        u16 distFlag = 0;
        ਪੂਰਨ eDist;

        /* If there are accumulator registers but no min() or max() functions
        ** without FILTER clauses, allocate register regAcc. Register regAcc
        ** will contain 0 the first time the inner loop runs, and 1 thereafter.
        ** The code generated by updateAccumulator() uses this to ensure
        ** that the accumulator registers are (a) updated only once if
        ** there are no min() or max functions or (b) always updated for the
        ** first row visited by the aggregate, so that they are updated at
        ** least once even if the FILTER clause means the min() or max()
        ** function visits zero rows.  */
        ਜੇ( pAggInfo->n_ਸੰਚਾਇਕ ){
          ਲਈ(i=0; i<pAggInfo->nFunc; i++){
            ਜੇ( ਵਿਅੰਜਕ_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              ਜਾਰੀ;
            }
            ਜੇ( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              ਰੋਕੋ;
            }
          }
          ਜੇ( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 0, regAcc);
          }
        }ਨਹੀਂਤਾਂ ਜੇ( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        ਸੌਂਪੋ_Aggregate_Registers(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        ਪ੍ਰਤਿਜ੍ਞਾ( p->p_ਸਮੂਹ_By==0 );
        reset_ਸੰਚਾਇਕ(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        ਪ੍ਰਤਿਜ੍ਞਾ( minMaxFlag==WHERE_ORDERBY_ਸਾਧਾਰਨ || p_Min_Max_ਕ੍ਰਮ_By!=0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_Min_Max_ਕ੍ਰਮ_By==0 || p_Min_Max_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_Min_Max_ਕ੍ਰਮ_By,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        ਜੇ( pWInfo==0 ){
          ਜਾਓ select_ਸਿਰਾ;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        update_ਸੰਚਾਇਕ(pParse, regAcc, pAggInfo, eDist);
        ਜੇ( eDist!=WHERE_DISTINCT_NOOP ){
          ਬਣਤਰ AggInfo_func *pF = pAggInfo->aFunc;
          ਜੇ( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        ਜੇ( regAcc ) sqlite3_Vdbe_ਜੋੜੋ_Op2(v, OP_Integer, 1, regAcc);
        ਜੇ( minMaxFlag ){
          sqlite3_Where_Min_Max_Opt_Early_ਬਾਹਰਲਾ(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_ਸਿਰਾ(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By = 0;
      sqlite3_ਵਿਅੰਜਕ_If_False(pParse, pHaving, addr_ਸਿਰਾ, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addr_ਸਿਰਾ, addr_ਸਿਰਾ);
    }
    sqlite3VdbeResolveLabel(v, addr_ਸਿਰਾ);

  } /* endif aggregate query */

  ਜੇ( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explain_ਆਰਜ਼ੀ_ਸਾਰਣੀ(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  ਜੇ( s_ਛਾਂਟੋ.p_ਕ੍ਰਮ_By ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p->pEList==pEList );
    generate_ਛਾਂਟੋ_ਪੂਛ(pParse, p, &s_ਛਾਂਟੋ, pEList->n_ਵਿਅੰਜਕ, pDest);
  }

  /* Jump here to skip this query
  */
  sqlite3VdbeResolveLabel(v, i_ਸਿਰਾ);

  /* The SELECT has been coded. If there is an error in the Parse structure,
  ** set the return code to 1. Otherwise 0. */
  rc = (pParse->nErr>0);

  /* Control jumps to here if an error is encountered above, or upon
  ** successful coding of the SELECT.
  */
select_ਸਿਰਾ:
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 || db->mallocFailed==1 );
  ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, p_Min_Max_ਕ੍ਰਮ_By);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  ਜੇ( pAggInfo && !db->mallocFailed ){
#ਜੇ TREETRACE_ENABLED
    ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#ਜੇ_ਅੰਤ
    ਲਈ(i=0; i<pAggInfo->n_ਕਾਲਮ; i++){
      ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pAggInfo->a_ਖਾਨਾ[i].pCExpr;
      ਜੇ( p_ਵਿਅੰਜਕ==0 ) ਜਾਰੀ;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->pAggInfo==pAggInfo );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->iAgg==i );
    }
    ਲਈ(i=0; i<pAggInfo->nFunc; i++){
      ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pAggInfo->aFunc[i].pFExpr;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->pAggInfo==pAggInfo );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->iAgg==i );
    }
  }
#ਜੇ_ਅੰਤ

#ਜੇ TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  ਜੇ( (sqlite3_ਰੁੱਖ_Trace & 0x40000)!=0 && Explain_Query_Plan_ਮਾਪਾ(pParse)==0 ){
    sqlite3_ਰੁੱਖ_View_Select(0, p, 0);
  }
#ਜੇ_ਅੰਤ
  Explain_Query_Plan_ਕੱਢੋ(pParse);
  ਵਾਪਸ rc;
}
