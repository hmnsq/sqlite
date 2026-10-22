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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ DistinctCtx DistinctCtx;
ರಚನೆ DistinctCtx {
  u8 isTnct;      /* 0: Not distinct. 1: DISTINCT  2: DISTINCT and ORDER BY */
  u8 eTnctType;   /* One of the WHERE_DISTINCT_* operators */
  ಪೂರ್ಣಾಂಕ tabTnct;    /* Ephemeral table used for DISTINCT processing */
  ಪೂರ್ಣಾಂಕ addrTnct;   /* Address of OP_OpenEphemeral opcode for tabTnct */
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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ವಿಂಗಡಿಸು_Ctx ವಿಂಗಡಿಸು_Ctx;
ರಚನೆ ವಿಂಗಡಿಸು_Ctx {
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;   /* The ORDER BY (or GROUP BY clause) */
  ಪೂರ್ಣಾಂಕ nOBSat;           /* Number of ORDER BY terms satisfied by indices */
  ಪೂರ್ಣಾಂಕ iECursor;         /* Cursor number for the sorter */
  ಪೂರ್ಣಾಂಕ regReturn;        /* Register holding block-output return address */
  ಪೂರ್ಣಾಂಕ label_Bk_ಹೊರಹೋಗು;       /* Start label for the block-output subroutine */
  ಪೂರ್ಣಾಂಕ addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ;    /* Address of the OP_SorterOpen or OP_OpenEphemeral */
  ಪೂರ್ಣಾಂಕ label_ಮುಗಿದ;        /* Jump here when done, ex: LIMIT reached */
  ಪೂರ್ಣಾಂಕ labelOBLopt;      /* Jump here when sorter is full */
  u8 ವಿಂಗಡಿಸು_Flags;         /* Zero or more SORTFLAG_* bits */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
  u8 nDefer;            /* Number of valid entries in aDefer[] */
  ರಚನೆ DeferredCsr {
    ಕೋಷ್ಟಕ *pTab;        /* Table definition */
    ಪೂರ್ಣಾಂಕ iCsr;           /* Cursor number for table */
    ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ;           /* Number of PK columns for table pTab (>=1) */
  } aDefer[4];
#ಆದರೆ_ಅಂತ್ಯ
  ರಚನೆ ಅಡ್ಡಸಾಲು_ತುಂಬು_Info *p_Deferred_ಅಡ್ಡಸಾಲು_ತುಂಬು;  /* Deferred row loading info or NULL */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
  ಪೂರ್ಣಾಂಕ addr_ತಳ್ಳು;         /* First instruction to push data into sorter */
  ಪೂರ್ಣಾಂಕ addr_ತಳ್ಳು_ಕೊನೆ;      /* Last instruction that pushes data into sorter */
#ಆದರೆ_ಅಂತ್ಯ
};
#ವ್ಯಾಖ್ಯಾನಿಸು SORTFLAG_UseSorter  0x01   /* Use SorterOpen instead of OpenEphemeral */

/*
** Delete all the content of a Select structure.  Deallocate the structure
** itself depending on the value of bFree
**
** If bFree==1, call sqlite3DbFree() on the p object.
** If bFree==0, Leave the first Select object unfreed
*/
ಸ್ಥಿರ ಶೂನ್ಯ clearSelect(sqlite3 *db, Select *p, ಪೂರ್ಣಾಂಕ bFree){
  ಪ್ರತಿಜ್ಞಾ( db!=0 );
  ಆಗ( p ){
    Select *p_ಪೂರ್ವ = p->p_ಪೂರ್ವ;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->pEList);
    sqlite3SrcListDelete(db, p->pSrc);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->pWhere);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->p_ಗುಂಪು_By);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->pHaving);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p->p_ಕ್ರಮ_By);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->p_ಎಲ್ಲೆ);
    ಆದರೆ( OK_IF_ALWAYS_TRUE(p->pWith) ) sqlite3WithDelete(db, p->pWith);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( OK_IF_ALWAYS_TRUE(p->pWinDefn) ){
      sqlite3_ಕಿಟಕಿ_List_Delete(db, p->pWinDefn);
    }
    ಆಗ( p->pWin ){
      ಪ್ರತಿಜ್ಞಾ( p->pWin->ppThis==&p->pWin );
      sqlite3_ಕಿಟಕಿ_Unlink_From_Select(p->pWin);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( bFree ) sqlite3DbNNFreeNN(db, p);
    p = p_ಪೂರ್ವ;
    bFree = 1;
  }
}

/*
** Initialize a SelectDest structure.
*/
ಶೂನ್ಯ sqlite3SelectDestInit(SelectDest *pDest, ಪೂರ್ಣಾಂಕ eDest, ಪೂರ್ಣಾಂಕ iParm){
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
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುಂಪು_By,   /* the GROUP BY clause */
  ಅಭಿವ್ಯಕ್ತಿ *pHaving,        /* the HAVING clause */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,   /* the ORDER BY clause */
  u32 selFlags,         /* Flag parameters, such as SF_Distinct */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ          /* LIMIT value.  NULL means not used */
){
  Select *pNew, *pAllocated;
  Select standin;
  pAllocated = pNew = sqlite3DbMallocRawNN(pParse->db, ಗಾತ್ರ(*pNew) );
  ಆದರೆ( pNew==0 ){
    ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed );
    pNew = &standin;
  }
  ಆದರೆ( pEList==0 ){
    pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0,
                                   sqlite3_ಅಭಿವ್ಯಕ್ತಿ(pParse->db,TK_ASTERISK,0));
  }
  pNew->pEList = pEList;
  pNew->op = TK_SELECT;
  pNew->selFlags = selFlags;
  pNew->i_ಎಲ್ಲೆ = 0;
  pNew->i_ಸ್ಥಾನಾಂತರ = 0;
  pNew->selId = ++pParse->nSelect;
  pNew->n_Select_ಅಡ್ಡಸಾಲು = 0;
  ಆದರೆ( pSrc==0 ) pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  pNew->pSrc = pSrc;
  pNew->pWhere = pWhere;
  pNew->p_ಗುಂಪು_By = p_ಗುಂಪು_By;
  pNew->pHaving = pHaving;
  pNew->p_ಕ್ರಮ_By = p_ಕ್ರಮ_By;
  pNew->p_ಪೂರ್ವ = 0;
  pNew->p_ಮುಂದಿನ = 0;
  pNew->p_ಎಲ್ಲೆ = p_ಎಲ್ಲೆ;
  pNew->pWith = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  pNew->pWin = 0;
  pNew->pWinDefn = 0;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( pParse->db->mallocFailed ) {
    clearSelect(pParse->db, pNew, pNew!=&standin);
    pAllocated = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪ್ರತಿಜ್ಞಾ( pNew->pSrc!=0 || pParse->nErr>0 );
  }
  ಹಿಂತಿರುಗಿ pAllocated;
}


/*
** Delete the given Select structure and all of its substructures.
*/
ಶೂನ್ಯ sqlite3SelectDelete(sqlite3 *db, Select *p){
  ಆದರೆ( OK_IF_ALWAYS_TRUE(p) ) clearSelect(db, p, 1);
}
ಶೂನ್ಯ sqlite3SelectDeleteGeneric(sqlite3 *db, ಶೂನ್ಯ *p){
  ಆದರೆ( ALWAYS(p) ) clearSelect(db, (Select*)p, 1);
}

/*
** Return a pointer to the right-most SELECT statement in a compound.
*/
ಸ್ಥಿರ Select *ಕಂಡುಹಿಡಿ_Rightmost(Select *p){
  ಆಗ( p->p_ಮುಂದಿನ ) p = p->p_ಮುಂದಿನ;
  ಹಿಂತಿರುಗಿ p;
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
ಪೂರ್ಣಾಂಕ sqlite3JoinType(Parse *pParse, Token *pA, Token *pB, Token *pC){
  ಪೂರ್ಣಾಂಕ jointype = 0;
  Token *apAll[3];
  Token *p;
                             /*   0123456789 123456789 123456789 123 */
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ z_ಕೀಲಿ_ಪಠ್ಯ[] = "naturaleftouterightfullinnercross";
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
    u8 i;        /* Beginning of keyword text in zKeyText[] */
    u8 nChar;    /* Length of the keyword in characters */
    u8 ಸಂಕೇತ_2;     /* Join type mask */
  } aKeyword[] = {
    /* (0) natural */ { 0,  7, JT_NATURAL                },
    /* (1) left    */ { 6,  4, JT_ಎಡ|JT_OUTER          },
    /* (2) outer   */ { 10, 5, JT_OUTER                  },
    /* (3) right   */ { 14, 5, JT_ಬಲ|JT_OUTER         },
    /* (4) full    */ { 19, 4, JT_ಎಡ|JT_ಬಲ|JT_OUTER },
    /* (5) inner   */ { 23, 5, JT_INNER                  },
    /* (6) cross   */ { 28, 5, JT_INNER|JT_CROSS         },
  };
  ಪೂರ್ಣಾಂಕ i, j;
  apAll[0] = pA;
  apAll[1] = pB;
  apAll[2] = pC;
  ಗಾಗಿ(i=0; i<3 && apAll[i]; i++){
    p = apAll[i];
    ಗಾಗಿ(j=0; j<Array_ಗಾತ್ರ(aKeyword); j++){
      ಆದರೆ( p->n==aKeyword[j].nChar
          && sqlite3StrNICmp((ಅಕ್ಷರ*)p->z, &z_ಕೀಲಿ_ಪಠ್ಯ[aKeyword[j].i], p->n)==0 ){
        jointype |= aKeyword[j].ಸಂಕೇತ_2;
        ನಿಲ್ಲಿಸು;
      }
    }
    testcase( j==0 || j==1 || j==2 || j==3 || j==4 || j==5 || j==6 );
    ಆದರೆ( j>=Array_ಗಾತ್ರ(aKeyword) ){
      jointype |= JT_ERROR;
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ(
     (jointype & (JT_INNER|JT_OUTER))==(JT_INNER|JT_OUTER) ||
     (jointype & JT_ERROR)!=0 ||
     (jointype & (JT_OUTER|JT_ಎಡ|JT_ಬಲ))==JT_OUTER
  ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSp1 = " ";
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSp2 = " ";
    ಆದರೆ( pB==0 ){ zSp1++; }
    ಆದರೆ( pC==0 ){ zSp2++; }
    sqlite3_Error_ಸಂದೇಶ(pParse, "unknown join type: "
       "%T%s%T%s%T", pA, zSp1, pB, zSp2, pC);
    jointype = JT_INNER;
  }
  ಹಿಂತಿರುಗಿ jointype;
}

/*
** Return the index of a column in a table.  Return -1 if the column
** is not contained in the table.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(ಕೋಷ್ಟಕ *pTab, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು){
  ಪೂರ್ಣಾಂಕ i;
  u8 h;
  ಸ್ಥಿರಾಂಕ ಸ್ತಂಭ *a_ಲಂಬಸಾಲು;
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;

  h = sqlite3StrIHash(z_ಲಂಬಸಾಲು);
  a_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು;
  n_ಲಂಬಸಾಲು = pTab->n_ಲಂಬಸಾಲು;

  /* See if the aHx gives us a lucky match */
  i = pTab->aHx[h % ಗಾತ್ರ(pTab->aHx)];
  ಪ್ರತಿಜ್ಞಾ( i<n_ಲಂಬಸಾಲು );
  ಆದರೆ( a_ಲಂಬಸಾಲು[i].hName==h
   && sqlite3StrICmp(a_ಲಂಬಸಾಲು[i].zCnName, z_ಲಂಬಸಾಲು)==0
  ){
    ಹಿಂತಿರುಗಿ i;
  }

  /* No lucky match from the hash table.  Do a full search. */
  i = 0;
  ಆಗ( 1 /*exit-by-break*/ ){
    ಆದರೆ( a_ಲಂಬಸಾಲು[i].hName==h
     && sqlite3StrICmp(a_ಲಂಬಸಾಲು[i].zCnName, z_ಲಂಬಸಾಲು)==0
    ){
      ಹಿಂತಿರುಗಿ i;
    }
    i++;
    ಆದರೆ( i>=n_ಲಂಬಸಾಲು ) ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ -1;
}

/*
** Mark a subquery result column as having been used.
*/
ಶೂನ್ಯ sqlite3_Src_ಅಂಶ_ಸ್ತಂಭ_ಬಳಸಿದ(Src_ಅಂಶ *p_ಅಂಶ, ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು){
  ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ!=0 );
  ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಂಕ)p_ಅಂಶ->fg.isNestedFrom == IsNestedFrom(p_ಅಂಶ) );
  ಆದರೆ( p_ಅಂಶ->fg.isNestedFrom ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pResults;
    ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.isSubquery );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->u4.pSubq!=0 );
    ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->u4.pSubq->pSelect!=0 );
    pResults = p_ಅಂಶ->u4.pSubq->pSelect->pEList;
    ಪ್ರತಿಜ್ಞಾ( pResults!=0 );
    ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು>=0 && i_ಲಂಬಸಾಲು<pResults->n_ಅಭಿವ್ಯಕ್ತಿ );
    pResults->a[i_ಲಂಬಸಾಲು].fg.b_ಬಳಸಿದ = 1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೋಷ್ಟಕ_And_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(
  SrcList *pSrc,       /* Array of tables to search */
  ಪೂರ್ಣಾಂಕ i_ಆರಂಭ,          /* First member of pSrc->a[] to check */
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ,            /* Last member of pSrc->a[] to check */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು,    /* Name of the column we are looking for */
  ಪೂರ್ಣಾಂಕ *piTab,          /* Write index of pSrc->a[] here */
  ಪೂರ್ಣಾಂಕ *pi_ಲಂಬಸಾಲು,          /* Write index of pSrc->a[*piTab].pSTab->aCol[] here */
  ಪೂರ್ಣಾಂಕ bIgnoreHidden    /* Ignore hidden columns */
){
  ಪೂರ್ಣಾಂಕ i;               /* For looping over tables in pSrc */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;            /* Index of column matching zCol */

  ಪ್ರತಿಜ್ಞಾ( i_ಕೊನೆ<pSrc->nSrc );
  ಪ್ರತಿಜ್ಞಾ( i_ಆರಂಭ>=0 );
  ಪ್ರತಿಜ್ಞಾ( (piTab==0)==(pi_ಲಂಬಸಾಲು==0) );  /* Both or neither are NULL */

  ಗಾಗಿ(i=i_ಆರಂಭ; i<=i_ಕೊನೆ; i++){
    i_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pSrc->a[i].pSTab, z_ಲಂಬಸಾಲು);
    ಆದರೆ( i_ಲಂಬಸಾಲು>=0
     && (bIgnoreHidden==0 || Is_Hidden_ಸ್ತಂಭ(&pSrc->a[i].pSTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು])==0)
    ){
      ಆದರೆ( piTab ){
        sqlite3_Src_ಅಂಶ_ಸ್ತಂಭ_ಬಳಸಿದ(&pSrc->a[i], i_ಲಂಬಸಾಲು);
        *piTab = i;
        *pi_ಲಂಬಸಾಲು = i_ಲಂಬಸಾಲು;
      }
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಶೂನ್ಯ sqlite3_ಗಣ_Join_ಅಭಿವ್ಯಕ್ತಿ(ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ i_ಕೋಷ್ಟಕ, u32 joinFlag){
  ಪ್ರತಿಜ್ಞಾ( joinFlag==EP_OuterON || joinFlag==EP_InnerON );
  ಆಗ( p ){
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p, joinFlag);
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_TokenOnly|EP_Reduced) );
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_VVAProperty(p, EP_NoReduce);
    p->w.iJoin = i_ಕೋಷ್ಟಕ;
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) ){
      ಆದರೆ( p->x.pList ){
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ(i=0; i<p->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
          sqlite3_ಗಣ_Join_ಅಭಿವ್ಯಕ್ತಿ(p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, i_ಕೋಷ್ಟಕ, joinFlag);
        }
      }
    }
    sqlite3_ಗಣ_Join_ಅಭಿವ್ಯಕ್ತಿ(p->p_ಎಡ, i_ಕೋಷ್ಟಕ, joinFlag);
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
ಸ್ಥಿರ ಶೂನ್ಯ unset_Join_ಅಭಿವ್ಯಕ್ತಿ(ಅಭಿವ್ಯಕ್ತಿ *p, ಪೂರ್ಣಾಂಕ i_ಕೋಷ್ಟಕ, ಪೂರ್ಣಾಂಕ nullable){
  ಆಗ( p ){
    ಆದರೆ( i_ಕೋಷ್ಟಕ<0 || (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_OuterON) && p->w.iJoin==i_ಕೋಷ್ಟಕ) ){
      ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p, EP_OuterON|EP_InnerON);
      ಆದರೆ( i_ಕೋಷ್ಟಕ>=0 ) ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p, EP_InnerON);
    }
    ಆದರೆ( p->op==TK_ಸ್ತಂಭ && p->i_ಕೋಷ್ಟಕ==i_ಕೋಷ್ಟಕ && !nullable ){
      ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p, EP_CanBeNull);
    }
    ಆದರೆ( p->op==TK_FUNCTION ){
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p) );
      ಪ್ರತಿಜ್ಞಾ( p->p_ಎಡ==0 );
      ಆದರೆ( p->x.pList ){
        ಪೂರ್ಣಾಂಕ i;
        ಗಾಗಿ(i=0; i<p->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
          unset_Join_ಅಭಿವ್ಯಕ್ತಿ(p->x.pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, i_ಕೋಷ್ಟಕ, nullable);
        }
      }
    }
    unset_Join_ಅಭಿವ್ಯಕ್ತಿ(p->p_ಎಡ, i_ಕೋಷ್ಟಕ, nullable);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3ProcessJoin(Parse *pParse, Select *p){
  SrcList *pSrc;                  /* All tables in the FROM clause */
  ಪೂರ್ಣಾಂಕ i, j;                       /* Loop counters */
  Src_ಅಂಶ *p_ಎಡ;                 /* Left table being joined */
  Src_ಅಂಶ *p_ಬಲ;                /* Right table being joined */
  ಪೂರ್ಣಾಂಕ b_ನೋಡಿದ_TFunc = 0;             /* True if have seen table-valued function */

  pSrc = p->pSrc;
  p_ಎಡ = &pSrc->a[0];
  p_ಬಲ = &p_ಎಡ[1];
  ಗಾಗಿ(i=0; i<pSrc->nSrc-1; i++, p_ಬಲ++, p_ಎಡ++){
    ಕೋಷ್ಟಕ *p_ಬಲ_Tab = p_ಬಲ->pSTab;
    u32 joinType;

    ಆದರೆ( NEVER(p_ಎಡ->pSTab==0 || p_ಬಲ_Tab==0) ) ಮುಂದುವರಿ;
    joinType = (p_ಬಲ->fg.jointype & JT_OUTER)!=0 ? EP_OuterON : EP_InnerON;

    /* If this is a NATURAL join, synthesize an appropriate USING clause
    ** to specify which columns should be joined.
    */
    ಆದರೆ( p_ಬಲ->fg.jointype & JT_NATURAL ){
      IdList *pUsing = 0;
      ಆದರೆ( p_ಬಲ->fg.isUsing || p_ಬಲ->u3.pOn ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "a NATURAL join may not have "
           "an ON or USING clause", 0);
        ಹಿಂತಿರುಗಿ 1;
      }
      ಗಾಗಿ(j=0; j<p_ಬಲ_Tab->n_ಲಂಬಸಾಲು; j++){
        ಅಕ್ಷರ *zName;   /* Name of column in the right table */

        ಆದರೆ( Is_Hidden_ಸ್ತಂಭ(&p_ಬಲ_Tab->a_ಲಂಬಸಾಲು[j]) ) ಮುಂದುವರಿ;
        zName = p_ಬಲ_Tab->a_ಲಂಬಸಾಲು[j].zCnName;
        ಆದರೆ( ಕೋಷ್ಟಕ_And_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pSrc, 0, i, zName, 0, 0, 1) ){
          pUsing = sqlite3IdListAppend(pParse, pUsing, 0);
          ಆದರೆ( pUsing ){
            ಪ್ರತಿಜ್ಞಾ( pUsing->nId>0 );
            ಪ್ರತಿಜ್ಞಾ( pUsing->a[pUsing->nId-1].zName==0 );
            pUsing->a[pUsing->nId-1].zName = sqlite3DbStrDup(pParse->db, zName);
          }
        }
      }
      ಆದರೆ( pUsing ){
        p_ಬಲ->fg.isUsing = 1;
        p_ಬಲ->fg.isSynthUsing = 1;
        p_ಬಲ->u3.pUsing = pUsing;
      }
      ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ 1;
    }

    /* Create extra terms on the WHERE clause for each column named
    ** in the USING clause.  Example: If the two tables to be joined are
    ** A and B and the USING clause names X, Y, and Z, then add this
    ** to the WHERE clause:    A.X=B.X AND A.Y=B.Y AND A.Z=B.Z
    ** Report an error if any column mentioned in the USING clause is
    ** not contained in both tables to be joined.
    */
    ಆದರೆ( p_ಬಲ->fg.isUsing ){
      IdList *pList = p_ಬಲ->u3.pUsing;
      sqlite3 *db = pParse->db;
      ಪ್ರತಿಜ್ಞಾ( pList!=0 );
      ಗಾಗಿ(j=0; j<pList->nId; j++){
        ಅಕ್ಷರ *zName;     /* Name of the term in the USING clause */
        ಪೂರ್ಣಾಂಕ i_ಎಡ;       /* Table on the left with matching column name */
        ಪೂರ್ಣಾಂಕ i_ಎಡ_ಲಂಬಸಾಲು;    /* Column number of matching column on the left */
        ಪೂರ್ಣಾಂಕ i_ಬಲ_ಲಂಬಸಾಲು;   /* Column number of matching column on the right */
        ಅಭಿವ್ಯಕ್ತಿ *pE1;       /* Reference to the column on the LEFT of the join */
        ಅಭಿವ್ಯಕ್ತಿ *pE2;       /* Reference to the column on the RIGHT of the join */
        ಅಭಿವ್ಯಕ್ತಿ *pEq;       /* Equality constraint.  pE1 == pE2 */

        zName = pList->a[j].zName;
        i_ಬಲ_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(p_ಬಲ_Tab, zName);
        ಆದರೆ( i_ಬಲ_ಲಂಬಸಾಲು<0
         || ಕೋಷ್ಟಕ_And_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pSrc, 0, i, zName, &i_ಎಡ, &i_ಎಡ_ಲಂಬಸಾಲು,
                                p_ಬಲ->fg.isSynthUsing)==0
        ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "cannot join using column %s - column "
            "not present in both tables", zName);
          ಹಿಂತಿರುಗಿ 1;
        }
        pE1 = sqlite3_Create_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(db, pSrc, i_ಎಡ, i_ಎಡ_ಲಂಬಸಾಲು);
        sqlite3_Src_ಅಂಶ_ಸ್ತಂಭ_ಬಳಸಿದ(&pSrc->a[i_ಎಡ], i_ಎಡ_ಲಂಬಸಾಲು);
        ಆದರೆ( (pSrc->a[0].fg.jointype & JT_LTORJ)!=0 && pParse->nErr==0 ){
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
          ಅಭಿವ್ಯಕ್ತಿ_List *p_Func_ವಾದಗಳು = 0;   /* Arguments to the coalesce() */
          ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Token tkCoalesce = { "coalesce", 8 };
          ಪ್ರತಿಜ್ಞಾ( pE1!=0 );
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pE1, EP_CanBeNull);
          ಆಗ( ಕೋಷ್ಟಕ_And_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pSrc, i_ಎಡ+1, i, zName, &i_ಎಡ, &i_ಎಡ_ಲಂಬಸಾಲು,
                                     p_ಬಲ->fg.isSynthUsing)!=0 ){
            ಆದರೆ( pSrc->a[i_ಎಡ].fg.isUsing==0
             || sqlite3_Id_List_ಸೂಚ್ಯಂಕ(pSrc->a[i_ಎಡ].u3.pUsing, zName)<0
            ){
              sqlite3_Error_ಸಂದೇಶ(pParse, "ambiguous reference to %s in USING()",
                              zName);
              ನಿಲ್ಲಿಸು;
            }
            p_Func_ವಾದಗಳು = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, p_Func_ವಾದಗಳು, pE1);
            pE1 = sqlite3_Create_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(db, pSrc, i_ಎಡ, i_ಎಡ_ಲಂಬಸಾಲು);
            sqlite3_Src_ಅಂಶ_ಸ್ತಂಭ_ಬಳಸಿದ(&pSrc->a[i_ಎಡ], i_ಎಡ_ಲಂಬಸಾಲು);
          }
          ಆದರೆ( p_Func_ವಾದಗಳು ){
            p_Func_ವಾದಗಳು = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, p_Func_ವಾದಗಳು, pE1);
            pE1 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function(pParse, p_Func_ವಾದಗಳು, &tkCoalesce, 0);
            ಆದರೆ( pE1 ){
              pE1->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_DEFER;
            }
          }
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (pSrc->a[i+1].fg.jointype & JT_ಎಡ)!=0 && pParse->nErr==0 ){
          ಪ್ರತಿಜ್ಞಾ( pE1!=0 );
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pE1, EP_CanBeNull);
        }
        pE2 = sqlite3_Create_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(db, pSrc, i+1, i_ಬಲ_ಲಂಬಸಾಲು);
        sqlite3_Src_ಅಂಶ_ಸ್ತಂಭ_ಬಳಸಿದ(p_ಬಲ, i_ಬಲ_ಲಂಬಸಾಲು);
        pEq = sqlite3PExpr(pParse, TK_EQ, pE1, pE2);
        ಪ್ರತಿಜ್ಞಾ( pE2!=0 || pEq==0 );
        ಆದರೆ( pEq ){
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pEq, joinType);
          ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pEq, EP_TokenOnly|EP_Reduced) );
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_VVAProperty(pEq, EP_NoReduce);
          pEq->w.iJoin = pE2->i_ಕೋಷ್ಟಕ;
        }
        p->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, p->pWhere, pEq);
      }
    }

    /* Add the ON clause to the end of the WHERE clause, connected by
    ** an AND operator.
    */
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಬಲ->u3.pOn ){
      sqlite3_ಗಣ_Join_ಅಭಿವ್ಯಕ್ತಿ(p_ಬಲ->u3.pOn, p_ಬಲ->iCursor, joinType);
      p->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, p->pWhere, p_ಬಲ->u3.pOn);
      p_ಬಲ->u3.pOn = 0;
      p_ಬಲ->fg.isOn = 1;
      p->selFlags |= SF_On_ಗಮ್ಯ_Where;
    }

    ಆದರೆ( b_ನೋಡಿದ_TFunc==0 ){
      b_ನೋಡಿದ_TFunc = (p_ಎಡ->fg.isTabFunc && p_ಎಡ->u1.pFuncArg);
    }

    /* We also need to call sqlite3SelectCheckOnClauses() to verify that
    ** table-valued function arguments do not illegally refer to any tables to
    ** their right. This test is required if either (a) there is a RIGHT JOIN
    ** in the SrcList, or (b) there is a LEFT JOIN to the right of a 
    ** table-valued function.  */
    ಆದರೆ( (p_ಬಲ->fg.isTabFunc && joinType==EP_OuterON && p_ಬಲ->u1.pFuncArg)
     || (b_ನೋಡಿದ_TFunc && joinType==EP_OuterON)
    ){
      p->selFlags |= SF_On_ಗಮ್ಯ_Where;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** An instance of this object holds information (beyond pParse and pSelect)
** needed to load the next result row that is to be added to the sorter.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಅಡ್ಡಸಾಲು_ತುಂಬು_Info ಅಡ್ಡಸಾಲು_ತುಂಬು_Info;
ರಚನೆ ಅಡ್ಡಸಾಲು_ತುಂಬು_Info {
  ಪೂರ್ಣಾಂಕ reg_ಫಲಿತಾಂಶ;               /* Store results in array of registers here */
  u8 ecelFlags;                /* Flag argument to ExprCodeExprList() */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
  ಅಭಿವ್ಯಕ್ತಿ_List *pExtra;            /* Extra columns needed by sorter refs */
  ಪೂರ್ಣಾಂಕ reg_Extra_ಫಲಿತಾಂಶ;          /* Where to load the extra columns */
#ಆದರೆ_ಅಂತ್ಯ
};

/*
** This routine does the work of loading query data into an array of
** registers so that it can be added to the sorter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ inner_Loop_ತುಂಬು_ಅಡ್ಡಸಾಲು(
  Parse *pParse,             /* Statement under construction */
  Select *pSelect,           /* The query being coded */
  ಅಡ್ಡಸಾಲು_ತುಂಬು_Info *pInfo         /* Info needed to complete the row load */
){
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSelect->pEList, pInfo->reg_ಫಲಿತಾಂಶ,
                          0, pInfo->ecelFlags);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
  ಆದರೆ( pInfo->pExtra ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pInfo->pExtra, pInfo->reg_Extra_ಫಲಿತಾಂಶ, 0, 0);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(pParse->db, pInfo->pExtra);
  }
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Code the OP_MakeRecord instruction that generates the entry to be
** added into the sorter.
**
** Return the register in which the result is stored.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ make_Sorter_ದಾಖಲೆ(
  Parse *pParse,
  ವಿಂಗಡಿಸು_Ctx *p_ವಿಂಗಡಿಸು,
  Select *pSelect,
  ಪೂರ್ಣಾಂಕ reg_ತಳ,
  ಪೂರ್ಣಾಂಕ n_ತಳ
){
  ಪೂರ್ಣಾಂಕ nOBSat = p_ವಿಂಗಡಿಸು->nOBSat;
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ reg_ಹೊರಹೋಗು = ++pParse->nMem;
  ಆದರೆ( p_ವಿಂಗಡಿಸು->p_Deferred_ಅಡ್ಡಸಾಲು_ತುಂಬು ){
    inner_Loop_ತುಂಬು_ಅಡ್ಡಸಾಲು(pParse, pSelect, p_ವಿಂಗಡಿಸು->p_Deferred_ಅಡ್ಡಸಾಲು_ತುಂಬು);
  }
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg_ತಳ+nOBSat, n_ತಳ-nOBSat, reg_ಹೊರಹೋಗು);
  ಹಿಂತಿರುಗಿ reg_ಹೊರಹೋಗು;
}

/*
** Generate code that will push the record in registers regData
** through regData+nData-1 onto the sorter.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ತಳ್ಳು_Onto_Sorter(
  Parse *pParse,         /* Parser context */
  ವಿಂಗಡಿಸು_Ctx *p_ವಿಂಗಡಿಸು,        /* Information about the ORDER BY clause */
  Select *pSelect,       /* The whole SELECT statement */
  ಪೂರ್ಣಾಂಕ reg_ದತ್ತಾಂಶ,           /* First register holding data to be sorted */
  ಪೂರ್ಣಾಂಕ reg_Orig_ದತ್ತಾಂಶ,       /* First register holding data before packing */
  ಪೂರ್ಣಾಂಕ n_ದತ್ತಾಂಶ,             /* Number of elements in the regData data array */
  ಪೂರ್ಣಾಂಕ n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg         /* No. of reg prior to regData available for use */
){
  Vdbe *v = pParse->pVdbe;                         /* Stmt under construction */
  ಪೂರ್ಣಾಂಕ bSeq = ((p_ವಿಂಗಡಿಸು->ವಿಂಗಡಿಸು_Flags & SORTFLAG_UseSorter)==0);
  ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ = p_ವಿಂಗಡಿಸು->p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ;              /* No. of ORDER BY terms */
  ಪೂರ್ಣಾಂಕ n_ತಳ = n_ಅಭಿವ್ಯಕ್ತಿ + bSeq + n_ದತ್ತಾಂಶ;                /* Fields in sorter record */
  ಪೂರ್ಣಾಂಕ reg_ತಳ;                                     /* Regs for sorter record */
  ಪೂರ್ಣಾಂಕ reg_ದಾಖಲೆ = 0;                               /* Assembled sorter record */
  ಪೂರ್ಣಾಂಕ nOBSat = p_ವಿಂಗಡಿಸು->nOBSat;                      /* ORDER BY terms to skip */
  ಪೂರ್ಣಾಂಕ op;                            /* Opcode to add sorter record to sorter */
  ಪೂರ್ಣಾಂಕ i_ಎಲ್ಲೆ;                        /* LIMIT counter */
  ಪೂರ್ಣಾಂಕ iSkip = 0;                     /* End of the sorter insert loop */

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
  ಪ್ರತಿಜ್ಞಾ( n_ದತ್ತಾಂಶ==1 || reg_ದತ್ತಾಂಶ==reg_Orig_ದತ್ತಾಂಶ || reg_Orig_ದತ್ತಾಂಶ==0 );

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
  p_ವಿಂಗಡಿಸು->addr_ತಳ್ಳು = sqlite3VdbeCurrentAddr(v);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg ){
    ಪ್ರತಿಜ್ಞಾ( n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg==n_ಅಭಿವ್ಯಕ್ತಿ+bSeq );
    reg_ತಳ = reg_ದತ್ತಾಂಶ - n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg;
  }ಇಲ್ಲದಿದ್ದರೆ{
    reg_ತಳ = pParse->nMem + 1;
    pParse->nMem += n_ತಳ;
  }
  ಪ್ರತಿಜ್ಞಾ( pSelect->i_ಸ್ಥಾನಾಂತರ==0 || pSelect->i_ಎಲ್ಲೆ!=0 );
  i_ಎಲ್ಲೆ = pSelect->i_ಸ್ಥಾನಾಂತರ ? pSelect->i_ಸ್ಥಾನಾಂತರ+1 : pSelect->i_ಎಲ್ಲೆ;
  p_ವಿಂಗಡಿಸು->label_ಮುಗಿದ = sqlite3VdbeMakeLabel(pParse);
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, p_ವಿಂಗಡಿಸು->p_ಕ್ರಮ_By, reg_ತಳ, reg_Orig_ದತ್ತಾಂಶ,
                          SQLITE_ECEL_DUP | (reg_Orig_ದತ್ತಾಂಶ? SQLITE_ECEL_REF : 0));
  ಆದರೆ( bSeq ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Sequence, p_ವಿಂಗಡಿಸು->iECursor, reg_ತಳ+n_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಆದರೆ( n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg==0 && n_ದತ್ತಾಂಶ>0 ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Move(pParse, reg_ದತ್ತಾಂಶ, reg_ತಳ+n_ಅಭಿವ್ಯಕ್ತಿ+bSeq, n_ದತ್ತಾಂಶ);
  }
  ಆದರೆ( nOBSat>0 ){
    ಪೂರ್ಣಾಂಕ reg_Prev_ಕೀಲಿ;   /* The first nOBSat columns of the previous row */
    ಪೂರ್ಣಾಂಕ addr_ಮೊದಲ;    /* Address of the OP_IfNot opcode */
    ಪೂರ್ಣಾಂಕ addrJmp;      /* Address of the OP_Jump opcode */
    VdbeOp *pOp;      /* Opcode that opens the sorter */
    ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ;         /* Number of sorting key columns, including OP_Sequence */
    ಕೀಲಿ_Info *pKI;     /* Original KeyInfo on the sorter table */

    reg_ದಾಖಲೆ = make_Sorter_ದಾಖಲೆ(pParse, p_ವಿಂಗಡಿಸು, pSelect, reg_ತಳ, n_ತಳ);
    reg_Prev_ಕೀಲಿ = pParse->nMem+1;
    pParse->nMem += p_ವಿಂಗಡಿಸು->nOBSat;
    n_ಕೀಲಿ = n_ಅಭಿವ್ಯಕ್ತಿ - p_ವಿಂಗಡಿಸು->nOBSat + bSeq;
    ಆದರೆ( bSeq ){
      addr_ಮೊದಲ = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IfNot, reg_ತಳ+n_ಅಭಿವ್ಯಕ್ತಿ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      addr_ಮೊದಲ = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_SequenceTest, p_ವಿಂಗಡಿಸು->iECursor);
    }
    VdbeCoverage(v);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Compare, reg_Prev_ಕೀಲಿ, reg_ತಳ, p_ವಿಂಗಡಿಸು->nOBSat);
    pOp = sqlite3VdbeGetOp(v, p_ವಿಂಗಡಿಸು->addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ);
    ಆದರೆ( pParse->db->mallocFailed ) ಹಿಂತಿರುಗಿ;
    pOp->p2 = n_ಕೀಲಿ + n_ದತ್ತಾಂಶ;
    pKI = pOp->p4.p_ಕೀಲಿ_Info;
    memset(pKI->a_ವಿಂಗಡಿಸು_Flags, 0, pKI->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ); /* Makes OP_Jump testable */
    sqlite3VdbeChangeP4(v, -1, (ಅಕ್ಷರ*)pKI, P4_KEYINFO);
    testcase( pKI->n_All_ಕ್ಷೇತ್ರ > pKI->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ+2 );
    pOp->p4.p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse,p_ವಿಂಗಡಿಸು->p_ಕ್ರಮ_By,nOBSat,
                                           pKI->n_All_ಕ್ಷೇತ್ರ-pKI->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ-1);
    pOp = 0; /* Ensure pOp not used after sqlite3VdbeAddOp3() */
    addrJmp = sqlite3VdbeCurrentAddr(v);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Jump, addrJmp+1, 0, addrJmp+1); VdbeCoverage(v);
    p_ವಿಂಗಡಿಸು->label_Bk_ಹೊರಹೋಗು = sqlite3VdbeMakeLabel(pParse);
    p_ವಿಂಗಡಿಸು->regReturn = ++pParse->nMem;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, p_ವಿಂಗಡಿಸು->regReturn, p_ವಿಂಗಡಿಸು->label_Bk_ಹೊರಹೋಗು);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_ResetSorter, p_ವಿಂಗಡಿಸು->iECursor);
    ಆದರೆ( i_ಎಲ್ಲೆ ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IfNot, i_ಎಲ್ಲೆ, p_ವಿಂಗಡಿಸು->label_ಮುಗಿದ);
      VdbeCoverage(v);
    }
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_ಮೊದಲ);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Move(pParse, reg_ತಳ, reg_Prev_ಕೀಲಿ, p_ವಿಂಗಡಿಸು->nOBSat);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrJmp);
  }
  ಆದರೆ( i_ಎಲ್ಲೆ ){
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
    ಪೂರ್ಣಾಂಕ iCsr = p_ವಿಂಗಡಿಸು->iECursor;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IfNotZero, i_ಎಲ್ಲೆ, sqlite3VdbeCurrentAddr(v)+4);
    VdbeCoverage(v);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೊನೆಯ, iCsr, 0);
    iSkip = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IdxLE,
                                 iCsr, 0, reg_ತಳ+nOBSat, n_ಅಭಿವ್ಯಕ್ತಿ-nOBSat);
    VdbeCoverage(v);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Delete, iCsr);
  }
  ಆದರೆ( reg_ದಾಖಲೆ==0 ){
    reg_ದಾಖಲೆ = make_Sorter_ದಾಖಲೆ(pParse, p_ವಿಂಗಡಿಸು, pSelect, reg_ತಳ, n_ತಳ);
  }
  ಆದರೆ( p_ವಿಂಗಡಿಸು->ವಿಂಗಡಿಸು_Flags & SORTFLAG_UseSorter ){
    op = OP_Sorter_ಸೇರಿಸು;
  }ಇಲ್ಲದಿದ್ದರೆ{
    op = OP_Idx_ಸೇರಿಸು;
  }
  sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, op, p_ವಿಂಗಡಿಸು->iECursor, reg_ದಾಖಲೆ,
                       reg_ತಳ+nOBSat, n_ತಳ-nOBSat);
  ಆದರೆ( iSkip ){
    sqlite3VdbeChangeP2(v, iSkip,
         p_ವಿಂಗಡಿಸು->labelOBLopt ? p_ವಿಂಗಡಿಸು->labelOBLopt : sqlite3VdbeCurrentAddr(v));
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
  p_ವಿಂಗಡಿಸು->addr_ತಳ್ಳು_ಕೊನೆ = sqlite3VdbeCurrentAddr(v)-1;
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Add code to implement the OFFSET
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂಕೇತ_ಸ್ಥಾನಾಂತರ(
  Vdbe *v,          /* Generate code into this VM */
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ,      /* Register holding the offset counter */
  ಪೂರ್ಣಾಂಕ iContinue     /* Jump here to skip the current record */
){
  ಆದರೆ( i_ಸ್ಥಾನಾಂತರ>0 ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_IfPos, i_ಸ್ಥಾನಾಂತರ, iContinue, 1); VdbeCoverage(v);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_Distinct(
  Parse *pParse,     /* Parsing and code generating context */
  ಪೂರ್ಣಾಂಕ eTnctType,     /* WHERE_DISTINCT_* value */
  ಪೂರ್ಣಾಂಕ iTab,          /* A sorting index used to test for distinctness */
  ಪೂರ್ಣಾಂಕ addrRepeat,    /* Jump to here if not distinct */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,  /* Expression for each element */
  ಪೂರ್ಣಾಂಕ regElem        /* First element */
){
  ಪೂರ್ಣಾಂಕ iRet = 0;
  ಪೂರ್ಣಾಂಕ n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು = pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
  Vdbe *v = pParse->pVdbe;

  ಆಯ್ಕೆ( eTnctType ){
    ಸ್ಥಿತಿ WHERE_DISTINCT_ORDERED: {
      ಪೂರ್ಣಾಂಕ i;
      ಪೂರ್ಣಾಂಕ iJump;              /* Jump destination */
      ಪೂರ್ಣಾಂಕ regPrev;            /* Previous row content */

      /* Allocate space for the previous row */
      iRet = regPrev = pParse->nMem+1;
      pParse->nMem += n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು;

      iJump = sqlite3VdbeCurrentAddr(v) + n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು;
      ಗಾಗಿ(i=0; i<n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು; i++){
        CollSeq *pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
        ಆದರೆ( i<n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು-1 ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Ne, regElem+i, iJump, regPrev+i);
          VdbeCoverage(v);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Eq, regElem+i, addrRepeat, regPrev+i);
          VdbeCoverage(v);
         }
        sqlite3VdbeChangeP4(v, -1, (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)pColl, P4_COLLSEQ);
        sqlite3VdbeChangeP5(v, SQLITE_NULLEQ);
      }
      ಪ್ರತಿಜ್ಞಾ( sqlite3VdbeCurrentAddr(v)==iJump || pParse->db->mallocFailed );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Copy, regElem, regPrev, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು-1);
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ WHERE_DISTINCT_UNIQUE: {
      /* nothing to do */
      ನಿಲ್ಲಿಸು;
    }

    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಪೂರ್ಣಾಂಕ r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, iTab, addrRepeat, regElem, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು);
      VdbeCoverage(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, regElem, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು, r1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iTab, r1, regElem, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು);
      sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);
      iRet = iTab;
      ನಿಲ್ಲಿಸು;
    }
  }

  ಹಿಂತಿರುಗಿ iRet;
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
  ಪೂರ್ಣಾಂಕ eTnctType,     /* WHERE_DISTINCT_* value */
  ಪೂರ್ಣಾಂಕ iVal,          /* Value returned by codeDistinct() */
  ಪೂರ್ಣಾಂಕ iOpenEphAddr   /* Address of OP_OpenEphemeral instruction for iTab */
){
  ಆದರೆ( pParse->nErr==0
   && (eTnctType==WHERE_DISTINCT_UNIQUE || eTnctType==WHERE_DISTINCT_ORDERED)
  ){
    Vdbe *v = pParse->pVdbe;
    sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, iOpenEphAddr);
    ಆದರೆ( sqlite3VdbeGetOp(v, iOpenEphAddr+1)->opcode==OP_Explain ){
      sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, iOpenEphAddr+1);
    }
    ಆದರೆ( eTnctType==WHERE_DISTINCT_ORDERED ){
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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
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
  ವಿಂಗಡಿಸು_Ctx *p_ವಿಂಗಡಿಸು,                 /* Sorter context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,               /* Expressions destined for sorter */
  ಅಭಿವ್ಯಕ್ತಿ_List **ppExtra              /* Expressions to append to sorter record */
){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ nDefer = 0;
  ಅಭಿವ್ಯಕ್ತಿ_List *pExtra = 0;
  ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ = &pEList->a[i];
    ಆದರೆ( p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು==0 ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ;
      ಕೋಷ್ಟಕ *pTab;
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ
       && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ>=0
       && ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) )
       && (pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab)!=0
       && Is_Ordinary_ಕೋಷ್ಟಕ(pTab)
       && (pTab->a_ಲಂಬಸಾಲು[p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ].ಲಂಬಸಾಲು_Flags & COLFLAG_SORTERREF)!=0
      ){
        ಪೂರ್ಣಾಂಕ j;
        ಗಾಗಿ(j=0; j<nDefer; j++){
          ಆದರೆ( p_ವಿಂಗಡಿಸು->aDefer[j].iCsr==p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ ) ನಿಲ್ಲಿಸು;
        }
        ಆದರೆ( j==nDefer ){
          ಆದರೆ( nDefer==Array_ಗಾತ್ರ(p_ವಿಂಗಡಿಸು->aDefer) ){
            ಮುಂದುವರಿ;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ = 1;
            ಪೂರ್ಣಾಂಕ k;
            ಸೂಚ್ಯಂಕ *pPk = 0;
            ಆದರೆ( !HasRowid(pTab) ){
              pPk = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
              n_ಕೀಲಿ = pPk->n_ಕೀಲಿ_ಲಂಬಸಾಲು;
            }
            ಗಾಗಿ(k=0; k<n_ಕೀಲಿ; k++){
              ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3PExpr(pParse, TK_ಸ್ತಂಭ, 0, 0);
              ಆದರೆ( pNew ){
                pNew->i_ಕೋಷ್ಟಕ = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ;
                ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(pNew) );
                pNew->y.pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
                pNew->i_ಸ್ತಂಭ = pPk ? pPk->ai_ಸ್ತಂಭ[k] : -1;
                pExtra = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pExtra, pNew);
              }
            }
            p_ವಿಂಗಡಿಸು->aDefer[nDefer].pTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
            p_ವಿಂಗಡಿಸು->aDefer[nDefer].iCsr = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ;
            p_ವಿಂಗಡಿಸು->aDefer[nDefer].n_ಕೀಲಿ = n_ಕೀಲಿ;
            nDefer++;
          }
        }
        p_ಅಂಶ->fg.bSorterRef = 1;
      }
    }
  }
  p_ವಿಂಗಡಿಸು->nDefer = (u8)nDefer;
  *ppExtra = pExtra;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3CursorBloomable(Parse *pParse, ಪೂರ್ಣಾಂಕ iCur, ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು){
  ಪೂರ್ಣಾಂಕ i,k;
  Vdbe *v = pParse->pVdbe;
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ 1;
  ಪ್ರತಿಜ್ಞಾ( v );
  ಗಾಗಿ(k=sqlite3VdbeCurrentAddr(v)-1; k>0; k--){
    ಸ್ಥಿರಾಂಕ VdbeOp *pOp = sqlite3VdbeGetOp(v, k);
    ಸ್ಥಿರಾಂಕ ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
    ಆದರೆ( pOp->p1!=iCur ) ಮುಂದುವರಿ;
    ಆದರೆ( pOp->opcode!=OP_OpenRead
     && pOp->opcode!=OP_OpenWrite
     && pOp->opcode!=OP_OpenEphemeral
    ){
      ಮುಂದುವರಿ;
    }
    ಪ್ರತಿಜ್ಞಾ( pOp->p4type==P4_KEYINFO );
    p_ಕೀಲಿ_Info = (ಸ್ಥಿರಾಂಕ ಕೀಲಿ_Info*)pOp->p4.p_ಕೀಲಿ_Info;
    ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Info!=0 );
    ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
      ಪ್ರತಿಜ್ಞಾ( sqlite3_Is_ದ್ವಿಮಾನ(p_ಕೀಲಿ_Info->aColl[i]) );
    }
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */


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
  ಪೂರ್ಣಾಂಕ srcTab,             /* Pull data from this table if non-negative */
  ವಿಂಗಡಿಸು_Ctx *p_ವಿಂಗಡಿಸು,         /* If not NULL, info on how to process ORDER BY */
  DistinctCtx *pDistinct, /* If not NULL, info on how to process DISTINCT */
  SelectDest *pDest,      /* How to dispose of the results */
  ಪೂರ್ಣಾಂಕ iContinue,          /* Jump here to continue with next row */
  ಪೂರ್ಣಾಂಕ iBreak              /* Jump here to break out of the inner loop */
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ hasDistinct;            /* True if the DISTINCT keyword is present */
  ಪೂರ್ಣಾಂಕ eDest = pDest->eDest;   /* How to dispose of results */
  ಪೂರ್ಣಾಂಕ iParm = pDest->iSDParm; /* First argument to disposal method */
  ಪೂರ್ಣಾಂಕ n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು;             /* Number of result columns */
  ಪೂರ್ಣಾಂಕ n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg = 0;         /* Number of extra registers before regResult */
  ಅಡ್ಡಸಾಲು_ತುಂಬು_Info s_ಅಡ್ಡಸಾಲು_ತುಂಬು_Info;   /* Info for deferred row loading */

  /* Usually, regResult is the first cell in an array of memory cells
  ** containing the current result row. In this case regOrig is set to the
  ** same value. However, if the results are being sent to the sorter, the
  ** values for any expressions that are also part of the sort-key are omitted
  ** from this array. In this case regOrig is set to zero.  */
  ಪೂರ್ಣಾಂಕ reg_ಫಲಿತಾಂಶ;              /* Start of memory holding current results */
  ಪೂರ್ಣಾಂಕ regOrig;                /* Start of memory holding full result (or 0) */

  ಪ್ರತಿಜ್ಞಾ( v );
  ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );
  hasDistinct = pDistinct ? pDistinct->eTnctType : WHERE_DISTINCT_NOOP;
  ಆದರೆ( p_ವಿಂಗಡಿಸು && p_ವಿಂಗಡಿಸು->p_ಕ್ರಮ_By==0 ) p_ವಿಂಗಡಿಸು = 0;
  ಆದರೆ( p_ವಿಂಗಡಿಸು==0 && !hasDistinct ){
    ಪ್ರತಿಜ್ಞಾ( iContinue!=0 );
    ಸಂಕೇತ_ಸ್ಥಾನಾಂತರ(v, p->i_ಸ್ಥಾನಾಂತರ, iContinue);
  }

  /* Pull the requested columns.
  */
  n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು = p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;

  ಆದರೆ( pDest->iSdst==0 ){
    ಆದರೆ( p_ವಿಂಗಡಿಸು ){
      n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg = p_ವಿಂಗಡಿಸು->p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( !(p_ವಿಂಗಡಿಸು->ವಿಂಗಡಿಸು_Flags & SORTFLAG_UseSorter) ) n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg++;
      pParse->nMem += n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg;
    }
    pDest->iSdst = pParse->nMem+1;
    pParse->nMem += n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pDest->iSdst+n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು > pParse->nMem ){
    /* This is an error condition that can result, for example, when a SELECT
    ** on the right-hand side of an INSERT contains more result columns than
    ** there are columns in the table on the left.  The error will be caught
    ** and reported later.  But we need to make sure enough memory is allocated
    ** to avoid other spurious errors in the meantime. */
    pParse->nMem += n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು;
  }
  pDest->nSdst = n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು;
  regOrig = reg_ಫಲಿತಾಂಶ = pDest->iSdst;
  ಆದರೆ( srcTab>=0 ){
    ಗಾಗಿ(i=0; i<n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು; i++){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, srcTab, i, reg_ಫಲಿತಾಂಶ+i);
      VdbeComment((v, "%s", p->pEList->a[i].zEName));
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( eDest!=SRT_Exists ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
    ಅಭಿವ್ಯಕ್ತಿ_List *pExtra = 0;
#ಆದರೆ_ಅಂತ್ಯ
    /* If the destination is an EXISTS(...) expression, the actual
    ** values returned by the SELECT are not required.
    */
    u8 ecelFlags;    /* "ecel" is an abbreviation of "ExprCodeExprList" */
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
    ಆದರೆ( eDest==SRT_Mem || eDest==SRT_Output || eDest==SRT_Coroutine ){
      ecelFlags = SQLITE_ECEL_DUP;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ecelFlags = 0;
    }
    ಆದರೆ( p_ವಿಂಗಡಿಸು && hasDistinct==0 && eDest!=SRT_EphemTab && eDest!=SRT_ಕೋಷ್ಟಕ ){
      /* For each expression in p->pEList that is a copy of an expression in
      ** the ORDER BY clause (pSort->pOrderBy), set the associated
      ** iOrderByCol value to one more than the index of the ORDER BY
      ** expression within the sort-key that pushOntoSorter() will generate.
      ** This allows the p->pEList field to be omitted from the sorted record,
      ** saving space and CPU cycles.  */
      ecelFlags |= (SQLITE_ECEL_OMITREF|SQLITE_ECEL_REF);

      ಗಾಗಿ(i=p_ವಿಂಗಡಿಸು->nOBSat; i<p_ವಿಂಗಡಿಸು->p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
        ಪೂರ್ಣಾಂಕ j;
        ಆದರೆ( (j = p_ವಿಂಗಡಿಸು->p_ಕ್ರಮ_By->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು)>0 ){
          p->pEList->a[j-1].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = i+1-p_ವಿಂಗಡಿಸು->nOBSat;
        }
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
      select_ಅಭಿವ್ಯಕ್ತಿ_Defer(pParse, p_ವಿಂಗಡಿಸು, p->pEList, &pExtra);
      ಆದರೆ( pExtra && pParse->db->mallocFailed==0 ){
        /* If there are any extra PK columns to add to the sorter records,
        ** allocate extra memory cells and adjust the OpenEphemeral
        ** instruction to account for the larger records. This is only
        ** required if there are one or more WITHOUT ROWID tables with
        ** composite primary keys in the SortCtx.aDefer[] array.  */
        VdbeOp *pOp = sqlite3VdbeGetOp(v, p_ವಿಂಗಡಿಸು->addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ);
        pOp->p2 += (pExtra->n_ಅಭಿವ್ಯಕ್ತಿ - p_ವಿಂಗಡಿಸು->nDefer);
        pOp->p4.p_ಕೀಲಿ_Info->n_All_ಕ್ಷೇತ್ರ += (pExtra->n_ಅಭಿವ್ಯಕ್ತಿ - p_ವಿಂಗಡಿಸು->nDefer);
        pParse->nMem += pExtra->n_ಅಭಿವ್ಯಕ್ತಿ;
      }
#ಆದರೆ_ಅಂತ್ಯ

      /* Adjust nResultCol to account for columns that are omitted
      ** from the sorter by the optimizations in this branch */
      pEList = p->pEList;
      ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
        ಆದರೆ( pEList->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು>0
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
         || pEList->a[i].fg.bSorterRef
#ಆದರೆ_ಅಂತ್ಯ
        ){
          n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು--;
          regOrig = 0;
        }
      }

      testcase( regOrig );
      testcase( eDest==SRT_ಗಣ );
      testcase( eDest==SRT_Mem );
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      ಪ್ರತಿಜ್ಞಾ( eDest==SRT_ಗಣ || eDest==SRT_Mem
           || eDest==SRT_Coroutine || eDest==SRT_Output
           || eDest==SRT_Upfrom );
    }
    s_ಅಡ್ಡಸಾಲು_ತುಂಬು_Info.reg_ಫಲಿತಾಂಶ = reg_ಫಲಿತಾಂಶ;
    s_ಅಡ್ಡಸಾಲು_ತುಂಬು_Info.ecelFlags = ecelFlags;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
    s_ಅಡ್ಡಸಾಲು_ತುಂಬು_Info.pExtra = pExtra;
    s_ಅಡ್ಡಸಾಲು_ತುಂಬು_Info.reg_Extra_ಫಲಿತಾಂಶ = reg_ಫಲಿತಾಂಶ + n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು;
    ಆದರೆ( pExtra ) n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು += pExtra->n_ಅಭಿವ್ಯಕ್ತಿ;
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p->i_ಎಲ್ಲೆ
     && (ecelFlags & SQLITE_ECEL_OMITREF)!=0
     && n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg>0
    ){
      ಪ್ರತಿಜ್ಞಾ( p_ವಿಂಗಡಿಸು!=0 );
      ಪ್ರತಿಜ್ಞಾ( hasDistinct==0 );
      p_ವಿಂಗಡಿಸು->p_Deferred_ಅಡ್ಡಸಾಲು_ತುಂಬು = &s_ಅಡ್ಡಸಾಲು_ತುಂಬು_Info;
      regOrig = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      inner_Loop_ತುಂಬು_ಅಡ್ಡಸಾಲು(pParse, p, &s_ಅಡ್ಡಸಾಲು_ತುಂಬು_Info);
    }
  }

  /* If the DISTINCT keyword was present on the SELECT statement
  ** and this row has been seen before, then do not make this row
  ** part of the result.
  */
  ಆದರೆ( hasDistinct ){
    ಪೂರ್ಣಾಂಕ eType = pDistinct->eTnctType;
    ಪೂರ್ಣಾಂಕ iTab = pDistinct->tabTnct;
    ಪ್ರತಿಜ್ಞಾ( n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು==p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
    iTab = ಸಂಕೇತ_Distinct(pParse, eType, iTab, iContinue, p->pEList, reg_ಫಲಿತಾಂಶ);
    fixDistinctOpenEph(pParse, eType, iTab, pDistinct->addrTnct);
    ಆದರೆ( p_ವಿಂಗಡಿಸು==0 ){
      ಸಂಕೇತ_ಸ್ಥಾನಾಂತರ(v, p->i_ಸ್ಥಾನಾಂತರ, iContinue);
    }
  }

  ಆಯ್ಕೆ( eDest ){
    /* Store the result as data using a unique key.
    */
    ಸ್ಥಿತಿ SRT_Fifo:
    ಸ್ಥಿತಿ SRT_DistFifo:
    ಸ್ಥಿತಿ SRT_ಕೋಷ್ಟಕ:
    ಸ್ಥಿತಿ SRT_EphemTab: {
      ಪೂರ್ಣಾಂಕ r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg+1);
      testcase( eDest==SRT_ಕೋಷ್ಟಕ );
      testcase( eDest==SRT_EphemTab );
      testcase( eDest==SRT_Fifo );
      testcase( eDest==SRT_DistFifo );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg_ಫಲಿತಾಂಶ, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು, r1+n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg);
#ಆದರೆ !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ಆದರೆ( eDest==SRT_ಕೋಷ್ಟಕ && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
      ಆದರೆ( eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index. If the current row is already present
        ** in the index, do not write it to the output. If not, add the
        ** current row to the index and proceed with writing it to the
        ** output table as well.  */
        ಪೂರ್ಣಾಂಕ addr = sqlite3VdbeCurrentAddr(v) + 4;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, iParm+1, addr, r1, 0);
        VdbeCoverage(v);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iParm+1, r1,reg_ಫಲಿತಾಂಶ,n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು);
        ಪ್ರತಿಜ್ಞಾ( p_ವಿಂಗಡಿಸು==0 );
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( p_ವಿಂಗಡಿಸು ){
        ಪ್ರತಿಜ್ಞಾ( reg_ಫಲಿತಾಂಶ==regOrig );
        ತಳ್ಳು_Onto_Sorter(pParse, p_ವಿಂಗಡಿಸು, p, r1+n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg, regOrig, 1, n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ r2 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, iParm, r2);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iParm, r1, r2);
        sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
        sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r2);
      }
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, r1, n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg+1);
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ SRT_Upfrom: {
      ಆದರೆ( p_ವಿಂಗಡಿಸು ){
        ತಳ್ಳು_Onto_Sorter(
            pParse, p_ವಿಂಗಡಿಸು, p, reg_ಫಲಿತಾಂಶ, regOrig, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು, n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ i2 = pDest->iSDParm2;
        ಪೂರ್ಣಾಂಕ r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);

        /* If the UPDATE FROM join is an aggregate that matches no rows, it
        ** might still be trying to return one row, because that is what
        ** aggregates do.  Don't record that empty row in the output table. */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IsNull, reg_ಫಲಿತಾಂಶ, iBreak); VdbeCoverage(v);

        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ,
                          reg_ಫಲಿತಾಂಶ+(i2<0), n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು-(i2<0), r1);
        ಆದರೆ( i2<0 ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iParm, r1, reg_ಫಲಿತಾಂಶ);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, reg_ಫಲಿತಾಂಶ, i2);
        }
      }
      ನಿಲ್ಲಿಸು;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)" construct,
    ** then there should be a single item on the stack.  Write this
    ** item into the set table with bogus data.
    */
    ಸ್ಥಿತಿ SRT_ಗಣ: {
      ಆದರೆ( p_ವಿಂಗಡಿಸು ){
        /* At first glance you would think we could optimize out the
        ** ORDER BY in this case since the order of entries in the set
        ** does not matter.  But there might be a LIMIT clause, in which
        ** case the order does matter */
        ತಳ್ಳು_Onto_Sorter(
            pParse, p_ವಿಂಗಡಿಸು, p, reg_ಫಲಿತಾಂಶ, regOrig, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು, n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg);
        pDest->iSDParm2 = 0; /* Signal that any Bloom filter is unpopulated */
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
        ಪ್ರತಿಜ್ಞಾ( sqlite3Strlen30(pDest->zAffSdst)==n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Make_ದಾಖಲೆ, reg_ಫಲಿತಾಂಶ, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು,
            r1, pDest->zAffSdst, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, reg_ಫಲಿತಾಂಶ, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು);
        ಆದರೆ( pDest->iSDParm2 ){
          ಪ್ರತಿಜ್ಞಾ( sqlite3CursorBloomable(pParse,iParm,n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು) );
          sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Filter_ಕೂಡಿಸು, pDest->iSDParm2, 0,
                               reg_ಫಲಿತಾಂಶ, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು);
          ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
        }
        sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);
      }
      ನಿಲ್ಲಿಸು;
    }


    /* If any row exist in the result set, record that fact and abort.
    */
    ಸ್ಥಿತಿ SRT_Exists: {
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, iParm);
      /* The LIMIT clause will terminate the loop for us */
      ನಿಲ್ಲಿಸು;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell or array of
    ** memory cells and break out of the scan loop.
    */
    ಸ್ಥಿತಿ SRT_Mem: {
      ಆದರೆ( p_ವಿಂಗಡಿಸು ){
        ಪ್ರತಿಜ್ಞಾ( n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು<=pDest->nSdst );
        ತಳ್ಳು_Onto_Sorter(
            pParse, p_ವಿಂಗಡಿಸು, p, reg_ಫಲಿತಾಂಶ, regOrig, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು, n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg);
        pDest->iSDParm = reg_ಫಲಿತಾಂಶ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು==pDest->nSdst );
        ಆದರೆ( reg_ಫಲಿತಾಂಶ!=iParm ){
          /* This occurs in cases where the SELECT had both a DISTINCT and
          ** an OFFSET clause.  */
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Copy, reg_ಫಲಿತಾಂಶ, iParm, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು-1);
        }
        /* The LIMIT clause will jump out of the loop for us */
      }
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_OMIT_SUBQUERY */

    ಸ್ಥಿತಿ SRT_Coroutine:       /* Send data to a co-routine */
    ಸ್ಥಿತಿ SRT_Output: {        /* Return the results */
      testcase( eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      ಆದರೆ( p_ವಿಂಗಡಿಸು ){
        ತಳ್ಳು_Onto_Sorter(pParse, p_ವಿಂಗಡಿಸು, p, reg_ಫಲಿತಾಂಶ, regOrig, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು,
                       n_ಪೂರ್ವಪ್ರತ್ಯಯ_Reg);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( eDest==SRT_Coroutine ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Yield, pDest->iSDParm);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, reg_ಫಲಿತಾಂಶ, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು);
      }
      ನಿಲ್ಲಿಸು;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    ಸ್ಥಿತಿ SRT_Dist_ಸರತಿ:
    ಸ್ಥಿತಿ SRT_ಸರತಿ: {
      ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ;
      ಪೂರ್ಣಾಂಕ r1, r2, r3;
      ಪೂರ್ಣಾಂಕ addrTest = 0;
      ಅಭಿವ್ಯಕ್ತಿ_List *pSO;
      pSO = pDest->p_ಕ್ರಮ_By;
      ಪ್ರತಿಜ್ಞಾ( pSO );
      n_ಕೀಲಿ = pSO->n_ಅಭಿವ್ಯಕ್ತಿ;
      r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      r2 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_ಕೀಲಿ+2);
      r3 = r2+n_ಕೀಲಿ+1;
      ಆದರೆ( eDest==SRT_Dist_ಸರತಿ ){
        /* If the destination is DistQueue, then cursor (iParm+1) is open
        ** on a second ephemeral index that holds all values every previously
        ** added to the queue. */
        addrTest = sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_ಸಿಕ್ಕಿತು, iParm+1, 0,
                                        reg_ಫಲಿತಾಂಶ, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು);
        VdbeCoverage(v);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg_ಫಲಿತಾಂಶ, n_ಫಲಿತಾಂಶ_ಲಂಬಸಾಲು, r3);
      ಆದರೆ( eDest==SRT_Dist_ಸರತಿ ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Idx_ಸೇರಿಸು, iParm+1, r3);
        sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
      }
      ಗಾಗಿ(i=0; i<n_ಕೀಲಿ; i++){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_SCopy,
                          reg_ಫಲಿತಾಂಶ + pSO->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು - 1,
                          r2+i);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Sequence, iParm, r2+n_ಕೀಲಿ);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, r2, n_ಕೀಲಿ+2, r1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, r2, n_ಕೀಲಿ+2);
      ಆದರೆ( addrTest ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrTest);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, r2, n_ಕೀಲಿ+2);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_CTE */



#ಆದರೆ !defined(SQLITE_OMIT_TRIGGER)
    /* Discard the results.  This is used for SELECT statements inside
    ** the body of a TRIGGER.  The purpose of such selects is to call
    ** user-defined functions that have side effects.  We do not care
    ** about the actual results of the select.
    */
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಪ್ರತಿಜ್ಞಾ( eDest==SRT_Discard );
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }

  /* Jump to the end of the loop if the LIMIT is reached.  Except, if
  ** there is a sorter, in which case the sorter has already limited
  ** the output for us.
  */
  ಆದರೆ( p_ವಿಂಗಡಿಸು==0 && p->i_ಎಲ್ಲೆ ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_DecrJumpZero, p->i_ಎಲ್ಲೆ, iBreak); VdbeCoverage(v);
  }
}

/*
** Allocate a KeyInfo object sufficient for an index of N key columns and
** X extra columns.
*/
ಕೀಲಿ_Info *sqlite3_ಕೀಲಿ_Info_Alloc(sqlite3 *db, ಪೂರ್ಣಾಂಕ N, ಪೂರ್ಣಾಂಕ X){
  ಪೂರ್ಣಾಂಕ nExtra = (N+X)*(ಗಾತ್ರ(CollSeq*)+1);
  ಕೀಲಿ_Info *p;
  ಪ್ರತಿಜ್ಞಾ( X>=0 );
  ಆದರೆ( NEVER(N+X>0xffff) ) ಹಿಂತಿರುಗಿ (ಕೀಲಿ_Info*)sqlite3OomFault(db);
  p = sqlite3DbMallocRawNN(db, SZ_KEYINFO(0) + nExtra);
  ಆದರೆ( p ){
    p->a_ವಿಂಗಡಿಸು_Flags = (u8*)&p->aColl[N+X];
    p->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ = (u16)N;
    p->n_All_ಕ್ಷೇತ್ರ = (u16)(N+X);
    p->enc = ENC(db);
    p->db = db;
    p->nRef = 1;
    memset(p->aColl, 0, nExtra);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ (ಕೀಲಿ_Info*)sqlite3OomFault(db);
  }
  ಹಿಂತಿರುಗಿ p;
}

/*
** Deallocate a KeyInfo object
*/
ಶೂನ್ಯ sqlite3_ಕೀಲಿ_Info_Unref(ಕೀಲಿ_Info *p){
  ಆದರೆ( p ){
    ಪ್ರತಿಜ್ಞಾ( p->db!=0 );
    ಪ್ರತಿಜ್ಞಾ( p->nRef>0 );
    p->nRef--;
    ಆದರೆ( p->nRef==0 ) sqlite3DbNNFreeNN(p->db, p);
  }
}

/*
** Make a new pointer to a KeyInfo object
*/
ಕೀಲಿ_Info *sqlite3_ಕೀಲಿ_Info_Ref(ಕೀಲಿ_Info *p){
  ಆದರೆ( p ){
    ಪ್ರತಿಜ್ಞಾ( p->nRef>0 );
    p->nRef++;
  }
  ಹಿಂತಿರುಗಿ p;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
/*
** Return TRUE if a KeyInfo object can be change.  The KeyInfo object
** can only be changed if this is just a single reference to the object.
**
** This routine is used only inside of assert() statements.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಕೀಲಿ_Info_Is_Writeable(ಕೀಲಿ_Info *p){ ಹಿಂತಿರುಗಿ p->nRef==1; }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */

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
  ಪೂರ್ಣಾಂಕ i_ಆರಂಭ,          /* Begin with this column of pList */
  ಪೂರ್ಣಾಂಕ nExtra           /* Add this many extra columns to the end */
){
  ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ;
  ಕೀಲಿ_Info *pInfo;
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಂಕ i;

  n_ಅಭಿವ್ಯಕ್ತಿ = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
  pInfo = sqlite3_ಕೀಲಿ_Info_Alloc(db, n_ಅಭಿವ್ಯಕ್ತಿ-i_ಆರಂಭ, nExtra+1);
  ಆದರೆ( pInfo ){
    ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(pInfo) );
    ಗಾಗಿ(i=i_ಆರಂಭ, p_ಅಂಶ=pList->a+i_ಆರಂಭ; i<n_ಅಭಿವ್ಯಕ್ತಿ; i++, p_ಅಂಶ++){
      pInfo->aColl[i-i_ಆರಂಭ] = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_NNColl_Seq(pParse, p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ);
      pInfo->a_ವಿಂಗಡಿಸು_Flags[i-i_ಆರಂಭ] = p_ಅಂಶ->fg.ವಿಂಗಡಿಸು_Flags;
    }
  }
  ಹಿಂತಿರುಗಿ pInfo;
}

/*
** Name of the connection operator, used for error messages.
*/
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sqlite3SelectOpName(ಪೂರ್ಣಾಂಕ id){
  ಅಕ್ಷರ *z;
  ಆಯ್ಕೆ( id ){
    ಸ್ಥಿತಿ TK_ALL:       z = "UNION ALL";   ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_INTERSECT: z = "INTERSECT";   ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_EXCEPT:    z = "EXCEPT";      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:           z = "UNION";       ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ z;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN
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
ಸ್ಥಿರ ಶೂನ್ಯ explain_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ(Parse *pParse, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUsage){
  ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s", zUsage));
}

/*
** Assign expression b to lvalue a. A second, no-op, version of this macro
** is provided when SQLITE_OMIT_EXPLAIN is defined. This allows the code
** in sqlite3Select() to assign values to structure member variables that
** only exist if SQLITE_OMIT_EXPLAIN is not defined without polluting the
** code with #ifndef directives.
*/
# define explain_ಗಣ_Integer(a, b) a = b

#ಇಲ್ಲದಿದ್ದರೆ
/* No-op versions of the explainXXX() functions and macros. */
# define explain_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ(y,z)
# define explain_ಗಣ_Integer(y,z)
#ಆದರೆ_ಅಂತ್ಯ


/*
** If the inner loop was generated using a non-null pOrderBy argument,
** then the results were placed in a sorter.  After the loop is terminated
** we need to run the sorter and output the results.  The following
** routine generates the code needed to do that.
*/
ಸ್ಥಿರ ಶೂನ್ಯ generate_ವಿಂಗಡಿಸು_ಬಾಲ(
  Parse *pParse,    /* Parsing context */
  Select *p,        /* The SELECT statement */
  ವಿಂಗಡಿಸು_Ctx *p_ವಿಂಗಡಿಸು,   /* Information on the ORDER BY clause */
  ಪೂರ್ಣಾಂಕ n_ಸ್ತಂಭ,      /* Number of columns of data */
  SelectDest *pDest /* Write the sorted results here */
){
  Vdbe *v = pParse->pVdbe;                     /* The prepared statement */
  ಪೂರ್ಣಾಂಕ addrBreak = p_ವಿಂಗಡಿಸು->label_ಮುಗಿದ;            /* Jump here to exit loop */
  ಪೂರ್ಣಾಂಕ addrContinue = sqlite3VdbeMakeLabel(pParse);/* Jump here for next cycle */
  ಪೂರ್ಣಾಂಕ addr;                       /* Top of output loop. Jump for Next. */
  ಪೂರ್ಣಾಂಕ addrOnce = 0;
  ಪೂರ್ಣಾಂಕ iTab;
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By = p_ವಿಂಗಡಿಸು->p_ಕ್ರಮ_By;
  ಪೂರ್ಣಾಂಕ eDest = pDest->eDest;
  ಪೂರ್ಣಾಂಕ iParm = pDest->iSDParm;
  ಪೂರ್ಣಾಂಕ reg_ಅಡ್ಡಸಾಲು;
  ಪೂರ್ಣಾಂಕ regRowid;
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;
  ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ;                       /* Number of key columns in sorter record */
  ಪೂರ್ಣಾಂಕ i_ವಿಂಗಡಿಸು_Tab;                   /* Sorter cursor to read from */
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ bSeq;                       /* True if sorter record includes seq. no. */
  ಪೂರ್ಣಾಂಕ n_Ref_ಕೀಲಿ = 0;
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *a_ಹೊರಹೋಗು_Ex = p->pEList->a;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
  ಪೂರ್ಣಾಂಕ addrExplain;                /* Address of OP_Explain instruction */
#ಆದರೆ_ಅಂತ್ಯ

  n_ಕೀಲಿ = p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ - p_ವಿಂಗಡಿಸು->nOBSat;
  ಆದರೆ( p_ವಿಂಗಡಿಸು->nOBSat==0 || n_ಕೀಲಿ==1 ){
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR %sORDER BY", p_ವಿಂಗಡಿಸು->nOBSat?"LAST TERM OF ":""
    ));
  }ಇಲ್ಲದಿದ್ದರೆ{
    ExplainQueryPlan2(addrExplain, (pParse, 0,
      "USE TEMP B-TREE FOR LAST %d TERMS OF ORDER BY", n_ಕೀಲಿ
    ));
  }
  sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExplain,p_ವಿಂಗಡಿಸು->addr_ತಳ್ಳು,p_ವಿಂಗಡಿಸು->addr_ತಳ್ಳು_ಕೊನೆ);
  sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Counters(v, addrExplain, addrExplain, p_ವಿಂಗಡಿಸು->addr_ತಳ್ಳು);


  ಪ್ರತಿಜ್ಞಾ( addrBreak<0 );
  ಆದರೆ( p_ವಿಂಗಡಿಸು->label_Bk_ಹೊರಹೋಗು ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, p_ವಿಂಗಡಿಸು->regReturn, p_ವಿಂಗಡಿಸು->label_Bk_ಹೊರಹೋಗು);
    sqlite3VdbeGoto(v, addrBreak);
    sqlite3VdbeResolveLabel(v, p_ವಿಂಗಡಿಸು->label_Bk_ಹೊರಹೋಗು);
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
  /* Open any cursors needed for sorter-reference expressions */
  ಗಾಗಿ(i=0; i<p_ವಿಂಗಡಿಸು->nDefer; i++){
    ಕೋಷ್ಟಕ *pTab = p_ವಿಂಗಡಿಸು->aDefer[i].pTab;
    ಪೂರ್ಣಾಂಕ iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pTab->pSchema);
    sqlite3_Open_ಕೋಷ್ಟಕ(pParse, p_ವಿಂಗಡಿಸು->aDefer[i].iCsr, iDb, pTab, OP_OpenRead);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, p_ವಿಂಗಡಿಸು->aDefer[i].iCsr,
                      sqlite3VdbeCurrentAddr(v)+1);
    VdbeCoverage(v);
    n_Ref_ಕೀಲಿ = MAX(n_Ref_ಕೀಲಿ, p_ವಿಂಗಡಿಸು->aDefer[i].n_ಕೀಲಿ);
  }
#ಆದರೆ_ಅಂತ್ಯ

  iTab = p_ವಿಂಗಡಿಸು->iECursor;
  ಆದರೆ( eDest==SRT_Output || eDest==SRT_Coroutine || eDest==SRT_Mem ){
    ಆದರೆ( eDest==SRT_Mem && p->i_ಸ್ಥಾನಾಂತರ ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Null, 0, pDest->iSdst);
    }
    regRowid = 0;
    reg_ಅಡ್ಡಸಾಲು = pDest->iSdst;
  }ಇಲ್ಲದಿದ್ದರೆ{
    regRowid = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
    ಆದರೆ( eDest==SRT_EphemTab || eDest==SRT_ಕೋಷ್ಟಕ ){
      reg_ಅಡ್ಡಸಾಲು = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      n_ಸ್ತಂಭ = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      reg_ಅಡ್ಡಸಾಲು = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_ಸ್ತಂಭ);
    }
  }
  ಆದರೆ( p_ವಿಂಗಡಿಸು->ವಿಂಗಡಿಸು_Flags & SORTFLAG_UseSorter ){
    ಪೂರ್ಣಾಂಕ reg_ವಿಂಗಡಿಸು_ಹೊರಹೋಗು = ++pParse->nMem;
    i_ವಿಂಗಡಿಸು_Tab = pParse->nTab++;
    ಆದರೆ( p_ವಿಂಗಡಿಸು->label_Bk_ಹೊರಹೋಗು ){
      addrOnce = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once); VdbeCoverage(v);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenPseudo, i_ವಿಂಗಡಿಸು_Tab, reg_ವಿಂಗಡಿಸು_ಹೊರಹೋಗು,
        n_ಕೀಲಿ+1+n_ಸ್ತಂಭ+n_Ref_ಕೀಲಿ);
    ಆದರೆ( addrOnce ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addrOnce);
    addr = 1 + sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Sorter_ವಿಂಗಡಿಸು, iTab, addrBreak);
    VdbeCoverage(v);
    ಪ್ರತಿಜ್ಞಾ( p->i_ಎಲ್ಲೆ==0 && p->i_ಸ್ಥಾನಾಂತರ==0 );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Sorter_ದತ್ತಾಂಶ, iTab, reg_ವಿಂಗಡಿಸು_ಹೊರಹೋಗು, i_ವಿಂಗಡಿಸು_Tab);
    bSeq = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    addr = 1 + sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ವಿಂಗಡಿಸು, iTab, addrBreak); VdbeCoverage(v);
    ಸಂಕೇತ_ಸ್ಥಾನಾಂತರ(v, p->i_ಸ್ಥಾನಾಂತರ, addrContinue);
    i_ವಿಂಗಡಿಸು_Tab = iTab;
    bSeq = 1;
    ಆದರೆ( p->i_ಸ್ಥಾನಾಂತರ>0 ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಕೂಡಿಸು_Imm, p->i_ಎಲ್ಲೆ, -1);
    }
  }
  ಗಾಗಿ(i=0, i_ಲಂಬಸಾಲು=n_ಕೀಲಿ+bSeq-1; i<n_ಸ್ತಂಭ; i++){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
    ಆದರೆ( a_ಹೊರಹೋಗು_Ex[i].fg.bSorterRef ) ಮುಂದುವರಿ;
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( a_ಹೊರಹೋಗು_Ex[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು==0 ) i_ಲಂಬಸಾಲು++;
  }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
  ಆದರೆ( p_ವಿಂಗಡಿಸು->nDefer ){
    ಪೂರ್ಣಾಂಕ i_ಕೀಲಿ = i_ಲಂಬಸಾಲು+1;
    ಪೂರ್ಣಾಂಕ reg_ಕೀಲಿ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_Ref_ಕೀಲಿ);

    ಗಾಗಿ(i=0; i<p_ವಿಂಗಡಿಸು->nDefer; i++){
      ಪೂರ್ಣಾಂಕ iCsr = p_ವಿಂಗಡಿಸು->aDefer[i].iCsr;
      ಕೋಷ್ಟಕ *pTab = p_ವಿಂಗಡಿಸು->aDefer[i].pTab;
      ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ = p_ವಿಂಗಡಿಸು->aDefer[i].n_ಕೀಲಿ;

      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Null_ಅಡ್ಡಸಾಲು, iCsr);
      ಆದರೆ( HasRowid(pTab) ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, i_ವಿಂಗಡಿಸು_Tab, i_ಕೀಲಿ++, reg_ಕೀಲಿ);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_SeekRowid, iCsr,
            sqlite3VdbeCurrentAddr(v)+1, reg_ಕೀಲಿ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ k;
        ಪೂರ್ಣಾಂಕ iJmp;
        ಪ್ರತಿಜ್ಞಾ( sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab)->n_ಕೀಲಿ_ಲಂಬಸಾಲು==n_ಕೀಲಿ );
        ಗಾಗಿ(k=0; k<n_ಕೀಲಿ; k++){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, i_ವಿಂಗಡಿಸು_Tab, i_ಕೀಲಿ++, reg_ಕೀಲಿ+k);
        }
        iJmp = sqlite3VdbeCurrentAddr(v);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_SeekGE, iCsr, iJmp+2, reg_ಕೀಲಿ, n_ಕೀಲಿ);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_IdxLE, iCsr, iJmp+3, reg_ಕೀಲಿ, n_ಕೀಲಿ);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Null_ಅಡ್ಡಸಾಲು, iCsr);
      }
    }
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, reg_ಕೀಲಿ, n_Ref_ಕೀಲಿ);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಗಾಗಿ(i=n_ಸ್ತಂಭ-1; i>=0; i--){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_SORTER_REFERENCES
    ಆದರೆ( a_ಹೊರಹೋಗು_Ex[i].fg.bSorterRef ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, a_ಹೊರಹೋಗು_Ex[i].p_ಅಭಿವ್ಯಕ್ತಿ, reg_ಅಡ್ಡಸಾಲು+i);
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      ಪೂರ್ಣಾಂಕ iRead;
      ಆದರೆ( a_ಹೊರಹೋಗು_Ex[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು ){
        iRead = a_ಹೊರಹೋಗು_Ex[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು-1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        iRead = i_ಲಂಬಸಾಲು--;
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, i_ವಿಂಗಡಿಸು_Tab, iRead, reg_ಅಡ್ಡಸಾಲು+i);
      VdbeComment((v, "%s", a_ಹೊರಹೋಗು_Ex[i].zEName));
    }
  }
  sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExplain, addrExplain, -1);
  ಆಯ್ಕೆ( eDest ){
    ಸ್ಥಿತಿ SRT_ಕೋಷ್ಟಕ:
    ಸ್ಥಿತಿ SRT_EphemTab: {
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, i_ವಿಂಗಡಿಸು_Tab, n_ಕೀಲಿ+bSeq, reg_ಅಡ್ಡಸಾಲು);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, iParm, regRowid);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iParm, reg_ಅಡ್ಡಸಾಲು, regRowid);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    ಸ್ಥಿತಿ SRT_ಗಣ: {
      ಪ್ರತಿಜ್ಞಾ( n_ಸ್ತಂಭ==sqlite3Strlen30(pDest->zAffSdst) );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Make_ದಾಖಲೆ, reg_ಅಡ್ಡಸಾಲು, n_ಸ್ತಂಭ, regRowid,
                        pDest->zAffSdst, n_ಸ್ತಂಭ);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iParm, regRowid, reg_ಅಡ್ಡಸಾಲು, n_ಸ್ತಂಭ);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ SRT_Mem: {
      /* The LIMIT clause will terminate the loop for us */
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ SRT_Upfrom: {
      ಪೂರ್ಣಾಂಕ i2 = pDest->iSDParm2;
      ಪೂರ್ಣಾಂಕ r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ,reg_ಅಡ್ಡಸಾಲು+(i2<0),n_ಸ್ತಂಭ-(i2<0),r1);
      ಆದರೆ( i2<0 ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iParm, r1, reg_ಅಡ್ಡಸಾಲು);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, reg_ಅಡ್ಡಸಾಲು, i2);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಪ್ರತಿಜ್ಞಾ( eDest==SRT_Output || eDest==SRT_Coroutine );
      testcase( eDest==SRT_Output );
      testcase( eDest==SRT_Coroutine );
      ಆದರೆ( eDest==SRT_Output ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, pDest->iSdst, n_ಸ್ತಂಭ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Yield, pDest->iSDParm);
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( regRowid ){
    ಆದರೆ( eDest==SRT_ಗಣ ){
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, reg_ಅಡ್ಡಸಾಲು, n_ಸ್ತಂಭ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, reg_ಅಡ್ಡಸಾಲು);
    }
    sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regRowid);
  }
  /* The bottom of the loop
  */
  sqlite3VdbeResolveLabel(v, addrContinue);
  ಆದರೆ( p_ವಿಂಗಡಿಸು->ವಿಂಗಡಿಸು_Flags & SORTFLAG_UseSorter ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Sorter_ಮುಂದಿನ, iTab, addr); VdbeCoverage(v);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, iTab, addr); VdbeCoverage(v);
  }
  sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExplain, sqlite3VdbeCurrentAddr(v)-1, -1);
  ಆದರೆ( p_ವಿಂಗಡಿಸು->regReturn ) sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Return, p_ವಿಂಗಡಿಸು->regReturn);
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_METADATA
# define ಸ್ತಂಭ_Type(A,B,C,D,E) ಸ್ತಂಭ_Type_Impl(A,B,C,D,E)
#ಇಲ್ಲದಿದ್ದರೆ /* if !defined(SQLITE_ENABLE_COLUMN_METADATA) */
# define ಸ್ತಂಭ_Type(A,B,C,D,E) ಸ್ತಂಭ_Type_Impl(A,B)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ತಂಭ_Type_Impl(
  NameContext *pNC,
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_METADATA
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ
#ಇಲ್ಲದಿದ್ದರೆ
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pzOrigDb,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pzOrigTab,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **pz_Orig_ಲಂಬಸಾಲು
#ಆದರೆ_ಅಂತ್ಯ
){
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zType = 0;
  ಪೂರ್ಣಾಂಕ j;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_METADATA
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zOrigDb = 0;
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *zOrigTab = 0;
  ಅಕ್ಷರ ಸ್ಥಿರಾಂಕ *z_Orig_ಲಂಬಸಾಲು = 0;
#ಆದರೆ_ಅಂತ್ಯ

  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಪ್ರತಿಜ್ಞಾ( pNC->pSrcList!=0 );
  ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಸ್ಥಿತಿ TK_ಸ್ತಂಭ: {
      /* The expression is a column. Locate the table the column is being
      ** extracted from in NameContext.pSrcList. This table may be real
      ** database table or a subquery.
      */
      ಕೋಷ್ಟಕ *pTab = 0;            /* Table structure column is extracted from */
      Select *pS = 0;             /* Select the column is extracted from */
      ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;  /* Index of column in pTab */
      ಆಗ( pNC && !pTab ){
        SrcList *pTabList = pNC->pSrcList;
        ಗಾಗಿ(j=0;j<pTabList->nSrc && pTabList->a[j].iCursor!=p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ;j++);
        ಆದರೆ( j<pTabList->nSrc ){
          pTab = pTabList->a[j].pSTab;
          ಆದರೆ( pTabList->a[j].fg.isSubquery ){
            pS = pTabList->a[j].u4.pSubq->pSelect;
          }ಇಲ್ಲದಿದ್ದರೆ{
            pS = 0;
          }
        }ಇಲ್ಲದಿದ್ದರೆ{
          pNC = pNC->p_ಮುಂದಿನ;
        }
      }

      ಆದರೆ( pTab==0 ){
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
        ನಿಲ್ಲಿಸು;
      }

      ಪ್ರತಿಜ್ಞಾ( pTab && ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) && p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab==pTab );
      ಆದರೆ( pS ){
        /* The "table" is actually a sub-select or a view in the FROM clause
        ** of the SELECT statement. Return the declaration type and origin
        ** data for the result-set column of the sub-select.
        */
        ಆದರೆ( i_ಲಂಬಸಾಲು<pS->pEList->n_ಅಭಿವ್ಯಕ್ತಿ
         && (!ViewCanHaveRowid || i_ಲಂಬಸಾಲು>=0)
        ){
          /* If iCol is less than zero, then the expression requests the
          ** rowid of the sub-select or view. This expression is legal (see
          ** test case misc2.2.2) - it always evaluates to NULL.
          */
          NameContext sNC;
          ಅಭಿವ್ಯಕ್ತಿ *p = pS->pEList->a[i_ಲಂಬಸಾಲು].p_ಅಭಿವ್ಯಕ್ತಿ;
          sNC.pSrcList = pS->pSrc;
          sNC.p_ಮುಂದಿನ = pNC;
          sNC.pParse = pNC->pParse;
          zType = ಸ್ತಂಭ_Type(&sNC, p,&zOrigDb,&zOrigTab,&z_Orig_ಲಂಬಸಾಲು);
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* A real table or a CTE table */
        ಪ್ರತಿಜ್ಞಾ( !pS );
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_METADATA
        ಆದರೆ( i_ಲಂಬಸಾಲು<0 ) i_ಲಂಬಸಾಲು = pTab->iPKey;
        ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು==XN_ROWID || (i_ಲಂಬಸಾಲು>=0 && i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು) );
        ಆದರೆ( i_ಲಂಬಸಾಲು<0 ){
          zType = "INTEGER";
          z_Orig_ಲಂಬಸಾಲು = "rowid";
        }ಇಲ್ಲದಿದ್ದರೆ{
          z_Orig_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName;
          zType = sqlite3_ಸ್ತಂಭ_Type(&pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು],0);
        }
        zOrigTab = pTab->zName;
        ಆದರೆ( pNC->pParse && pTab->pSchema ){
          ಪೂರ್ಣಾಂಕ iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pNC->pParse->db, pTab->pSchema);
          zOrigDb = pNC->pParse->db->aDb[iDb].zDbSName;
        }
#ಇಲ್ಲದಿದ್ದರೆ
        ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು==XN_ROWID || (i_ಲಂಬಸಾಲು>=0 && i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು) );
        ಆದರೆ( i_ಲಂಬಸಾಲು<0 ){
          zType = "INTEGER";
        }ಇಲ್ಲದಿದ್ದರೆ{
          zType = sqlite3_ಸ್ತಂಭ_Type(&pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು],0);
        }
#ಆದರೆ_ಅಂತ್ಯ
      }
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    ಸ್ಥಿತಿ TK_SELECT: {
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
      sNC.p_ಮುಂದಿನ = pNC;
      sNC.pParse = pNC->pParse;
      zType = ಸ್ತಂಭ_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_ಲಂಬಸಾಲು);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_METADATA 
  ಆದರೆ( pzOrigDb ){
    ಪ್ರತಿಜ್ಞಾ( pzOrigTab && pz_Orig_ಲಂಬಸಾಲು );
    *pzOrigDb = zOrigDb;
    *pzOrigTab = zOrigTab;
    *pz_Orig_ಲಂಬಸಾಲು = z_Orig_ಲಂಬಸಾಲು;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ zType;
}

/*
** Generate code that will tell the VDBE the declaration types of columns
** in the result set.
*/
ಸ್ಥಿರ ಶೂನ್ಯ generate_ಸ್ತಂಭ_Types(
  Parse *pParse,      /* Parser context */
  SrcList *pTabList,  /* List of tables */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList    /* Expressions defining the result set */
){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_DECLTYPE
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ i;
  NameContext sNC;
  sNC.pSrcList = pTabList;
  sNC.pParse = pParse;
  sNC.p_ಮುಂದಿನ = 0;
  ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_ಸ್ತಂಭ_METADATA
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOrigDb = 0;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOrigTab = 0;
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_Orig_ಲಂಬಸಾಲು = 0;
    zType = ಸ್ತಂಭ_Type(&sNC, p, &zOrigDb, &zOrigTab, &z_Orig_ಲಂಬಸಾಲು);

    /* The vdbe must make its own copy of the column-type and other
    ** column specific strings, in case the schema is reset before this
    ** virtual machine is deleted.
    */
    sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, i, COLNAME_DATABASE, zOrigDb, SQLITE_TRANSIENT);
    sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, i, COLNAME_ಕೋಷ್ಟಕ, zOrigTab, SQLITE_TRANSIENT);
    sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, i, COLNAME_ಸ್ತಂಭ, z_Orig_ಲಂಬಸಾಲು, SQLITE_TRANSIENT);
#ಇಲ್ಲದಿದ್ದರೆ
    zType = ಸ್ತಂಭ_Type(&sNC, p, 0, 0, 0);
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, i, COLNAME_DECLTYPE, zType, SQLITE_TRANSIENT);
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(pParse);
  ಬಳಸದ_PARAMETER(pTabList);
  ಬಳಸದ_PARAMETER(pEList);
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_DECLTYPE) */
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
ಶೂನ್ಯ sqlite3_Generate_ಸ್ತಂಭ_ಹೆಸರುಗಳು(
  Parse *pParse,      /* Parser context */
  Select *pSelect     /* Generate column names for this SELECT statement */
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ i;
  ಕೋಷ್ಟಕ *pTab;
  SrcList *pTabList;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  sqlite3 *db = pParse->db;
  ಪೂರ್ಣಾಂಕ ತುಂಬಿದ_Name;    /* TABLE.COLUMN if no AS clause and is a direct table ref */
  ಪೂರ್ಣಾಂಕ srcName;     /* COLUMN or TABLE.COLUMN if no AS clause and is direct */

  ಆದರೆ( pParse->ಲಂಬಸಾಲು_ಹೆಸರುಗಳು_ಗಣ ) ಹಿಂತಿರುಗಿ;
  /* Column names are determined by the left-most term of a compound select */
  ಆಗ( pSelect->p_ಪೂರ್ವ ) pSelect = pSelect->p_ಪೂರ್ವ;
  TREETRACE(0x80,pParse,pSelect,("generating column names\n"));
  pTabList = pSelect->pSrc;
  pEList = pSelect->pEList;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTabList!=0 );
  pParse->ಲಂಬಸಾಲು_ಹೆಸರುಗಳು_ಗಣ = 1;
  ತುಂಬಿದ_Name = (db->flags & SQLITE_ತುಂಬಿದ_ಲಂಬಸಾಲು_ಹೆಸರುಗಳು)!=0;
  srcName = (db->flags & SQLITE_Short_ಲಂಬಸಾಲು_ಹೆಸರುಗಳು)!=0 || ತುಂಬಿದ_Name;
  sqlite3_Vdbe_ಗಣ_ಅಂಕೆ_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ(v, pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
  ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p = pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;

    ಪ್ರತಿಜ್ಞಾ( p!=0 );
    ಪ್ರತಿಜ್ಞಾ( p->op!=TK_AGG_ಸ್ತಂಭ );  /* Agg processing has not run yet */
    ಪ್ರತಿಜ್ಞಾ( p->op!=TK_ಸ್ತಂಭ
        || (ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p) && p->y.pTab!=0) ); /* Covering idx not yet coded */
    ಆದರೆ( pEList->a[i].zEName && pEList->a[i].fg.eEName==ENAME_NAME ){
      /* An AS clause always takes first priority */
      ಅಕ್ಷರ *zName = pEList->a[i].zEName;
      sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, i, COLNAME_NAME, zName, SQLITE_TRANSIENT);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( srcName && p->op==TK_ಸ್ತಂಭ ){
      ಅಕ್ಷರ *z_ಲಂಬಸಾಲು;
      ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = p->i_ಸ್ತಂಭ;
      pTab = p->y.pTab;
      ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
      ಆದರೆ( i_ಲಂಬಸಾಲು<0 ) i_ಲಂಬಸಾಲು = pTab->iPKey;
      ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು==-1 || (i_ಲಂಬಸಾಲು>=0 && i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು) );
      ಆದರೆ( i_ಲಂಬಸಾಲು<0 ){
        z_ಲಂಬಸಾಲು = "rowid";
      }ಇಲ್ಲದಿದ್ದರೆ{
        z_ಲಂಬಸಾಲು = pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName;
      }
      ಆದರೆ( ತುಂಬಿದ_Name ){
        ಅಕ್ಷರ *zName = 0;
        zName = sqlite3MPrintf(db, "%s.%s", pTab->zName, z_ಲಂಬಸಾಲು);
        sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, i, COLNAME_NAME, zName, SQLITE_DYNAMIC);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, i, COLNAME_NAME, z_ಲಂಬಸಾಲು, SQLITE_TRANSIENT);
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = pEList->a[i].zEName;
      z = z==0 ? sqlite3MPrintf(db, "column%d", i+1) : sqlite3DbStrDup(db, z);
      sqlite3_Vdbe_ಗಣ_ಲಂಬಸಾಲು_Name(v, i, COLNAME_NAME, z, SQLITE_DYNAMIC);
    }
  }
  generate_ಸ್ತಂಭ_Types(pParse, pTabList, pEList);
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
ಪೂರ್ಣಾಂಕ sqlite3_Columns_From_ಅಭಿವ್ಯಕ್ತಿ_List(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,       /* Expr list from which to derive column names */
  i16 *pn_ಲಂಬಸಾಲು,             /* Write the number of columns here */
  ಸ್ತಂಭ **pa_ಲಂಬಸಾಲು          /* Write the new column list here */
){
  sqlite3 *db = pParse->db;   /* Database connection */
  ಪೂರ್ಣಾಂಕ i, j;                   /* Loop counters */
  u32 cnt;                    /* Index added to make the name unique */
  ಸ್ತಂಭ *a_ಲಂಬಸಾಲು, *p_ಲಂಬಸಾಲು;        /* For looping over result columns */
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;                   /* Number of columns in the result set */
  ಅಕ್ಷರ *zName;                /* Column name */
  ಪೂರ್ಣಾಂಕ nName;                  /* Size of name in zName[] */
  Hash ht;                    /* Hash table of column names */
  ಕೋಷ್ಟಕ *pTab;

  sqlite3HashInit(&ht);
  ಆದರೆ( pEList ){
    n_ಲಂಬಸಾಲು = pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
    a_ಲಂಬಸಾಲು = sqlite3DbMallocZero(db, ಗಾತ್ರ(a_ಲಂಬಸಾಲು[0])*n_ಲಂಬಸಾಲು);
    testcase( a_ಲಂಬಸಾಲು==0 );
    ಆದರೆ( n_ಲಂಬಸಾಲು>32767 ) n_ಲಂಬಸಾಲು = 32767;
  }ಇಲ್ಲದಿದ್ದರೆ{
    n_ಲಂಬಸಾಲು = 0;
    a_ಲಂಬಸಾಲು = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( n_ಲಂಬಸಾಲು==(i16)n_ಲಂಬಸಾಲು );
  *pn_ಲಂಬಸಾಲು = n_ಲಂಬಸಾಲು;
  *pa_ಲಂಬಸಾಲು = a_ಲಂಬಸಾಲು;

  ಗಾಗಿ(i=0, p_ಲಂಬಸಾಲು=a_ಲಂಬಸಾಲು; i<n_ಲಂಬಸಾಲು && !pParse->nErr; i++, p_ಲಂಬಸಾಲು++){
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *pX = &pEList->a[i];
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *pCollide;
    /* Get an appropriate name for the column
    */
    ಆದರೆ( (zName = pX->zEName)!=0 && pX->fg.eEName==ENAME_NAME ){
      /* If the column contains an "AS <name>" phrase, use <name> as the name */
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಅಭಿವ್ಯಕ್ತಿ *p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pX->p_ಅಭಿವ್ಯಕ್ತಿ);
      ಆಗ( ALWAYS(p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ!=0) && p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->op==TK_DOT ){
        p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ = p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        ಪ್ರತಿಜ್ಞಾ( p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      }
      ಆದರೆ( p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ
       && ALWAYS( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ) )
       && ALWAYS( p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->y.pTab!=0 )
      ){
        /* For columns use the column name name */
        ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
        pTab = p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
        ಆದರೆ( i_ಲಂಬಸಾಲು<0 ) i_ಲಂಬಸಾಲು = pTab->iPKey;
        zName = i_ಲಂಬಸಾಲು>=0 ? pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].zCnName : "rowid";
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
        zName = p_ಲಂಬಸಾಲು_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* Use the original text of the column expression as its name */
        ಪ್ರತಿಜ್ಞಾ( zName==pX->zEName );  /* pointer comparison intended */
      }
    }
    ಆದರೆ( zName && !sqlite3IsTrueOrFalse(zName) ){
      zName = sqlite3DbStrDup(db, zName);
    }ಇಲ್ಲದಿದ್ದರೆ{
      zName = sqlite3MPrintf(db,"column%d",i+1);
    }

    /* Make sure the column name is unique.  If the name is not unique,
    ** append an integer to the name so that it becomes unique.
    */
    cnt = 0;
    ಆಗ( zName && (pCollide = sqlite3_Hash_ಕಂಡುಹಿಡಿ(&ht, zName))!=0 ){
      ಆದರೆ( pCollide->fg.b_Using_ಪದ ){
        p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags |= COLFLAG_NOEXPAND;
      }
      nName = sqlite3Strlen30(zName);
      ಆದರೆ( nName>0 ){
        ಗಾಗಿ(j=nName-1; j>0 && sqlite3Isdigit(zName[j]); j--){}
        ಆದರೆ( zName[j]==':' ) nName = j;
      }
      zName = sqlite3MPrintf(db, "%.*z:%u", nName, zName, ++cnt);
      sqlite3_ಪ್ರಗತಿ_Check(pParse);
      ಆದರೆ( cnt>3 ){
        sqlite3_randomness(ಗಾತ್ರ(cnt), &cnt);
      }
    }
    p_ಲಂಬಸಾಲು->zCnName = zName;
    p_ಲಂಬಸಾಲು->hName = sqlite3StrIHash(zName);
    ಆದರೆ( pX->fg.bNoExpand ){
      p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags |= COLFLAG_NOEXPAND;
    }
    sqlite3_ಸ್ತಂಭ_Properties_From_Name(0, p_ಲಂಬಸಾಲು);
    ಆದರೆ( zName && sqlite3_Hash_ಸೇರಿಸು(&ht, zName, pX)==pX ){
      sqlite3OomFault(db);
    }
  }
  sqlite3HashClear(&ht);
  ಆದರೆ( pParse->nErr ){
    ಗಾಗಿ(j=0; j<i; j++){
      sqlite3DbFree(db, a_ಲಂಬಸಾಲು[j].zCnName);
    }
    sqlite3DbFree(db, a_ಲಂಬಸಾಲು);
    *pa_ಲಂಬಸಾಲು = 0;
    *pn_ಲಂಬಸಾಲು = 0;
    ಹಿಂತಿರುಗಿ pParse->rc;
  }
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಶೂನ್ಯ sqlite3_Subquery_ಸ್ತಂಭ_Types(
  Parse *pParse,      /* Parsing contexts */
  ಕೋಷ್ಟಕ *pTab,        /* Add column type information to this table */
  Select *pSelect,    /* SELECT used to determine types and collations */
  ಅಕ್ಷರ aff            /* Default affinity. */
){
  sqlite3 *db = pParse->db;
  ಸ್ತಂಭ *p_ಲಂಬಸಾಲು;
  CollSeq *pColl;
  ಪೂರ್ಣಾಂಕ i,j;
  ಅಭಿವ್ಯಕ್ತಿ *p;
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *a;
  NameContext sNC;

  ಪ್ರತಿಜ್ಞಾ( pSelect!=0 );
  ಪ್ರತಿಜ್ಞಾ( (pSelect->selFlags & SF_Resolved)!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTab->n_ಲಂಬಸಾಲು==pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ || pParse->nErr>0 );
  ಪ್ರತಿಜ್ಞಾ( aff==SQLITE_AFF_NONE || aff==SQLITE_AFF_BLOB );
  ಆದರೆ( db->mallocFailed || ಒಳ_RENAME_OBJECT ) ಹಿಂತಿರುಗಿ;
  ಆಗ( pSelect->p_ಪೂರ್ವ ) pSelect = pSelect->p_ಪೂರ್ವ;
  a = pSelect->pEList->a;
  memset(&sNC, 0, ಗಾತ್ರ(sNC));
  sNC.pSrcList = pSelect->pSrc;
  ಗಾಗಿ(i=0, p_ಲಂಬಸಾಲು=pTab->a_ಲಂಬಸಾಲು; i<pTab->n_ಲಂಬಸಾಲು; i++, p_ಲಂಬಸಾಲು++){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType;
    i64 n;
    ಪೂರ್ಣಾಂಕ m = 0;
    Select *pS2 = pSelect;
    pTab->tabFlags |= (p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags & COLFLAG_NOINSERT);
    p = a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    /* pCol->szEst = ... // Column size est for SELECT tables never used */
    p_ಲಂಬಸಾಲು->affinity = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p);
    ಆಗ( p_ಲಂಬಸಾಲು->affinity<=SQLITE_AFF_NONE && pS2->p_ಮುಂದಿನ!=0 ){
      m |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ದತ್ತಾಂಶ_Type(pS2->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
      pS2 = pS2->p_ಮುಂದಿನ;
      p_ಲಂಬಸಾಲು->affinity = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pS2->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
    }
    ಆದರೆ( p_ಲಂಬಸಾಲು->affinity<=SQLITE_AFF_NONE ){
      p_ಲಂಬಸಾಲು->affinity = aff;
    }
    ಆದರೆ( p_ಲಂಬಸಾಲು->affinity>=SQLITE_AFF_ಪಠ್ಯ && (pS2->p_ಮುಂದಿನ || pS2!=pSelect) ){
      ಗಾಗಿ(pS2=pS2->p_ಮುಂದಿನ; pS2; pS2=pS2->p_ಮುಂದಿನ){
        m |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ದತ್ತಾಂಶ_Type(pS2->pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
      }
      ಆದರೆ( p_ಲಂಬಸಾಲು->affinity==SQLITE_AFF_ಪಠ್ಯ && (m&0x01)!=0 ){
        p_ಲಂಬಸಾಲು->affinity = SQLITE_AFF_BLOB;
      }ಇಲ್ಲದಿದ್ದರೆ
      ಆದರೆ( p_ಲಂಬಸಾಲು->affinity>=SQLITE_AFF_NUMERIC && (m&0x02)!=0 ){
        p_ಲಂಬಸಾಲು->affinity = SQLITE_AFF_BLOB;
      }
      ಆದರೆ( p_ಲಂಬಸಾಲು->affinity>=SQLITE_AFF_NUMERIC && p->op==TK_CAST ){
        p_ಲಂಬಸಾಲು->affinity = SQLITE_AFF_FLEXNUM;
      }
    }
    zType = ಸ್ತಂಭ_Type(&sNC, p, 0, 0, 0);
    ಆದರೆ( zType==0 || p_ಲಂಬಸಾಲು->affinity!=sqlite3AffinityType(zType, 0) ){
      ಆದರೆ( p_ಲಂಬಸಾಲು->affinity==SQLITE_AFF_NUMERIC
       || p_ಲಂಬಸಾಲು->affinity==SQLITE_AFF_FLEXNUM
      ){
        zType = "NUM";
      }ಇಲ್ಲದಿದ್ದರೆ{
        zType = 0;
        ಗಾಗಿ(j=1; j<SQLITE_N_STDTYPE; j++){
          ಆದರೆ( sqlite3StdTypeAffinity[j]==p_ಲಂಬಸಾಲು->affinity ){
            zType = sqlite3StdType[j];
            ನಿಲ್ಲಿಸು;
          }
        }
      }
    }
    ಆದರೆ( zType ){
      ಸ್ಥಿರಾಂಕ i64 k = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(zType);
      n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p_ಲಂಬಸಾಲು->zCnName);
      p_ಲಂಬಸಾಲು->zCnName = sqlite3DbReallocOrFree(db, p_ಲಂಬಸಾಲು->zCnName, n+k+2);
      p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags &= ~(COLFLAG_HASTYPE|COLFLAG_HASCOLL);
      ಆದರೆ( p_ಲಂಬಸಾಲು->zCnName ){
        ಸ್ಮರಣೆ_ನಕಲು(&p_ಲಂಬಸಾಲು->zCnName[n+1], zType, k+1);
        p_ಲಂಬಸಾಲು->ಲಂಬಸಾಲು_Flags |= COLFLAG_HASTYPE;
      }
    }
    pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p);
    ಆದರೆ( pColl ){
      ಪ್ರತಿಜ್ಞಾ( pTab->p_ಸೂಚ್ಯಂಕ==0 );
      sqlite3_ಸ್ತಂಭ_ಗಣ_Coll(db, p_ಲಂಬಸಾಲು, pColl->zName);
    }
  }
  pTab->sz_Tab_ಅಡ್ಡಸಾಲು = 1; /* Any non-zero value works */
}

/*
** Check the current subquery nesting depth.  Return true and set an
** error if it has gone too deep.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_Subquery_Nesting_ಆಳ(Parse *pParse){
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
  ಆದರೆ( pParse->nNestSel >= pParse->db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "VIEWs and/or subqueries nested too deep");
    ಹಿಂತಿರುಗಿ 1;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Given a SELECT statement, generate a Table structure that describes
** the result set of that SELECT.
*/
ಕೋಷ್ಟಕ *sqlite3_ಫಲಿತಾಂಶ_ಗಣ_ದ_Select(Parse *pParse, Select *pSelect, ಅಕ್ಷರ aff){
  ಕೋಷ್ಟಕ *pTab;
  sqlite3 *db = pParse->db;
  u64 savedFlags;

  pParse->nNestSel++;
  ಆದರೆ( check_Subquery_Nesting_ಆಳ(pParse) ) ಹಿಂತಿರುಗಿ 0;
  savedFlags = db->flags;
  db->flags &= ~(u64)SQLITE_ತುಂಬಿದ_ಲಂಬಸಾಲು_ಹೆಸರುಗಳು;
  db->flags |= SQLITE_Short_ಲಂಬಸಾಲು_ಹೆಸರುಗಳು;
  sqlite3SelectPrep(pParse, pSelect, 0);
  db->flags = savedFlags;
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ 0;
  ಆಗ( pSelect->p_ಪೂರ್ವ ) pSelect = pSelect->p_ಪೂರ್ವ;
  pTab = sqlite3DbMallocZero(db, ಗಾತ್ರ(ಕೋಷ್ಟಕ) );
  ಆದರೆ( pTab==0 ){
    ಹಿಂತಿರುಗಿ 0;
  }
  pTab->nTabRef = 1;
  pTab->zName = 0;
  pTab->n_ಅಡ್ಡಸಾಲು_Log_Est = 200; ಪ್ರತಿಜ್ಞಾ( 200==sqlite3LogEst(1048576) );
  sqlite3_Columns_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSelect->pEList, &pTab->n_ಲಂಬಸಾಲು, &pTab->a_ಲಂಬಸಾಲು);
  sqlite3_Subquery_ಸ್ತಂಭ_Types(pParse, pTab, pSelect, aff);
  pTab->iPKey = -1;
  ಆದರೆ( db->mallocFailed ){
    sqlite3_Delete_ಕೋಷ್ಟಕ(db, pTab);
    ಹಿಂತಿರುಗಿ 0;
  }
  pParse->nNestSel--;
  ಪ್ರತಿಜ್ಞಾ( pParse->nNestSel>=0 );
  ಹಿಂತಿರುಗಿ pTab;
}

/*
** Get a VDBE for the given parser context.  Create a new one if necessary.
** If an error occurs, return NULL and leave a message in pParse.
*/
Vdbe *sqlite3GetVdbe(Parse *pParse){
  ಆದರೆ( pParse->pVdbe ){
    ಹಿಂತಿರುಗಿ pParse->pVdbe;
  }
  ಆದರೆ( pParse->pToplevel==0 ){
    pParse->ok_Const_ಗುಣಕ = 1;
  }
  ಹಿಂತಿರುಗಿ sqlite3VdbeCreate(pParse);
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
ಸ್ಥಿರ ಶೂನ್ಯ compute_ಎಲ್ಲೆ_Registers(Parse *pParse, Select *p, ಪೂರ್ಣಾಂಕ iBreak){
  Vdbe *v = 0;
  ಪೂರ್ಣಾಂಕ i_ಎಲ್ಲೆ = 0;
  ಪೂರ್ಣಾಂಕ i_ಸ್ಥಾನಾಂತರ;
  ಪೂರ್ಣಾಂಕ n;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ = p->p_ಎಲ್ಲೆ;

  ಆದರೆ( p->i_ಎಲ್ಲೆ ) ಹಿಂತಿರುಗಿ;

  /*
  ** "LIMIT -1" always shows all rows.  There is some
  ** controversy about what the correct behavior should be.
  ** The current implementation interprets "LIMIT 0" to mean
  ** no rows.
  */
  ಆದರೆ( p_ಎಲ್ಲೆ ){
    ಪ್ರತಿಜ್ಞಾ( p_ಎಲ್ಲೆ->op==TK_ಎಲ್ಲೆ );
    ಪ್ರತಿಜ್ಞಾ( p_ಎಲ್ಲೆ->p_ಎಡ!=0 );
    p->i_ಎಲ್ಲೆ = i_ಎಲ್ಲೆ = ++pParse->nMem;
    v = sqlite3GetVdbe(pParse);
    ಪ್ರತಿಜ್ಞಾ( v!=0 );
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(p_ಎಲ್ಲೆ->p_ಎಡ, &n, pParse, 0) ){
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, n, i_ಎಲ್ಲೆ);
      VdbeComment((v, "LIMIT counter"));
      ಆದರೆ( n==0 ){
        sqlite3VdbeGoto(v, iBreak);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( n>=0 && p->n_Select_ಅಡ್ಡಸಾಲು>sqlite3LogEst((u64)n) ){
        p->n_Select_ಅಡ್ಡಸಾಲು = sqlite3LogEst((u64)n);
        p->selFlags |= SF_Fixed_ಎಲ್ಲೆ;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಎಲ್ಲೆ->p_ಎಡ, i_ಎಲ್ಲೆ);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_MustBeInt, i_ಎಲ್ಲೆ); VdbeCoverage(v);
      VdbeComment((v, "LIMIT counter"));
      sqlite3MayAbort(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IfNot, i_ಎಲ್ಲೆ, iBreak); VdbeCoverage(v);
    }
    ಆದರೆ( p_ಎಲ್ಲೆ->p_ಬಲ ){
      p->i_ಸ್ಥಾನಾಂತರ = i_ಸ್ಥಾನಾಂತರ = ++pParse->nMem;
      pParse->nMem++;   /* Allocate an extra register for limit+offset */
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಎಲ್ಲೆ->p_ಬಲ, i_ಸ್ಥಾನಾಂತರ);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_MustBeInt, i_ಸ್ಥಾನಾಂತರ); VdbeCoverage(v);
      sqlite3MayAbort(pParse);
      VdbeComment((v, "OFFSET counter"));
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ಥಾನಾಂತರ_ಎಲ್ಲೆ, i_ಎಲ್ಲೆ, i_ಸ್ಥಾನಾಂತರ+1, i_ಸ್ಥಾನಾಂತರ);
      VdbeComment((v, "LIMIT+OFFSET"));
    }
  }
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPOUND_SELECT
/*
** Return the appropriate collating sequence for the iCol-th column of
** the result set for the compound-select statement "p".  Return NULL if
** the column has no default collating sequence.
**
** The collating sequence for the compound select is taken from the
** left-most term of the select that has a collating sequence.
*/
ಸ್ಥಿರ CollSeq *multiSelectCollSeq(Parse *pParse, Select *p, ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು){
  CollSeq *pRet;
  ಆದರೆ( p->p_ಪೂರ್ವ ){
    pRet = multiSelectCollSeq(pParse, p->p_ಪೂರ್ವ, i_ಲಂಬಸಾಲು);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pRet = 0;
  }
  ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು>=0 );
  /* iCol must be less than p->pEList->nExpr.  Otherwise an error would
  ** have been thrown during name resolution and we would not have gotten
  ** this far */
  ಆದರೆ( pRet==0 && ALWAYS(i_ಲಂಬಸಾಲು<p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ) ){
    pRet = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p->pEList->a[i_ಲಂಬಸಾಲು].p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಹಿಂತಿರುಗಿ pRet;
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
ಸ್ಥಿರ ಕೀಲಿ_Info *multi_Select_By_ವಿಲೀನ_ಕೀಲಿ_Info(Parse *pParse, Select *p, ಪೂರ್ಣಾಂಕ nExtra){
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By = p->p_ಕ್ರಮ_By;
  ಪೂರ್ಣಾಂಕ n_ಕ್ರಮ_By = (p_ಕ್ರಮ_By!=0) ? p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
  sqlite3 *db = pParse->db;
  ಕೀಲಿ_Info *pRet = sqlite3_ಕೀಲಿ_Info_Alloc(db, n_ಕ್ರಮ_By+nExtra, 1);
  ಆದರೆ( pRet ){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<n_ಕ್ರಮ_By; i++){
      ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ = &p_ಕ್ರಮ_By->a[i];
      ಅಭಿವ್ಯಕ್ತಿ *p_ಪದ = p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ;
      CollSeq *pColl;

      ಆದರೆ( p_ಪದ->flags & EP_Collate ){
        pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಪದ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pColl = multiSelectCollSeq(pParse, p, p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು-1);
        ಆದರೆ( pColl==0 ) pColl = db->pDfltColl;
        p_ಕ್ರಮ_By->a[i].p_ಅಭಿವ್ಯಕ್ತಿ =
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(pParse, p_ಪದ, pColl->zName);
      }
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(pRet) );
      pRet->aColl[i] = pColl;
      pRet->a_ವಿಂಗಡಿಸು_Flags[i] = p_ಕ್ರಮ_By->a[i].fg.ವಿಂಗಡಿಸು_Flags;
    }
  }

  ಹಿಂತಿರುಗಿ pRet;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
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
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು = p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;  /* Number of columns in the recursive table */
  Vdbe *v = pParse->pVdbe;      /* The prepared statement under construction */
  Select *p_ಸಿದ್ಧತೆ;               /* The setup query */
  Select *p_ಮೊದಲ_Rec;            /* Left-most recursive term */
  ಪೂರ್ಣಾಂಕ addr_ತುದಿ;                  /* Top of the loop */
  ಪೂರ್ಣಾಂಕ addrCont, addrBreak;      /* CONTINUE and BREAK addresses */
  ಪೂರ್ಣಾಂಕ iCurrent = 0;             /* The Current table */
  ಪೂರ್ಣಾಂಕ regCurrent;               /* Register holding Current table */
  ಪೂರ್ಣಾಂಕ i_ಸರತಿ;                   /* The Queue table */
  ಪೂರ್ಣಾಂಕ iDistinct = 0;            /* To ensure unique results if UNION */
  ಪೂರ್ಣಾಂಕ eDest = SRT_Fifo;         /* How to write to Queue */
  SelectDest dest_ಸರತಿ;         /* SelectDest targeting the Queue table */
  ಪೂರ್ಣಾಂಕ i;                        /* Loop counter */
  ಪೂರ್ಣಾಂಕ rc;                       /* Result code */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;           /* The ORDER BY clause */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ;                 /* Saved LIMIT and OFFSET */
  ಪೂರ್ಣಾಂಕ reg_ಎಲ್ಲೆ, reg_ಸ್ಥಾನಾಂತರ;      /* Registers used by LIMIT and OFFSET */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಆದರೆ( p->pWin ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "cannot use window functions in recursive queries");
    ಹಿಂತಿರುಗಿ;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* Obtain authorization to do a recursive query */
  ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_RECURSIVE, 0, 0, 0) ) ಹಿಂತಿರುಗಿ;

  /* Process the LIMIT and OFFSET clauses, if they exist */
  addrBreak = sqlite3VdbeMakeLabel(pParse);
  p->n_Select_ಅಡ್ಡಸಾಲು = 320;  /* 4 billion rows */
  compute_ಎಲ್ಲೆ_Registers(pParse, p, addrBreak);
  p_ಎಲ್ಲೆ = p->p_ಎಲ್ಲೆ;
  reg_ಎಲ್ಲೆ = p->i_ಎಲ್ಲೆ;
  reg_ಸ್ಥಾನಾಂತರ = p->i_ಸ್ಥಾನಾಂತರ;
  p->p_ಎಲ್ಲೆ = 0;
  p->i_ಎಲ್ಲೆ = p->i_ಸ್ಥಾನಾಂತರ = 0;
  p_ಕ್ರಮ_By = p->p_ಕ್ರಮ_By;

  /* Locate the cursor number of the Current table */
  ಗಾಗಿ(i=0; ALWAYS(i<pSrc->nSrc); i++){
    ಆದರೆ( pSrc->a[i].fg.isRecursive ){
      iCurrent = pSrc->a[i].iCursor;
      ನಿಲ್ಲಿಸು;
    }
  }

  /* Allocate cursors numbers for Queue and Distinct.  The cursor number for
  ** the Distinct table must be exactly one greater than Queue in order
  ** for the SRT_DistFifo and SRT_DistQueue destinations to work. */
  i_ಸರತಿ = pParse->nTab++;
  ಆದರೆ( p->op==TK_UNION ){
    eDest = p_ಕ್ರಮ_By ? SRT_Dist_ಸರತಿ : SRT_DistFifo;
    iDistinct = pParse->nTab++;
  }ಇಲ್ಲದಿದ್ದರೆ{
    eDest = p_ಕ್ರಮ_By ? SRT_ಸರತಿ : SRT_Fifo;
  }
  sqlite3SelectDestInit(&dest_ಸರತಿ, eDest, i_ಸರತಿ);

  /* Allocate cursors for Current, Queue, and Distinct. */
  regCurrent = ++pParse->nMem;
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenPseudo, iCurrent, regCurrent, n_ಲಂಬಸಾಲು);
  ಆದರೆ( p_ಕ್ರಮ_By ){
    ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = multi_Select_By_ವಿಲೀನ_ಕೀಲಿ_Info(pParse, p, 1);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_OpenEphemeral, i_ಸರತಿ, p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ+2, 0,
                      (ಅಕ್ಷರ*)p_ಕೀಲಿ_Info, P4_KEYINFO);
    dest_ಸರತಿ.p_ಕ್ರಮ_By = p_ಕ್ರಮ_By;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenEphemeral, i_ಸರತಿ, n_ಲಂಬಸಾಲು);
  }
  VdbeComment((v, "Queue table"));
  ಆದರೆ( iDistinct ){
    /* Generate an ephemeral table used to enforce distinctness on the
    ** output of the recursive part of the CTE.
    */
    ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;            /* Collating sequence for the result set */
    CollSeq **apColl;             /* For looping through pKeyInfo->aColl[] */

    ಪ್ರತಿಜ್ಞಾ( p->p_ಮುಂದಿನ==0 );
    ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );
    n_ಲಂಬಸಾಲು = p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
    p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_Alloc(pParse->db, n_ಲಂಬಸಾಲು, 1);
    ಆದರೆ( p_ಕೀಲಿ_Info ){
      ಗಾಗಿ(i=0, apColl=p_ಕೀಲಿ_Info->aColl; i<n_ಲಂಬಸಾಲು; i++, apColl++){
        *apColl = multiSelectCollSeq(pParse, p, i);
        ಆದರೆ( 0==*apColl ){
          *apColl = pParse->db->pDfltColl;
        }
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_OpenEphemeral, iDistinct, n_ಲಂಬಸಾಲು, 0,
                           (ಶೂನ್ಯ*)p_ಕೀಲಿ_Info, P4_KEYINFO);
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( pParse->nErr>0 );
    }
  }

  /* Detach the ORDER BY clause from the compound SELECT */
  p->p_ಕ್ರಮ_By = 0;

  /* Figure out how many elements of the compound SELECT are part of the
  ** recursive query.  Make sure no recursive elements use aggregate
  ** functions.  Mark the recursive elements as UNION ALL even if they
  ** are really UNION because the distinctness will be enforced by the
  ** iDistinct table.  pFirstRec is left pointing to the left-most
  ** recursive term of the CTE.
  */
  ಗಾಗಿ(p_ಮೊದಲ_Rec=p; ALWAYS(p_ಮೊದಲ_Rec!=0); p_ಮೊದಲ_Rec=p_ಮೊದಲ_Rec->p_ಪೂರ್ವ){
    ಆದರೆ( p_ಮೊದಲ_Rec->selFlags & SF_Aggregate ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "recursive aggregate queries not supported");
      ಹೋಗು ಕೊನೆ_ದ_recursive_query;
    }
    p_ಮೊದಲ_Rec->op = TK_ALL;
    ಆದರೆ( (p_ಮೊದಲ_Rec->p_ಪೂರ್ವ->selFlags & SF_Recursive)==0 ) ನಿಲ್ಲಿಸು;
  }

  /* Store the results of the setup-query in Queue. */
  p_ಸಿದ್ಧತೆ = p_ಮೊದಲ_Rec->p_ಪೂರ್ವ;
  p_ಸಿದ್ಧತೆ->p_ಮುಂದಿನ = 0;
  ExplainQueryPlan((pParse, 1, "SETUP"));
  rc = sqlite3Select(pParse, p_ಸಿದ್ಧತೆ, &dest_ಸರತಿ);
  p_ಸಿದ್ಧತೆ->p_ಮುಂದಿನ = p;
  ಆದರೆ( rc ) ಹೋಗು ಕೊನೆ_ದ_recursive_query;

  /* Find the next row in the Queue and output that row */
  addr_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Rewind, i_ಸರತಿ, addrBreak); VdbeCoverage(v);

  /* Transfer the next row in Queue over to Current */
  sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Null_ಅಡ್ಡಸಾಲು, iCurrent); /* To reset column cache */
  ಆದರೆ( p_ಕ್ರಮ_By ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, i_ಸರತಿ, p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ+1, regCurrent);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಅಡ್ಡಸಾಲು_ದತ್ತಾಂಶ, i_ಸರತಿ, regCurrent);
  }
  sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Delete, i_ಸರತಿ);

  /* Output the single row in Current */
  addrCont = sqlite3VdbeMakeLabel(pParse);
  ಸಂಕೇತ_ಸ್ಥಾನಾಂತರ(v, reg_ಸ್ಥಾನಾಂತರ, addrCont);
  selectInnerLoop(pParse, p, iCurrent,
      0, 0, pDest, addrCont, addrBreak);
  ಆದರೆ( reg_ಎಲ್ಲೆ ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_DecrJumpZero, reg_ಎಲ್ಲೆ, addrBreak);
    VdbeCoverage(v);
  }
  sqlite3VdbeResolveLabel(v, addrCont);

  /* Execute the recursive SELECT taking the single row in Current as
  ** the value for the recursive-table. Store the results in the Queue.
  */
  p_ಮೊದಲ_Rec->p_ಪೂರ್ವ = 0;
  ExplainQueryPlan((pParse, 1, "RECURSIVE STEP"));
  sqlite3Select(pParse, p, &dest_ಸರತಿ);
  ಪ್ರತಿಜ್ಞಾ( p_ಮೊದಲ_Rec->p_ಪೂರ್ವ==0 );
  p_ಮೊದಲ_Rec->p_ಪೂರ್ವ = p_ಸಿದ್ಧತೆ;

  /* Keep running the loop until the Queue is empty */
  sqlite3VdbeGoto(v, addr_ತುದಿ);
  sqlite3VdbeResolveLabel(v, addrBreak);

ಕೊನೆ_ದ_recursive_query:
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(pParse->db, p->p_ಕ್ರಮ_By);
  p->p_ಕ್ರಮ_By = p_ಕ್ರಮ_By;
  p->p_ಎಲ್ಲೆ = p_ಎಲ್ಲೆ;
  ಹಿಂತಿರುಗಿ;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_CTE */

/* Forward references */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multi_Select_By_ವಿಲೀನ(
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multi_Select_ಮೌಲ್ಯಗಳು(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ಪೂರ್ಣಾಂಕ n_ಅಡ್ಡಸಾಲು = 1;
  ಪೂರ್ಣಾಂಕ rc = 0;
  ಪೂರ್ಣಾಂಕ bShowAll = p->p_ಎಲ್ಲೆ==0;
  ಪ್ರತಿಜ್ಞಾ( p->selFlags & SF_Multi_ಮೌಲ್ಯ );
  ಮಾಡು{
    ಪ್ರತಿಜ್ಞಾ( p->selFlags & SF_ಮೌಲ್ಯಗಳು );
    ಪ್ರತಿಜ್ಞಾ( p->op==TK_ALL || (p->op==TK_SELECT && p->p_ಪೂರ್ವ==0) );
    ಪ್ರತಿಜ್ಞಾ( p->p_ಮುಂದಿನ==0 || p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ==p->p_ಮುಂದಿನ->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( p->pWin ) ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p->p_ಪೂರ್ವ==0 ) ನಿಲ್ಲಿಸು;
    ಪ್ರತಿಜ್ಞಾ( p->p_ಪೂರ್ವ->p_ಮುಂದಿನ==p );
    p = p->p_ಪೂರ್ವ;
    n_ಅಡ್ಡಸಾಲು += bShowAll;
  }ಆಗ(1);
  ExplainQueryPlan((pParse, 0, "SCAN %d CONSTANT ROW%s", n_ಅಡ್ಡಸಾಲು,
                    n_ಅಡ್ಡಸಾಲು==1 ? "" : "S"));
  ಆಗ( p ){
    selectInnerLoop(pParse, p, -1, 0, 0, pDest, 1, 1);
    ಆದರೆ( !bShowAll ) ನಿಲ್ಲಿಸು;
    p->n_Select_ಅಡ್ಡಸಾಲು = n_ಅಡ್ಡಸಾಲು;
    p = p->p_ಮುಂದಿನ;
  }
  ಹಿಂತಿರುಗಿ rc;
}

/*
** Return true if the SELECT statement which is known to be the recursive
** part of a recursive CTE still has its anchor terms attached.  If the
** anchor terms have already been removed, then return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hasAnchor(Select *p){
  ಆಗ( p && (p->selFlags & SF_Recursive)!=0 ){ p = p->p_ಪೂರ್ವ; }
  ಹಿಂತಿರುಗಿ p!=0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ union_With_ಎಲ್ಲೆ_One(sqlite3 *db, Select *p){
  ಪೂರ್ಣಾಂಕ v;
  ಆದರೆ( p->op!=TK_UNION ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p->p_ಎಲ್ಲೆ==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p->p_ಎಲ್ಲೆ->p_ಬಲ ) ಹಿಂತಿರುಗಿ 0;  /* No OFFSET */
  v = 0;
  ಪ್ರತಿಜ್ಞಾ( p->p_ಎಲ್ಲೆ->p_ಎಡ!=0 );
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(p->p_ಎಲ್ಲೆ->p_ಎಡ, &v, 0, 0)==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( v!=1 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( OptimizationDisabled(db, SQLITE_Union_ಎಲ್ಲೆ) ) ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ 1;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multiSelect(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ಪೂರ್ಣಾಂಕ rc = SQLITE_OK;   /* Success code from a subroutine */
  Select *p_ಪೂರ್ವ;       /* Another SELECT immediately to our left */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest dest;      /* Alternative data destination */
  Select *pDelete = 0;  /* Chain of simple selects to delete */
  sqlite3 *db;          /* Database connection */

  /* Make sure there is no ORDER BY or LIMIT clause on prior SELECTs.  Only
  ** the last (right-most) SELECT in the series may have an ORDER BY or LIMIT.
  */
  ಪ್ರತಿಜ್ಞಾ( p && p->p_ಪೂರ್ವ );  /* Calling function guarantees this much */
  ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Recursive)==0 || p->op==TK_ALL || p->op==TK_UNION );
  ಪ್ರತಿಜ್ಞಾ( p->selFlags & SF_Compound );
  db = pParse->db;
  p_ಪೂರ್ವ = p->p_ಪೂರ್ವ;
  dest = *pDest;
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ->p_ಕ್ರಮ_By==0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ->p_ಎಲ್ಲೆ==0 );

  v = sqlite3GetVdbe(pParse);
  ಪ್ರತಿಜ್ಞಾ( v!=0 );  /* The VDBE already created by calling function */

  /* Create the destination temporary table if necessary
  */
  ಆದರೆ( dest.eDest==SRT_EphemTab ){
    ಪ್ರತಿಜ್ಞಾ( p->pEList );
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenEphemeral, dest.iSDParm, p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
    dest.eDest = SRT_ಕೋಷ್ಟಕ;
  }

  /* Special handling for a compound-select that originates as a VALUES clause.
  */
  ಆದರೆ( p->selFlags & SF_Multi_ಮೌಲ್ಯ ){
    rc = multi_Select_ಮೌಲ್ಯಗಳು(pParse, p, &dest);
    ಆದರೆ( rc>=0 ) ಹೋಗು multi_select_ಕೊನೆ;
    rc = SQLITE_OK;
  }

  /* Make sure all SELECTs in the statement have the same number of elements
  ** in their result sets.
  */
  ಪ್ರತಿಜ್ಞಾ( p->pEList && p_ಪೂರ್ವ->pEList );
  ಪ್ರತಿಜ್ಞಾ( p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ==p_ಪೂರ್ವ->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
  ಆದರೆ( (p->selFlags & SF_Recursive)!=0 && hasAnchor(p) ){
    generateWithRecursiveQuery(pParse, p, &dest);
  }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ( p->p_ಕ್ರಮ_By ){
    /* If the compound has an ORDER BY clause, then always use the merge
    ** algorithm. */
    ಹಿಂತಿರುಗಿ multi_Select_By_ವಿಲೀನ(pParse, p, pDest);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p->op!=TK_ALL && !union_With_ಎಲ್ಲೆ_One(db,p) ){
    /* If the compound is EXCEPT, INTERSECT, or UNION (anything other than
    ** UNION ALL) then also always use the merge algorithm.  However, the
    ** multiSelectByMerge() routine requires that the compound have an
    ** ORDER BY clause, and it doesn't right now.  So invent one first. */
    ಅಭಿವ್ಯಕ್ತಿ *pOne = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, 1);
    p->p_ಕ್ರಮ_By = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, pOne);
    ಆದರೆ( pParse->nErr ) ಹೋಗು multi_select_ಕೊನೆ;
    ಪ್ರತಿಜ್ಞಾ( p->p_ಕ್ರಮ_By!=0 );
    p->p_ಕ್ರಮ_By->a[0].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = 1;
    ಹಿಂತಿರುಗಿ multi_Select_By_ವಿಲೀನ(pParse, p, pDest);
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* For a UNION ALL compound without ORDER BY, or for a UNION with a 
    ** LIMIT of exactly 1 and no OFFSET and no ORDER BY, simply run the left
    ** query, then run the right query */
    ಪೂರ್ಣಾಂಕ addr = 0;
    ಪೂರ್ಣಾಂಕ n_ಎಲ್ಲೆ = 0;  /* Initialize to suppress harmless compiler warning */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN
    ಆದರೆ( p_ಪೂರ್ವ->p_ಪೂರ್ವ==0 ){
      ExplainQueryPlan((pParse, 1, "COMPOUND QUERY"));
      ExplainQueryPlan((pParse, 1, "LEFT-MOST SUBQUERY"));
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಪ್ರತಿಜ್ಞಾ( !p_ಪೂರ್ವ->p_ಎಲ್ಲೆ );
    p_ಪೂರ್ವ->i_ಎಲ್ಲೆ = p->i_ಎಲ್ಲೆ;
    p_ಪೂರ್ವ->i_ಸ್ಥಾನಾಂತರ = p->i_ಸ್ಥಾನಾಂತರ;
    p_ಪೂರ್ವ->p_ಎಲ್ಲೆ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p->p_ಎಲ್ಲೆ, 0);
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL left...\n"));
    rc = sqlite3Select(pParse, p_ಪೂರ್ವ, &dest);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಪೂರ್ವ->p_ಎಲ್ಲೆ);
    p_ಪೂರ್ವ->p_ಎಲ್ಲೆ = 0;
    ಆದರೆ( rc ){
      ಹೋಗು multi_select_ಕೊನೆ;
    }
    p->p_ಪೂರ್ವ = 0;
    p->i_ಎಲ್ಲೆ = p_ಪೂರ್ವ->i_ಎಲ್ಲೆ;
    p->i_ಸ್ಥಾನಾಂತರ = p_ಪೂರ್ವ->i_ಸ್ಥಾನಾಂತರ;
    ಆದರೆ( p->i_ಎಲ್ಲೆ ){
      addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IfNot, p->i_ಎಲ್ಲೆ); VdbeCoverage(v);
      VdbeComment((v, "Jump ahead if LIMIT reached"));
      ಆದರೆ( p->i_ಸ್ಥಾನಾಂತರ ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ಥಾನಾಂತರ_ಎಲ್ಲೆ,
                          p->i_ಎಲ್ಲೆ, p->i_ಸ್ಥಾನಾಂತರ+1, p->i_ಸ್ಥಾನಾಂತರ);
      }
    }
    ExplainQueryPlan((pParse, 1, "UNION ALL"));
    TREETRACE(0x200, pParse, p, ("multiSelect UNION ALL right...\n"));
    rc = sqlite3Select(pParse, p, &dest);
    testcase( rc!=SQLITE_OK );
    pDelete = p->p_ಪೂರ್ವ;
    p->p_ಪೂರ್ವ = p_ಪೂರ್ವ;
    p->n_Select_ಅಡ್ಡಸಾಲು = sqlite3_Log_Est_ಕೂಡಿಸು(p->n_Select_ಅಡ್ಡಸಾಲು, p_ಪೂರ್ವ->n_Select_ಅಡ್ಡಸಾಲು);
    ಆದರೆ( p->p_ಎಲ್ಲೆ
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(p->p_ಎಲ್ಲೆ->p_ಎಡ, &n_ಎಲ್ಲೆ, pParse, 0)
     && n_ಎಲ್ಲೆ>0 && p->n_Select_ಅಡ್ಡಸಾಲು > sqlite3LogEst((u64)n_ಎಲ್ಲೆ)
    ){
      p->n_Select_ಅಡ್ಡಸಾಲು = sqlite3LogEst((u64)n_ಎಲ್ಲೆ);
    }
    ಆದರೆ( addr ){
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN
    ಆದರೆ( p->p_ಮುಂದಿನ==0 ){
      Explain_Query_Plan_ತೆಗೆ(pParse);
    }
#ಆದರೆ_ಅಂತ್ಯ
  }

multi_select_ಕೊನೆ:
  pDest->iSdst = dest.iSdst;
  pDest->nSdst = dest.nSdst;
  pDest->iSDParm2 = dest.iSDParm2;
  ಆದರೆ( pDelete ){
    sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, sqlite3SelectDeleteGeneric, pDelete);
  }
  ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_COMPOUND_SELECT */

/*
** Error message for when two or more terms of a compound select have different
** size result sets.
*/
ಶೂನ್ಯ sqlite3_Select_Wrong_ಅಂಕೆ_Terms_Error(Parse *pParse, Select *p){
  ಆದರೆ( p->selFlags & SF_ಮೌಲ್ಯಗಳು ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "all VALUES must have the same number of terms");
  }ಇಲ್ಲದಿದ್ದರೆ{
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ generateOutputSubroutine(
  Parse *pParse,          /* Parsing context */
  Select *p,              /* The SELECT statement */
  SelectDest *p_ಒಳ,        /* Coroutine supplying data */
  SelectDest *pDest,      /* Where to send the data */
  ಪೂರ್ಣಾಂಕ regReturn,          /* The return address register */
  ಪೂರ್ಣಾಂಕ regPrev,            /* Previous result register.  No uniqueness if 0 */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Info,      /* For comparing with previous entry */
  ಪೂರ್ಣಾಂಕ iBreak              /* Jump here if we hit the LIMIT */
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ iContinue;
  ಪೂರ್ಣಾಂಕ addr;

  ಪ್ರತಿಜ್ಞಾ( p_ಒಳ->eDest==SRT_Coroutine );

  addr = sqlite3VdbeCurrentAddr(v);
  iContinue = sqlite3VdbeMakeLabel(pParse);

  /* Suppress duplicates for UNION, EXCEPT, and INTERSECT
  */
  ಆದರೆ( regPrev ){
    ಪೂರ್ಣಾಂಕ addr1, addr2;
    addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_IfNot, regPrev); VdbeCoverage(v);
    addr2 = sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Compare, p_ಒಳ->iSdst, regPrev+1, p_ಒಳ->nSdst,
                              (ಅಕ್ಷರ*)sqlite3_ಕೀಲಿ_Info_Ref(p_ಕೀಲಿ_Info), P4_KEYINFO);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Jump, addr2+2, iContinue, addr2+2); VdbeCoverage(v);
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Copy, p_ಒಳ->iSdst, regPrev+1, p_ಒಳ->nSdst-1);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, regPrev);
  }
  ಆದರೆ( pParse->db->mallocFailed ) ಹಿಂತಿರುಗಿ 0;

  /* Suppress the first OFFSET entries if there is an OFFSET clause
  */
  ಸಂಕೇತ_ಸ್ಥಾನಾಂತರ(v, p->i_ಸ್ಥಾನಾಂತರ, iContinue);

  ಆಯ್ಕೆ( pDest->eDest ){
    /* Store the result as data using a unique key.
    */
    ಸ್ಥಿತಿ SRT_Fifo:
    ಸ್ಥಿತಿ SRT_DistFifo:
    ಸ್ಥಿತಿ SRT_ಕೋಷ್ಟಕ:
    ಸ್ಥಿತಿ SRT_EphemTab: {
      ಪೂರ್ಣಾಂಕ r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      ಪೂರ್ಣಾಂಕ r2 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      ಪೂರ್ಣಾಂಕ iParm = pDest->iSDParm;
      testcase( pDest->eDest==SRT_ಕೋಷ್ಟಕ );
      testcase( pDest->eDest==SRT_EphemTab );
      testcase( pDest->eDest==SRT_Fifo );
      testcase( pDest->eDest==SRT_DistFifo );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, p_ಒಳ->iSdst, p_ಒಳ->nSdst, r1);
#ಆದರೆ !defined(SQLITE_ENABLE_NULL_TRIM) && defined(SQLITE_DEBUG)
      /* A destination of SRT_Table and a non-zero iSDParm2 parameter means
      ** that this is an "UPDATE ... FROM" on a virtual table or view. In this
      ** case set the p5 parameter of the OP_MakeRecord to OPFLAG_NOCHNG_MAGIC.
      ** This does not affect operation in any way - it just allows MakeRecord
      ** to process OPFLAG_NOCHANGE values without an assert() failing. */
      ಆದರೆ( pDest->eDest==SRT_ಕೋಷ್ಟಕ && pDest->iSDParm2 ){
        sqlite3VdbeChangeP5(v, OPFLAG_NOCHNG_MAGIC);
      }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
      ಆದರೆ( pDest->eDest==SRT_DistFifo ){
        /* If the destination is DistFifo, then cursor (iParm+1) is open
        ** on an ephemeral index that is used to enforce uniqueness on the
        ** total result.  At this point, we are processing the setup portion
        ** of the recursive CTE using the merge algorithm, so the results are
        ** guaranteed to be unique anyhow.  But we still need to populate the
        ** (iParm+1) cursor for use by the subsequent recursive phase.
        */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iParm+1, r1,
                             p_ಒಳ->iSdst, p_ಒಳ->nSdst);
      }
#ಆದರೆ_ಅಂತ್ಯ
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_NewRowid, iParm, r2);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸೇರಿಸು, iParm, r1, r2);
      sqlite3VdbeChangeP5(v, OPFLAG_APPEND);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r2);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);
      ನಿಲ್ಲಿಸು;
    }

    /* If any row exist in the result set, record that fact and abort.
    */
    ಸ್ಥಿತಿ SRT_Exists: {
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, pDest->iSDParm);
      /* The LIMIT clause will terminate the loop for us */
      ನಿಲ್ಲಿಸು;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    /* If we are creating a set for an "expr IN (SELECT ...)".
    */
    ಸ್ಥಿತಿ SRT_ಗಣ: {
      ಪೂರ್ಣಾಂಕ r1;
      testcase( p_ಒಳ->nSdst>1 );
      r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Make_ದಾಖಲೆ, p_ಒಳ->iSdst, p_ಒಳ->nSdst,
          r1, pDest->zAffSdst, p_ಒಳ->nSdst);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, pDest->iSDParm, r1,
                           p_ಒಳ->iSdst, p_ಒಳ->nSdst);
      ಆದರೆ( pDest->iSDParm2>0 ){
        ಪ್ರತಿಜ್ಞಾ( sqlite3CursorBloomable(pParse, pDest->iSDParm, p_ಒಳ->nSdst) );
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Filter_ಕೂಡಿಸು, pDest->iSDParm2, 0,
                             p_ಒಳ->iSdst, p_ಒಳ->nSdst);
        ExplainQueryPlan((pParse, 0, "CREATE BLOOM FILTER"));
      }
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);
      ನಿಲ್ಲಿಸು;
    }

    /* If this is a scalar select that is part of an expression, then
    ** store the results in the appropriate memory cell and break out
    ** of the scan loop.  Note that the select might return multiple columns
    ** if it is the RHS of a row-value IN operator.
    */
    ಸ್ಥಿತಿ SRT_Mem: {
      testcase( p_ಒಳ->nSdst>1 );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Move(pParse, p_ಒಳ->iSdst, pDest->iSDParm, p_ಒಳ->nSdst);
      /* The LIMIT clause will jump out of the loop for us */
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* #ifndef SQLITE_OMIT_SUBQUERY */

    /* The results are stored in a sequence of registers
    ** starting at pDest->iSdst.  Then the co-routine yields.
    */
    ಸ್ಥಿತಿ SRT_Coroutine: {
      ಆದರೆ( pDest->iSdst==0 ){
        pDest->iSdst = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, p_ಒಳ->nSdst);
        pDest->nSdst = p_ಒಳ->nSdst;
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Move(pParse, p_ಒಳ->iSdst, pDest->iSdst, p_ಒಳ->nSdst);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Yield, pDest->iSDParm);
      ನಿಲ್ಲಿಸು;
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
    /* Write the results into a priority queue that is order according to
    ** pDest->pOrderBy (in pSO).  pDest->iSDParm (in iParm) is the cursor for an
    ** index with pSO->nExpr+2 columns.  Build a key using pSO for the first
    ** pSO->nExpr columns, then make sure all keys are unique by adding a
    ** final OP_Sequence column.  The last column is the record as a blob.
    */
    ಸ್ಥಿತಿ SRT_Dist_ಸರತಿ:
    ಸ್ಥಿತಿ SRT_ಸರತಿ: {
      ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ;
      ಪೂರ್ಣಾಂಕ r1, r2, r3, ii;
      ಅಭಿವ್ಯಕ್ತಿ_List *pSO;
      ಪೂರ್ಣಾಂಕ iParm = pDest->iSDParm;
      pSO = pDest->p_ಕ್ರಮ_By;
      ಪ್ರತಿಜ್ಞಾ( pSO );
      n_ಕೀಲಿ = pSO->n_ಅಭಿವ್ಯಕ್ತಿ;
      r1 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
      r2 = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_ಕೀಲಿ+2);
      r3 = r2+n_ಕೀಲಿ+1;

      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, p_ಒಳ->iSdst, p_ಒಳ->nSdst, r3);
      ಆದರೆ( pDest->eDest==SRT_Dist_ಸರತಿ ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Idx_ಸೇರಿಸು, iParm+1, r3);
      }
      ಗಾಗಿ(ii=0; ii<n_ಕೀಲಿ; ii++){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_SCopy,
                          p_ಒಳ->iSdst + pSO->a[ii].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು - 1,
                          r2+ii);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Sequence, iParm, r2+n_ಕೀಲಿ);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, r2, n_ಕೀಲಿ+2, r1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, iParm, r1, r2, n_ಕೀಲಿ+2);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, r1);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, r2, n_ಕೀಲಿ+2);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_CTE */

    /* Ignore the output */
    ಸ್ಥಿತಿ SRT_Discard: {
      ನಿಲ್ಲಿಸು;
    }

    /* If none of the above, then the result destination must be
    ** SRT_Output.
    **
    ** For SRT_Output, results are stored in a sequence of registers. 
    ** Then the OP_ResultRow opcode is used to cause sqlite3_step() to
    ** return the next row of result.
    */
    ಪೂರ್ವನಿಯೋಜಿತ: {
      ಪ್ರತಿಜ್ಞಾ( pDest->eDest==SRT_Output );
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಫಲಿತಾಂಶ_ಅಡ್ಡಸಾಲು, p_ಒಳ->iSdst, p_ಒಳ->nSdst);
      ನಿಲ್ಲಿಸು;
    }
  }

  /* Jump to the end of the loop if the LIMIT is reached.
  */
  ಆದರೆ( p->i_ಎಲ್ಲೆ ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_DecrJumpZero, p->i_ಎಲ್ಲೆ, iBreak); VdbeCoverage(v);
  }

  /* Generate the subroutine return
  */
  sqlite3VdbeResolveLabel(v, iContinue);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Return, regReturn);

  ಹಿಂತಿರುಗಿ addr;
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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPOUND_SELECT
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ multi_Select_By_ವಿಲೀನ(
  Parse *pParse,        /* Parsing context */
  Select *p,            /* The right-most of SELECTs to be coded */
  SelectDest *pDest     /* What to do with query results */
){
  ಪೂರ್ಣಾಂಕ i, j;             /* Loop counters */
  Select *p_ಪೂರ್ವ;       /* Another SELECT immediately to our left */
  Select *p_ವಿಭಜಿಸು;       /* Left-most SELECT in the right-hand group */
  ಪೂರ್ಣಾಂಕ nSelect;          /* Number of SELECT statements in the compound */
  Vdbe *v;              /* Generate code to this VDBE */
  SelectDest destA;     /* Destination for coroutine A */
  SelectDest destB;     /* Destination for coroutine B */
  ಪೂರ್ಣಾಂಕ regAddrA;         /* Address register for select-A coroutine */
  ಪೂರ್ಣಾಂಕ regAddrB;         /* Address register for select-B coroutine */
  ಪೂರ್ಣಾಂಕ addrSelectA;      /* Address of the select-A coroutine */
  ಪೂರ್ಣಾಂಕ addrSelectB;      /* Address of the select-B coroutine */
  ಪೂರ್ಣಾಂಕ reg_ಹೊರಹೋಗು_A;          /* Address register for the output-A subroutine */
  ಪೂರ್ಣಾಂಕ reg_ಹೊರಹೋಗು_B;          /* Address register for the output-B subroutine */
  ಪೂರ್ಣಾಂಕ addr_ಹೊರಹೋಗು_A;         /* Address of the output-A subroutine */
  ಪೂರ್ಣಾಂಕ addr_ಹೊರಹೋಗು_B = 0;     /* Address of the output-B subroutine */
  ಪೂರ್ಣಾಂಕ addrEofA;         /* Address of the select-A-exhausted subroutine */
  ಪೂರ್ಣಾಂಕ addrEofA_noB;     /* Alternate addrEofA if B is uninitialized */
  ಪೂರ್ಣಾಂಕ addrEofB;         /* Address of the select-B-exhausted subroutine */
  ಪೂರ್ಣಾಂಕ addrAltB;         /* Address of the A<B subroutine */
  ಪೂರ್ಣಾಂಕ addrAeqB;         /* Address of the A==B subroutine */
  ಪೂರ್ಣಾಂಕ addrAgtB;         /* Address of the A>B subroutine */
  ಪೂರ್ಣಾಂಕ reg_ಎಲ್ಲೆ_A;        /* Limit register for select-A */
  ಪೂರ್ಣಾಂಕ reg_ಎಲ್ಲೆ_B;        /* Limit register for select-A */
  ಪೂರ್ಣಾಂಕ regPrev;          /* A range of registers to hold previous output */
  ಪೂರ್ಣಾಂಕ saved_ಎಲ್ಲೆ;       /* Saved value of p->iLimit */
  ಪೂರ್ಣಾಂಕ saved_ಸ್ಥಾನಾಂತರ;      /* Saved value of p->iOffset */
  ಪೂರ್ಣಾಂಕ labelCmpr;        /* Label for the start of the merge algorithm */
  ಪೂರ್ಣಾಂಕ label_ಕೊನೆ;         /* Label for the end of the overall SELECT stmt */
  ಪೂರ್ಣಾಂಕ addr1;            /* Jump instructions that get retargeted */
  ಪೂರ್ಣಾಂಕ op;               /* One of TK_ALL, TK_UNION, TK_EXCEPT, TK_INTERSECT */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_Dup = 0; /* Comparison information for duplicate removal */
  ಕೀಲಿ_Info *p_ಕೀಲಿ_ವಿಲೀನ;   /* Comparison information for merging rows */
  sqlite3 *db;          /* Database connection */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;   /* The ORDER BY clause */
  ಪೂರ್ಣಾಂಕ n_ಕ್ರಮ_By;         /* Number of terms in the ORDER BY clause */
  u32 *a_ಕ್ರಮಬದಲಾವಣೆ;        /* Mapping from ORDER BY terms to result set columns */

  ಪ್ರತಿಜ್ಞಾ( p->p_ಕ್ರಮ_By!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಕೀಲಿ_Dup==0 ); /* "Managed" code needs this.  Ticket #3382. */
  db = pParse->db;
  v = pParse->pVdbe;
  ಪ್ರತಿಜ್ಞಾ( v!=0 );       /* Already thrown the error if VDBE alloc failed */
  label_ಕೊನೆ = sqlite3VdbeMakeLabel(pParse);
  labelCmpr = sqlite3VdbeMakeLabel(pParse);


  /* Patch up the ORDER BY clause
  */
  op = p->op; 
  ಪ್ರತಿಜ್ಞಾ( p->p_ಪೂರ್ವ->p_ಕ್ರಮ_By==0 );
  p_ಕ್ರಮ_By = p->p_ಕ್ರಮ_By;
  ಪ್ರತಿಜ್ಞಾ( p_ಕ್ರಮ_By );
  n_ಕ್ರಮ_By = p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ;

  /* For operators other than UNION ALL we have to make sure that
  ** the ORDER BY clause covers every term of the result set.  Add
  ** terms to the ORDER BY clause as necessary.  Do the same for UNION ALL
  ** if any operator to the left is not UNION ALL, so that the ORDER BY
  ** carries the collation of every column into that sub-compound.
  */
  ಗಾಗಿ(p_ಪೂರ್ವ=p; p_ಪೂರ್ವ && p_ಪೂರ್ವ->op==TK_ALL; p_ಪೂರ್ವ=p_ಪೂರ್ವ->p_ಪೂರ್ವ){}
  ಆದರೆ( p_ಪೂರ್ವ && p_ಪೂರ್ವ->p_ಪೂರ್ವ ){
    ಗಾಗಿ(i=1; db->mallocFailed==0 && i<=p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
      ಗಾಗಿ(j=0, p_ಅಂಶ=p_ಕ್ರಮ_By->a; j<n_ಕ್ರಮ_By; j++, p_ಅಂಶ++){
        ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ!=0 );
        ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು>0 );
        ಆದರೆ( p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು==i ) ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( j==n_ಕ್ರಮ_By ){
        ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, i);
        ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM_BKPT;
        p->p_ಕ್ರಮ_By = p_ಕ್ರಮ_By = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, p_ಕ್ರಮ_By, pNew);
        ಆದರೆ( p_ಕ್ರಮ_By ) p_ಕ್ರಮ_By->a[n_ಕ್ರಮ_By++].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = (u16)i;
      }
    }
  }

  /* Compute the comparison permutation and keyinfo that is used with
  ** the permutation to determine if the next row of results comes
  ** from selectA or selectB.  Also add literal collations to the
  ** ORDER BY clause terms so that when selectA and selectB are
  ** evaluated, they use the correct collation.
  */
  a_ಕ್ರಮಬದಲಾವಣೆ = sqlite3DbMallocRawNN(db, ಗಾತ್ರ(u32)*(n_ಕ್ರಮ_By + 1));
  ಆದರೆ( a_ಕ್ರಮಬದಲಾವಣೆ ){
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
    ಪೂರ್ಣಾಂಕ bKeep = 0;
    a_ಕ್ರಮಬದಲಾವಣೆ[0] = n_ಕ್ರಮ_By;
    ಗಾಗಿ(i=1, p_ಅಂಶ=p_ಕ್ರಮ_By->a; i<=n_ಕ್ರಮ_By; i++, p_ಅಂಶ++){
      ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು>0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು<=p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
      a_ಕ್ರಮಬದಲಾವಣೆ[i] = p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು - 1;
      ಆದರೆ( a_ಕ್ರಮಬದಲಾವಣೆ[i]!=(u32)i-1 ) bKeep = 1;
    }
     ಆದರೆ( bKeep==0 ){
      sqlite3DbFreeNN(db, a_ಕ್ರಮಬದಲಾವಣೆ);
      a_ಕ್ರಮಬದಲಾವಣೆ = 0;
    }
  }
  p_ಕೀಲಿ_ವಿಲೀನ = multi_Select_By_ವಿಲೀನ_ಕೀಲಿ_Info(pParse, p, 1);

  /* Allocate a range of temporary registers and the KeyInfo needed
  ** for the logic that removes duplicate result rows when the
  ** operator is UNION, EXCEPT, or INTERSECT (but not UNION ALL).
  */
  ಆದರೆ( op==TK_ALL ){
    regPrev = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ n_ಅಭಿವ್ಯಕ್ತಿ = p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
    ಪ್ರತಿಜ್ಞಾ( n_ಕ್ರಮ_By>=n_ಅಭಿವ್ಯಕ್ತಿ || db->mallocFailed );
    regPrev = pParse->nMem+1;
    pParse->nMem += n_ಅಭಿವ್ಯಕ್ತಿ+1;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, regPrev);
    p_ಕೀಲಿ_Dup = sqlite3_ಕೀಲಿ_Info_Alloc(db, n_ಅಭಿವ್ಯಕ್ತಿ, 1);
    ಆದರೆ( p_ಕೀಲಿ_Dup ){
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಕೀಲಿ_Info_Is_Writeable(p_ಕೀಲಿ_Dup) );
      /* Use the ORDER BY collations, not multiSelectCollSeq(p), because
      ** p might be just one part of a larger compound, in which case
      ** the ORDER BY carries the COLLATE of the larger compound. */
      ಗಾಗಿ(i=0; ALWAYS(p_ಕೀಲಿ_ವಿಲೀನ) && i<n_ಕ್ರಮ_By; i++){
        p_ಕೀಲಿ_Dup->aColl[p_ಕ್ರಮ_By->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು-1] = p_ಕೀಲಿ_ವಿಲೀನ->aColl[i];
      }
    }
  }

  /* Separate the left and the right query from one another
  */
  nSelect = 1;
  ಆದರೆ( (op==TK_ALL || op==TK_UNION)
   && OptimizationEnabled(db, SQLITE_ಸಮತೋಲಿತ_ವಿಲೀನ)
  ){
    ಗಾಗಿ(p_ವಿಭಜಿಸು=p; p_ವಿಭಜಿಸು->p_ಪೂರ್ವ!=0 && p_ವಿಭಜಿಸು->op==op; p_ವಿಭಜಿಸು=p_ವಿಭಜಿಸು->p_ಪೂರ್ವ){
      nSelect++;
      ಪ್ರತಿಜ್ಞಾ( p_ವಿಭಜಿಸು->p_ಪೂರ್ವ->p_ಮುಂದಿನ==p_ವಿಭಜಿಸು );
    }
  }
  ಆದರೆ( nSelect<=3 ){
    p_ವಿಭಜಿಸು = p;
  }ಇಲ್ಲದಿದ್ದರೆ{
    p_ವಿಭಜಿಸು = p;
    ಗಾಗಿ(i=2; i<nSelect; i+=2){ p_ವಿಭಜಿಸು = p_ವಿಭಜಿಸು->p_ಪೂರ್ವ; }
  }
  p_ಪೂರ್ವ = p_ವಿಭಜಿಸು->p_ಪೂರ್ವ;
  ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ!=0 );
  p_ವಿಭಜಿಸು->p_ಪೂರ್ವ = 0;
  p_ಪೂರ್ವ->p_ಮುಂದಿನ = 0;
  ಪ್ರತಿಜ್ಞಾ( p->p_ಕ್ರಮ_By == p_ಕ್ರಮ_By );
  ಪ್ರತಿಜ್ಞಾ( p_ಕ್ರಮ_By!=0 || db->mallocFailed );
  p_ಪೂರ್ವ->p_ಕ್ರಮ_By = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(pParse->db, p_ಕ್ರಮ_By, 0);
  sqlite3_Resolve_ಕ್ರಮ_ಗುಂಪು_By(pParse, p, p->p_ಕ್ರಮ_By, "ORDER");
  sqlite3_Resolve_ಕ್ರಮ_ಗುಂಪು_By(pParse, p_ಪೂರ್ವ, p_ಪೂರ್ವ->p_ಕ್ರಮ_By, "ORDER");

  /* Compute the limit registers */
  compute_ಎಲ್ಲೆ_Registers(pParse, p, label_ಕೊನೆ);
  ಆದರೆ( p->i_ಎಲ್ಲೆ && (op==TK_ALL || union_With_ಎಲ್ಲೆ_One(db,p)) ){
    reg_ಎಲ್ಲೆ_A = ++pParse->nMem;
    reg_ಎಲ್ಲೆ_B = ++pParse->nMem;
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Copy, p->i_ಸ್ಥಾನಾಂತರ ? p->i_ಸ್ಥಾನಾಂತರ+1 : p->i_ಎಲ್ಲೆ,
                                  reg_ಎಲ್ಲೆ_A);
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Copy, reg_ಎಲ್ಲೆ_A, reg_ಎಲ್ಲೆ_B);
  }ಇಲ್ಲದಿದ್ದರೆ{
    reg_ಎಲ್ಲೆ_A = reg_ಎಲ್ಲೆ_B = 0;
  }
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p->p_ಎಲ್ಲೆ);
  p->p_ಎಲ್ಲೆ = 0;

  regAddrA = ++pParse->nMem;
  regAddrB = ++pParse->nMem;
  reg_ಹೊರಹೋಗು_A = ++pParse->nMem;
  reg_ಹೊರಹೋಗು_B = ++pParse->nMem;
  sqlite3SelectDestInit(&destA, SRT_Coroutine, regAddrA);
  sqlite3SelectDestInit(&destB, SRT_Coroutine, regAddrB);

  ExplainQueryPlan((pParse, 1, "MERGE (%s)", sqlite3SelectOpName(p->op)));

  /* Generate a coroutine to evaluate the SELECT statement to the
  ** left of the compound operator - the "A" select.
  */
  addrSelectA = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_InitCoroutine, regAddrA, 0, addrSelectA);
  VdbeComment((v, "SUBR: next-A"));
  p_ಪೂರ್ವ->i_ಎಲ್ಲೆ = reg_ಎಲ್ಲೆ_A;
  ExplainQueryPlan((pParse, 1, "LEFT"));
  sqlite3Select(pParse, p_ಪೂರ್ವ, &destA);
  sqlite3_Vdbe_ಕೊನೆ_Coroutine(v, regAddrA);
  sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);

  /* Generate a coroutine to evaluate the SELECT statement on
  ** the right - the "B" select
  */
  addrSelectB = sqlite3VdbeCurrentAddr(v) + 1;
  addr1 = sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_InitCoroutine, regAddrB, 0, addrSelectB);
  VdbeComment((v, "SUBR: next-B"));
  saved_ಎಲ್ಲೆ = p->i_ಎಲ್ಲೆ;
  saved_ಸ್ಥಾನಾಂತರ = p->i_ಸ್ಥಾನಾಂತರ;
  p->i_ಎಲ್ಲೆ = reg_ಎಲ್ಲೆ_B;
  p->i_ಸ್ಥಾನಾಂತರ = 0; 
  ExplainQueryPlan((pParse, 1, "RIGHT"));
  sqlite3Select(pParse, p, &destB);
  p->i_ಎಲ್ಲೆ = saved_ಎಲ್ಲೆ;
  p->i_ಸ್ಥಾನಾಂತರ = saved_ಸ್ಥಾನಾಂತರ;
  sqlite3_Vdbe_ಕೊನೆ_Coroutine(v, regAddrB);

  /* Generate a subroutine that outputs the current row of the A
  ** select as the next output row of the compound select.
  */
  VdbeNoopComment((v, "SUBR: out-A"));
  addr_ಹೊರಹೋಗು_A = generateOutputSubroutine(pParse,
                 p, &destA, pDest, reg_ಹೊರಹೋಗು_A,
                 regPrev, p_ಕೀಲಿ_Dup, label_ಕೊನೆ);
 
  /* Generate a subroutine that outputs the current row of the B
  ** select as the next output row of the compound select.
  */
  ಆದರೆ( op==TK_ALL || op==TK_UNION ){
    VdbeNoopComment((v, "SUBR: out-B"));
    addr_ಹೊರಹೋಗು_B = generateOutputSubroutine(pParse,
                 p, &destB, pDest, reg_ಹೊರಹೋಗು_B,
                 regPrev, p_ಕೀಲಿ_Dup, label_ಕೊನೆ);
  }
  sqlite3_ಕೀಲಿ_Info_Unref(p_ಕೀಲಿ_Dup);

  /* Generate a subroutine to run when the results from select A
  ** are exhausted and only data in select B remains.
  */
  ಆದರೆ( op==TK_EXCEPT || op==TK_INTERSECT ){
    addrEofA_noB = addrEofA = label_ಕೊನೆ;
  }ಇಲ್ಲದಿದ್ದರೆ{ 
    VdbeNoopComment((v, "SUBR: eof-A"));
    addrEofA = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, reg_ಹೊರಹೋಗು_B, addr_ಹೊರಹೋಗು_B);
    VdbeComment((v, "out-B"));
    addrEofA_noB = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Yield, regAddrB, label_ಕೊನೆ);
                                     VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, addrEofA);
    p->n_Select_ಅಡ್ಡಸಾಲು = sqlite3_Log_Est_ಕೂಡಿಸು(p->n_Select_ಅಡ್ಡಸಾಲು, p_ಪೂರ್ವ->n_Select_ಅಡ್ಡಸಾಲು);
  }

  /* Generate a subroutine to run when the results from select B
  ** are exhausted and only data in select A remains.
  */
  ಆದರೆ( op==TK_INTERSECT ){
    addrEofB = addrEofA;
    ಆದರೆ( p->n_Select_ಅಡ್ಡಸಾಲು > p_ಪೂರ್ವ->n_Select_ಅಡ್ಡಸಾಲು ) p->n_Select_ಅಡ್ಡಸಾಲು = p_ಪೂರ್ವ->n_Select_ಅಡ್ಡಸಾಲು;
  }ಇಲ್ಲದಿದ್ದರೆ{ 
    VdbeNoopComment((v, "SUBR: eof-B"));
    addrEofB = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, reg_ಹೊರಹೋಗು_A, addr_ಹೊರಹೋಗು_A);
    VdbeComment((v, "out-A"));
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Yield, regAddrA, label_ಕೊನೆ); VdbeCoverage(v);
    VdbeComment((v, "next-A"));
    sqlite3VdbeGoto(v, addrEofB);
  }

  /* Generate code to handle the case of A<B
  */
  addrAltB = sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, reg_ಹೊರಹೋಗು_A, addr_ಹೊರಹೋಗು_A);
  VdbeComment((v, "out-A"));
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Yield, regAddrA, addrEofA); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  sqlite3VdbeGoto(v, labelCmpr);

  /* Generate code to handle the case of A==B
  */
  ಆದರೆ( op==TK_ALL ){
    addrAeqB = addrAltB;
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op==TK_INTERSECT ){
    addrAeqB = addrAltB;
    addrAltB++;
  }ಇಲ್ಲದಿದ್ದರೆ{
    addrAeqB = addrAltB + 1;
  }

  /* Generate code to handle the case of A>B
  */
  addrAgtB = sqlite3VdbeCurrentAddr(v);
  ಆದರೆ( op==TK_ALL || op==TK_UNION ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, reg_ಹೊರಹೋಗು_B, addr_ಹೊರಹೋಗು_B);
    VdbeComment((v, "out-B"));
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
    VdbeComment((v, "next-B"));
    sqlite3VdbeGoto(v, labelCmpr);
  }ಇಲ್ಲದಿದ್ದರೆ{
    addrAgtB++; /* Just do next-B.  Might as well use the next-B call
                ** in the next code block */
  }

  /* This code runs once to initialize everything.
  */
  sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Yield, regAddrA, addrEofA_noB); VdbeCoverage(v);
  VdbeComment((v, "next-A"));
  /* v---  Also the A>B case for EXCEPT and INTERSECT */
  sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Yield, regAddrB, addrEofB); VdbeCoverage(v);
  VdbeComment((v, "next-B"));

  /* Implement the main merge loop
  */
  ಆದರೆ( a_ಕ್ರಮಬದಲಾವಣೆ!=0 ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Permutation, 0, 0, 0, (ಅಕ್ಷರ*)a_ಕ್ರಮಬದಲಾವಣೆ, P4_INTARRAY);
  }
  sqlite3VdbeResolveLabel(v, labelCmpr);
  sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Compare, destA.iSdst, destB.iSdst, n_ಕ್ರಮ_By,
                         (ಅಕ್ಷರ*)p_ಕೀಲಿ_ವಿಲೀನ, P4_KEYINFO);
  ಆದರೆ( a_ಕ್ರಮಬದಲಾವಣೆ!=0 ){
    sqlite3VdbeChangeP5(v, OPFLAG_ಕ್ರಮಬದಲಾವಣೆ);
  }
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Jump, addrAltB, addrAeqB, addrAgtB);
  VdbeCoverageIf(v, op==TK_ALL);
  VdbeCoverageIf(v, op==TK_UNION);
  VdbeCoverageIf(v, op==TK_EXCEPT);
  VdbeCoverageIf(v, op==TK_INTERSECT);

  /* Jump to the this point in order to terminate the query.
  */
  sqlite3VdbeResolveLabel(v, label_ಕೊನೆ);

  /* Make arrangements to free the 2nd and subsequent arms of the compound
  ** after the parse has finished */
  ಆದರೆ( p_ವಿಭಜಿಸು->p_ಪೂರ್ವ ){
    sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, sqlite3SelectDeleteGeneric, p_ವಿಭಜಿಸು->p_ಪೂರ್ವ);
  }
  p_ವಿಭಜಿಸು->p_ಪೂರ್ವ = p_ಪೂರ್ವ;
  p_ಪೂರ್ವ->p_ಮುಂದಿನ = p_ವಿಭಜಿಸು;
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_ಪೂರ್ವ->p_ಕ್ರಮ_By);
  p_ಪೂರ್ವ->p_ಕ್ರಮ_By = 0;

  /*** TBD:  Insert subroutine calls to close cursors on incomplete
  **** subqueries ****/
  Explain_Query_Plan_ತೆಗೆ(pParse);
  ಹಿಂತಿರುಗಿ pParse->nErr!=0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)

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
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ SubstContext {
  Parse *pParse;            /* The parsing context */
  ಪೂರ್ಣಾಂಕ i_ಕೋಷ್ಟಕ;               /* Replace references to this table */
  ಪೂರ್ಣಾಂಕ i_New_ಕೋಷ್ಟಕ;            /* New table number */
  ಪೂರ್ಣಾಂಕ isOuterJoin;          /* Add TK_IF_NULL_ROW opcodes on each replacement */
  ಪೂರ್ಣಾಂಕ n_Sel_ಆಳ;            /* Depth of sub-query recursion.  Top==1 */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;         /* Replacement expressions */
  ಅಭಿವ್ಯಕ್ತಿ_List *pCList;         /* Collation sequences for replacement expr */
} SubstContext;

/* Forward Declarations */
ಸ್ಥಿರ ಶೂನ್ಯ subst_ಅಭಿವ್ಯಕ್ತಿ_List(SubstContext*, ಅಭಿವ್ಯಕ್ತಿ_List*);
ಸ್ಥಿರ ಶೂನ್ಯ substSelect(SubstContext*, Select*, ಪೂರ್ಣಾಂಕ);

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
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON|EP_InnerON)
   && p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin==pSubst->i_ಕೋಷ್ಟಕ
  ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) );
    p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin = pSubst->i_New_ಕೋಷ್ಟಕ;
  }
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ
   && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==pSubst->i_ಕೋಷ್ಟಕ
   && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Fixed_ಲಂಬಸಾಲು)
  ){
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ<0 ){
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
    {
      ಅಭಿವ್ಯಕ್ತಿ *pNew;
      ಪೂರ್ಣಾಂಕ i_ಸ್ತಂಭ;
      ಅಭಿವ್ಯಕ್ತಿ *pCopy;
      ಅಭಿವ್ಯಕ್ತಿ if_Null_ಅಡ್ಡಸಾಲು;
      i_ಸ್ತಂಭ = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
      ಪ್ರತಿಜ್ಞಾ( i_ಸ್ತಂಭ>=0 );
      ಪ್ರತಿಜ್ಞಾ( pSubst->pEList!=0 && i_ಸ್ತಂಭ<pSubst->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==0 );
      pCopy = pSubst->pEList->a[i_ಸ್ತಂಭ].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_ಸದಿಶ(pCopy) ){
        sqlite3_ಸದಿಶ_Error_ಸಂದೇಶ(pSubst->pParse, pCopy);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3 *db = pSubst->pParse->db;
        ಆದರೆ( pSubst->isOuterJoin
         && (pCopy->op!=TK_ಸ್ತಂಭ || pCopy->i_ಕೋಷ್ಟಕ!=pSubst->i_New_ಕೋಷ್ಟಕ)
        ){
          memset(&if_Null_ಅಡ್ಡಸಾಲು, 0, ಗಾತ್ರ(if_Null_ಅಡ್ಡಸಾಲು));
          if_Null_ಅಡ್ಡಸಾಲು.op = TK_IF_NULL_ಅಡ್ಡಸಾಲು;
          if_Null_ಅಡ್ಡಸಾಲು.p_ಎಡ = pCopy;
          if_Null_ಅಡ್ಡಸಾಲು.i_ಕೋಷ್ಟಕ = pSubst->i_New_ಕೋಷ್ಟಕ;
          if_Null_ಅಡ್ಡಸಾಲು.i_ಸ್ತಂಭ = -99;
          if_Null_ಅಡ್ಡಸಾಲು.flags = EP_If_Null_ಅಡ್ಡಸಾಲು;
          pCopy = &if_Null_ಅಡ್ಡಸಾಲು;
        }
        testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pCopy, EP_Subquery) );
        pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pCopy, 0);
        ಆದರೆ( db->mallocFailed ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pNew);
          ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ;
        }
        ಆದರೆ( pSubst->isOuterJoin ){
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pNew, EP_CanBeNull);
        }
        ಆದರೆ( pNew->op==TK_TRUEFALSE ){
          pNew->u.i_ಮೌಲ್ಯ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(pNew);
          pNew->op = TK_INTEGER;
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pNew, EP_Int_ಮೌಲ್ಯ);
        }

        /* Ensure that the expression now has an implicit collation sequence,
        ** just as it did when it was a column of a view or sub-query. */
        {
          CollSeq *pNat = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pSubst->pParse, pNew);
          CollSeq *pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pSubst->pParse,
                pSubst->pCList->a[i_ಸ್ತಂಭ].p_ಅಭಿವ್ಯಕ್ತಿ
          );
          ಆದರೆ( pNat!=pColl || (pNew->op!=TK_ಸ್ತಂಭ && pNew->op!=TK_COLLATE) ){
            pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(pSubst->pParse, pNew,
                (pColl ? pColl->zName : "BINARY")
            );
          }
        }
        ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(pNew, EP_Collate);
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_OuterON|EP_InnerON) ){
          sqlite3_ಗಣ_Join_ಅಭಿವ್ಯಕ್ತಿ(pNew, p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin,
                             p_ಅಭಿವ್ಯಕ್ತಿ->flags & (EP_OuterON|EP_InnerON));
        }
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
        p_ಅಭಿವ್ಯಕ್ತಿ = pNew;
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ಅಡ್ಡಸಾಲು && p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==pSubst->i_ಕೋಷ್ಟಕ ){
      p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = pSubst->i_New_ಕೋಷ್ಟಕ;
    }
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION && p_ಅಭಿವ್ಯಕ್ತಿ->op2>=pSubst->n_Sel_ಆಳ ){
      p_ಅಭಿವ್ಯಕ್ತಿ->op2--;
    }
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
      substSelect(pSubst, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect, 1);
    }ಇಲ್ಲದಿದ್ದರೆ{
      subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
      ಕಿಟಕಿ *pWin = p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin;
      pWin->pFilter = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, pWin->pFilter);
      subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, pWin->p_ವಿಭಜನೆ);
      subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, pWin->p_ಕ್ರಮ_By);
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ;
}
ಸ್ಥಿರ ಶೂನ್ಯ subst_ಅಭಿವ್ಯಕ್ತಿ_List(
  SubstContext *pSubst, /* Description of the substitution */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList       /* List to scan and in which to make substitutes */
){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pList==0 ) ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ);
  }
}
ಸ್ಥಿರ ಶೂನ್ಯ substSelect(
  SubstContext *pSubst, /* Description of the substitution */
  Select *p,            /* SELECT statement in which to make substitutions */
  ಪೂರ್ಣಾಂಕ do_ಪೂರ್ವ           /* Do substitutes on p->pPrior too */
){
  SrcList *pSrc;
  Src_ಅಂಶ *p_ಅಂಶ;
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( !p ) ಹಿಂತಿರುಗಿ;
  pSubst->n_Sel_ಆಳ++;
  ಮಾಡು{
    subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, p->pEList);
    subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, p->p_ಗುಂಪು_By);
    subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, p->p_ಕ್ರಮ_By);
    p->pHaving = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, p->pHaving);
    p->pWhere = subst_ಅಭಿವ್ಯಕ್ತಿ(pSubst, p->pWhere);
    pSrc = p->pSrc;
    ಪ್ರತಿಜ್ಞಾ( pSrc!=0 );
    ಗಾಗಿ(i=pSrc->nSrc, p_ಅಂಶ=pSrc->a; i>0; i--, p_ಅಂಶ++){
      ಆದರೆ( p_ಅಂಶ->fg.isSubquery ){
        substSelect(pSubst, p_ಅಂಶ->u4.pSubq->pSelect, 1);
      }
      ಆದರೆ( p_ಅಂಶ->fg.isTabFunc ){
        subst_ಅಭಿವ್ಯಕ್ತಿ_List(pSubst, p_ಅಂಶ->u1.pFuncArg);
      }
    }
  }ಆಗ( do_ಪೂರ್ವ && (p = p->p_ಪೂರ್ವ)!=0 );
  pSubst->n_Sel_ಆಳ--;
}
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ಆದರೆ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
/*
** pSelect is a SELECT statement and pSrcItem is one item in the FROM
** clause of that SELECT.
**
** This routine scans the entire SELECT statement and recomputes the
** pSrcItem->colUsed mask.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ recompute_Columns_ಬಳಸಿದ_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  Src_ಅಂಶ *p_ಅಂಶ;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_ಸ್ತಂಭ ) ಹಿಂತಿರುಗಿ WRC_Continue;
  p_ಅಂಶ = pWalker->u.p_Src_ಅಂಶ;
  ಆದರೆ( p_ಅಂಶ->iCursor!=p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ<0 ) ಹಿಂತಿರುಗಿ WRC_Continue;
  p_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಲಂಬಸಾಲು_ಬಳಸಿದ(p_ಅಭಿವ್ಯಕ್ತಿ);
  ಹಿಂತಿರುಗಿ WRC_Continue;
}
ಸ್ಥಿರ ಶೂನ್ಯ recompute_Columns_ಬಳಸಿದ(
  Select *pSelect,                 /* The complete SELECT statement */
  Src_ಅಂಶ *p_Src_ಅಂಶ                /* Which FROM clause item to recompute */
){
  Walker w;
  ಆದರೆ( NEVER(p_Src_ಅಂಶ->pSTab==0) ) ಹಿಂತಿರುಗಿ;
  memset(&w, 0, ಗಾತ್ರ(w));
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = recompute_Columns_ಬಳಸಿದ_ಅಭಿವ್ಯಕ್ತಿ;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.u.p_Src_ಅಂಶ = p_Src_ಅಂಶ;
  p_Src_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ = 0;
  sqlite3WalkSelect(&w, pSelect);
}
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ಆದರೆ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
  ಪೂರ್ಣಾಂಕ *aCsrMap,                   /* Array to store cursor mappings in */
  SrcList *pSrc,                  /* FROM clause to renumber */
  ಪೂರ್ಣಾಂಕ iExcept                     /* FROM clause item to skip */
){
  ಪೂರ್ಣಾಂಕ i;
  Src_ಅಂಶ *p_ಅಂಶ;
  ಗಾಗಿ(i=0, p_ಅಂಶ=pSrc->a; i<pSrc->nSrc; i++, p_ಅಂಶ++){
    ಆದರೆ( i!=iExcept ){
      Select *p;
      ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->iCursor < aCsrMap[0] );
      ಆದರೆ( !p_ಅಂಶ->fg.isRecursive || aCsrMap[p_ಅಂಶ->iCursor+1]==0 ){
        aCsrMap[p_ಅಂಶ->iCursor+1] = pParse->nTab++;
      }
      p_ಅಂಶ->iCursor = aCsrMap[p_ಅಂಶ->iCursor+1];
      ಆದರೆ( p_ಅಂಶ->fg.isSubquery ){
        ಗಾಗಿ(p=p_ಅಂಶ->u4.pSubq->pSelect; p; p=p->p_ಪೂರ್ವ){
          srclistRenumberCursors(pParse, aCsrMap, p->pSrc, -1);
        }
      }
    }
  }
}

/*
** *piCursor is a cursor number.  Change it if it needs to be mapped.
*/
ಸ್ಥಿರ ಶೂನ್ಯ renumberCursorDoMapping(Walker *pWalker, ಪೂರ್ಣಾಂಕ *piCursor){
  ಪೂರ್ಣಾಂಕ *aCsrMap = pWalker->u.ai_ಲಂಬಸಾಲು;
  ಪೂರ್ಣಾಂಕ iCsr = *piCursor;
  ಆದರೆ( iCsr < aCsrMap[0] && aCsrMap[iCsr+1]>0 ){
    *piCursor = aCsrMap[iCsr+1];
  }
}

/*
** Expression walker callback used by renumberCursors() to update
** Expr objects to match newly assigned cursor numbers.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ renumberCursorsCb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ op = p_ಅಭಿವ್ಯಕ್ತಿ->op;
  ಆದರೆ( op==TK_ಸ್ತಂಭ || op==TK_IF_NULL_ಅಡ್ಡಸಾಲು ){
    renumberCursorDoMapping(pWalker, &p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ);
  }
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, (EP_OuterON|EP_InnerON)) ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) );
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) );
    renumberCursorDoMapping(pWalker, &p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin);
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
  ಪೂರ್ಣಾಂಕ iExcept,                    /* FROM clause item to skip */
  ಪೂರ್ಣಾಂಕ *aCsrMap                    /* Working space */
){
  Walker w;
  srclistRenumberCursors(pParse, aCsrMap, p->pSrc, iExcept);
  memset(&w, 0, ಗಾತ್ರ(w));
  w.u.ai_ಲಂಬಸಾಲು = aCsrMap;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = renumberCursorsCb;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  sqlite3WalkSelect(&w, p);
}
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** If pSel is not part of a compound SELECT, return a pointer to its
** expression list. Otherwise, return a pointer to the expression list
** of the leftmost SELECT in the compound.
*/
ಸ್ಥಿರ ಅಭಿವ್ಯಕ್ತಿ_List *ಕಂಡುಹಿಡಿ_Leftmost_Exprlist(Select *pSel){
  ಆಗ( pSel->p_ಪೂರ್ವ ){
    pSel = pSel->p_ಪೂರ್ವ;
  }
  ಹಿಂತಿರುಗಿ pSel->pEList;
}

/*
** Return true if any of the result-set columns in the compound query
** have incompatible affinities on one or more arms of the compound.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ compoundHasDifferentAffinities(Select *p){
  ಪೂರ್ಣಾಂಕ ii;
  ಅಭಿವ್ಯಕ್ತಿ_List *pList;
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->p_ಪೂರ್ವ!=0 );
  pList = p->pEList;
  ಗಾಗಿ(ii=0; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
    ಅಕ್ಷರ aff;
    Select *pSub1;
    ಪ್ರತಿಜ್ಞಾ( pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
    aff = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ);
    ಗಾಗಿ(pSub1=p->p_ಪೂರ್ವ; pSub1; pSub1=pSub1->p_ಪೂರ್ವ){
      ಪ್ರತಿಜ್ಞಾ( pSub1->pEList!=0 );
      ಪ್ರತಿಜ್ಞಾ( pSub1->pEList->n_ಅಭಿವ್ಯಕ್ತಿ>ii );
      ಪ್ರತಿಜ್ಞಾ( pSub1->pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(pSub1->pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ)!=aff ){
        ಹಿಂತಿರುಗಿ 1;
      }
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ flattenSubquery(
  Parse *pParse,       /* Parsing context */
  Select *p,           /* The parent or outer SELECT statement */
  ಪೂರ್ಣಾಂಕ iFrom,           /* Index in p->pSrc->a[] of the inner subquery */
  ಪೂರ್ಣಾಂಕ isAgg            /* True if outer SELECT uses aggregate functions */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSavedAuthContext = pParse->zAuthContext;
  Select *p_ಪೋಷಕ;    /* Current UNION ALL term of the other query */
  Select *pSub;       /* The inner query or "subquery" */
  Select *pSub1;      /* Pointer to the rightmost select in sub-query */
  SrcList *pSrc;      /* The FROM clause of the outer query */
  SrcList *pSubSrc;   /* The FROM clause of the subquery */
  ಪೂರ್ಣಾಂಕ i_ಪೋಷಕ;        /* VDBE cursor number of the pSub result set temp table */
  ಪೂರ್ಣಾಂಕ i_New_ಪೋಷಕ = -1;/* Replacement table for iParent */
  ಪೂರ್ಣಾಂಕ isOuterJoin = 0; /* True if pSub is the right side of a LEFT JOIN */   
  ಪೂರ್ಣಾಂಕ i;              /* Loop counter */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere;                    /* The WHERE clause */
  Src_ಅಂಶ *pSubitem;               /* The subquery */
  sqlite3 *db = pParse->db;
  Walker w;                        /* Walker to persist agginfo data */
  ಪೂರ್ಣಾಂಕ *aCsrMap = 0;

  /* Check to see if flattening is permitted.  Return 0 if not.
  */
  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಪ್ರತಿಜ್ಞಾ( p->p_ಪೂರ್ವ==0 );
  ಆದರೆ( OptimizationDisabled(db, SQLITE_QueryFlattener) ) ಹಿಂತಿರುಗಿ 0;
  pSrc = p->pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSrc && iFrom>=0 && iFrom<pSrc->nSrc );
  pSubitem = &pSrc->a[iFrom];
  i_ಪೋಷಕ = pSubitem->iCursor;
  ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.isSubquery );
  pSub = pSubitem->u4.pSubq->pSelect;
  ಪ್ರತಿಜ್ಞಾ( pSub!=0 );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಆದರೆ( p->pWin || pSub->pWin ) ಹಿಂತಿರುಗಿ 0;                  /* Restriction (25) */
#ಆದರೆ_ಅಂತ್ಯ

  pSubSrc = pSub->pSrc;
  ಪ್ರತಿಜ್ಞಾ( pSubSrc );
  /* Prior to version 3.1.2, when LIMIT and OFFSET had to be simple constants,
  ** not arbitrary expressions, we allowed some combining of LIMIT and OFFSET
  ** because they could be computed at compile-time.  But when LIMIT and OFFSET
  ** became arbitrary expressions, we were forced to add restrictions (13)
  ** and (14). */
  ಆದರೆ( pSub->p_ಎಲ್ಲೆ && p->p_ಎಲ್ಲೆ ) ಹಿಂತಿರುಗಿ 0;              /* Restriction (13) */
  ಆದರೆ( pSub->p_ಎಲ್ಲೆ && pSub->p_ಎಲ್ಲೆ->p_ಬಲ ) ಹಿಂತಿರುಗಿ 0;   /* Restriction (14) */
  ಆದರೆ( (p->selFlags & SF_Compound)!=0 && pSub->p_ಎಲ್ಲೆ ){
    ಹಿಂತಿರುಗಿ 0;                                            /* Restriction (15) */
  }
  ಆದರೆ( pSubSrc->nSrc==0 ) ಹಿಂತಿರುಗಿ 0;                       /* Restriction (7)  */
  ಆದರೆ( pSub->selFlags & SF_Distinct ) ಹಿಂತಿರುಗಿ 0;           /* Restriction (4)  */
  ಆದರೆ( pSub->p_ಎಲ್ಲೆ && (pSrc->nSrc>1 || isAgg) ){
     ಹಿಂತಿರುಗಿ 0;         /* Restrictions (8)(9) */
  }
  ಆದರೆ( p->p_ಕ್ರಮ_By && pSub->p_ಕ್ರಮ_By ){
     ಹಿಂತಿರುಗಿ 0;                                           /* Restriction (11) */
  }
  ಆದರೆ( isAgg && pSub->p_ಕ್ರಮ_By ) ಹಿಂತಿರುಗಿ 0;                /* Restriction (16) */
  ಆದರೆ( pSub->p_ಎಲ್ಲೆ && p->pWhere ) ಹಿಂತಿರುಗಿ 0;              /* Restriction (19) */
  ಆದರೆ( pSub->p_ಎಲ್ಲೆ && (p->selFlags & SF_Distinct)!=0 ){
     ಹಿಂತಿರುಗಿ 0;         /* Restriction (21) */
  }
  ಆದರೆ( pSub->selFlags & (SF_Recursive) ){
    ಹಿಂತಿರುಗಿ 0; /* Restrictions (22) */
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
  ಆದರೆ( (pSubitem->fg.jointype & (JT_OUTER|JT_LTORJ))!=0 ){
    ಆದರೆ( pSubSrc->nSrc>1                        /* (3a) */
     /**** || IsVirtual(pSubSrc->a[0].pSTab)      (3b)-omitted */
     || (p->selFlags & SF_Distinct)!=0         /* (3d) */
     || (pSubitem->fg.jointype & JT_ಬಲ)!=0  /* (26) */
    ){
      ಹಿಂತಿರುಗಿ 0;
    }
    isOuterJoin = 1;
  }

  ಪ್ರತಿಜ್ಞಾ( pSubSrc->nSrc>0 );  /* True by restriction (7) */
  ಆದರೆ( iFrom>0 && (pSubSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
    ಹಿಂತಿರುಗಿ 0;   /* Restriction (27a) */
  }

  /* Condition (28) is blocked by the caller */
  ಪ್ರತಿಜ್ಞಾ( !pSubitem->fg.isCte || pSubitem->u2.pCteUse->eM10d!=M10d_Yes );

  /* Restriction (17): If the sub-query is a compound SELECT, then it must
  ** use only the UNION ALL operator. And none of the simple select queries
  ** that make up the compound SELECT are allowed to be aggregate or distinct
  ** queries.
  */
  ಆದರೆ( pSub->p_ಪೂರ್ವ ){
    ಪೂರ್ಣಾಂಕ ii;
    ಆದರೆ( pSub->p_ಕ್ರಮ_By ){
      ಹಿಂತಿರುಗಿ 0;  /* Restriction (20) */
    }
    ಆದರೆ( isAgg || (p->selFlags & SF_Distinct)!=0 || isOuterJoin>0 ){
      ಹಿಂತಿರುಗಿ 0; /* (17d1), (17d2), or (17f) */
    }
    ಗಾಗಿ(pSub1=pSub; pSub1; pSub1=pSub1->p_ಪೂರ್ವ){
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct );
      testcase( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))==SF_Aggregate );
      ಪ್ರತಿಜ್ಞಾ( pSub->pSrc!=0 );
      ಪ್ರತಿಜ್ಞಾ( (pSub->selFlags & SF_Recursive)==0 );
      ಪ್ರತಿಜ್ಞಾ( pSub->pEList->n_ಅಭಿವ್ಯಕ್ತಿ==pSub1->pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
      ಆದರೆ( (pSub1->selFlags & (SF_Distinct|SF_Aggregate))!=0    /* (17b) */
       || (pSub1->p_ಪೂರ್ವ && pSub1->op!=TK_ALL)                 /* (17a) */
       || pSub1->pSrc->nSrc<1                                  /* (17c) */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
       || pSub1->pWin                                          /* (17e) */
#ಆದರೆ_ಅಂತ್ಯ
      ){
        ಹಿಂತಿರುಗಿ 0;
      }
      ಆದರೆ( iFrom>0 && (pSub1->pSrc->a[0].fg.jointype & JT_LTORJ)!=0 ){
        /* Without this restriction, the JT_LTORJ flag would end up being
        ** omitted on left-hand tables of the right join that is being
        ** flattened. */
        ಹಿಂತಿರುಗಿ 0;   /* Restrictions (17g), (27b) */
      }
      testcase( pSub1->pSrc->nSrc>1 );
    }

    /* Restriction (18). */
    ಆದರೆ( p->p_ಕ್ರಮ_By ){
      ಗಾಗಿ(ii=0; ii<p->p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
        ಆದರೆ( p->p_ಕ್ರಮ_By->a[ii].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು==0 ) ಹಿಂತಿರುಗಿ 0;
      }
    }

    /* Restriction (23) */
    ಆದರೆ( (p->selFlags & SF_Recursive) ) ಹಿಂತಿರುಗಿ 0;

    /* Restriction (17h) */
    ಆದರೆ( compoundHasDifferentAffinities(pSub) ) ಹಿಂತಿರುಗಿ 0;

    ಆದರೆ( pSrc->nSrc>1 ){
      ಆದರೆ( pParse->nSelect>500 ) ಹಿಂತಿರುಗಿ 0;
      ಆದರೆ( OptimizationDisabled(db, SQLITE_FlttnUnionAll) ) ಹಿಂತಿರುಗಿ 0;
      aCsrMap = sqlite3DbMallocZero(db, ((i64)pParse->nTab+1)*ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
      ಆದರೆ( aCsrMap ) aCsrMap[0] = pParse->nTab;
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
  
  ಆದರೆ( ALWAYS(pSubitem->fg.isSubquery) ){
    pSub1 = sqlite3SubqueryDetach(db, pSubitem);
  }ಇಲ್ಲದಿದ್ದರೆ{
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
  ಗಾಗಿ(pSub=pSub->p_ಪೂರ್ವ; pSub; pSub=pSub->p_ಪೂರ್ವ){
    Select *pNew;
    ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By = p->p_ಕ್ರಮ_By;
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ = p->p_ಎಲ್ಲೆ;
    Select *p_ಪೂರ್ವ = p->p_ಪೂರ್ವ;
    ಕೋಷ್ಟಕ *p_ಅಂಶ_Tab = pSubitem->pSTab;
    pSubitem->pSTab = 0;
    p->p_ಕ್ರಮ_By = 0;
    p->p_ಪೂರ್ವ = 0;
    p->p_ಎಲ್ಲೆ = 0;
    pNew = sqlite3SelectDup(db, p, 0);
    p->p_ಎಲ್ಲೆ = p_ಎಲ್ಲೆ;
    p->p_ಕ್ರಮ_By = p_ಕ್ರಮ_By;
    p->op = TK_ALL;
    pSubitem->pSTab = p_ಅಂಶ_Tab;
    ಆದರೆ( pNew==0 ){
      p->p_ಪೂರ್ವ = p_ಪೂರ್ವ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      pNew->selId = ++pParse->nSelect;
      ಆದರೆ( aCsrMap && ALWAYS(db->mallocFailed==0) ){
        renumberCursors(pParse, pNew, iFrom, aCsrMap);
      }
      pNew->p_ಪೂರ್ವ = p_ಪೂರ್ವ;
      ಆದರೆ( p_ಪೂರ್ವ ) p_ಪೂರ್ವ->p_ಮುಂದಿನ = pNew;
      pNew->p_ಮುಂದಿನ = p;
      p->p_ಪೂರ್ವ = pNew;
      TREETRACE(0x4,pParse,p,("compound-subquery flattener"
                              " creates %u as peer\n",pNew->selId));
    }
    ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.isSubquery==0 );
  }
  sqlite3DbFree(db, aCsrMap);
  ಆದರೆ( db->mallocFailed ){
    ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.fixedSchema==0 );
    ಪ್ರತಿಜ್ಞಾ( pSubitem->fg.isSubquery==0 );
    ಪ್ರತಿಜ್ಞಾ( pSubitem->u4.zDatabase==0 );
    sqlite3_Src_ಅಂಶ_Attach_Subquery(pParse, pSubitem, pSub1, 0);
    ಹಿಂತಿರುಗಿ 1;
  }

  /* Defer deleting the Table object associated with the
  ** subquery until code generation is
  ** complete, since there may still exist Expr.pTab entries that
  ** refer to the subquery even after flattening.  Ticket #3346.
  **
  ** pSubitem->pSTab is always non-NULL by test restrictions and tests above.
  */
  ಆದರೆ( ALWAYS(pSubitem->pSTab!=0) ){
    ಕೋಷ್ಟಕ *p_Tab_ಗಮ್ಯ_Del = pSubitem->pSTab;
    ಆದರೆ( p_Tab_ಗಮ್ಯ_Del->nTabRef==1 ){
      Parse *pToplevel = sqlite3ParseToplevel(pParse);
      sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pToplevel, sqlite3_Delete_ಕೋಷ್ಟಕ_Generic, p_Tab_ಗಮ್ಯ_Del);
      testcase( pToplevel->earlyCleanup );
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_Tab_ಗಮ್ಯ_Del->nTabRef--;
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
  ಗಾಗಿ(p_ಪೋಷಕ=p; p_ಪೋಷಕ; p_ಪೋಷಕ=p_ಪೋಷಕ->p_ಪೂರ್ವ, pSub=pSub->p_ಪೂರ್ವ){
    ಪೂರ್ಣಾಂಕ nSubSrc;
    u8 jointype = p_ಪೋಷಕ->pSrc->a[iFrom].fg.jointype;
    ಪ್ರತಿಜ್ಞಾ( pSub!=0 );
    pSubSrc = pSub->pSrc;     /* FROM clause of subquery */
    nSubSrc = pSubSrc->nSrc;  /* Number of terms in subquery FROM clause */
    pSrc = p_ಪೋಷಕ->pSrc;     /* FROM clause of the outer query */

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
    ಆದರೆ( nSubSrc>1 ){
      pSrc = sqlite3SrcListEnlarge(pParse, pSrc, nSubSrc-1,iFrom+1);
      ಆದರೆ( pSrc==0 ) ನಿಲ್ಲಿಸು;
      p_ಪೋಷಕ->pSrc = pSrc;
    }

    /* Transfer the FROM clause terms from the subquery into the
    ** outer query.
    */
    i_New_ಪೋಷಕ = pSubSrc->a[0].iCursor;
    ಗಾಗಿ(i=0; i<nSubSrc; i++){
      Src_ಅಂಶ *p_ಅಂಶ = &pSrc->a[i+iFrom];
      ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.isTabFunc==0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.isSubquery
           || p_ಅಂಶ->fg.fixedSchema
           || p_ಅಂಶ->u4.zDatabase==0 );
      ಆದರೆ( p_ಅಂಶ->fg.isUsing ) sqlite3IdListDelete(db, p_ಅಂಶ->u3.pUsing);
      *p_ಅಂಶ = pSubSrc->a[i];
      p_ಅಂಶ->fg.jointype |= (jointype & JT_LTORJ);
      memset(&pSubSrc->a[i], 0, ಗಾತ್ರ(pSubSrc->a[i]));
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
    ಆದರೆ( pSub->p_ಕ್ರಮ_By ){
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
      ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By = pSub->p_ಕ್ರಮ_By;
      ಗಾಗಿ(i=0; i<p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
        p_ಕ್ರಮ_By->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = 0;
      }
      ಪ್ರತಿಜ್ಞಾ( p_ಪೋಷಕ->p_ಕ್ರಮ_By==0 );
      p_ಪೋಷಕ->p_ಕ್ರಮ_By = p_ಕ್ರಮ_By;
      pSub->p_ಕ್ರಮ_By = 0;
    }
    pWhere = pSub->pWhere;
    pSub->pWhere = 0;
    ಆದರೆ( isOuterJoin>0 ){
      ಪ್ರತಿಜ್ಞಾ( pSubSrc->nSrc==1 );
      sqlite3_ಗಣ_Join_ಅಭಿವ್ಯಕ್ತಿ(pWhere, i_New_ಪೋಷಕ, EP_OuterON);
    }
    ಆದರೆ( pWhere ){
      ಆದರೆ( p_ಪೋಷಕ->pWhere ){
        p_ಪೋಷಕ->pWhere = sqlite3PExpr(pParse, TK_AND, pWhere, p_ಪೋಷಕ->pWhere);
      }ಇಲ್ಲದಿದ್ದರೆ{
        p_ಪೋಷಕ->pWhere = pWhere;
      }
    }
    ಆದರೆ( db->mallocFailed==0 ){
      SubstContext x;
      x.pParse = pParse;
      x.i_ಕೋಷ್ಟಕ = i_ಪೋಷಕ;
      x.i_New_ಕೋಷ್ಟಕ = i_New_ಪೋಷಕ;
      x.isOuterJoin = isOuterJoin;
      x.n_Sel_ಆಳ = 0;
      x.pEList = pSub->pEList;
      x.pCList = ಕಂಡುಹಿಡಿ_Leftmost_Exprlist(pSub);
      substSelect(&x, p_ಪೋಷಕ, 0);
    }
 
    /* The flattened query is a compound if either the inner or the
    ** outer query is a compound. */
    p_ಪೋಷಕ->selFlags |= pSub->selFlags & SF_Compound;
    ಪ್ರತಿಜ್ಞಾ( (pSub->selFlags & SF_Distinct)==0 ); /* restriction (17b) */
 
    /*
    ** SELECT ... FROM (SELECT ... LIMIT a OFFSET b) LIMIT x OFFSET y;
    **
    ** One is tempted to try to add a and b to combine the limits.  But this
    ** does not work if either limit is negative.
    */
    ಆದರೆ( pSub->p_ಎಲ್ಲೆ ){
      p_ಪೋಷಕ->p_ಎಲ್ಲೆ = pSub->p_ಎಲ್ಲೆ;
      pSub->p_ಎಲ್ಲೆ = 0;
    }

    /* Recompute the SrcItem.colUsed masks for the flattened
    ** tables. */
    ಗಾಗಿ(i=0; i<nSubSrc; i++){
      recompute_Columns_ಬಳಸಿದ(p_ಪೋಷಕ, &pSrc->a[i+iFrom]);
    }
  }

  /* Finally, delete what is left of the subquery and return success.
  */
  sqlite3AggInfoPersistWalkerInit(&w, pParse);
  sqlite3WalkSelect(&w,pSub1);
  sqlite3SelectDelete(db, pSub1);

#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x4 ){
    TREETRACE(0x4,pParse,p,("After flattening:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** A structure to keep track of all of the column values that are fixed to
** a known value due to WHERE clause constraints of the form COLUMN=VALUE.
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ WhereConst WhereConst;
ರಚನೆ WhereConst {
  Parse *pParse;   /* Parsing context */
  u8 *pOomFault;   /* Pointer to pParse->db->mallocFailed */
  ಪೂರ್ಣಾಂಕ nConst;      /* Number for COLUMN=CONSTANT terms */
  ಪೂರ್ಣಾಂಕ nChng;       /* Number of times a constant is propagated */
  ಪೂರ್ಣಾಂಕ bHasAffBlob; /* At least one column in apExpr[] as affinity BLOB */
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
  ಅಭಿವ್ಯಕ್ತಿ *p_ಸ್ತಂಭ,       /* The COLUMN part of the constraint */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಮೌಲ್ಯ,        /* The VALUE part of the constraint */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ          /* Overall expression: COLUMN=VALUE or VALUE=COLUMN */
){
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( p_ಸ್ತಂಭ->op==TK_ಸ್ತಂಭ );
  ಪ್ರತಿಜ್ಞಾ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pConst->pParse, p_ಮೌಲ್ಯ) );
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಸ್ತಂಭ, EP_Fixed_ಲಂಬಸಾಲು) ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಮೌಲ್ಯ)!=0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( !sqlite3_Is_ದ್ವಿಮಾನ(sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare_Coll_Seq(pConst->pParse,p_ಅಭಿವ್ಯಕ್ತಿ)) ){
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Return_Subtype(pConst->pParse, p_ಮೌಲ್ಯ) ) ಹಿಂತಿರುಗಿ;

  /* 2018-10-25 ticket [cf5ed20f]
  ** Make sure the same pColumn is not inserted more than once */
  ಗಾಗಿ(i=0; i<pConst->nConst; i++){
    ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pE2 = pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[i*2];
    ಪ್ರತಿಜ್ಞಾ( pE2->op==TK_ಸ್ತಂಭ );
    ಆದರೆ( pE2->i_ಕೋಷ್ಟಕ==p_ಸ್ತಂಭ->i_ಕೋಷ್ಟಕ
     && pE2->i_ಸ್ತಂಭ==p_ಸ್ತಂಭ->i_ಸ್ತಂಭ
    ){
      ಹಿಂತಿರುಗಿ;  /* Already present.  Return without doing anything. */
    }
  }
  ಪ್ರತಿಜ್ಞಾ( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಸ್ತಂಭ)<=SQLITE_AFF_BLOB ){
    pConst->bHasAffBlob = 1;
  }

  pConst->nConst++;
  pConst->ap_ಅಭಿವ್ಯಕ್ತಿ = sqlite3DbReallocOrFree(pConst->pParse->db, pConst->ap_ಅಭಿವ್ಯಕ್ತಿ,
                         pConst->nConst*2*ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ*));
  ಆದರೆ( pConst->ap_ಅಭಿವ್ಯಕ್ತಿ==0 ){
    pConst->nConst = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[pConst->nConst*2-2] = p_ಸ್ತಂಭ;
    pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[pConst->nConst*2-1] = p_ಮೌಲ್ಯ;
  }
}

/*
** Find all terms of COLUMN=VALUE or VALUE=COLUMN in pExpr where VALUE
** is a constant expression and where the term must be true because it
** is part of the AND-connected terms of the expression.  For each term
** found, add it to the pConst structure.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಕಂಡುಹಿಡಿ_Const_ಒಳ_Where(WhereConst *pConst, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ, *p_ಎಡ;
  ಆದರೆ( NEVER(p_ಅಭಿವ್ಯಕ್ತಿ==0) ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, pConst->mExcludeOn) ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) );
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) );
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AND ){
    ಕಂಡುಹಿಡಿ_Const_ಒಳ_Where(pConst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    ಕಂಡುಹಿಡಿ_Const_ಒಳ_Where(pConst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_EQ ) ಹಿಂತಿರುಗಿ;
  p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
  p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
  ಪ್ರತಿಜ್ಞಾ( p_ಬಲ!=0 );
  ಪ್ರತಿಜ್ಞಾ( p_ಎಡ!=0 );
  ಆದರೆ( p_ಬಲ->op==TK_ಸ್ತಂಭ && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pConst->pParse, p_ಎಡ) ){
    const_ಸೇರಿಸು(pConst,p_ಬಲ,p_ಎಡ,p_ಅಭಿವ್ಯಕ್ತಿ);
  }
  ಆದರೆ( p_ಎಡ->op==TK_ಸ್ತಂಭ && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant(pConst->pParse, p_ಬಲ) ){
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite_One(
  WhereConst *pConst,
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,
  ಪೂರ್ಣಾಂಕ bIgnoreAffBlob
){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pConst->pOomFault[0] ) ಹಿಂತಿರುಗಿ WRC_Prune;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_ಸ್ತಂಭ ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Fixed_ಲಂಬಸಾಲು|pConst->mExcludeOn) ){
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Fixed_ಲಂಬಸಾಲು) );
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) );
    testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) );
    ಹಿಂತಿರುಗಿ WRC_Continue;
  }
  ಗಾಗಿ(i=0; i<pConst->nConst; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಸ್ತಂಭ = pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[i*2];
    ಆದರೆ( p_ಸ್ತಂಭ==p_ಅಭಿವ್ಯಕ್ತಿ ) ಮುಂದುವರಿ;
    ಆದರೆ( p_ಸ್ತಂಭ->i_ಕೋಷ್ಟಕ!=p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ ) ಮುಂದುವರಿ;
    ಆದರೆ( p_ಸ್ತಂಭ->i_ಸ್ತಂಭ!=p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ ) ಮುಂದುವರಿ;
    ಪ್ರತಿಜ್ಞಾ( SQLITE_AFF_NONE<SQLITE_AFF_BLOB );
    ಆದರೆ( bIgnoreAffBlob && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಸ್ತಂಭ)<=SQLITE_AFF_BLOB ){
      ನಿಲ್ಲಿಸು;
    }
    /* A match is found.  Add the EP_FixedCol property */
    pConst->nChng++;
    ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_ಎಲೆ);
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Fixed_ಲಂಬಸಾಲು);
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0 );
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pConst->pParse->db, pConst->ap_ಅಭಿವ್ಯಕ್ತಿ[i*2+1], 0);
    ಆದರೆ( pConst->pParse->db->mallocFailed ) ಹಿಂತಿರುಗಿ WRC_Prune;
    ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ WRC_Prune;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  WhereConst *pConst = pWalker->u.pConst;
  ಪ್ರತಿಜ್ಞಾ( TK_GT==TK_EQ+1 );
  ಪ್ರತಿಜ್ಞಾ( TK_LE==TK_EQ+2 );
  ಪ್ರತಿಜ್ಞಾ( TK_LT==TK_EQ+3 );
  ಪ್ರತಿಜ್ಞಾ( TK_GE==TK_EQ+4 );
  ಆದರೆ( pConst->bHasAffBlob ){
    ಆದರೆ( (p_ಅಭಿವ್ಯಕ್ತಿ->op>=TK_EQ && p_ಅಭಿವ್ಯಕ್ತಿ->op<=TK_GE)
     || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS
    ){
      propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite_One(pConst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ಆದರೆ( pConst->pOomFault[0] ) ಹಿಂತಿರುಗಿ WRC_Prune;
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Affinity(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ)!=SQLITE_AFF_ಪಠ್ಯ ){
        propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite_One(pConst, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, 0);
      }
    }
  }
  ಹಿಂತಿರುಗಿ propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite_One(pConst, p_ಅಭಿವ್ಯಕ್ತಿ, pConst->bHasAffBlob);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ propagateConstants(
  Parse *pParse,   /* The parsing context */
  Select *p        /* The query in which to propagate constants */
){
  WhereConst x;
  Walker w;
  ಪೂರ್ಣಾಂಕ nChng = 0;
  x.pParse = pParse;
  x.pOomFault = &pParse->db->mallocFailed;
  ಮಾಡು{
    x.nConst = 0;
    x.nChng = 0;
    x.ap_ಅಭಿವ್ಯಕ್ತಿ = 0;
    x.bHasAffBlob = 0;
    ಆದರೆ( ALWAYS(p->pSrc!=0)
     && p->pSrc->nSrc>0
     && (p->pSrc->a[0].fg.jointype & JT_LTORJ)!=0
    ){
      /* Do not propagate constants on any ON clause if there is a
      ** RIGHT JOIN anywhere in the query */
      x.mExcludeOn = EP_InnerON | EP_OuterON;
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Do not propagate constants through the ON clause of a LEFT JOIN */
      x.mExcludeOn = EP_OuterON;
    }
    ಕಂಡುಹಿಡಿ_Const_ಒಳ_Where(&x, p->pWhere);
    ಆದರೆ( x.nConst ){
      memset(&w, 0, ಗಾತ್ರ(w));
      w.pParse = pParse;
      w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = propagate_Constant_ಅಭಿವ್ಯಕ್ತಿ_Rewrite;
      w.xSelectCallback = sqlite3SelectWalkNoop;
      w.xSelectCallback2 = 0;
      w.walker_ಆಳ = 0;
      w.u.pConst = &x;
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p->pWhere);
      sqlite3DbFree(x.pParse->db, x.ap_ಅಭಿವ್ಯಕ್ತಿ);
      nChng += x.nChng;
    }
  }ಆಗ( x.nChng ); 
  ಹಿಂತಿರುಗಿ nChng;
}

#ಆದರೆ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
# ಆದರೆ !defined(SQLITE_OMIT_WINDOWFUNC)
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತಳ್ಳು_ಕೆಳಕ್ಕೆ_ಕಿಟಕಿ_Check(Parse *pParse, Select *pSubq, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪ್ರತಿಜ್ಞಾ( pSubq->pWin->p_ವಿಭಜನೆ );
  ಪ್ರತಿಜ್ಞಾ( (pSubq->selFlags & SF_MultiPart)==0 );
  ಪ್ರತಿಜ್ಞಾ( pSubq->p_ಪೂರ್ವ==0 );
  ಹಿಂತಿರುಗಿ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_ಗುಂಪು_By(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pSubq->pWin->p_ವಿಭಜನೆ);
}
# endif /* SQLITE_OMIT_WINDOWFUNC */
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

#ಆದರೆ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತಳ್ಳು_ಕೆಳಕ್ಕೆ_Where_Terms(
  Parse *pParse,        /* Parse context (for malloc() and error reporting) */
  Select *pSubq,        /* The subquery whose WHERE clause is to be augmented */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere,         /* The WHERE clause of the outer query */
  SrcList *pSrcList,    /* The complete from clause of the outer query */
  ಪೂರ್ಣಾಂಕ iSrc              /* Which FROM clause term to try to push into  */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew;
  Src_ಅಂಶ *pSrc;        /* The subquery FROM term into which WHERE is pushed */
  ಪೂರ್ಣಾಂಕ nChng = 0;
  pSrc = &pSrcList->a[iSrc];
  ಆದರೆ( pWhere==0 ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( pSubq->selFlags & (SF_Recursive|SF_MultiPart) ){
    ಹಿಂತಿರುಗಿ 0;           /* restrictions (2) and (11) */
  }
  ಆದರೆ( pSrc->fg.jointype & (JT_LTORJ|JT_ಬಲ) ){
    ಹಿಂತಿರುಗಿ 0;           /* restrictions (10) */
  }

  ಆದರೆ( pSubq->p_ಪೂರ್ವ ){
    Select *pSel;
    ಪೂರ್ಣಾಂಕ notUnionAll = 0;
    ಗಾಗಿ(pSel=pSubq; pSel; pSel=pSel->p_ಪೂರ್ವ){
      u8 op = pSel->op;
      ಪ್ರತಿಜ್ಞಾ( op==TK_ALL || op==TK_SELECT
           || op==TK_UNION || op==TK_INTERSECT || op==TK_EXCEPT );
      ಆದರೆ( op!=TK_ALL && op!=TK_SELECT ){
        notUnionAll = 1;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಆದರೆ( pSel->pWin ) ಹಿಂತಿರುಗಿ 0;    /* restriction (6b) */
#ಆದರೆ_ಅಂತ್ಯ
    }
    ಆದರೆ( notUnionAll ){
      /* If any of the compound arms are connected using UNION, INTERSECT,
      ** or EXCEPT, then we must ensure that none of the columns use a
      ** non-BINARY collating sequence. */
      ಗಾಗಿ(pSel=pSubq; pSel; pSel=pSel->p_ಪೂರ್ವ){
        ಪೂರ್ಣಾಂಕ ii;
        ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pList = pSel->pEList;
        ಪ್ರತಿಜ್ಞಾ( pList!=0 );
        ಗಾಗಿ(ii=0; ii<pList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
          CollSeq *pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ);
          ಆದರೆ( !sqlite3_Is_ದ್ವಿಮಾನ(pColl) ){
            ಹಿಂತಿರುಗಿ 0;  /* Restriction (8) */
          }
        }
      }
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( pSubq->pWin && pSubq->pWin->p_ವಿಭಜನೆ==0 ) ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Only the first term of a compound can have a WITH clause.  But make
  ** sure no other terms are marked SF_Recursive in case something changes
  ** in the future.
  */
  {
    Select *pX; 
    ಗಾಗಿ(pX=pSubq; pX; pX=pX->p_ಪೂರ್ವ){
      ಪ್ರತಿಜ್ಞಾ( (pX->selFlags & (SF_Recursive))==0 );
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( pSubq->p_ಎಲ್ಲೆ!=0 ){
    ಹಿಂತಿರುಗಿ 0; /* restriction (3) */
  }
  ಆಗ( pWhere->op==TK_AND ){
    nChng += ತಳ್ಳು_ಕೆಳಕ್ಕೆ_Where_Terms(pParse, pSubq, pWhere->p_ಬಲ, pSrcList, iSrc);
    pWhere = pWhere->p_ಎಡ;
  }

#ಆದರೆ 0 /* These checks now done by sqlite3ExprIsSingleTableConstraint() */
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pWhere, EP_OuterON|EP_InnerON) /* (9a) */
   && (pSrcList->a[0].fg.jointype & JT_LTORJ)!=0     /* Fast pre-test of (9c) */
  ){
    ಪೂರ್ಣಾಂಕ jj;
    ಗಾಗಿ(jj=0; jj<iSrc; jj++){
      ಆದರೆ( pWhere->w.iJoin==pSrcList->a[jj].iCursor ){
        /* If we reach this point, both (9a) and (9b) are satisfied.
        ** The following loop checks (9c):
        */
        ಗಾಗಿ(jj++; jj<iSrc; jj++){
          ಆದರೆ( (pSrcList->a[jj].fg.jointype & JT_ಬಲ)!=0 ){
            ಹಿಂತಿರುಗಿ 0;  /* restriction (9) */
          }
        }
      }
    }
  }
  ಆದರೆ( is_ಎಡ_Join
   && (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pWhere,EP_OuterON)==0
         || pWhere->w.iJoin!=iCursor)
  ){
    ಹಿಂತಿರುಗಿ 0; /* restriction (4) */
  }
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pWhere,EP_OuterON)
   && pWhere->w.iJoin!=iCursor
  ){
    ಹಿಂತಿರುಗಿ 0; /* restriction (5) */
  }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
  ಆದರೆ( ViewCanHaveRowid && (pWhere->op==TK_ISNULL || pWhere->op==TK_NOTNULL) ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = pWhere->p_ಎಡ;
    ಆದರೆ( ALWAYS(p_ಎಡ) 
     && p_ಎಡ->op==TK_ಸ್ತಂಭ
     && p_ಎಡ->i_ಸ್ತಂಭ < 0
    ){
      ಹಿಂತಿರುಗಿ 0;  /* Restriction (12) */
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Single_ಕೋಷ್ಟಕ_Constraint(pWhere, pSrcList, iSrc, 1) ){
    nChng++;
    pSubq->selFlags |= SF_ತಳ್ಳು_ಕೆಳಕ್ಕೆ;
    ಆಗ( pSubq ){
      SubstContext x;
      pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(pParse->db, pWhere, 0);
      unset_Join_ಅಭಿವ್ಯಕ್ತಿ(pNew, -1, 1);
      x.pParse = pParse;
      x.i_ಕೋಷ್ಟಕ = pSrc->iCursor;
      x.i_New_ಕೋಷ್ಟಕ = pSrc->iCursor;
      x.isOuterJoin = 0;
      x.n_Sel_ಆಳ = 0;
      x.pEList = pSubq->pEList;
      x.pCList = ಕಂಡುಹಿಡಿ_Leftmost_Exprlist(pSubq);
      pNew = subst_ಅಭಿವ್ಯಕ್ತಿ(&x, pNew);
      ಪ್ರತಿಜ್ಞಾ( pNew!=0 || pParse->nErr!=0 );
      ಆದರೆ( pParse->nErr==0 && pNew->op==TK_ಒಳ && ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pNew) ){
        ಪ್ರತಿಜ್ಞಾ( pNew->x.pSelect!=0 );
        pNew->x.pSelect->selFlags |= SF_Cloned_Rhs_ಒಳ;
        ಪ್ರತಿಜ್ಞಾ( pWhere!=0 );
        ಪ್ರತಿಜ್ಞಾ( pWhere->op==TK_ಒಳ );
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(pWhere) );
        ಪ್ರತಿಜ್ಞಾ( pWhere->x.pSelect!=0 );
        pWhere->x.pSelect->selFlags |= SF_Cloned_Rhs_ಒಳ;
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಆದರೆ( pSubq->pWin && 0==ತಳ್ಳು_ಕೆಳಕ್ಕೆ_ಕಿಟಕಿ_Check(pParse, pSubq, pNew) ){
        /* Restriction 6c has prevented push-down in this case */
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, pNew);
        nChng--;
        ನಿಲ್ಲಿಸು;
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( pSubq->selFlags & SF_Aggregate ){
        pSubq->pHaving = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pSubq->pHaving, pNew);
      }ಇಲ್ಲದಿದ್ದರೆ{
        pSubq->pWhere = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pParse, pSubq->pWhere, pNew);
      }
      pSubq = pSubq->p_ಪೂರ್ವ;
    }
  }
  ಹಿಂತಿರುಗಿ nChng;
}
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW) */

/*
** Check to see if a subquery contains result-set columns that are
** never used.  If it does, change the value of those result-set columns
** to NULL so that they do not cause unnecessary work to compute.
**
** Return the number of column that were changed to NULL.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ disable_ಬಳಸದ_Subquery_ಫಲಿತಾಂಶ_Columns(Src_ಅಂಶ *p_ಅಂಶ){
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;
  Select *pSub;      /* The subquery to be simplified */
  Select *pX;        /* For looping over compound elements of pSub */
  ಕೋಷ್ಟಕ *pTab;       /* The table that describes the subquery */
  ಪೂರ್ಣಾಂಕ j;             /* Column number */
  ಪೂರ್ಣಾಂಕ nChng = 0;     /* Number of columns converted to NULL */
  Bitmask ಲಂಬಸಾಲು_ಬಳಸಿದ;   /* Columns that may not be NULLed out */

  ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ!=0 );
  ಆದರೆ( p_ಅಂಶ->fg.isCorrelated || p_ಅಂಶ->fg.isCte ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->pSTab!=0 );
  pTab = p_ಅಂಶ->pSTab;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.isSubquery );
  pSub = p_ಅಂಶ->u4.pSubq->pSelect;
  ಪ್ರತಿಜ್ಞಾ( pSub->pEList->n_ಅಭಿವ್ಯಕ್ತಿ==pTab->n_ಲಂಬಸಾಲು );
  ಗಾಗಿ(pX=pSub; pX; pX=pX->p_ಪೂರ್ವ){
    ಆದರೆ( (pX->selFlags & (SF_Distinct|SF_Aggregate))!=0 ){
      testcase( pX->selFlags & SF_Distinct );
      testcase( pX->selFlags & SF_Aggregate );
      ಹಿಂತಿರುಗಿ 0;
    }
    ಆದರೆ( pX->p_ಪೂರ್ವ && pX->op!=TK_ALL ){
      /* This optimization does not work for compound subqueries that
      ** use UNION, INTERSECT, or EXCEPT.  Only UNION ALL is allowed. */
      ಹಿಂತಿರುಗಿ 0;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( pX->pWin ){
      /* This optimization does not work for subqueries that use window
      ** functions. */
      ಹಿಂತಿರುಗಿ 0;
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  ಲಂಬಸಾಲು_ಬಳಸಿದ = p_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ;
  ಆದರೆ( pSub->p_ಕ್ರಮ_By ){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList = pSub->p_ಕ್ರಮ_By;
    ಗಾಗಿ(j=0; j<pList->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
      u16 i_ಲಂಬಸಾಲು = pList->a[j].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು;
      ಆದರೆ( i_ಲಂಬಸಾಲು>0 ){
        i_ಲಂಬಸಾಲು--;
        ಲಂಬಸಾಲು_ಬಳಸಿದ |= ((Bitmask)1)<<(i_ಲಂಬಸಾಲು>=BMS ? BMS-1 : i_ಲಂಬಸಾಲು);
      }
    }
  }
  n_ಲಂಬಸಾಲು = pTab->n_ಲಂಬಸಾಲು;
  ಗಾಗಿ(j=0; j<n_ಲಂಬಸಾಲು; j++){
    Bitmask m = j<BMS-1 ? MASKBIT(j) : TOPBIT;
    ಆದರೆ( (m & ಲಂಬಸಾಲು_ಬಳಸಿದ)!=0 ) ಮುಂದುವರಿ;
    ಗಾಗಿ(pX=pSub; pX; pX=pX->p_ಪೂರ್ವ) {
      ಅಭಿವ್ಯಕ್ತಿ *pY = pX->pEList->a[j].p_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( pY->op==TK_NULL ) ಮುಂದುವರಿ;
      pY->op = TK_NULL;
      ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(pY, EP_Skip|EP_Unlikely);
      pX->selFlags |= SF_ತಳ್ಳು_ಕೆಳಕ್ಕೆ;
      nChng++;
    }
  }
  ಹಿಂತಿರುಗಿ nChng;
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
  ಪೂರ್ಣಾಂಕ eRet = WHERE_ORDERBY_ಸಾಮಾನ್ಯ;      /* Return value */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;                     /* Arguments to agg function */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFunc;                    /* Name of aggregate function pFunc */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;
  u8 ವಿಂಗಡಿಸು_Flags = 0;

  ಪ್ರತಿಜ್ಞಾ( *ppMinMax==0 );
  ಪ್ರತಿಜ್ಞಾ( pFunc->op==TK_AGG_FUNCTION );
  ಪ್ರತಿಜ್ಞಾ( !Is_ಕಿಟಕಿ_Func(pFunc) );
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pFunc) );
  pEList = pFunc->x.pList;
  ಆದರೆ( pEList==0
   || pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=1
   || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pFunc, EP_WinFunc)
   || OptimizationDisabled(db, SQLITE_MinMaxOpt)
  ){
    ಹಿಂತಿರುಗಿ eRet;
  }
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pFunc, EP_Int_ಮೌಲ್ಯ) );
  zFunc = pFunc->u.zToken;
  ಆದರೆ( sqlite3StrICmp(zFunc, "min")==0 ){
    eRet = WHERE_ORDERBY_MIN;
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ) ){
      ವಿಂಗಡಿಸು_Flags = KEYINFO_ಕ್ರಮ_BIGNULL;
    }
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3StrICmp(zFunc, "max")==0 ){
    eRet = WHERE_ORDERBY_MAX;
    ವಿಂಗಡಿಸು_Flags = KEYINFO_ಕ್ರಮ_DESC;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ eRet;
  }
  *ppMinMax = p_ಕ್ರಮ_By = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pEList, 0);
  ಪ್ರತಿಜ್ಞಾ( p_ಕ್ರಮ_By!=0 || db->mallocFailed );
  ಆದರೆ( p_ಕ್ರಮ_By ) p_ಕ್ರಮ_By->a[0].fg.ವಿಂಗಡಿಸು_Flags = ವಿಂಗಡಿಸು_Flags;
  ಹಿಂತಿರುಗಿ eRet;
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
ಸ್ಥಿರ ಕೋಷ್ಟಕ *is_Simple_ಎಣಿಕೆ(Select *p, AggInfo *pAggInfo){
  ಕೋಷ್ಟಕ *pTab;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;

  ಪ್ರತಿಜ್ಞಾ( !p->p_ಗುಂಪು_By );

  ಆದರೆ( p->pWhere
   || p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=1
   || p->pSrc->nSrc!=1
   || p->pSrc->a[0].fg.isSubquery
   || pAggInfo->nFunc!=1
   || p->pHaving
  ){
    ಹಿಂತಿರುಗಿ 0;
  }
  pTab = p->pSrc->a[0].pSTab;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( !IsView(pTab) );
  ಆದರೆ( !Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ) ಹಿಂತಿರುಗಿ 0;
  p_ಅಭಿವ್ಯಕ್ತಿ = p->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AGG_FUNCTION ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo!=pAggInfo ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( (pAggInfo->aFunc[0].pFunc->funcFlags&SQLITE_FUNC_ಎಣಿಕೆ)==0 ) ಹಿಂತಿರುಗಿ 0;
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->aFunc[0].pFExpr==p_ಅಭಿವ್ಯಕ್ತಿ );
  testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Distinct) );
  testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) );
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Distinct|EP_WinFunc) ) ಹಿಂತಿರುಗಿ 0;

  ಹಿಂತಿರುಗಿ pTab;
}

/*
** If the source-list item passed as an argument was augmented with an
** INDEXED BY clause, then try to locate the specified index. If there
** was such a clause and the named index cannot be found, return
** SQLITE_ERROR and leave an error in pParse. Otherwise, populate
** pFrom->pIndex and return SQLITE_OK.
*/
ಪೂರ್ಣಾಂಕ sqlite3_ಸೂಚ್ಯಂಕಿತ_By_Lookup(Parse *pParse, Src_ಅಂಶ *pFrom){
  ಕೋಷ್ಟಕ *pTab = pFrom->pSTab;
  ಅಕ್ಷರ *z_ಸೂಚ್ಯಂಕಿತ_By = pFrom->u1.z_ಸೂಚ್ಯಂಕಿತ_By;
  ಸೂಚ್ಯಂಕ *pIdx;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( pFrom->fg.is_ಸೂಚ್ಯಂಕಿತ_By!=0 );

  ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ;
      pIdx && sqlite3StrICmp(pIdx->zName, z_ಸೂಚ್ಯಂಕಿತ_By);
      pIdx=pIdx->p_ಮುಂದಿನ
  );
  ಆದರೆ( !pIdx ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "no such index: %s", z_ಸೂಚ್ಯಂಕಿತ_By, 0);
    pParse->checkSchema = 1;
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isCte==0 );
  pFrom->u2.pIBIndex = pIdx;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ convert_Compound_Select_ಗಮ್ಯ_Subquery(Walker *pWalker, Select *p){
  ಪೂರ್ಣಾಂಕ i;
  Select *pNew;
  Select *pX;
  sqlite3 *db;
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *a;
  SrcList *pNewSrc;
  Parse *pParse;
  Token dummy;

  ಆದರೆ( p->p_ಪೂರ್ವ==0 ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಆದರೆ( p->p_ಕ್ರಮ_By==0 ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಗಾಗಿ(pX=p; pX && (pX->op==TK_ALL || pX->op==TK_SELECT); pX=pX->p_ಪೂರ್ವ){}
  ಆದರೆ( pX==0 ) ಹಿಂತಿರುಗಿ WRC_Continue;
  a = p->p_ಕ್ರಮ_By->a;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  /* If iOrderByCol is already non-zero, then it has already been matched
  ** to a result column of the SELECT statement. This occurs when the
  ** SELECT is rewritten for window-functions processing and then passed
  ** to sqlite3SelectPrep() and similar a second time. The rewriting done
  ** by this function is not required in this case. */
  ಆದರೆ( a[0].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು ) ಹಿಂತಿರುಗಿ WRC_Continue;
#ಆದರೆ_ಅಂತ್ಯ
  ಗಾಗಿ(i=p->p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ-1; i>=0; i--){
    ಆದರೆ( a[i].p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_Collate ) ನಿಲ್ಲಿಸು;
  }
  ಆದರೆ( i<0 ) ಹಿಂತಿರುಗಿ WRC_Continue;

  /* If we reach this point, that means the transformation is required. */

  pParse = pWalker->pParse;
  db = pParse->db;
  pNew = sqlite3DbMallocZero(db, ಗಾತ್ರ(*pNew) );
  ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ WRC_Abort;
  memset(&dummy, 0, ಗಾತ್ರ(dummy));
  pNewSrc = sqlite3_Src_List_Append_From_ಪದ(pParse,0,0,0,&dummy,pNew,0);
  ಪ್ರತಿಜ್ಞಾ( pNewSrc!=0 || pParse->nErr );
  ಆದರೆ( pParse->nErr ){
    sqlite3SrcListDelete(db, pNewSrc);
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }
  *pNew = *p;
  p->pSrc = pNewSrc;
  p->pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ASTERISK, 0));
  p->op = TK_SELECT;
  p->pWhere = 0;
  p->p_ಗುಂಪು_By = 0;
  p->pHaving = 0;
  pNew->p_ಕ್ರಮ_By = 0;
  p->p_ಪೂರ್ವ = 0;
  p->p_ಮುಂದಿನ = 0;
  p->pWith = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  p->pWinDefn = 0;
#ಆದರೆ_ಅಂತ್ಯ
  p->selFlags &= ~(u32)(SF_Compound|SF_Distinct);
  ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Converted)==0 );
  p->selFlags |= SF_Converted;
  ಪ್ರತಿಜ್ಞಾ( pNew->p_ಪೂರ್ವ!=0 );
  pNew->p_ಪೂರ್ವ->p_ಮುಂದಿನ = pNew;
  pNew->p_ಎಲ್ಲೆ = 0;
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Check to see if the FROM clause term pFrom has table-valued function
** arguments.  If it does, leave an error message in pParse and return
** non-zero, since pFrom is not allowed to be a table-valued function.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cannotBeFunction(Parse *pParse, Src_ಅಂಶ *pFrom){
  ಆದರೆ( pFrom->fg.isTabFunc ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "'%s' is not a function", pFrom->zName);
    ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
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
ಸ್ಥಿರ ರಚನೆ Cte *ಹುಡುಕು_With(
  With *pWith,                    /* Current innermost WITH clause */
  Src_ಅಂಶ *p_ಅಂಶ,                 /* FROM clause element to resolve */
  With **ppContext                /* OUT: WITH clause return value belongs to */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName = p_ಅಂಶ->zName;
  With *p;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.fixedSchema || p_ಅಂಶ->u4.zDatabase==0 );
  ಪ್ರತಿಜ್ಞಾ( zName!=0 );
  ಗಾಗಿ(p=pWith; p; p=p->pOuter){
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<p->nCte; i++){
      ಆದರೆ( sqlite3StrICmp(zName, p->a[i].zName)==0 ){
        *ppContext = p;
        ಹಿಂತಿರುಗಿ &p->a[i];
      }
    }
    ಆದರೆ( p->bView ) ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ 0;
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
  ಆದರೆ( pWith ){
    ಆದರೆ( bFree ){
      pWith = (With*)sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, sqlite3WithDeleteGeneric,
                      pWith);
      ಆದರೆ( pWith==0 ) ಹಿಂತಿರುಗಿ 0;
    }
    ಆದರೆ( pParse->nErr==0 ){
      ಪ್ರತಿಜ್ಞಾ( pParse->pWith!=pWith );
      pWith->pOuter = pParse->pWith;
      pParse->pWith = pWith;
    }
  }
  ಹಿಂತಿರುಗಿ pWith;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ selectExpanderWalk(Walker*, Select*);

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resolve_From_ಪದ_ಗಮ್ಯ_Cte(
  Parse *pParse,                  /* The parsing context */
  Walker *pWalker,                /* Current tree walker */
  Src_ಅಂಶ *pFrom                  /* The FROM clause term to check */
){
  Cte *pCte;               /* Matched CTE (or NULL if no match) */
  With *pWith;             /* The matching WITH */

  ಪ್ರತಿಜ್ಞಾ( pFrom->pSTab==0 );
  ಆದರೆ( pParse->pWith==0 ){
    /* There are no WITH clauses in the stack.  No match is possible */
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( pParse->nErr ){
    /* Prior errors might have left pParse->pWith in a goofy state, so
    ** go no further. */
    ಹಿಂತಿರುಗಿ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( pFrom->fg.hadSchema==0 || pFrom->fg.notCte!=0 );
  ಆದರೆ( pFrom->fg.fixedSchema==0 && pFrom->u4.zDatabase!=0 ){
    /* The FROM term contains a schema qualifier (ex: main.t1) and so
    ** it cannot possibly be a CTE reference. */
    ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( pFrom->fg.notCte ){
    /* The FROM term is specifically excluded from matching a CTE.
    **   (1)  It is part of a trigger that used to have zDatabase but had
    **        zDatabase removed by sqlite3FixTriggerStep().
    **   (2)  This is the first term in the FROM clause of an UPDATE.
    */
    ಹಿಂತಿರುಗಿ 0;
  }
  pCte = ಹುಡುಕು_With(pParse->pWith, pFrom, &pWith);
  ಆದರೆ( pCte ){
    sqlite3 *db = pParse->db;
    ಕೋಷ್ಟಕ *pTab;
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
    Select *pSel;
    Select *p_ಎಡ;                /* Left-most SELECT statement */
    Select *p_Rec_ಪದ;             /* Left-most recursive term */
    ಪೂರ್ಣಾಂಕ bMayRecursive;            /* True if compound joined by UNION [ALL] */
    With *pSavedWith;             /* Initial value of pParse->pWith */
    ಪೂರ್ಣಾಂಕ iRecTab = -1;             /* Cursor for recursive table */
    CteUse *pCteUse;

    /* If pCte->zCteErr is non-NULL at this point, then this is an illegal
    ** recursive reference to CTE pCte. Leave an error in pParse and return
    ** early. If pCte->zCteErr is NULL, then this is not a recursive reference.
    ** In this case, proceed.  */
    ಆದರೆ( pCte->zCteErr ){
      sqlite3_Error_ಸಂದೇಶ(pParse, pCte->zCteErr, pCte->zName);
      ಹಿಂತಿರುಗಿ 2;
    }
    ಆದರೆ( cannotBeFunction(pParse, pFrom) ) ಹಿಂತಿರುಗಿ 2;

    ಪ್ರತಿಜ್ಞಾ( pFrom->pSTab==0 );
    pTab = sqlite3DbMallocZero(db, ಗಾತ್ರ(ಕೋಷ್ಟಕ));
    ಆದರೆ( pTab==0 ) ಹಿಂತಿರುಗಿ 2;
    pCteUse = pCte->pUse;
    ಆದರೆ( pCteUse==0 ){
      pCte->pUse = pCteUse = sqlite3DbMallocZero(db, ಗಾತ್ರ(pCteUse[0]));
      ಆದರೆ( pCteUse==0
       || sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse,sqlite3DbFree,pCteUse)==0
      ){
        sqlite3DbFree(db, pTab);
        ಹಿಂತಿರುಗಿ 2;
      }
      pCteUse->eM10d = pCte->eM10d;
    }
    pFrom->pSTab = pTab;
    pTab->nTabRef = 1;
    pTab->zName = sqlite3DbStrDup(db, pCte->zName);
    pTab->iPKey = -1;
    pTab->n_ಅಡ್ಡಸಾಲು_Log_Est = 200; ಪ್ರತಿಜ್ಞಾ( 200==sqlite3LogEst(1048576) );
    pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
    sqlite3_Src_ಅಂಶ_Attach_Subquery(pParse, pFrom, pCte->pSelect, 1);
    ಆದರೆ( db->mallocFailed ) ಹಿಂತಿರುಗಿ 2;
    ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery && pFrom->u4.pSubq );
    pSel = pFrom->u4.pSubq->pSelect;
    ಪ್ರತಿಜ್ಞಾ( pSel!=0 );
    pSel->selFlags |= SF_CopyCte;
    ಆದರೆ( pFrom->fg.is_ಸೂಚ್ಯಂಕಿತ_By ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "no such index: \"%s\"", pFrom->u1.z_ಸೂಚ್ಯಂಕಿತ_By);
      ಹಿಂತಿರುಗಿ 2;
    }
    ಪ್ರತಿಜ್ಞಾ( !pFrom->fg.is_ಸೂಚ್ಯಂಕಿತ_By );
    pFrom->fg.isCte = 1;
    pFrom->u2.pCteUse = pCteUse;
    pCteUse->nUse++;

    /* Check if this is a recursive CTE. */
    p_Rec_ಪದ = pSel;
    bMayRecursive = ( pSel->op==TK_ALL || pSel->op==TK_UNION );
    ಆಗ( bMayRecursive && p_Rec_ಪದ->op==pSel->op ){
      ಪೂರ್ಣಾಂಕ i;
      SrcList *pSrc = p_Rec_ಪದ->pSrc;
      ಪ್ರತಿಜ್ಞಾ( p_Rec_ಪದ->p_ಪೂರ್ವ!=0 );
      ಗಾಗಿ(i=0; i<pSrc->nSrc; i++){
        Src_ಅಂಶ *p_ಅಂಶ = &pSrc->a[i];
        ಆದರೆ( p_ಅಂಶ->zName!=0
         && !p_ಅಂಶ->fg.hadSchema
         && ALWAYS( !p_ಅಂಶ->fg.isSubquery )
         && (p_ಅಂಶ->fg.fixedSchema || p_ಅಂಶ->u4.zDatabase==0)
         && 0==sqlite3StrICmp(p_ಅಂಶ->zName, pCte->zName)
        ){
          p_ಅಂಶ->pSTab = pTab;
          pTab->nTabRef++;
          p_ಅಂಶ->fg.isRecursive = 1;
          ಆದರೆ( p_Rec_ಪದ->selFlags & SF_Recursive ){
            sqlite3_Error_ಸಂದೇಶ(pParse,
               "multiple references to recursive table: %s", pCte->zName
            );
            ಹಿಂತಿರುಗಿ 2;
          }
          p_Rec_ಪದ->selFlags |= SF_Recursive;
          ಆದರೆ( iRecTab<0 ) iRecTab = pParse->nTab++;
          p_ಅಂಶ->iCursor = iRecTab;
        }
      }
      ಆದರೆ( (p_Rec_ಪದ->selFlags & SF_Recursive)==0 ) ನಿಲ್ಲಿಸು;
      p_Rec_ಪದ = p_Rec_ಪದ->p_ಪೂರ್ವ;
    }

    ಆದರೆ( pParse->nTab >= db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_CTES]
     && db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_CTES]>0
    ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "too many CTEs");
      ಹಿಂತಿರುಗಿ 2;
    }
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
    pParse->nNestSel++;
    ಆದರೆ( check_Subquery_Nesting_ಆಳ(pParse) ) ಹಿಂತಿರುಗಿ 2;
#ಆದರೆ_ಅಂತ್ಯ
    pCte->zCteErr = "circular reference: %s";
    pSavedWith = pParse->pWith;
    pParse->pWith = pWith;
    ಆದರೆ( pSel->selFlags & SF_Recursive ){
      ಪೂರ್ಣಾಂಕ rc;
      ಪ್ರತಿಜ್ಞಾ( p_Rec_ಪದ!=0 );
      ಪ್ರತಿಜ್ಞಾ( (p_Rec_ಪದ->selFlags & SF_Recursive)==0 );
      ಪ್ರತಿಜ್ಞಾ( p_Rec_ಪದ->p_ಮುಂದಿನ!=0 );
      ಪ್ರತಿಜ್ಞಾ( (p_Rec_ಪದ->p_ಮುಂದಿನ->selFlags & SF_Recursive)!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_Rec_ಪದ->pWith==0 );
      p_Rec_ಪದ->pWith = pSel->pWith;
      rc = sqlite3WalkSelect(pWalker, p_Rec_ಪದ);
      p_Rec_ಪದ->pWith = 0;
      ಆದರೆ( rc ){
        pParse->pWith = pSavedWith;
        ಹಿಂತಿರುಗಿ 2;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಆದರೆ( selectExpanderWalk(pWalker, pSel) ){
        pParse->pWith = pSavedWith;
        ಹಿಂತಿರುಗಿ 2;
      }
    }
    pParse->pWith = pWith;
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
    pParse->nNestSel--;
    ಪ್ರತಿಜ್ಞಾ( pParse->nNestSel>=0 );
#ಆದರೆ_ಅಂತ್ಯ

    ಗಾಗಿ(p_ಎಡ=pSel; p_ಎಡ->p_ಪೂರ್ವ; p_ಎಡ=p_ಎಡ->p_ಪೂರ್ವ);
    pEList = p_ಎಡ->pEList;
    ಆದರೆ( pCte->p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ ){
      ಆದರೆ( pEList && pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=pCte->p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ->n_ಅಭಿವ್ಯಕ್ತಿ ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "table %s has %d values for %d columns",
            pCte->zName, pEList->n_ಅಭಿವ್ಯಕ್ತಿ, pCte->p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ->n_ಅಭಿವ್ಯಕ್ತಿ
        );
        pParse->pWith = pSavedWith;
        ಹಿಂತಿರುಗಿ 2;
      }
      pEList = pCte->p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;
    }

    sqlite3_Columns_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pEList, &pTab->n_ಲಂಬಸಾಲು, &pTab->a_ಲಂಬಸಾಲು);
    ಆದರೆ( bMayRecursive ){
      ಆದರೆ( pSel->selFlags & SF_Recursive ){
        pCte->zCteErr = "multiple recursive references: %s";
      }ಇಲ್ಲದಿದ್ದರೆ{
        pCte->zCteErr = "recursive reference in a subquery: %s";
      }
      sqlite3WalkSelect(pWalker, pSel);
    }
    pCte->zCteErr = 0;
    pParse->pWith = pSavedWith;
    ಹಿಂತಿರುಗಿ 1;  /* Success */
  }
  ಹಿಂತಿರುಗಿ 0;  /* No match */
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
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
  ಆದರೆ( OK_IF_ALWAYS_TRUE(pParse->pWith) && p->p_ಪೂರ್ವ==0 ){
    With *pWith = ಕಂಡುಹಿಡಿ_Rightmost(p)->pWith;
    ಆದರೆ( pWith!=0 ){
      ಪ್ರತಿಜ್ಞಾ( pParse->pWith==pWith || pParse->nErr );
      pParse->pWith = pWith->pOuter;
    }
  }
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** The SrcItem structure passed as the second argument represents a
** sub-query in the FROM clause of a SELECT statement. This function
** allocates and populates the SrcItem.pTab object. If successful,
** SQLITE_OK is returned. Otherwise, if an OOM error is encountered,
** SQLITE_NOMEM.
*/
ಪೂರ್ಣಾಂಕ sqlite3ExpandSubquery(Parse *pParse, Src_ಅಂಶ *pFrom){
  Select *pSel;
  ಕೋಷ್ಟಕ *pTab;

  ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery );
  ಪ್ರತಿಜ್ಞಾ( pFrom->u4.pSubq!=0 );
  pSel = pFrom->u4.pSubq->pSelect;
  ಪ್ರತಿಜ್ಞಾ( pSel );
  pFrom->pSTab = pTab = sqlite3DbMallocZero(pParse->db, ಗಾತ್ರ(ಕೋಷ್ಟಕ));
  ಆದರೆ( pTab==0 ) ಹಿಂತಿರುಗಿ SQLITE_NOMEM;
  pTab->nTabRef = 1;
  ಆದರೆ( pFrom->zAlias ){
    pTab->zName = sqlite3DbStrDup(pParse->db, pFrom->zAlias);
  }ಇಲ್ಲದಿದ್ದರೆ{
    pTab->zName = sqlite3MPrintf(pParse->db, "%!S", pFrom);
  }
  ಆಗ( pSel->p_ಪೂರ್ವ ){ pSel = pSel->p_ಪೂರ್ವ; }
  sqlite3_Columns_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSel->pEList,&pTab->n_ಲಂಬಸಾಲು,&pTab->a_ಲಂಬಸಾಲು);
  pTab->iPKey = -1;
  pTab->eTabType = TABTYP_VIEW;
  pTab->n_ಅಡ್ಡಸಾಲು_Log_Est = 200; ಪ್ರತಿಜ್ಞಾ( 200==sqlite3LogEst(1048576) );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
  /* The usual case - do not allow ROWID on a subquery */
  pTab->tabFlags |= TF_Ephemeral | TF_NoVisibleRowid;
#ಇಲ್ಲದಿದ್ದರೆ
  /* Legacy compatibility mode */
  pTab->tabFlags |= TF_Ephemeral | sqlite3Config.mNoVisibleRowid;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ pParse->nErr ? SQLITE_ERROR : SQLITE_OK;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಒಳ_Any_Using_Clause(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName, /* Name we are looking for */
  Src_ಅಂಶ *p_ತಳ,    /* The base SrcItem.  Looking at pBase[1] and following */
  ಪೂರ್ಣಾಂಕ N              /* How many SrcItems to check */
){
  ಆಗ( N>0 ){
    N--;
    p_ತಳ++;
    ಆದರೆ( p_ತಳ->fg.isUsing==0 ) ಮುಂದುವರಿ;
    ಆದರೆ( NEVER(p_ತಳ->u3.pUsing==0) ) ಮುಂದುವರಿ;
    ಆದರೆ( sqlite3_Id_List_ಸೂಚ್ಯಂಕ(p_ತಳ->u3.pUsing, zName)>=0 ) ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ selectExpander(Walker *pWalker, Select *p){
  Parse *pParse = pWalker->pParse;
  ಪೂರ್ಣಾಂಕ i, j, k, rc;
  SrcList *pTabList;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  Src_ಅಂಶ *pFrom;
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ *pE, *p_ಬಲ, *p_ಅಭಿವ್ಯಕ್ತಿ;
  u16 selFlags = p->selFlags;
  u32 elistFlags = 0;

  p->selFlags |= SF_Expanded;
  ಆದರೆ( db->mallocFailed  ){
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }
  ಪ್ರತಿಜ್ಞಾ( p->pSrc!=0 );
  ಆದರೆ( (selFlags & SF_Expanded)!=0 ){
    ಹಿಂತಿರುಗಿ WRC_Prune;
  }
  ಆದರೆ( pWalker->e_ಸಂಕೇತ ){
    /* Renumber selId because it has been copied from a view */
    p->selId = ++pParse->nSelect;
  }
  pTabList = p->pSrc;
  pEList = p->pEList;
  ಆದರೆ( pParse->pWith && (p->selFlags & SF_View) ){
    ಆದರೆ( p->pWith==0 ){
      p->pWith = (With*)sqlite3DbMallocZero(db, SZ_WITH(1) );
      ಆದರೆ( p->pWith==0 ){
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
    }
    p->pWith->bView = 1;
  }
  sqlite3_With_ತಳ್ಳು(pParse, p->pWith, 0);

  /* Make sure cursor numbers have been assigned to all entries in
  ** the FROM clause of the SELECT statement.
  */
  sqlite3_Src_List_ನಿಯೋಜಿಸು_Cursors(pParse, pTabList);

  /* Look up every table named in the FROM clause of the select.  If
  ** an entry of the FROM clause is a subquery instead of a table or view,
  ** then create a transient table structure to describe the subquery.
  */
  ಗಾಗಿ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    ಕೋಷ್ಟಕ *pTab;
    ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isRecursive==0 || pFrom->pSTab!=0 );
    ಆದರೆ( pFrom->pSTab ) ಮುಂದುವರಿ;
    ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isRecursive==0 );
    ಆದರೆ( pFrom->zName==0 ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
      Select *pSel;
      ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery && pFrom->u4.pSubq!=0 );
      pSel = pFrom->u4.pSubq->pSelect;
      /* A sub-query in the FROM clause of a SELECT */
      ಪ್ರತಿಜ್ಞಾ( pSel!=0 );
      ಪ್ರತಿಜ್ಞಾ( pFrom->pSTab==0 );
      ಆದರೆ( sqlite3WalkSelect(pWalker, pSel) ) ಹಿಂತಿರುಗಿ WRC_Abort;
      ಆದರೆ( sqlite3ExpandSubquery(pParse, pFrom) ) ಹಿಂತಿರುಗಿ WRC_Abort;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (rc = resolve_From_ಪದ_ಗಮ್ಯ_Cte(pParse, pWalker, pFrom))!=0 ){
      ಆದರೆ( rc>1 ) ಹಿಂತಿರುಗಿ WRC_Abort;
      pTab = pFrom->pSTab;
      ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
#ಆದರೆ_ಅಂತ್ಯ
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* An ordinary table or view name in the FROM clause */
      ಪ್ರತಿಜ್ಞಾ( pFrom->pSTab==0 );
      pFrom->pSTab = pTab = sqlite3_Locate_ಕೋಷ್ಟಕ_ಅಂಶ(pParse, 0, pFrom);
      ಆದರೆ( pTab==0 ) ಹಿಂತಿರುಗಿ WRC_Abort;
      ಆದರೆ( pTab->nTabRef>=0xffff ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "too many references to \"%s\": max 65535",
           pTab->zName);
        pFrom->pSTab = 0;
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
      pTab->nTabRef++;
      ಆದರೆ( !IsVirtual(pTab) && cannotBeFunction(pParse, pFrom) ){
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
#ಆದರೆ !defined(SQLITE_OMIT_VIEW) || !defined(SQLITE_OMIT_VIRTUALTABLE)
      ಆದರೆ( !Is_Ordinary_ಕೋಷ್ಟಕ(pTab) ){
        i16 n_ಲಂಬಸಾಲು;
        u8 e_ಸಂಕೇತ_Orig = pWalker->e_ಸಂಕೇತ;
        ಆದರೆ( sqlite3_View_Get_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, pTab) ) ಹಿಂತಿರುಗಿ WRC_Abort;
        ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery==0 );
        ಆದರೆ( IsView(pTab) ){
          ಆದರೆ( (db->flags & SQLITE_EnableView)==0
           && pTab->pSchema!=db->aDb[1].pSchema
          ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "access to view \"%s\" prohibited",
              pTab->zName);
          }
          sqlite3_Src_ಅಂಶ_Attach_Subquery(pParse, pFrom, pTab->u.view.pSelect, 1);
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_VIRTUALTABLE
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ALWAYS(IsVirtual(pTab))
         && (pFrom->fg.fromDDL || (pParse->prepFlags & SQLITE_PREPARE_FROM_DDL))
         && ALWAYS(pTab->u.vtab.p!=0)
         && pTab->u.vtab.p->eVtabRisk > ((db->flags & SQLITE_TrustedSchema)!=0)
        ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "unsafe use of virtual table \"%s\"",
                                  pTab->zName);
        }
        ಪ್ರತಿಜ್ಞಾ( SQLITE_VTABRISK_ಸಾಮಾನ್ಯ==1 && SQLITE_VTABRISK_ಮೇಲಿನ==2 );
#ಆದರೆ_ಅಂತ್ಯ
        n_ಲಂಬಸಾಲು = pTab->n_ಲಂಬಸಾಲು;
        pTab->n_ಲಂಬಸಾಲು = -1;
        pWalker->e_ಸಂಕೇತ = 1;  /* Turn on Select.selId renumbering */
        ಆದರೆ( pFrom->fg.isSubquery ){
          bft hasCompound = pParse->hasCompound;
          pParse->hasCompound = 1;
          selectExpanderWalk(pWalker, pFrom->u4.pSubq->pSelect);
          pParse->hasCompound = hasCompound;
        }
        pWalker->e_ಸಂಕೇತ = e_ಸಂಕೇತ_Orig;
        pTab->n_ಲಂಬಸಾಲು = n_ಲಂಬಸಾಲು;
      }
#ಆದರೆ_ಅಂತ್ಯ
    }

    /* Locate the index named by the INDEXED BY clause, if any. */
    ಆದರೆ( pFrom->fg.is_ಸೂಚ್ಯಂಕಿತ_By && sqlite3_ಸೂಚ್ಯಂಕಿತ_By_Lookup(pParse, pFrom) ){
      ಹಿಂತಿರುಗಿ WRC_Abort;
    }
  }

  /* Process NATURAL keywords, and ON and USING clauses of joins.
  */
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || pParse->nErr!=0 );
  ಆದರೆ( pParse->nErr || sqlite3ProcessJoin(pParse, p) ){
    ಹಿಂತಿರುಗಿ WRC_Abort;
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
  ಗಾಗಿ(k=0; k<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; k++){
    pE = pEList->a[k].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಆದರೆ( pE->op==TK_ASTERISK ) ನಿಲ್ಲಿಸು;
    ಪ್ರತಿಜ್ಞಾ( pE->op!=TK_DOT || pE->p_ಬಲ!=0 );
    ಪ್ರತಿಜ್ಞಾ( pE->op!=TK_DOT || (pE->p_ಎಡ!=0 && pE->p_ಎಡ->op==TK_ID) );
    ಆದರೆ( pE->op==TK_DOT && pE->p_ಬಲ->op==TK_ASTERISK ) ನಿಲ್ಲಿಸು;
    elistFlags |= pE->flags;
  }
  ಆದರೆ( k<pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
    /*
    ** If we get here it means the result set contains one or more "*"
    ** operators that need to be expanded.  Loop through each expression
    ** in the result set and expand them one by one.
    */
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *a = pEList->a;
    ಅಭಿವ್ಯಕ್ತಿ_List *pNew = 0;
    ಪೂರ್ಣಾಂಕ flags = pParse->db->flags;
    ಪೂರ್ಣಾಂಕ long_ಹೆಸರುಗಳು = (flags & SQLITE_ತುಂಬಿದ_ಲಂಬಸಾಲು_ಹೆಸರುಗಳು)!=0
                      && (flags & SQLITE_Short_ಲಂಬಸಾಲು_ಹೆಸರುಗಳು)==0;

    ಗಾಗಿ(k=0; k<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; k++){
      pE = a[k].p_ಅಭಿವ್ಯಕ್ತಿ;
      elistFlags |= pE->flags;
      p_ಬಲ = pE->p_ಬಲ;
      ಪ್ರತಿಜ್ಞಾ( pE->op!=TK_DOT || p_ಬಲ!=0 );
      ಆದರೆ( pE->op!=TK_ASTERISK
       && (pE->op!=TK_DOT || p_ಬಲ->op!=TK_ASTERISK)
      ){
        /* This particular expression does not need to be expanded.
        */
        pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, a[k].p_ಅಭಿವ್ಯಕ್ತಿ);
        ಆದರೆ( pNew ){
          pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1].zEName = a[k].zEName;
          pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1].fg.eEName = a[k].fg.eEName;
          a[k].zEName = 0;
        }
        a[k].p_ಅಭಿವ್ಯಕ್ತಿ = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* This expression is a "*" or a "TABLE.*" and needs to be
        ** expanded. */
        ಪೂರ್ಣಾಂಕ ಕೋಷ್ಟಕ_ನೋಡಿದ = 0;      /* Set to 1 when TABLE matches */
        ಅಕ್ಷರ *zTName = 0;       /* text of name of TABLE */
        ಪೂರ್ಣಾಂಕ iErrOfst;
        ಆದರೆ( pE->op==TK_DOT ){
          ಪ್ರತಿಜ್ಞಾ( (selFlags & SF_NestedFrom)==0 );
          ಪ್ರತಿಜ್ಞಾ( pE->p_ಎಡ!=0 );
          ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pE->p_ಎಡ, EP_Int_ಮೌಲ್ಯ) );
          zTName = pE->p_ಎಡ->u.zToken;
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_WOfst(pE->p_ಎಡ) );
          iErrOfst = pE->p_ಬಲ->w.iOfst;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪ್ರತಿಜ್ಞಾ( pE->p_ಎಡ==0 );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_WOfst(pE) );
          iErrOfst = pE->w.iOfst;
        }
        ಗಾಗಿ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
          ಪೂರ್ಣಾಂಕ n_ಕೂಡಿಸು;                    /* Number of cols including rowid */
          ಕೋಷ್ಟಕ *pTab = pFrom->pSTab;  /* Table for this data source */
          ಅಭಿವ್ಯಕ್ತಿ_List *pNestedFrom;       /* Result-set of a nested FROM clause */
          ಅಕ್ಷರ *zTabName;              /* AS name for this data source */
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSchemaName = 0; /* Schema name for this data source */
          ಪೂರ್ಣಾಂಕ iDb;                     /* Schema index for this data src */
          IdList *pUsing;              /* USING clause for pFrom[1] */

          ಆದರೆ( (zTabName = pFrom->zAlias)==0 ){
            zTabName = pTab->zName;
          }
          ಆದರೆ( db->mallocFailed ) ನಿಲ್ಲಿಸು;
          ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಂಕ)pFrom->fg.isNestedFrom == IsNestedFrom(pFrom) );
          ಆದರೆ( pFrom->fg.isNestedFrom ){
            ಪ್ರತಿಜ್ಞಾ( pFrom->fg.isSubquery && pFrom->u4.pSubq );
            ಪ್ರತಿಜ್ಞಾ( pFrom->u4.pSubq->pSelect!=0 );
            pNestedFrom = pFrom->u4.pSubq->pSelect->pEList;
            ಪ್ರತಿಜ್ಞಾ( pNestedFrom!=0 );
            ಪ್ರತಿಜ್ಞಾ( pNestedFrom->n_ಅಭಿವ್ಯಕ್ತಿ==pTab->n_ಲಂಬಸಾಲು );
            ಪ್ರತಿಜ್ಞಾ( VisibleRowid(pTab)==0 || ViewCanHaveRowid );
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಆದರೆ( zTName && sqlite3StrICmp(zTName, zTabName)!=0 ){
              ಮುಂದುವರಿ;
            }
            pNestedFrom = 0;
            iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(db, pTab->pSchema);
            zSchemaName = iDb>=0 ? db->aDb[iDb].zDbSName : "*";
          }
          ಆದರೆ( i+1<pTabList->nSrc
           && pFrom[1].fg.isUsing
           && (selFlags & SF_NestedFrom)!=0
          ){
            ಪೂರ್ಣಾಂಕ ii;
            pUsing = pFrom[1].u3.pUsing;
            ಗಾಗಿ(ii=0; ii<pUsing->nId; ii++){
              ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUName = pUsing->a[ii].zName;
              p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zUName);
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Error_ಸ್ಥಾನಾಂತರ(p_ಬಲ, iErrOfst);
              pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, p_ಬಲ);
              ಆದರೆ( pNew ){
                ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *pX = &pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1];
                ಪ್ರತಿಜ್ಞಾ( pX->zEName==0 );
                pX->zEName = sqlite3MPrintf(db,"..%s", zUName);
                pX->fg.eEName = ENAME_TAB;
                pX->fg.b_Using_ಪದ = 1;
              }
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            pUsing = 0;
          }

          n_ಕೂಡಿಸು = pTab->n_ಲಂಬಸಾಲು;
          ಆದರೆ( VisibleRowid(pTab) && (selFlags & SF_NestedFrom)!=0 ) n_ಕೂಡಿಸು++;
          ಗಾಗಿ(j=0; j<n_ಕೂಡಿಸು; j++){
            ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zName; 
            ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *pX; /* Newly added ExprList term */

            ಆದರೆ( j==pTab->n_ಲಂಬಸಾಲು ){
              zName = sqlite3RowidAlias(pTab);
              ಆದರೆ( zName==0 ) ಮುಂದುವರಿ;
            }ಇಲ್ಲದಿದ್ದರೆ{
              zName = pTab->a_ಲಂಬಸಾಲು[j].zCnName;

              /* If pTab is actually an SF_NestedFrom sub-select, do not
              ** expand any ENAME_ROWID columns.  */
              ಆದರೆ( pNestedFrom && pNestedFrom->a[j].fg.eEName==ENAME_ROWID ){
                ಮುಂದುವರಿ;
              }

              ಆದರೆ( zTName
               && pNestedFrom
               && sqlite3_ಹೊಂದಿಕೆ_EName(&pNestedFrom->a[j], 0, zTName, 0, 0)==0
              ){
                ಮುಂದುವರಿ;
              }

              /* If a column is marked as 'hidden', omit it from the expanded
              ** result-set list unless the SELECT has the SF_IncludeHidden
              ** bit set.
              */
              ಆದರೆ( (p->selFlags & SF_IncludeHidden)==0
                && Is_Hidden_ಸ್ತಂಭ(&pTab->a_ಲಂಬಸಾಲು[j])
              ){
                ಮುಂದುವರಿ;
              }
              ಆದರೆ( (pTab->a_ಲಂಬಸಾಲು[j].ಲಂಬಸಾಲು_Flags & COLFLAG_NOEXPAND)!=0
               && zTName==0
               && (selFlags & (SF_NestedFrom))==0
              ){
                ಮುಂದುವರಿ;
              }
            }
            ಪ್ರತಿಜ್ಞಾ( zName );
            ಕೋಷ್ಟಕ_ನೋಡಿದ = 1;

            ಆದರೆ( i>0 && zTName==0 && (selFlags & SF_NestedFrom)==0 ){
              ಆದರೆ( pFrom->fg.isUsing
               && sqlite3_Id_List_ಸೂಚ್ಯಂಕ(pFrom->u3.pUsing, zName)>=0
              ){
                /* In a join with a USING clause, omit columns in the
                ** using clause from the table on the right. */
                ಮುಂದುವರಿ;
              }
            }
            p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zName);
            ಆದರೆ( (pTabList->nSrc>1
                 && (  (pFrom->fg.jointype & JT_LTORJ)==0
                     || zTName!=0
                     || (selFlags & SF_NestedFrom)!=0
                     || !ಒಳ_Any_Using_Clause(zName,pFrom,pTabList->nSrc-i-1)
                    )
                )
             || ಒಳ_RENAME_OBJECT
            ){
              ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ;
              p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zTabName);
              p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_DOT, p_ಎಡ, p_ಬಲ);
              ಆದರೆ( ಒಳ_RENAME_OBJECT && pE->p_ಎಡ ){
                ಪ್ರತಿಜ್ಞಾ( pE->op==TK_DOT && pE->p_ಬಲ->op==TK_ASTERISK );
                sqlite3RenameTokenRemap(pParse, p_ಎಡ, pE->p_ಎಡ);
              }
              ಆದರೆ( zSchemaName ){
                p_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ(db, TK_ID, zSchemaName);
                p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_DOT, p_ಎಡ, p_ಅಭಿವ್ಯಕ್ತಿ);
              }
            }ಇಲ್ಲದಿದ್ದರೆ{
              p_ಅಭಿವ್ಯಕ್ತಿ = p_ಬಲ;
            }
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Error_ಸ್ಥಾನಾಂತರ(p_ಅಭಿವ್ಯಕ್ತಿ, iErrOfst);
            pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pNew, p_ಅಭಿವ್ಯಕ್ತಿ);
            ಆದರೆ( pNew==0 ){
              ನಿಲ್ಲಿಸು;  /* OOM */
            }
            pX = &pNew->a[pNew->n_ಅಭಿವ್ಯಕ್ತಿ-1];
            ಪ್ರತಿಜ್ಞಾ( pX->zEName==0 );
            ಆದರೆ( (selFlags & SF_NestedFrom)!=0 && !ಒಳ_RENAME_OBJECT ){
              ಆದರೆ( pNestedFrom && (!ViewCanHaveRowid || j<pNestedFrom->n_ಅಭಿವ್ಯಕ್ತಿ) ){
                ಪ್ರತಿಜ್ಞಾ( j<pNestedFrom->n_ಅಭಿವ್ಯಕ್ತಿ );
                pX->zEName = sqlite3DbStrDup(db, pNestedFrom->a[j].zEName);
                testcase( pX->zEName==0 );
              }ಇಲ್ಲದಿದ್ದರೆ{
                pX->zEName = sqlite3MPrintf(db, "%s.%s.%s",
                                           zSchemaName, zTabName, zName);
                testcase( pX->zEName==0 );
              }
              pX->fg.eEName = (j==pTab->n_ಲಂಬಸಾಲು ? ENAME_ROWID : ENAME_TAB);
              ಆದರೆ( (pFrom->fg.isUsing
                   && sqlite3_Id_List_ಸೂಚ್ಯಂಕ(pFrom->u3.pUsing, zName)>=0)
               || (pUsing && sqlite3_Id_List_ಸೂಚ್ಯಂಕ(pUsing, zName)>=0)
               || (j<pTab->n_ಲಂಬಸಾಲು && (pTab->a_ಲಂಬಸಾಲು[j].ಲಂಬಸಾಲು_Flags & COLFLAG_NOEXPAND))
              ){
                pX->fg.bNoExpand = 1;
              }
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( long_ಹೆಸರುಗಳು ){
              pX->zEName = sqlite3MPrintf(db, "%s.%s", zTabName, zName);
              pX->fg.eEName = ENAME_NAME;
            }ಇಲ್ಲದಿದ್ದರೆ{
              pX->zEName = sqlite3DbStrDup(db, zName);
              pX->fg.eEName = ENAME_NAME;
            }
          }
        }
        ಆದರೆ( !ಕೋಷ್ಟಕ_ನೋಡಿದ ){
          ಆದರೆ( zTName ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "no such table: %s", zTName);
          }ಇಲ್ಲದಿದ್ದರೆ{
            sqlite3_Error_ಸಂದೇಶ(pParse, "no tables specified");
          }
        }
      }
    }
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pEList);
    p->pEList = pNew;
  }
  ಆದರೆ( p->pEList ){
    ಆದರೆ( p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಸ್ತಂಭ] ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "too many columns in result set");
      ಹಿಂತಿರುಗಿ WRC_Abort;
    }
    ಆದರೆ( (elistFlags & (EP_HasFunc|EP_Subquery))!=0 ){
      p->selFlags |= SF_Complex_ಫಲಿತಾಂಶ;
    }
  }
#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x8 ){
    TREETRACE(0x8,pParse,p,("After result-set wildcard expansion:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

#ಆದರೆ SQLITE_DEBUG
/*
** Always assert.  This xSelectCallback2 implementation proves that the
** xSelectCallback2 is never invoked.
*/
ಶೂನ್ಯ sqlite3SelectWalkAssert2(Walker *Not_ಬಳಸಿದ, Select *NotUsed2){
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  ಪ್ರತಿಜ್ಞಾ( 0 );
}
#ಆದರೆ_ಅಂತ್ಯ

/*
** The first argument is a Walker object created by sqlite3SelectExpand().
** This function uses the walker to expand the SELECT statement passed
** as the second argument. If necessary, it also converts any compound
** SELECT statements that have COLLATE clauses on ORDER BY expressions
** to sub-queries. Return the return value of sqlite3WalkSelect().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ selectExpanderWalk(Walker *pWalker, Select *pSelect){
  ಪ್ರತಿಜ್ಞಾ( pWalker->x_ಅಭಿವ್ಯಕ್ತಿ_Callback==sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop );
  ಪ್ರತಿಜ್ಞಾ( pWalker->pParse!=0 );
  ಆದರೆ( OK_IF_ALWAYS_TRUE(pWalker->pParse->hasCompound) ){
    pWalker->xSelectCallback = convert_Compound_Select_ಗಮ್ಯ_Subquery;
    pWalker->xSelectCallback2 = 0;
    sqlite3WalkSelect(pWalker, pSelect);
  }
  pWalker->xSelectCallback = selectExpander;
  pWalker->xSelectCallback2 = sqlite3_Select_ತೆಗೆ_With;
  ಹಿಂತಿರುಗಿ sqlite3WalkSelect(pWalker, pSelect);
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
  w.e_ಸಂಕೇತ = 0;
  selectExpanderWalk(&w, pSelect);
}


#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
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
ಸ್ಥಿರ ಶೂನ್ಯ select_ಕೂಡಿಸು_Subquery_Type_Info(Walker *pWalker, Select *p){
  Parse *pParse;
  ಪೂರ್ಣಾಂಕ i;
  SrcList *pTabList;
  Src_ಅಂಶ *pFrom;

  ಆದರೆ( p->selFlags & SF_HasTypeInfo ) ಹಿಂತಿರುಗಿ;
  p->selFlags |= SF_HasTypeInfo;
  pParse = pWalker->pParse;
  ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Resolved) );
  pTabList = p->pSrc;
  ಗಾಗಿ(i=0, pFrom=pTabList->a; i<pTabList->nSrc; i++, pFrom++){
    ಕೋಷ್ಟಕ *pTab = pFrom->pSTab;
    ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
    ಆದರೆ( (pTab->tabFlags & TF_Ephemeral)!=0 && pFrom->fg.isSubquery ){
      /* A sub-query in the FROM clause of a SELECT */
      Select *pSel = pFrom->u4.pSubq->pSelect;
      sqlite3_Subquery_ಸ್ತಂಭ_Types(pParse, pTab, pSel, SQLITE_AFF_NONE);
    }
  }
}
#ಆದರೆ_ಅಂತ್ಯ


/*
** This routine adds datatype and collating sequence information to
** the Table structures of all FROM-clause subqueries in a
** SELECT statement.
**
** Use this routine after name resolution.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_Select_ಕೂಡಿಸು_Type_Info(Parse *pParse, Select *pSelect){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
  Walker w;
  w.xSelectCallback = sqlite3SelectWalkNoop;
  w.xSelectCallback2 = select_ಕೂಡಿಸು_Subquery_Type_Info;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop;
  w.pParse = pParse;
  sqlite3WalkSelect(&w, pSelect);
#ಆದರೆ_ಅಂತ್ಯ
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
  ಆದರೆ( pParse->db->mallocFailed ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( p->selFlags & SF_HasTypeInfo ) ಹಿಂತಿರುಗಿ;
  sqlite3SelectExpand(pParse, p);
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  sqlite3_Resolve_Select_ಹೆಸರುಗಳು(pParse, p, pOuterNC);
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  sqlite3_Select_ಕೂಡಿಸು_Type_Info(pParse, p);
}

#ಆದರೆ TREETRACE_ENABLED
/*
** Display all information about an AggInfo object
*/
ಸ್ಥಿರ ಶೂನ್ಯ printAggInfo(AggInfo *pAggInfo){
  ಪೂರ್ಣಾಂಕ ii;
  sqlite3DebugPrintf("AggInfo %d/%p:\n",
     pAggInfo->selId, pAggInfo);
  ಗಾಗಿ(ii=0; ii<pAggInfo->n_ಸ್ತಂಭ; ii++){
    ರಚನೆ Agg_Info_ಲಂಬಸಾಲು *p_ಲಂಬಸಾಲು = &pAggInfo->a_ಲಂಬಸಾಲು[ii];
    sqlite3DebugPrintf(
       "agg-column[%d] pTab=%s iTable=%d iColumn=%d iMem=%d"
       " iSorterColumn=%d %s\n",
       ii, p_ಲಂಬಸಾಲು->pTab ? p_ಲಂಬಸಾಲು->pTab->zName : "NULL",
       p_ಲಂಬಸಾಲು->i_ಕೋಷ್ಟಕ, p_ಲಂಬಸಾಲು->i_ಸ್ತಂಭ, pAggInfo->i_ಮೊದಲ_Reg+ii,
       p_ಲಂಬಸಾಲು->i_Sorter_ಸ್ತಂಭ,
       ii>=pAggInfo->n_ಸಂಚಯಕ ? "" : " Accumulator");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0, pAggInfo->a_ಲಂಬಸಾಲು[ii].pCExpr, 0);
  }
  ಗಾಗಿ(ii=0; ii<pAggInfo->nFunc; ii++){
    sqlite3DebugPrintf("agg-func[%d]: iMem=%d\n",
        ii, pAggInfo->i_ಮೊದಲ_Reg+pAggInfo->n_ಸ್ತಂಭ+ii);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0, pAggInfo->aFunc[ii].pFExpr, 0);
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* TREETRACE_ENABLED */

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
ಸ್ಥಿರ ಶೂನ್ಯ analyze_Agg_Func_ವಾದಗಳು(
  AggInfo *pAggInfo,
  NameContext *pNC
){
  ಪೂರ್ಣಾಂಕ i;
  ಪ್ರತಿಜ್ಞಾ( pAggInfo!=0 );
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->i_ಮೊದಲ_Reg==0 );
  pNC->ncFlags |= NC_ಒಳ_Agg_Func;
  ಗಾಗಿ(i=0; i<pAggInfo->nFunc; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pAggInfo->aFunc[i].pFExpr;
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION );
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(pNC, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList);
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ಕ್ರಮ );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(pNC, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಪ್ರತಿಜ್ಞಾ( !Is_ಕಿಟಕಿ_Func(p_ಅಭಿವ್ಯಕ್ತಿ) );
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Aggregates(pNC, p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin->pFilter);
    }
#ಆದರೆ_ಅಂತ್ಯ
  }
  pNC->ncFlags &= ~NC_ಒಳ_Agg_Func;
}

/*
** An index on expressions is being used in the inner loop of an
** aggregate query with a GROUP BY clause.  This routine attempts
** to adjust the AggInfo object to take advantage of index and to
** perhaps use the index as a covering index.
**
*/
ಸ್ಥಿರ ಶೂನ್ಯ optimize_Aggregate_Use_ದ_ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ(
  Parse *pParse,          /* Parsing context */
  Select *pSelect,        /* The SELECT statement being processed */
  AggInfo *pAggInfo,      /* The aggregate info */
  NameContext *pNC        /* Name context used to resolve agg-func args */
){
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->i_ಮೊದಲ_Reg==0 );
  ಪ್ರತಿಜ್ಞಾ( pSelect!=0 );
  ಪ್ರತಿಜ್ಞಾ( pSelect->p_ಗುಂಪು_By!=0 );
  pAggInfo->n_ಸ್ತಂಭ = pAggInfo->n_ಸಂಚಯಕ;
  ಆದರೆ( ALWAYS(pAggInfo->n_ವಿಂಗಡಣೆ_ಸ್ತಂಭ>0) ){
    ಪೂರ್ಣಾಂಕ mx = pSelect->p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ - 1;
    ಪೂರ್ಣಾಂಕ j, k;
    ಗಾಗಿ(j=0; j<pAggInfo->n_ಸ್ತಂಭ; j++){
      k = pAggInfo->a_ಲಂಬಸಾಲು[j].i_Sorter_ಸ್ತಂಭ;
      ಆದರೆ( k>mx ) mx = k;
    }
    pAggInfo->n_ವಿಂಗಡಣೆ_ಸ್ತಂಭ = mx+1;
  }
  analyze_Agg_Func_ವಾದಗಳು(pAggInfo, pNC);
#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x20 ){
    ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ *pIEpr;
    TREETRACE(0x20, pParse, pSelect,
        ("AggInfo (possibly) adjusted for Indexed Exprs\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, pSelect, 0);
    ಗಾಗಿ(pIEpr=pParse->pIdxEpr; pIEpr; pIEpr=pIEpr->pIENext){
      ಬರೆ("data-cursor=%d index={%d,%d}\n",
          pIEpr->i_ದತ್ತಾಂಶ_Cur, pIEpr->iIdxCur, pIEpr->i_Idx_ಲಂಬಸಾಲು);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0, pIEpr->p_ಅಭಿವ್ಯಕ್ತಿ, 0);
    }
    printAggInfo(pAggInfo);
  }
#ಇಲ್ಲದಿದ್ದರೆ
  ಬಳಸದ_PARAMETER(pSelect);
  ಬಳಸದ_PARAMETER(pParse);
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Walker callback for aggregateConvertIndexedExprRefToColumn().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ aggregate_Idx_Epr_Ref_ಗಮ್ಯ_ಲಂಬಸಾಲು_Callback(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  AggInfo *pAggInfo;
  ರಚನೆ Agg_Info_ಲಂಬಸಾಲು *p_ಲಂಬಸಾಲು;
  ಬಳಸದ_PARAMETER(pWalker);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==0 ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_ಸ್ತಂಭ ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IF_NULL_ಅಡ್ಡಸಾಲು ) ಹಿಂತಿರುಗಿ WRC_Continue;
  pAggInfo = p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo;
  ಆದರೆ( NEVER(p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=pAggInfo->n_ಸ್ತಂಭ) ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg>=0 );
  p_ಲಂಬಸಾಲು = &pAggInfo->a_ಲಂಬಸಾಲು[p_ಅಭಿವ್ಯಕ್ತಿ->iAgg];
  p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_AGG_ಸ್ತಂಭ;
  p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = p_ಲಂಬಸಾಲು->i_ಕೋಷ್ಟಕ;
  p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = p_ಲಂಬಸಾಲು->i_ಸ್ತಂಭ;
  ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Skip|EP_Collate|EP_Unlikely);
  ಹಿಂತಿರುಗಿ WRC_Prune;
}

/*
** Convert every pAggInfo->aFunc[].pExpr such that any node within
** those expressions that has pAppInfo set is changed into a TK_AGG_COLUMN
** opcode.
*/
ಸ್ಥಿರ ಶೂನ್ಯ aggregate_Convert_ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ_Ref_ಗಮ್ಯ_ಸ್ತಂಭ(AggInfo *pAggInfo){
  ಪೂರ್ಣಾಂಕ i;
  Walker w;
  memset(&w, 0, ಗಾತ್ರ(w));
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = aggregate_Idx_Epr_Ref_ಗಮ್ಯ_ಲಂಬಸಾಲು_Callback;
  ಗಾಗಿ(i=0; i<pAggInfo->nFunc; i++){
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
ಸ್ಥಿರ ಶೂನ್ಯ ನಿಯೋಜಿಸು_Aggregate_Registers(Parse *pParse, AggInfo *pAggInfo){
  ಪ್ರತಿಜ್ಞಾ( pAggInfo!=0 );
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->i_ಮೊದಲ_Reg==0 );
  pAggInfo->i_ಮೊದಲ_Reg = pParse->nMem + 1;
  pParse->nMem += pAggInfo->n_ಸ್ತಂಭ + pAggInfo->nFunc;
}

/*
** Reset the aggregate accumulator.
**
** The aggregate accumulator is a set of memory cells that hold
** intermediate results while calculating an aggregate.  This
** routine generates code that stores NULLs in all of those memory
** cells.
*/
ಸ್ಥಿರ ಶೂನ್ಯ reset_ಸಂಚಯಕ(Parse *pParse, AggInfo *pAggInfo){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ i;
  ರಚನೆ AggInfo_func *pFunc;
  ಪೂರ್ಣಾಂಕ nReg = pAggInfo->nFunc + pAggInfo->n_ಸ್ತಂಭ;
  ಪ್ರತಿಜ್ಞಾ( pAggInfo->i_ಮೊದಲ_Reg>0 );
  ಪ್ರತಿಜ್ಞಾ( pParse->db->pParse==pParse );
  ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ಆದರೆ( nReg==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Null, 0, pAggInfo->i_ಮೊದಲ_Reg,
                    pAggInfo->i_ಮೊದಲ_Reg+nReg-1);
  ಗಾಗಿ(pFunc=pAggInfo->aFunc, i=0; i<pAggInfo->nFunc; i++, pFunc++){
    ಆದರೆ( pFunc->iDistinct>=0 ){
      ಅಭಿವ್ಯಕ್ತಿ *pE = pFunc->pFExpr;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pE) );
      ಆದರೆ( pE->x.pList==0 || pE->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ!=1 ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "DISTINCT aggregates must have exactly one "
           "argument");
        pFunc->iDistinct = -1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pE->x.pList,0,0);
        pFunc->iDistAddr = sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_OpenEphemeral,
            pFunc->iDistinct, 0, 0, (ಅಕ್ಷರ*)p_ಕೀಲಿ_Info, P4_KEYINFO);
        ExplainQueryPlan((pParse, 0, "USE TEMP B-TREE FOR %s(DISTINCT)",
                          pFunc->pFunc->zName));
      }
    }
    ಆದರೆ( pFunc->iOBTab>=0 ){
      ಅಭಿವ್ಯಕ್ತಿ_List *pOBList;
      ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
      ಪೂರ್ಣಾಂಕ nExtra = 0;
      ಪ್ರತಿಜ್ಞಾ( pFunc->pFExpr->p_ಎಡ!=0 );
      ಪ್ರತಿಜ್ಞಾ( pFunc->pFExpr->p_ಎಡ->op==TK_ಕ್ರಮ );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pFunc->pFExpr->p_ಎಡ) );
      ಪ್ರತಿಜ್ಞಾ( pFunc->pFunc!=0 );
      pOBList = pFunc->pFExpr->p_ಎಡ->x.pList;
      ಆದರೆ( !pFunc->bOBUnique ){
        nExtra++;  /* One extra column for the OP_Sequence */
      }
      ಆದರೆ( pFunc->bOBPayload ){
        /* extra columns for the function arguments */
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pFunc->pFExpr) );
        ಪ್ರತಿಜ್ಞಾ( pFunc->pFExpr->x.pList!=0 );
        nExtra += pFunc->pFExpr->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      }
      ಆದರೆ( pFunc->bUseSubtype ){
        nExtra += pFunc->pFExpr->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      }
      p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pOBList, 0, nExtra);
      ಆದರೆ( !pFunc->bOBUnique && pParse->nErr==0 ){
        p_ಕೀಲಿ_Info->n_ಕೀಲಿ_ಕ್ಷೇತ್ರ++;
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_OpenEphemeral,
            pFunc->iOBTab, pOBList->n_ಅಭಿವ್ಯಕ್ತಿ+nExtra, 0,
            (ಅಕ್ಷರ*)p_ಕೀಲಿ_Info, P4_KEYINFO);
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
  ಪೂರ್ಣಾಂಕ i;
  ರಚನೆ AggInfo_func *pF;
  ಗಾಗಿ(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pF->pFExpr) );
    ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
    pList = pF->pFExpr->x.pList;
    ಆದರೆ( pF->iOBTab>=0 ){
      /* For an ORDER BY aggregate, calls to OP_AggStep were deferred.  Inputs
      ** were stored in emphermal table pF->iOBTab.  Here, we extract those
      ** inputs (in ORDER BY order) and make all calls to OP_AggStep
      ** before doing the OP_AggFinal call. */
      ಪೂರ್ಣಾಂಕ i_ತುದಿ;        /* Start of loop for extracting columns */
      ಪೂರ್ಣಾಂಕ nArg;        /* Number of columns to extract */
      ಪೂರ್ಣಾಂಕ n_ಕೀಲಿ;        /* Key columns to be skipped */
      ಪೂರ್ಣಾಂಕ regAgg;      /* Extract into this array */
      ಪೂರ್ಣಾಂಕ j;           /* Loop counter */
     
      ಪ್ರತಿಜ್ಞಾ( pF->pFunc!=0 );
      nArg = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      regAgg = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, nArg);

      ಆದರೆ( pF->bOBPayload==0 ){
        n_ಕೀಲಿ = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( pF->pFExpr->p_ಎಡ!=0 );
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pF->pFExpr->p_ಎಡ) );
        ಪ್ರತಿಜ್ಞಾ( pF->pFExpr->p_ಎಡ->x.pList!=0 );
        n_ಕೀಲಿ = pF->pFExpr->p_ಎಡ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ;
        ಆದರೆ( ALWAYS(!pF->bOBUnique) ) n_ಕೀಲಿ++;
      }
      i_ತುದಿ = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Rewind, pF->iOBTab); VdbeCoverage(v);
      ಗಾಗಿ(j=nArg-1; j>=0; j--){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, pF->iOBTab, n_ಕೀಲಿ+j, regAgg+j);
      }
      ಆದರೆ( pF->bUseSubtype ){
        ಪೂರ್ಣಾಂಕ regSubtype = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
        ಪೂರ್ಣಾಂಕ i_ತಳ_ಲಂಬಸಾಲು = n_ಕೀಲಿ + nArg + (pF->bOBPayload==0 && pF->bOBUnique==0);
        ಗಾಗಿ(j=nArg-1; j>=0; j--){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, pF->iOBTab, i_ತಳ_ಲಂಬಸಾಲು+j, regSubtype);
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಗಣ_Subtype, regSubtype, regAgg+j);
        }
        sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, regSubtype);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Agg_ಹೆಜ್ಜೆ, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಮುಂದಿನ, pF->iOBTab, i_ತುದಿ+1); VdbeCoverage(v);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, i_ತುದಿ);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, regAgg, nArg);
    }
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_AggFinal, AggInfoFuncReg(pAggInfo,i),
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
ಸ್ಥಿರ ಶೂನ್ಯ update_ಸಂಚಯಕ(
  Parse *pParse,
  ಪೂರ್ಣಾಂಕ regAcc,
  AggInfo *pAggInfo,
  ಪೂರ್ಣಾಂಕ eDistinctType
){
  Vdbe *v = pParse->pVdbe;
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ reg_ದೊರಕಿತು = 0;
  ಪೂರ್ಣಾಂಕ addr_ದೊರಕಿತು_Test = 0;
  ರಚನೆ AggInfo_func *pF;
  ರಚನೆ Agg_Info_ಲಂಬಸಾಲು *pC;

  ಪ್ರತಿಜ್ಞಾ( pAggInfo->i_ಮೊದಲ_Reg>0 );
  ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  pAggInfo->ನೇರ_ಬಹುಲಕ = 1;
  ಗಾಗಿ(i=0, pF=pAggInfo->aFunc; i<pAggInfo->nFunc; i++, pF++){
    ಪೂರ್ಣಾಂಕ nArg;
    ಪೂರ್ಣಾಂಕ addr_ಮುಂದಿನ = 0;
    ಪೂರ್ಣಾಂಕ regAgg;
    ಪೂರ್ಣಾಂಕ regAggSz = 0;
    ಪೂರ್ಣಾಂಕ regDistinct = 0;
    ಅಭಿವ್ಯಕ್ತಿ_List *pList;
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pF->pFExpr) );
    ಪ್ರತಿಜ್ಞಾ( !Is_ಕಿಟಕಿ_Func(pF->pFExpr) );
    ಪ್ರತಿಜ್ಞಾ( pF->pFunc!=0 );
    pList = pF->pFExpr->x.pList;
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pF->pFExpr, EP_WinFunc) ){
      ಅಭಿವ್ಯಕ್ತಿ *pFilter = pF->pFExpr->y.pWin->pFilter;
      ಆದರೆ( pAggInfo->n_ಸಂಚಯಕ
       && (pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL)
       && regAcc
      ){
        /* If regAcc==0, there there exists some min() or max() function
        ** without a FILTER clause that will ensure the magnet registers
        ** are populated. */
        ಆದರೆ( reg_ದೊರಕಿತು==0 ) reg_ದೊರಕಿತು = ++pParse->nMem;
        /* If this is the first row of the group (regAcc contains 0), clear the
        ** "magnet" register regHit so that the accumulator registers
        ** are populated if the FILTER clause jumps over the the
        ** invocation of min() or max() altogether. Or, if this is not
        ** the first row (regAcc contains 1), set the magnet register so that
        ** the accumulators are not populated unless the min()/max() is invoked
        ** and indicates that they should be.  */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Copy, regAcc, reg_ದೊರಕಿತು);
      }
      addr_ಮುಂದಿನ = sqlite3VdbeMakeLabel(pParse);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pFilter, addr_ಮುಂದಿನ, SQLITE_JUMPIFNULL);
    }
    ಆದರೆ( pF->iOBTab>=0 ){
      /* Instead of invoking AggStep, we must push the arguments that would
      ** have been passed to AggStep onto the sorting table. */
      ಪೂರ್ಣಾಂಕ jj;                /* Registered used so far in building the record */
      ಅಭಿವ್ಯಕ್ತಿ_List *pOBList;     /* The ORDER BY clause */
      ಪ್ರತಿಜ್ಞಾ( pList!=0 );
      nArg = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಪ್ರತಿಜ್ಞಾ( nArg>0 );
      ಪ್ರತಿಜ್ಞಾ( pF->pFExpr->p_ಎಡ!=0 );
      ಪ್ರತಿಜ್ಞಾ( pF->pFExpr->p_ಎಡ->op==TK_ಕ್ರಮ );
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pF->pFExpr->p_ಎಡ) );
      pOBList = pF->pFExpr->p_ಎಡ->x.pList;
      ಪ್ರತಿಜ್ಞಾ( pOBList!=0 );
      ಪ್ರತಿಜ್ಞಾ( pOBList->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
      regAggSz = pOBList->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( !pF->bOBUnique ){
        regAggSz++;   /* One register for OP_Sequence */
      }
      ಆದರೆ( pF->bOBPayload ){
        regAggSz += nArg;
      }
      ಆದರೆ( pF->bUseSubtype ){
        regAggSz += nArg;
      }
      regAggSz++;  /* One extra register to hold result of MakeRecord */
      regAgg = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, regAggSz);
      regDistinct = regAgg;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pOBList, regAgg, 0, SQLITE_ECEL_DUP);
      jj = pOBList->n_ಅಭಿವ್ಯಕ್ತಿ;
      ಆದರೆ( !pF->bOBUnique ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Sequence, pF->iOBTab, regAgg+jj);
        jj++;
      }
      ಆದರೆ( pF->bOBPayload ){
        regDistinct = regAgg+jj;
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pList, regDistinct, 0, SQLITE_ECEL_DUP);
        jj += nArg;
      }
      ಆದರೆ( pF->bUseSubtype ){
        ಪೂರ್ಣಾಂಕ kk;
        ಪೂರ್ಣಾಂಕ reg_ತಳ = pF->bOBPayload ? regDistinct : regAgg;
        ಗಾಗಿ(kk=0; kk<nArg; kk++, jj++){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_GetSubtype, reg_ತಳ+kk, regAgg+jj);
        }
      }
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pList ){
      nArg = pList->n_ಅಭಿವ್ಯಕ್ತಿ;
      regAgg = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, nArg);
      regDistinct = regAgg;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pList, regAgg, 0, SQLITE_ECEL_DUP);
    }ಇಲ್ಲದಿದ್ದರೆ{
      nArg = 0;
      regAgg = 0;
    }
    ಆದರೆ( pF->iDistinct>=0 && pList ){
      ಆದರೆ( addr_ಮುಂದಿನ==0 ){
        addr_ಮುಂದಿನ = sqlite3VdbeMakeLabel(pParse);
      }
      pF->iDistinct = ಸಂಕೇತ_Distinct(pParse, eDistinctType,
          pF->iDistinct, addr_ಮುಂದಿನ, pList, regDistinct);
    }
    ಆದರೆ( pF->iOBTab>=0 ){
      /* Insert a new record into the ORDER BY table */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, regAgg, regAggSz-1,
                        regAgg+regAggSz-1);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_Idx_ಸೇರಿಸು, pF->iOBTab, regAgg+regAggSz-1,
                           regAgg, regAggSz-1);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, regAgg, regAggSz);
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Invoke the AggStep function */
      ಆದರೆ( pF->pFunc->funcFlags & SQLITE_FUNC_NEEDCOLL ){
        CollSeq *pColl = 0;
        ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
        ಪೂರ್ಣಾಂಕ j;
        ಪ್ರತಿಜ್ಞಾ( pList!=0 );  /* pList!=0 if pF->pFunc has NEEDCOLL */
        ಗಾಗಿ(j=0, p_ಅಂಶ=pList->a; !pColl && j<nArg; j++, p_ಅಂಶ++){
          pColl = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Coll_Seq(pParse, p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ);
        }
        ಆದರೆ( !pColl ){
          pColl = pParse->db->pDfltColl;
        }
        ಆದರೆ( reg_ದೊರಕಿತು==0 && pAggInfo->n_ಸಂಚಯಕ ) reg_ದೊರಕಿತು = ++pParse->nMem;
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_CollSeq, reg_ದೊರಕಿತು, 0, 0,
                         (ಅಕ್ಷರ *)pColl, P4_COLLSEQ);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Agg_ಹೆಜ್ಜೆ, 0, regAgg, AggInfoFuncReg(pAggInfo,i));
      sqlite3VdbeAppendP4(v, pF->pFunc, P4_FUNCDEF);
      sqlite3VdbeChangeP5(v, (u16)nArg);
      sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, regAgg, nArg);
    }
    ಆದರೆ( addr_ಮುಂದಿನ ){
      sqlite3VdbeResolveLabel(v, addr_ಮುಂದಿನ);
    }
    ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( reg_ದೊರಕಿತು==0 && pAggInfo->n_ಸಂಚಯಕ ){
    reg_ದೊರಕಿತು = regAcc;
  }
  ಆದರೆ( reg_ದೊರಕಿತು ){
    addr_ದೊರಕಿತು_Test = sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_If, reg_ದೊರಕಿತು); VdbeCoverage(v);
  }
  ಗಾಗಿ(i=0, pC=pAggInfo->a_ಲಂಬಸಾಲು; i<pAggInfo->n_ಸಂಚಯಕ; i++, pC++){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, pC->pCExpr, Agg_Info_ಸ್ತಂಭ_Reg(pAggInfo,i));
    ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ;
  }

  pAggInfo->ನೇರ_ಬಹುಲಕ = 0;
  ಆದರೆ( addr_ದೊರಕಿತು_Test ){
    sqlite3_Vdbe_Jump_ಇಲ್ಲಿ_Or_ತೆಗೆ_Inst(v, addr_ದೊರಕಿತು_Test);
  }
}

/*
** Add a single OP_Explain instruction to the VDBE to explain a simple
** count(*) query ("SELECT count(*) FROM pTab").
*/
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_EXPLAIN
ಸ್ಥಿರ ಶೂನ್ಯ explain_Simple_ಎಣಿಕೆ(
  Parse *pParse,                  /* Parse context */
  ಕೋಷ್ಟಕ *pTab,                    /* Table being queried */
  ಸೂಚ್ಯಂಕ *pIdx                     /* Index used to optimize scan, or NULL */
){
  ಆದರೆ( pParse->explain==2 ){
    ಪೂರ್ಣಾಂಕ bCover = (pIdx!=0 && (HasRowid(pTab) || !Is_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pIdx)));
    sqlite3VdbeExplain(pParse, 0, "SCAN %s%s%s",
        pTab->zName,
        bCover ? " USING COVERING INDEX " : "",
        bCover ? pIdx->zName : ""
    );
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define explain_Simple_ಎಣಿಕೆ(a,b,c)
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ having_ಗಮ್ಯ_Where_ಅಭಿವ್ಯಕ್ತಿ_Cb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AND ){
    Select *pS = pWalker->u.pSelect;
    /* This routine is called before the HAVING clause of the current
    ** SELECT is analyzed for aggregates. So if pExpr->pAggInfo is set
    ** here, it indicates that the expression is a correlated reference to a
    ** column from an outer aggregate query, or an aggregate function that
    ** belongs to an outer query. Do not move the expression to the WHERE
    ** clause in this obscure case, as doing so may corrupt the outer Select
    ** statements AggInfo structure.  */
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Constant_Or_ಗುಂಪು_By(pWalker->pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pS->p_ಗುಂಪು_By)
     && ಅಭಿವ್ಯಕ್ತಿ_Always_False(p_ಅಭಿವ್ಯಕ್ತಿ)==0
     && p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==0
    ){
      sqlite3 *db = pWalker->pParse->db;
      ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, 1);
      ಆದರೆ( pNew ){
        ಅಭಿವ್ಯಕ್ತಿ *pWhere = pS->pWhere;
        ಅದಲುಬದಲು(ಅಭಿವ್ಯಕ್ತಿ, *pNew, *p_ಅಭಿವ್ಯಕ್ತಿ);
        pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_And(pWalker->pParse, pWhere, pNew);
        pS->pWhere = pNew;
        pWalker->e_ಸಂಕೇತ = 1;
      }
    }
    ಹಿಂತಿರುಗಿ WRC_Prune;
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಸ್ಥಿರ ಶೂನ್ಯ having_ಗಮ್ಯ_Where(Parse *pParse, Select *p){
  Walker sWalker;
  memset(&sWalker, 0, ಗಾತ್ರ(sWalker));
  sWalker.pParse = pParse;
  sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = having_ಗಮ್ಯ_Where_ಅಭಿವ್ಯಕ್ತಿ_Cb;
  sWalker.u.pSelect = p;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p->pHaving);
#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sWalker.e_ಸಂಕೇತ && (sqlite3_ವೃಕ್ಷ_Trace & 0x100)!=0 ){
    TREETRACE(0x100,pParse,p,("Move HAVING terms into WHERE:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
}

/*
** Check to see if the pThis entry of pTabList is a self-join of another view.
** Search FROM-clause entries in the range of iFirst..iEnd, including iFirst
** but stopping before iEnd.
**
** If pThis is a self-join, then return the SrcItem for the first other
** instance of that view found.  If pThis is not a self-join then return 0.
*/
ಸ್ಥಿರ Src_ಅಂಶ *isSelfJoinView(
  SrcList *pTabList,           /* Search for self-joins in this FROM clause */
  Src_ಅಂಶ *pThis,              /* Search for prior reference to this subquery */
  ಪೂರ್ಣಾಂಕ i_ಮೊದಲ, ಪೂರ್ಣಾಂಕ i_ಕೊನೆ        /* Range of FROM-clause entries to search. */
){
  Src_ಅಂಶ *p_ಅಂಶ;
  Select *pSel;
  ಪ್ರತಿಜ್ಞಾ( pThis->fg.isSubquery );
  pSel = pThis->u4.pSubq->pSelect;
  ಪ್ರತಿಜ್ಞಾ( pSel!=0 );
  ಆದರೆ( pSel->selFlags & SF_ತಳ್ಳು_ಕೆಳಕ್ಕೆ ) ಹಿಂತಿರುಗಿ 0;
  ಆಗ( i_ಮೊದಲ<i_ಕೊನೆ ){
    Select *pS1;
    p_ಅಂಶ = &pTabList->a[i_ಮೊದಲ++];
    ಆದರೆ( !p_ಅಂಶ->fg.isSubquery ) ಮುಂದುವರಿ;
    ಆದರೆ( p_ಅಂಶ->fg.ಮೂಲಕ_Coroutine ) ಮುಂದುವರಿ;
    ಆದರೆ( p_ಅಂಶ->zName==0 ) ಮುಂದುವರಿ;
    ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->pSTab!=0 );
    ಪ್ರತಿಜ್ಞಾ( pThis->pSTab!=0 );
    ಆದರೆ( p_ಅಂಶ->pSTab->pSchema!=pThis->pSTab->pSchema ) ಮುಂದುವರಿ;
    ಆದರೆ( sqlite3_stricmp(p_ಅಂಶ->zName, pThis->zName)!=0 ) ಮುಂದುವರಿ;
    pS1 = p_ಅಂಶ->u4.pSubq->pSelect;
    ಆದರೆ( p_ಅಂಶ->pSTab->pSchema==0 && pSel->selId!=pS1->selId ){
      /* The query flattener left two different CTE tables with identical
      ** names in the same FROM clause. */
      ಮುಂದುವರಿ;
    }
    ಆದರೆ( pS1->selFlags & SF_ತಳ್ಳು_ಕೆಳಕ್ಕೆ ){
      /* The view was modified by some other optimization such as
      ** pushDownWhereTerms() */
      ಮುಂದುವರಿ;
    }
    ಹಿಂತಿರುಗಿ p_ಅಂಶ;
  }
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Deallocate a single AggInfo object
*/
ಸ್ಥಿರ ಶೂನ್ಯ agginfoFree(sqlite3 *db, ಶೂನ್ಯ *pArg){
  AggInfo *p = (AggInfo*)pArg;
  sqlite3DbFree(db, p->a_ಲಂಬಸಾಲು);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_ದ_View_Optimization(Parse *pParse, Select *p){
  Select *pSub, *p_ಪೂರ್ವ;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ;
  ಅಭಿವ್ಯಕ್ತಿ *p_ಎಣಿಕೆ;
  sqlite3 *db;
  Src_ಅಂಶ *pFrom;
  ಆದರೆ( (p->selFlags & SF_Aggregate)==0 ) ಹಿಂತಿರುಗಿ 0;   /* This is an aggregate */
  ಆದರೆ( p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=1 ) ಹಿಂತಿರುಗಿ 0;               /* Single result column */
  ಆದರೆ( p->pWhere ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p->pHaving ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p->p_ಗುಂಪು_By ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p->p_ಕ್ರಮ_By ) ಹಿಂತಿರುಗಿ 0;
  p_ಅಭಿವ್ಯಕ್ತಿ = p->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op!=TK_AGG_FUNCTION ) ಹಿಂತಿರುಗಿ 0;        /* Result is an aggregate */
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_UToken(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಆದರೆ( sqlite3_stricmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,"count") ) ಹಿಂತಿರುಗಿ 0;  /* Is count() */
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList!=0 ) ಹಿಂತಿರುಗಿ 0;                 /* Must be count(*) */
  ಆದರೆ( p->pSrc->nSrc!=1 ) ಹಿಂತಿರುಗಿ 0;                  /* One table in FROM  */
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ) ಹಿಂತಿರುಗಿ 0;/* Not a window function */
  pFrom = p->pSrc->a;
  ಆದರೆ( pFrom->fg.isSubquery==0 ) ಹಿಂತಿರುಗಿ 0;           /* FROM is a subquery */
  ಆದರೆ( (p->selFlags & SF_Correlated)!=0 ) ಹಿಂತಿರುಗಿ 0;  /* Not a correlated subq */
  pSub = pFrom->u4.pSubq->pSelect;
  ಆದರೆ( pSub->p_ಪೂರ್ವ==0 ) ಹಿಂತಿರುಗಿ 0;                   /* Must be a compound */
  ಆದರೆ( pSub->selFlags & SF_CopyCte ) ಹಿಂತಿರುಗಿ 0;       /* Not a CTE */
  ಮಾಡು{
    ಆದರೆ( pSub->op!=TK_ALL && pSub->p_ಪೂರ್ವ ) ಹಿಂತಿರುಗಿ 0;  /* Must be UNION ALL */
    ಆದರೆ( pSub->pWhere ) ಹಿಂತಿರುಗಿ 0;                      /* No WHERE clause */
    ಆದರೆ( pSub->p_ಎಲ್ಲೆ ) ಹಿಂತಿರುಗಿ 0;                      /* No LIMIT clause */
    ಆದರೆ( pSub->selFlags & (SF_Aggregate|SF_Distinct) ){
       testcase( pSub->selFlags & SF_Aggregate );
       testcase( pSub->selFlags & SF_Distinct );
       ಹಿಂತಿರುಗಿ 0;     /* Not an aggregate nor DISTINCT */
    }
    ಪ್ರತಿಜ್ಞಾ( pSub->pHaving==0 );  /* Due to the previous */
    pSub = pSub->p_ಪೂರ್ವ;                              /* Repeat over compound */
  }ಆಗ( pSub );

  /* If we reach this point then it is OK to perform the transformation */

  db = pParse->db;
  p_ಎಣಿಕೆ = p_ಅಭಿವ್ಯಕ್ತಿ;
  p_ಅಭಿವ್ಯಕ್ತಿ = 0;
  pSub = sqlite3SubqueryDetach(db, pFrom);
  sqlite3SrcListDelete(db, p->pSrc);
  p->pSrc = sqlite3DbMallocZero(pParse->db, SZ_SRCLIST_1);
  ಆಗ( pSub ){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಪದ;
    p_ಪೂರ್ವ = pSub->p_ಪೂರ್ವ;
    pSub->p_ಪೂರ್ವ = 0;
    pSub->p_ಮುಂದಿನ = 0;
    pSub->selFlags |= SF_Aggregate;
    pSub->selFlags &= ~(u32)SF_Compound;
    pSub->n_Select_ಅಡ್ಡಸಾಲು = 0;
    sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic, pSub->pEList);
    p_ಪದ = p_ಪೂರ್ವ ? sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಎಣಿಕೆ, 0) : p_ಎಣಿಕೆ;
    pSub->pEList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, 0, p_ಪದ);
    p_ಪದ = sqlite3PExpr(pParse, TK_SELECT, 0, 0);
    sqlite3_PExpr_ಕೂಡಿಸು_Select(pParse, p_ಪದ, pSub);
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ){
      p_ಅಭಿವ್ಯಕ್ತಿ = p_ಪದ;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3PExpr(pParse, TK_PLUS, p_ಪದ, p_ಅಭಿವ್ಯಕ್ತಿ);
    }
    ಆದರೆ( pParse->nErr ){
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ);
      p_ಅಭಿವ್ಯಕ್ತಿ = 0;
    }
    pSub = p_ಪೂರ್ವ;
  }
  p->pEList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ;
  p->selFlags &= ~(u32)SF_Aggregate;

#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x200 ){
    TREETRACE(0x200,pParse,p,("After count-of-view optimization:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ 1;
}

/*
** If any term of pSrc, or any SF_NestedFrom sub-query, is not the same
** as pSrcItem but has the same alias as p0, then return true.
** Otherwise return false.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sameSrcAlias(Src_ಅಂಶ *p0, SrcList *pSrc){
  ಪೂರ್ಣಾಂಕ i;
  ಗಾಗಿ(i=0; i<pSrc->nSrc; i++){
    Src_ಅಂಶ *p1 = &pSrc->a[i];
    ಆದರೆ( p1==p0 ) ಮುಂದುವರಿ;
    ಆದರೆ( p0->pSTab==p1->pSTab && 0==sqlite3_stricmp(p0->zAlias, p1->zAlias) ){
      ಹಿಂತಿರುಗಿ 1;
    }
    ಆದರೆ( p1->fg.isSubquery
     && (p1->u4.pSubq->pSelect->selFlags & SF_NestedFrom)!=0
     && sameSrcAlias(p0, p1->u4.pSubq->pSelect->pSrc)
    ){
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ from_Clause_ಪದ_Can_Be_Coroutine(
  Parse *pParse,          /* Parsing context */
  SrcList *pTabList,      /* FROM clause */
  ಪೂರ್ಣಾಂಕ i,                  /* Which term of the FROM clause holds the subquery */
  ಪೂರ್ಣಾಂಕ selFlags            /* Flags on the SELECT statement */
){
  Src_ಅಂಶ *p_ಅಂಶ = &pTabList->a[i];
  ಆದರೆ( p_ಅಂಶ->fg.isCte ){
    ಸ್ಥಿರಾಂಕ CteUse *pCteUse = p_ಅಂಶ->u2.pCteUse;
    ಆದರೆ( pCteUse->eM10d==M10d_Yes ) ಹಿಂತಿರುಗಿ 0;                          /* (2a) */
    ಆದರೆ( pCteUse->nUse>=2 && pCteUse->eM10d!=M10d_No ) ಹಿಂತಿರುಗಿ 0;       /* (2b) */
  }
  ಆದರೆ( pTabList->a[0].fg.jointype & JT_LTORJ ) ಹಿಂತಿರುಗಿ 0;               /* (3)  */
  ಆದರೆ( OptimizationDisabled(pParse->db, SQLITE_Coroutines) ) ಹಿಂತಿರುಗಿ 0; /* (4)  */
  ಆದರೆ( isSelfJoinView(pTabList, p_ಅಂಶ, i+1, pTabList->nSrc)!=0 ){
    ಹಿಂತಿರುಗಿ 0;                                                          /* (5) */
  }
  ಆದರೆ( i==0 ){
    ಆದರೆ( pTabList->nSrc==1 ) ಹಿಂತಿರುಗಿ 1;                             /* (1a) */
    ಆದರೆ( pTabList->a[1].fg.jointype & JT_CROSS ) ಹಿಂತಿರುಗಿ 1;         /* (1b) */
    ಆದರೆ( selFlags & SF_UpdateFrom )              ಹಿಂತಿರುಗಿ 0;         /* (1c-iii) */
    ಹಿಂತಿರುಗಿ 1;
  }
  ಆದರೆ( selFlags & SF_UpdateFrom ) ಹಿಂತಿರುಗಿ 0;                        /* (1c-iii) */
  ಆಗ( 1 /*exit-by-break*/ ){
    ಆದರೆ( p_ಅಂಶ->fg.jointype & (JT_OUTER|JT_CROSS)  ) ಹಿಂತಿರುಗಿ 0;     /* (1c-ii) */
    ಆದರೆ( i==0 ) ನಿಲ್ಲಿಸು;
    i--;
    p_ಅಂಶ--;
    ಆದರೆ( p_ಅಂಶ->fg.isSubquery ) ಹಿಂತಿರುಗಿ 0;                          /* (1c-i) */
  }
  ಹಿಂತಿರುಗಿ 1;
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
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ exists_ಗಮ್ಯ_Join(
  Parse *pParse,  /* Parsing context */
  Select *p,      /* The SELECT statement being optimized */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere    /* part of the WHERE clause currently being examined */
){
  ಆದರೆ( pParse->nErr==0
   && pWhere!=0
   && !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pWhere, EP_OuterON|EP_InnerON) 
   && ALWAYS(p->pSrc!=0)
   && p->pSrc->nSrc<BMS
   && (p->p_ಎಲ್ಲೆ==0 || p->p_ಎಲ್ಲೆ->p_ಬಲ==0)
  ){
    ಆದರೆ( pWhere->op==TK_AND ){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = pWhere->p_ಬಲ;
      exists_ಗಮ್ಯ_Join(pParse, p, pWhere->p_ಎಡ);
      exists_ಗಮ್ಯ_Join(pParse, p, p_ಬಲ);
    }
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pWhere->op==TK_EXISTS ){
      Select *pSub = pWhere->x.pSelect;
      ಅಭಿವ್ಯಕ್ತಿ *pSubWhere = pSub->pWhere;
      ಆದರೆ( pSub->pSrc->nSrc==1
       && (pSub->selFlags & SF_Aggregate)==0
       && !pSub->pSrc->a[0].fg.isSubquery
       && pSub->p_ಎಲ್ಲೆ==0
       && pSub->p_ಪೂರ್ವ==0
      ){
        /* Before combining the sub-select with the parent, renumber the 
        ** cursor used by the subselect. This is because the EXISTS expression
        ** might be a copy of another EXISTS expression from somewhere
        ** else in the tree, and in this case it is important that it use
        ** a unique cursor number.  */
        sqlite3 *db = pParse->db;
        ಪೂರ್ಣಾಂಕ *aCsrMap = sqlite3DbMallocZero(db, (pParse->nTab+2)*ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ));
        ಆದರೆ( aCsrMap==0 ) ಹಿಂತಿರುಗಿ;
        aCsrMap[0] = (pParse->nTab+1);
        renumberCursors(pParse, pSub, -1, aCsrMap);
        sqlite3DbFree(db, aCsrMap);

        memset(pWhere, 0, ಗಾತ್ರ(*pWhere));
        pWhere->op = TK_INTEGER;
        pWhere->u.i_ಮೌಲ್ಯ = 1;
        ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pWhere, EP_Int_ಮೌಲ್ಯ);
        ಪ್ರತಿಜ್ಞಾ( p->pWhere!=0 );
        pSub->pSrc->a[0].fg.fromExists = 1;
        p->pSrc = sqlite3SrcListAppendList(pParse, p->pSrc, pSub->pSrc);
        ಆದರೆ( pSubWhere ){
          p->pWhere = sqlite3PExpr(pParse, TK_AND, p->pWhere, pSubWhere);
          pSub->pWhere = 0;
        }
        pSub->pSrc = 0;
        sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, sqlite3SelectDeleteGeneric, pSub);
        recompute_Columns_ಬಳಸಿದ(p, &p->pSrc->a[p->pSrc->nSrc-1]);
#ಆದರೆ TREETRACE_ENABLED
        ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x100000 ){
          TREETRACE(0x100000,pParse,p,
                    ("After EXISTS-to-JOIN optimization:\n"));
          sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
        }
#ಆದರೆ_ಅಂತ್ಯ
      }
    }
  }
}

/*
** Type used for Walker callbacks by selectCheckOnClauses().
*/
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ CheckOnCtx CheckOnCtx;
ರಚನೆ CheckOnCtx {
  SrcList *pSrc;       /* SrcList for this context */
  ಪೂರ್ಣಾಂಕ iJoin;           /* Cursors must be left of this one, if not negative */
  ಪೂರ್ಣಾಂಕ bFuncArg;        /* True for table-function arg */
  CheckOnCtx *p_ಪೋಷಕ; /* Parent context */
};

/*
** True if the SrcList passed as the only argument contains at least
** one RIGHT or FULL JOIN. False otherwise.  
*/
#ವ್ಯಾಖ್ಯಾನಿಸು has_ಬಲ_Join(pSrc) (((pSrc)->a[0].fg.jointype & JT_LTORJ)!=0)

/*
** The xExpr callback for the search of invalid ON clause terms.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ select_Check_On_Clauses_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;

  /* Check if pExpr is root or near-root of an ON clause constraint that needs
  ** to be checked to ensure that it does not refer to tables in its FROM
  ** clause to the right of itself. i.e. it is either:
  **
  **   + an ON clause on an OUTER join, or
  **   + an ON clause on an INNER join within a FROM that features at
  **     least one RIGHT or FULL join.
  */
  ಆದರೆ( (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON))
   || (ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) && has_ಬಲ_Join(pCtx->pSrc))
  ){
    /* If CheckOnCtx.iJoin is already set, then fall through and process
    ** this expression node as normal. Or, if CheckOnCtx.iJoin is still 0,
    ** set it to the cursor number of the RHS of the join to which this
    ** ON expression was attached and then iterate through the entire 
    ** expression.  */
    ಪ್ರತಿಜ್ಞಾ( pCtx->iJoin<0 || pCtx->iJoin==p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin );
    ಆದರೆ( pCtx->iJoin<0 ){
      pCtx->iJoin = p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin;
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
      pCtx->iJoin = -1;
      ಹಿಂತಿರುಗಿ WRC_Prune;
    }
  }

  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ ){
    /* A column expression. Find the SrcList (if any) to which it refers.
    ** Then, if CheckOnCtx.iJoin indicates that this expression is part of an
    ** ON clause from that SrcList (i.e. if iJoin is non-zero), check that it
    ** does not refer to a table to the right of CheckOnCtx.iJoin. */
    ಪೂರ್ಣಾಂಕ iTab = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ;
    ಮಾಡು {
      SrcList *pSrc = pCtx->pSrc;
      ಪೂರ್ಣಾಂಕ nSrc = pSrc->nSrc;
      ಪೂರ್ಣಾಂಕ ii;
      ಗಾಗಿ(ii=0; ii<nSrc && pSrc->a[ii].iCursor!=iTab; ii++){}
      ಆದರೆ( ii<nSrc ){
        /* pSrc is the FROM clause that contains iTab */
        ಆದರೆ( pCtx->iJoin>=0 ){
          ಗಾಗಿ(ii--; ii>=0 && pSrc->a[ii].iCursor!=pCtx->iJoin; ii--){}
          ಆದರೆ( ii>=0 ){
            /* Table iJoin appears to the left of table iTab in the SrcList.
            ** Therefore the expression refers to a table to its right. */
            sqlite3_Error_ಸಂದೇಶ(pWalker->pParse, 
                "%s references tables to its right",
                (pCtx->bFuncArg ? "table-function argument" : "ON clause")
            );
            ಹಿಂತಿರುಗಿ WRC_Abort;
          }
        }
        ನಿಲ್ಲಿಸು;
      }
      pCtx = pCtx->p_ಪೋಷಕ;
    }ಆಗ( pCtx );
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** The xSelect callback for the search of invalid ON clause terms.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ selectCheckOnClausesSelect(Walker *pWalker, Select *pSelect){
  CheckOnCtx *pCtx = pWalker->u.pCheckOnCtx;
  ಆದರೆ( pSelect->pSrc==pCtx->pSrc || pSelect->pSrc->nSrc==0 ){
    ಹಿಂತಿರುಗಿ WRC_Continue;
  }ಇಲ್ಲದಿದ್ದರೆ{
    CheckOnCtx sCtx;
    memset(&sCtx, 0, ಗಾತ್ರ(sCtx));
    sCtx.pSrc = pSelect->pSrc;
    sCtx.p_ಪೋಷಕ = pCtx;
    sCtx.iJoin = -1;
    pWalker->u.pCheckOnCtx = &sCtx;
    sqlite3WalkSelect(pWalker, pSelect);
    pWalker->u.pCheckOnCtx = pCtx;
    pSelect->selFlags &= ~SF_On_ಗಮ್ಯ_Where;
    ಹಿಂತಿರುಗಿ WRC_Prune;
  }
}

/*
** Check all ON clauses in pSelect to verify that they do not reference
** columns to the right.
*/
ಶೂನ್ಯ sqlite3SelectCheckOnClauses(Parse *pParse, Select *pSelect){
  Walker w;
  CheckOnCtx sCtx;
  ಪೂರ್ಣಾಂಕ ii;
  ಪ್ರತಿಜ್ಞಾ( pSelect->selFlags & SF_On_ಗಮ್ಯ_Where );
  ಪ್ರತಿಜ್ಞಾ( pSelect->pSrc!=0 && pSelect->pSrc->nSrc>=2 );
  memset(&w, 0, ಗಾತ್ರ(w));
  w.pParse = pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = select_Check_On_Clauses_ಅಭಿವ್ಯಕ್ತಿ;
  w.xSelectCallback = selectCheckOnClausesSelect;
  w.u.pCheckOnCtx = &sCtx;
  memset(&sCtx, 0, ಗಾತ್ರ(sCtx));
  sCtx.pSrc = pSelect->pSrc;
  sCtx.iJoin = -1;
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, pSelect->pWhere);
  pSelect->selFlags &= ~SF_On_ಗಮ್ಯ_Where;

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
  ಗಾಗಿ(ii=0; ii<pSelect->pSrc->nSrc; ii++){
    Src_ಅಂಶ *p_ಅಂಶ = &pSelect->pSrc->a[ii];
    ಆದರೆ( p_ಅಂಶ->fg.isTabFunc ){
      sCtx.iJoin = -1;
      ಆದರೆ( (p_ಅಂಶ->fg.jointype & (JT_LTORJ|JT_ಬಲ)) ){
        sCtx.iJoin = p_ಅಂಶ->iCursor;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪೂರ್ಣಾಂಕ jj;
        ಗಾಗಿ(jj=ii+1; jj<pSelect->pSrc->nSrc; jj++){
          Src_ಅಂಶ *pItem2 = &pSelect->pSrc->a[jj];
          ಆದರೆ( pItem2->fg.jointype & JT_OUTER ){
            sCtx.iJoin = pItem2[-1].iCursor;
            ನಿಲ್ಲಿಸು;
          }
        }
      }
      ಆದರೆ( sCtx.iJoin>=0 ){
        sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(&w, p_ಅಂಶ->u1.pFuncArg);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sqlite3_Copy_ವಿಂಗಡಿಸು_ಕ್ರಮ(ಅಭಿವ್ಯಕ್ತಿ_List *p1, ಅಭಿವ್ಯಕ್ತಿ_List *p2){
  ಪ್ರತಿಜ್ಞಾ( p1 );
  ಆದರೆ( p2 && p1->n_ಅಭಿವ್ಯಕ್ತಿ==p2->n_ಅಭಿವ್ಯಕ್ತಿ ){
    ಪೂರ್ಣಾಂಕ ii;
    ಗಾಗಿ(ii=0; ii<p1->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
      u8 ವಿಂಗಡಿಸು_Flags;
      ವಿಂಗಡಿಸು_Flags = p2->a[ii].fg.ವಿಂಗಡಿಸು_Flags & KEYINFO_ಕ್ರಮ_DESC;
      p1->a[ii].fg.ವಿಂಗಡಿಸು_Flags = ವಿಂಗಡಿಸು_Flags;
    }
    ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ sqlite3Select(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  SelectDest *pDest      /* What to do with the query results */
){
  ಪೂರ್ಣಾಂಕ i, j;              /* Loop counters */
  WhereInfo *pWInfo;     /* Return from sqlite3WhereBegin() */
  Vdbe *v;               /* The virtual machine under construction */
  ಪೂರ್ಣಾಂಕ isAgg;             /* True for select lists like "count(*)" */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList = 0;  /* List of columns to extract. */
  SrcList *pTabList;     /* List of tables to select from */
  ಅಭಿವ್ಯಕ್ತಿ *pWhere;          /* The WHERE clause.  May be NULL */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುಂಪು_By;    /* The GROUP BY clause.  May be NULL */
  ಅಭಿವ್ಯಕ್ತಿ *pHaving;         /* The HAVING clause.  May be NULL */
  AggInfo *pAggInfo = 0; /* Aggregate information */
  ಪೂರ್ಣಾಂಕ rc = 1;            /* Value to return from this function */
  DistinctCtx sDistinct; /* Info on how to code the DISTINCT keyword */
  ವಿಂಗಡಿಸು_Ctx s_ವಿಂಗಡಿಸು;         /* Info on how to code the ORDER BY clause */
  ಪೂರ್ಣಾಂಕ i_ಕೊನೆ;              /* Address of the end of the query */
  sqlite3 *db;           /* The database connection */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_Min_Max_ಕ್ರಮ_By = 0;  /* Added ORDER BY for min/max queries */
  u8 minMaxFlag;                 /* Flag for min/max queries */

  db = pParse->db;
  ಪ್ರತಿಜ್ಞಾ( pParse==db->pParse );
  v = sqlite3GetVdbe(pParse);
  ಆದರೆ( p==0 || pParse->nErr ){
    ಹಿಂತಿರುಗಿ 1;
  }
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
  ಆದರೆ( sqlite3AuthCheck(pParse, SQLITE_SELECT, 0, 0, 0) ) ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p, ("begin processing:\n", pParse->addrExplain));
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x10000 ){
    ಆದರೆ( (sqlite3_ವೃಕ್ಷ_Trace & 0x10001)==0x10000 ){
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(0, "In sqlite3Select() at %s:%d",
                           __FILE__, __LINE__);
    }
    sqlite3ShowSelect(p);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* tag-select-0100 */
  ಪ್ರತಿಜ್ಞಾ( p->p_ಕ್ರಮ_By==0 || pDest->eDest!=SRT_DistFifo );
  ಪ್ರತಿಜ್ಞಾ( p->p_ಕ್ರಮ_By==0 || pDest->eDest!=SRT_Fifo );
  ಪ್ರತಿಜ್ಞಾ( p->p_ಕ್ರಮ_By==0 || pDest->eDest!=SRT_Dist_ಸರತಿ );
  ಪ್ರತಿಜ್ಞಾ( p->p_ಕ್ರಮ_By==0 || pDest->eDest!=SRT_ಸರತಿ );
  ಆದರೆ( IgnorableDistinct(pDest) ){
    ಪ್ರತಿಜ್ಞಾ(pDest->eDest==SRT_Exists     || pDest->eDest==SRT_Discard ||
           pDest->eDest==SRT_Dist_ಸರತಿ  || pDest->eDest==SRT_DistFifo );
    /* All of these destinations are also able to ignore the ORDER BY clause */
    ಆದರೆ( p->p_ಕ್ರಮ_By ){
#ಆದರೆ TREETRACE_ENABLED
      TREETRACE(0x800,pParse,p, ("dropping superfluous ORDER BY:\n"));
      ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x800 ){
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(0, p->p_ಕ್ರಮ_By, 0, "ORDERBY");
      }
#ಆದರೆ_ಅಂತ್ಯ   
      sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic,
                              p->p_ಕ್ರಮ_By);
      testcase( pParse->earlyCleanup );
      p->p_ಕ್ರಮ_By = 0;
    }
    p->selFlags &= ~(u32)SF_Distinct;
  }
  sqlite3SelectPrep(pParse, p, 0);
  ಆದರೆ( pParse->nErr ){
    ಹೋಗು select_ಕೊನೆ;
  }
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );
  ಪ್ರತಿಜ್ಞಾ( p->pEList!=0 );
#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x10 ){
    TREETRACE(0x10,pParse,p, ("after name resolution:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If the SF_UFSrcCheck flag is set, then this function is being called
  ** as part of populating the temp table for an UPDATE...FROM statement.
  ** In this case, it is an error if the target object (pSrc->a[0]) name
  ** or alias is duplicated within FROM clause (pSrc->a[1..n]). 
  **
  ** Postgres disallows this case too. The reason is that some other
  ** systems handle this case differently, and not all the same way,
  ** which is just confusing. To avoid this, we follow PG's lead and
  ** disallow it altogether.  */
  ಆದರೆ( p->selFlags & SF_UFSrcCheck ){
    Src_ಅಂಶ *p0 = &p->pSrc->a[0];
    ಆದರೆ( sameSrcAlias(p0, p->pSrc) ){
      sqlite3_Error_ಸಂದೇಶ(pParse,
          "target object/alias may not appear in FROM clause: %s",
          p0->zAlias ? p0->zAlias : p0->pSTab->zName
      );
      ಹೋಗು select_ಕೊನೆ;
    }

    /* Clear the SF_UFSrcCheck flag. The check has already been performed,
    ** and leaving this flag set can cause errors if a compound sub-query
    ** in p->pSrc is flattened into this query and this function called
    ** again as part of compound SELECT processing.  */
    p->selFlags &= ~(u32)SF_UFSrcCheck;
  }

  ಆದರೆ( pDest->eDest==SRT_Output ){
    sqlite3_Generate_ಸ್ತಂಭ_ಹೆಸರುಗಳು(pParse, p);
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
  ಆದರೆ( sqlite3_ಕಿಟಕಿ_Rewrite(pParse, p) ){
    ಪ್ರತಿಜ್ಞಾ( pParse->nErr );
    ಹೋಗು select_ಕೊನೆ;
  }
#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( p->pWin && (sqlite3_ವೃಕ್ಷ_Trace & 0x40)!=0 ){
    TREETRACE(0x40,pParse,p, ("after window rewrite:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */
  pTabList = p->pSrc;
  isAgg = (p->selFlags & SF_Aggregate)!=0;
  memset(&s_ವಿಂಗಡಿಸು, 0, ಗಾತ್ರ(s_ವಿಂಗಡಿಸು));
  s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By = p->p_ಕ್ರಮ_By;

  /* Try to do various optimizations (flattening subqueries, and strength
  ** reduction of join operators) in the FROM clause up into the main query
  ** tag-select-0200
  */
#ಆದರೆ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
  ಗಾಗಿ(i=0; !p->p_ಪೂರ್ವ && i<pTabList->nSrc; i++){
    Src_ಅಂಶ *p_ಅಂಶ = &pTabList->a[i];
    Select *pSub = p_ಅಂಶ->fg.isSubquery ? p_ಅಂಶ->u4.pSubq->pSelect : 0;
    ಕೋಷ್ಟಕ *pTab = p_ಅಂಶ->pSTab;

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
    ಆದರೆ( (p_ಅಂಶ->fg.jointype & (JT_ಎಡ|JT_LTORJ))!=0
     && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸೂಚಿಸುತ್ತದೆ_Non_Null_ಅಡ್ಡಸಾಲು(p->pWhere, p_ಅಂಶ->iCursor,
                                     p_ಅಂಶ->fg.jointype & JT_LTORJ)
     && OptimizationEnabled(db, SQLITE_SimplifyJoin)
    ){
      ಆದರೆ( p_ಅಂಶ->fg.jointype & JT_ಎಡ ){
        ಆದರೆ( p_ಅಂಶ->fg.jointype & JT_ಬಲ ){
          TREETRACE(0x1000,pParse,p,
                    ("FULL-JOIN simplifies to RIGHT-JOIN on term %d\n",i));
          p_ಅಂಶ->fg.jointype &= ~JT_ಎಡ;
        }ಇಲ್ಲದಿದ್ದರೆ{
          TREETRACE(0x1000,pParse,p,
                    ("LEFT-JOIN simplifies to JOIN on term %d\n",i));
          p_ಅಂಶ->fg.jointype &= ~(JT_ಎಡ|JT_OUTER);
          unset_Join_ಅಭಿವ್ಯಕ್ತಿ(p->pWhere, p_ಅಂಶ->iCursor, 0);
        }
      }
      ಆದರೆ( p_ಅಂಶ->fg.jointype & JT_LTORJ ){
        ಗಾಗಿ(j=i+1; j<pTabList->nSrc; j++){
          Src_ಅಂಶ *pI2 = &pTabList->a[j];
          ಆದರೆ( pI2->fg.jointype & JT_ಬಲ ){
            ಆದರೆ( pI2->fg.jointype & JT_ಎಡ ){
              TREETRACE(0x1000,pParse,p,
                        ("FULL-JOIN simplifies to LEFT-JOIN on term %d\n",j));
              pI2->fg.jointype &= ~JT_ಬಲ;
            }ಇಲ್ಲದಿದ್ದರೆ{
              TREETRACE(0x1000,pParse,p,
                        ("RIGHT-JOIN simplifies to JOIN on term %d\n",j));
              pI2->fg.jointype &= ~(JT_ಬಲ|JT_OUTER);
              unset_Join_ಅಭಿವ್ಯಕ್ತಿ(p->pWhere, pI2->iCursor, 1);
            }
          }
        }
        ಗಾಗಿ(j=pTabList->nSrc-1; j>=0; j--){
          pTabList->a[j].fg.jointype &= ~JT_LTORJ;
          ಆದರೆ( pTabList->a[j].fg.jointype & JT_ಬಲ ) ನಿಲ್ಲಿಸು;
        }
      }
    }

    /* No further action if this term of the FROM clause is not a subquery */
    ಆದರೆ( pSub==0 ) ಮುಂದುವರಿ;

    /* Catch mismatch in the declared columns of a view and the number of
    ** columns in the SELECT on the RHS */
    ಆದರೆ( pTab->n_ಲಂಬಸಾಲು!=pSub->pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "expected %d columns for '%s' but got %d",
                      pTab->n_ಲಂಬಸಾಲು, pTab->zName, pSub->pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
      ಹೋಗು select_ಕೊನೆ;
    }

    /* Do not attempt the usual optimizations (flattening and ORDER BY
    ** elimination) on a MATERIALIZED common table expression because
    ** a MATERIALIZED common table expression is an optimization fence.
    */
    ಆದರೆ( p_ಅಂಶ->fg.isCte && p_ಅಂಶ->u2.pCteUse->eM10d==M10d_Yes ){
      ಮುಂದುವರಿ;
    }

    /* Do not try to flatten an aggregate subquery.
    **
    ** Flattening an aggregate subquery is only possible if the outer query
    ** is not a join.  But if the outer query is not a join, then the subquery
    ** will be implemented as a co-routine and there is no advantage to
    ** flattening in that case.
    */
    ಆದರೆ( (pSub->selFlags & SF_Aggregate)!=0 ) ಮುಂದುವರಿ;
    ಪ್ರತಿಜ್ಞಾ( pSub->p_ಗುಂಪು_By==0 );

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
    ಆದರೆ( pSub->p_ಕ್ರಮ_By!=0
     && (p->p_ಕ್ರಮ_By!=0 || pTabList->nSrc>1)      /* Condition (5) */
     && pSub->p_ಎಲ್ಲೆ==0                           /* Condition (1) */
     && (pSub->selFlags & (SF_ಕ್ರಮ_By_Reqd|SF_Recursive))==0  /* (2) and (6) */
     && (p->selFlags & SF_ಕ್ರಮ_By_Reqd)==0         /* Condition (3) and (4) */
     && OptimizationEnabled(db, SQLITE_Omit_ಕ್ರಮ_By)
    ){
      TREETRACE(0x800,pParse,p,
                ("omit superfluous ORDER BY on %r FROM-clause subquery\n",i+1));
      sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete_Generic,
                              pSub->p_ಕ್ರಮ_By);
      pSub->p_ಕ್ರಮ_By = 0;
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
    ಆದರೆ( pSub->p_ಕ್ರಮ_By!=0
     && i==0
     && (p->selFlags & SF_Complex_ಫಲಿತಾಂಶ)!=0
     && (pTabList->nSrc==1
         || (pTabList->a[1].fg.jointype&(JT_OUTER|JT_CROSS))!=0)
    ){
      ಮುಂದುವರಿ;
    }

    /* tag-select-0240 */
    ಆದರೆ( flattenSubquery(pParse, p, i, isAgg) ){
      ಆದರೆ( pParse->nErr ) ಹೋಗು select_ಕೊನೆ;
      /* This subquery can be absorbed into its parent. */
      i = -1;
    }
    pTabList = p->pSrc;
    ಆದರೆ( db->mallocFailed ) ಹೋಗು select_ಕೊನೆ;
    ಆದರೆ( !IgnorableOrderby(pDest) ){
      s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By = p->p_ಕ್ರಮ_By;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_COMPOUND_SELECT
  /* Handle compound SELECT statements using the separate multiSelect()
  ** procedure.  tag-select-0300
  */
  ಆದರೆ( p->p_ಪೂರ್ವ ){
    rc = multiSelect(pParse, p, pDest);
#ಆದರೆ TREETRACE_ENABLED
    TREETRACE(0x400,pParse,p,("end compound-select processing\n"));
    ಆದರೆ( (sqlite3_ವೃಕ್ಷ_Trace & 0x400)!=0 && Explain_Query_Plan_ಪೋಷಕ(pParse)==0 ){
      sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p->p_ಮುಂದಿನ==0 ) Explain_Query_Plan_ತೆಗೆ(pParse);
    ಹಿಂತಿರುಗಿ rc;
  }
#ಆದರೆ_ಅಂತ್ಯ

  /* If there may be an "EXISTS (SELECT ...)" in the WHERE clause, attempt
  ** to change it into a join.  */
  ಆದರೆ( pParse->bHasExists && OptimizationEnabled(db,SQLITE_Exists_ಗಮ್ಯ_Join) ){
    exists_ಗಮ್ಯ_Join(pParse, p, p->pWhere);
    pTabList = p->pSrc;
  }

  /* Do the WHERE-clause constant propagation optimization if this is
  ** a join.  No need to spend time on this operation for non-join queries
  ** as the equivalent optimization will be handled by query planner in
  ** sqlite3WhereBegin().  tag-select-0330
  */
  ಆದರೆ( p->pWhere!=0
   && p->pWhere->op==TK_AND
   && OptimizationEnabled(db, SQLITE_PropagateConst)
   && propagateConstants(pParse, p)
  ){
#ಆದರೆ TREETRACE_ENABLED
    ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x2000 ){
      TREETRACE(0x2000,pParse,p,("After constant propagation:\n"));
      sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
    }
#ಆದರೆ_ಅಂತ್ಯ
  }ಇಲ್ಲದಿದ್ದರೆ{
    TREETRACE(0x2000,pParse,p,("Constant propagation not helpful\n"));
  }

  /* tag-select-0350 */
  ಆದರೆ( OptimizationEnabled(db, SQLITE_QueryFlattener|SQLITE_ಎಣಿಕೆ_ದ_View)
   && ಎಣಿಕೆ_ದ_View_Optimization(pParse, p)
  ){
    ಆದರೆ( db->mallocFailed ) ಹೋಗು select_ಕೊನೆ;
    pTabList = p->pSrc;
  }

  /* Loop over all terms in the FROM clause and do two things for each term:
  **
  **   (1) Authorize unreferenced tables
  **   (2) Generate code for all sub-queries
  **
  ** tag-select-0400
  */
  ಗಾಗಿ(i=0; i<pTabList->nSrc; i++){
    Src_ಅಂಶ *p_ಅಂಶ = &pTabList->a[i];
    Src_ಅಂಶ *p_ಪೂರ್ವ;
    SelectDest dest;
    Subquery *pSubq;
    Select *pSub;
#ಆದರೆ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSavedAuthContext;
#ಆದರೆ_ಅಂತ್ಯ

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
    ಆದರೆ( p_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ==0 && p_ಅಂಶ->zName!=0 ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;
      ಆದರೆ( p_ಅಂಶ->fg.fixedSchema ){
        ಪೂರ್ಣಾಂಕ iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, p_ಅಂಶ->u4.pSchema);
        zDb = db->aDb[iDb].zDbSName;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಂಶ->fg.isSubquery ){
        zDb = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        zDb = p_ಅಂಶ->u4.zDatabase;
      }
      sqlite3AuthCheck(pParse, SQLITE_READ, p_ಅಂಶ->zName, "", zDb);
    }

#ಆದರೆ !defined(SQLITE_OMIT_SUBQUERY) || !defined(SQLITE_OMIT_VIEW)
    /* Generate code for all sub-queries in the FROM clause
    */
    ಆದರೆ( p_ಅಂಶ->fg.isSubquery==0 ) ಮುಂದುವರಿ;
    pSubq = p_ಅಂಶ->u4.pSubq;
    ಪ್ರತಿಜ್ಞಾ( pSubq!=0 );
    pSub = pSubq->pSelect;

    /* The code for a subquery should only be generated once. */
    ಆದರೆ( pSubq->addrFillSub!=0 ) ಮುಂದುವರಿ;

    /* Increment Parse.nHeight by the height of the largest expression
    ** tree referred to by this, the parent select. The child select
    ** may contain expression trees of at most
    ** (SQLITE_MAX_EXPR_DEPTH-Parse.nHeight) height. This is a bit
    ** more conservative than necessary, but much easier than enforcing
    ** an exact limit.
    */
    pParse->n_ಎತ್ತರ += sqlite3_Select_ಅಭಿವ್ಯಕ್ತಿ_ಎತ್ತರ(p);

    /* Make copies of constant WHERE-clause terms in the outer query down
    ** inside the subquery.  This can help the subquery to run more efficiently.
    ** This is the "predicate push-down optimization".  tag-select-0420
    */
    ಆದರೆ( OptimizationEnabled(db, SQLITE_ತಳ್ಳು_ಕೆಳಕ್ಕೆ)
     && (p_ಅಂಶ->fg.isCte==0
         || (p_ಅಂಶ->u2.pCteUse->eM10d!=M10d_Yes && p_ಅಂಶ->u2.pCteUse->nUse<2))
     && ತಳ್ಳು_ಕೆಳಕ್ಕೆ_Where_Terms(pParse, pSub, p->pWhere, pTabList, i)
    ){
#ಆದರೆ TREETRACE_ENABLED
      ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("After WHERE-clause push-down into subquery %d:\n", pSub->selId));
        sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಪ್ರತಿಜ್ಞಾ( pSubq->pSelect && (pSub->selFlags & SF_ತಳ್ಳು_ಕೆಳಕ್ಕೆ)!=0 );
    }ಇಲ್ಲದಿದ್ದರೆ{
      TREETRACE(0x4000,pParse,p,("WHERE-clause push-down not possible\n"));
    }

    /* Convert unused result columns of the subquery into simple NULL
    ** expressions, to avoid unneeded searching and computation.
    ** tag-select-0440
    */
    ಆದರೆ( OptimizationEnabled(db, SQLITE_Null_ಬಳಸದ_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ)
     && disable_ಬಳಸದ_Subquery_ಫಲಿತಾಂಶ_Columns(p_ಅಂಶ)
    ){
#ಆದರೆ TREETRACE_ENABLED
      ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x4000 ){
        TREETRACE(0x4000,pParse,p,
            ("Change unused result columns to NULL for subquery %d:\n",
             pSub->selId));
        sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
    }

    zSavedAuthContext = pParse->zAuthContext;
    pParse->zAuthContext = p_ಅಂಶ->zName;

    /* Generate byte-code to implement the subquery  tag-select-0480
    */
    ಆದರೆ( from_Clause_ಪದ_Can_Be_Coroutine(pParse, pTabList, i, p->selFlags) ){
      /* Implement a co-routine that will return a single row of the result
      ** set on each invocation.  tag-select-0482
      */
      ಪೂರ್ಣಾಂಕ addr_ತುದಿ = sqlite3VdbeCurrentAddr(v)+1;
    
      pSubq->regReturn = ++pParse->nMem;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_InitCoroutine, pSubq->regReturn, 0, addr_ತುದಿ);
      VdbeComment((v, "%!S", p_ಅಂಶ));
      pSubq->addrFillSub = addr_ತುದಿ;
      sqlite3SelectDestInit(&dest, SRT_Coroutine, pSubq->regReturn);
      ExplainQueryPlan((pParse, 1, "CO-ROUTINE %!S", p_ಅಂಶ));
      sqlite3Select(pParse, pSub, &dest);
      p_ಅಂಶ->pSTab->n_ಅಡ್ಡಸಾಲು_Log_Est = pSub->n_Select_ಅಡ್ಡಸಾಲು;
      p_ಅಂಶ->fg.ಮೂಲಕ_Coroutine = 1;
      pSubq->reg_ಫಲಿತಾಂಶ = dest.iSdst;
      sqlite3_Vdbe_ಕೊನೆ_Coroutine(v, pSubq->regReturn);
      VdbeComment((v, "end %!S", p_ಅಂಶ));
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr_ತುದಿ-1);
      sqlite3_Clear_ತಾತ್ಕಾಲಿಕ_Reg_Cache(pParse);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಂಶ->fg.isCte && p_ಅಂಶ->u2.pCteUse->addrM9e>0 ){
      /* This is a CTE for which materialization code has already been
      ** generated.  Invoke the subroutine to compute the materialization,
      ** then make the pItem->iCursor be a copy of the ephemeral table that
      ** holds the result of the materialization. tag-select-0484 */
      CteUse *pCteUse = p_ಅಂಶ->u2.pCteUse;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, pCteUse->regRtn, pCteUse->addrM9e);
      ಆದರೆ( p_ಅಂಶ->iCursor!=pCteUse->iCur ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenDup, p_ಅಂಶ->iCursor, pCteUse->iCur);
        VdbeComment((v, "%!S", p_ಅಂಶ));
      }
      pSub->n_Select_ಅಡ್ಡಸಾಲು = pCteUse->n_ಅಡ್ಡಸಾಲು_Est;
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( (p_ಪೂರ್ವ = isSelfJoinView(pTabList, p_ಅಂಶ, 0, i))!=0 ){
      /* This view has already been materialized by a prior entry in
      ** this same FROM clause.  Reuse it.  tag-select-0486 */
      Subquery *p_ಪೂರ್ವ_Subq;
      ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ->fg.isSubquery );
      p_ಪೂರ್ವ_Subq = p_ಪೂರ್ವ->u4.pSubq;
      ಪ್ರತಿಜ್ಞಾ( p_ಪೂರ್ವ_Subq!=0 );
      ಆದರೆ( p_ಪೂರ್ವ_Subq->addrFillSub ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, p_ಪೂರ್ವ_Subq->regReturn,
                                       p_ಪೂರ್ವ_Subq->addrFillSub);
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenDup, p_ಅಂಶ->iCursor, p_ಪೂರ್ವ->iCursor);
      pSub->n_Select_ಅಡ್ಡಸಾಲು = p_ಪೂರ್ವ_Subq->pSelect->n_Select_ಅಡ್ಡಸಾಲು;
    }ಇಲ್ಲದಿದ್ದರೆ{
      /* Materialize the view.  If the view is not correlated, generate a
      ** subroutine to do the materialization so that subsequent uses of
      ** the same view can reuse the materialization.  tag-select-0488 */
      ಪೂರ್ಣಾಂಕ ತುದಿ_Addr;
      ಪೂರ್ಣಾಂಕ onceAddr = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
      ಪೂರ್ಣಾಂಕ addrExplain;
#ಆದರೆ_ಅಂತ್ಯ

      pSubq->regReturn = ++pParse->nMem;
      ತುದಿ_Addr = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Goto);
      pSubq->addrFillSub = ತುದಿ_Addr+1;
      p_ಅಂಶ->fg.isMaterialized = 1;
      ಆದರೆ( p_ಅಂಶ->fg.isCorrelated==0 ){
        /* If the subquery is not correlated and if we are not inside of
        ** a trigger, then we only need to compute the value of the subquery
        ** once. */
        onceAddr = sqlite3_Vdbe_ಕೂಡಿಸು_Op0(v, OP_Once); VdbeCoverage(v);
        VdbeComment((v, "materialize %!S", p_ಅಂಶ));
      }ಇಲ್ಲದಿದ್ದರೆ{
        VdbeNoopComment((v, "materialize %!S", p_ಅಂಶ));
      }
      sqlite3SelectDestInit(&dest, SRT_EphemTab, p_ಅಂಶ->iCursor);

      ExplainQueryPlan2(addrExplain, (pParse, 1, "MATERIALIZE %!S", p_ಅಂಶ));
      sqlite3Select(pParse, pSub, &dest);
      p_ಅಂಶ->pSTab->n_ಅಡ್ಡಸಾಲು_Log_Est = pSub->n_Select_ಅಡ್ಡಸಾಲು;
      ಆದರೆ( onceAddr ) sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, onceAddr);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Return, pSubq->regReturn, ತುದಿ_Addr+1);
      VdbeComment((v, "end %!S", p_ಅಂಶ));
      sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExplain, addrExplain, -1);
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, ತುದಿ_Addr);
      sqlite3_Clear_ತಾತ್ಕಾಲಿಕ_Reg_Cache(pParse);
      ಆದರೆ( p_ಅಂಶ->fg.isCte && p_ಅಂಶ->fg.isCorrelated==0 ){
        CteUse *pCteUse = p_ಅಂಶ->u2.pCteUse;
        pCteUse->addrM9e = pSubq->addrFillSub;
        pCteUse->regRtn = pSubq->regReturn;
        pCteUse->iCur = p_ಅಂಶ->iCursor;
        pCteUse->n_ಅಡ್ಡಸಾಲು_Est = pSub->n_Select_ಅಡ್ಡಸಾಲು;
      }
    }
    ಆದರೆ( db->mallocFailed ) ಹೋಗು select_ಕೊನೆ;
    pParse->n_ಎತ್ತರ -= sqlite3_Select_ಅಭಿವ್ಯಕ್ತಿ_ಎತ್ತರ(p);
    pParse->zAuthContext = zSavedAuthContext;
#ಆದರೆ_ಅಂತ್ಯ
  }

  /* Various elements of the SELECT copied into local variables for
  ** convenience */
  pEList = p->pEList;
  pWhere = p->pWhere;
  p_ಗುಂಪು_By = p->p_ಗುಂಪು_By;
  pHaving = p->pHaving;
  sDistinct.isTnct = (p->selFlags & SF_Distinct)!=0;

#ಆದರೆ TREETRACE_ENABLED
  ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x8000 ){
    TREETRACE(0x8000,pParse,p,("After all FROM-clause analysis:\n"));
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ

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
  ಆದರೆ( (p->selFlags & (SF_Distinct|SF_Aggregate))==SF_Distinct
   && sqlite3_Copy_ವಿಂಗಡಿಸು_ಕ್ರಮ(pEList, s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By)
   && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(pEList, s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By, -1)==0
   && OptimizationEnabled(db, SQLITE_ಗುಂಪು_By_ಕ್ರಮ)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
   && p->pWin==0
#ಆದರೆ_ಅಂತ್ಯ
  ){
    p->selFlags &= ~(u32)SF_Distinct;
    p_ಗುಂಪು_By = p->p_ಗುಂಪು_By = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, pEList, 0);
    ಆದರೆ( p_ಗುಂಪು_By ){
      ಗಾಗಿ(i=0; i<p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
        p_ಗುಂಪು_By->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = i+1;
      }
    }
    p->selFlags |= SF_Aggregate;
    /* Notice that even thought SF_Distinct has been cleared from p->selFlags,
    ** the sDistinct.isTnct is still set.  Hence, isTnct represents the
    ** original setting of the SF_Distinct flag, not the current setting */
    ಪ್ರತಿಜ್ಞಾ( sDistinct.isTnct );
    sDistinct.isTnct = 2;

#ಆದರೆ TREETRACE_ENABLED
    ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x20000 ){
      TREETRACE(0x20000,pParse,p,("Transform DISTINCT into GROUP BY:\n"));
      sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
    }
#ಆದರೆ_ಅಂತ್ಯ
  }

  /* If there is an ORDER BY clause, then create an ephemeral index to
  ** do the sorting.  But this sorting ephemeral index might end up
  ** being unused if the data can be extracted in pre-sorted order.
  ** If that is the case, then the OP_OpenEphemeral instruction will be
  ** changed to an OP_Noop once we figure out that the sorting index is
  ** not needed.  The sSort.addrSortIndex variable is used to facilitate
  ** that change.  tag-select-0600
  */
  ಆದರೆ( s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By ){
    ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;
    p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(
        pParse, s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By, 0, pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
    s_ವಿಂಗಡಿಸು.iECursor = pParse->nTab++;
    s_ವಿಂಗಡಿಸು.addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ =
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_OpenEphemeral,
          s_ವಿಂಗಡಿಸು.iECursor, s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ+1+pEList->n_ಅಭಿವ್ಯಕ್ತಿ, 0,
          (ಅಕ್ಷರ*)p_ಕೀಲಿ_Info, P4_KEYINFO
      );
  }ಇಲ್ಲದಿದ್ದರೆ{
    s_ವಿಂಗಡಿಸು.addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ = -1;
  }

  /* If the output is destined for a temporary table, open that table.
  ** tag-select-0630
  */
  ಆದರೆ( pDest->eDest==SRT_EphemTab ){
    sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_OpenEphemeral, pDest->iSDParm, pEList->n_ಅಭಿವ್ಯಕ್ತಿ);
    ಆದರೆ( p->selFlags & SF_NestedFrom ){
      /* Delete or NULL-out result columns that will never be used */
      ಪೂರ್ಣಾಂಕ ii;
      ಗಾಗಿ(ii=pEList->n_ಅಭಿವ್ಯಕ್ತಿ-1; ii>0 && pEList->a[ii].fg.b_ಬಳಸಿದ==0; ii--){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ);
        sqlite3DbFree(db, pEList->a[ii].zEName);
        pEList->n_ಅಭಿವ್ಯಕ್ತಿ--;
      }
      ಗಾಗಿ(ii=0; ii<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; ii++){
        ಆದರೆ( pEList->a[ii].fg.b_ಬಳಸಿದ==0 ) pEList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
      }
    }
  }

  /* Set the limiter.  tag-select-0650
  */
  i_ಕೊನೆ = sqlite3VdbeMakeLabel(pParse);
  ಆದರೆ( (p->selFlags & SF_Fixed_ಎಲ್ಲೆ)==0 ){
    p->n_Select_ಅಡ್ಡಸಾಲು = 320;  /* 4 billion rows */
  }
  ಆದರೆ( p->p_ಎಲ್ಲೆ ) compute_ಎಲ್ಲೆ_Registers(pParse, p, i_ಕೊನೆ);
  ಆದರೆ( p->i_ಎಲ್ಲೆ==0 && s_ವಿಂಗಡಿಸು.addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ>=0 ){
    sqlite3VdbeChangeOpcode(v, s_ವಿಂಗಡಿಸು.addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ, OP_SorterOpen);
    s_ವಿಂಗಡಿಸು.ವಿಂಗಡಿಸು_Flags |= SORTFLAG_UseSorter;
  }

  /* Open an ephemeral index to use for the distinct set. tag-select-0680
  */
  ಆದರೆ( p->selFlags & SF_Distinct ){
    sDistinct.tabTnct = pParse->nTab++;
    sDistinct.addrTnct = sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_OpenEphemeral,
                       sDistinct.tabTnct, 0, 0,
                       (ಅಕ್ಷರ*)sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, p->pEList,0,0),
                       P4_KEYINFO);
    sqlite3VdbeChangeP5(v, BTREE_UNORDERED);
    sDistinct.eTnctType = WHERE_DISTINCT_UNORDERED;
  }ಇಲ್ಲದಿದ್ದರೆ{
    sDistinct.eTnctType = WHERE_DISTINCT_NOOP;
  }

  ಆದರೆ( !isAgg && p_ಗುಂಪು_By==0 ){
    /* No aggregate functions and no GROUP BY clause.  tag-select-0700 */
    u16 wctrlFlags = (sDistinct.isTnct ? WHERE_WANT_DISTINCT : 0)
                   | (p->selFlags & SF_Fixed_ಎಲ್ಲೆ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಕಿಟಕಿ *pWin = p->pWin;      /* Main window object (or NULL) */
    ಆದರೆ( pWin ){
      sqlite3_ಕಿಟಕಿ_ಸಂಕೇತ_Init(pParse, p);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಪ್ರತಿಜ್ಞಾ( WHERE_USE_ಎಲ್ಲೆ==SF_Fixed_ಎಲ್ಲೆ );


    /* Begin the database scan. */
    TREETRACE(0x2,pParse,p,("WhereBegin\n"));
    pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By,
                               p->pEList, p, wctrlFlags, p->n_Select_ಅಡ್ಡಸಾಲು);
    ಆದರೆ( pWInfo==0 ) ಹೋಗು select_ಕೊನೆ;
    ಆದರೆ( sqlite3_Where_Output_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ(pWInfo) < p->n_Select_ಅಡ್ಡಸಾಲು ){
      p->n_Select_ಅಡ್ಡಸಾಲು = sqlite3_Where_Output_ಅಡ್ಡಸಾಲು_ಎಣಿಕೆ(pWInfo);
      ಆದರೆ( pDest->eDest<=SRT_Dist_ಸರತಿ && pDest->eDest>=SRT_DistFifo ){
        /* TUNING: For a UNION CTE, because UNION is implies DISTINCT,
        ** reduce the estimated output row count by 8 (LogEst 30). 
        ** Search for tag-20250414a to see other cases */
        p->n_Select_ಅಡ್ಡಸಾಲು -= 30;
      }
    }
    ಆದರೆ( sDistinct.isTnct && sqlite3WhereIsDistinct(pWInfo) ){
      sDistinct.eTnctType = sqlite3WhereIsDistinct(pWInfo);
    }
    ಆದರೆ( s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By ){
      s_ವಿಂಗಡಿಸು.nOBSat = sqlite3WhereIsOrdered(pWInfo);
      s_ವಿಂಗಡಿಸು.labelOBLopt = sqlite3_Where_ಕ್ರಮ_By_ಎಲ್ಲೆ_Opt_Label(pWInfo);
      ಆದರೆ( s_ವಿಂಗಡಿಸು.nOBSat==s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ ){
        s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By = 0;
      }
    }
    TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));

    /* If sorting index that was created by a prior OP_OpenEphemeral
    ** instruction ended up not being needed, then change the OP_OpenEphemeral
    ** into an OP_Noop.
    */
    ಆದರೆ( s_ವಿಂಗಡಿಸು.addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ>=0 && s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By==0 ){
      sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, s_ವಿಂಗಡಿಸು.addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ);
    }

    ಪ್ರತಿಜ್ಞಾ( p->pEList==pEList );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( pWin ){
      ಪೂರ್ಣಾಂಕ addrGosub = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಂಕ iCont = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಂಕ iBreak = sqlite3VdbeMakeLabel(pParse);
      ಪೂರ್ಣಾಂಕ regGosub = ++pParse->nMem;

      sqlite3_ಕಿಟಕಿ_ಸಂಕೇತ_ಹೆಜ್ಜೆ(pParse, p, pWInfo, regGosub, addrGosub);

      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Goto, 0, iBreak);
      sqlite3VdbeResolveLabel(v, addrGosub);
      VdbeNoopComment((v, "inner-loop subroutine"));
      s_ವಿಂಗಡಿಸು.labelOBLopt = 0;
      selectInnerLoop(pParse, p, -1, &s_ವಿಂಗಡಿಸು, &sDistinct, pDest, iCont, iBreak);
      sqlite3VdbeResolveLabel(v, iCont);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Return, regGosub);
      VdbeComment((v, "end inner-loop subroutine"));
      sqlite3VdbeResolveLabel(v, iBreak);
    }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */
    {
      /* Use the standard inner loop. */
      selectInnerLoop(pParse, p, -1, &s_ವಿಂಗಡಿಸು, &sDistinct, pDest,
          sqlite3WhereContinueLabel(pWInfo),
          sqlite3WhereBreakLabel(pWInfo));

      /* End the database scan loop.
      */
      TREETRACE(0x2,pParse,p,("WhereEnd\n"));
      sqlite3_Where_ಕೊನೆ(pWInfo);
    }
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* This case is for when there exist aggregate functions or a GROUP BY
    ** clause or both.  tag-select-0800 */
    NameContext sNC;    /* Name context for processing aggregate information */
    ಪೂರ್ಣಾಂಕ iAMem;          /* First Mem address for storing current GROUP BY */
    ಪೂರ್ಣಾಂಕ iBMem;          /* First Mem address for previous GROUP BY */
    ಪೂರ್ಣಾಂಕ iUseFlag;       /* Mem address holding flag indicating that at least
                        ** one row of the input to the aggregator has been
                        ** processed */
    ಪೂರ್ಣಾಂಕ iAbortFlag;     /* Mem address which causes query abort if positive */
    ಪೂರ್ಣಾಂಕ ಗುಂಪು_By_ವಿಂಗಡಿಸು;    /* Rows come from source in GROUP BY order */
    ಪೂರ್ಣಾಂಕ addr_ಕೊನೆ;        /* End of processing for this SELECT */
    ಪೂರ್ಣಾಂಕ ವಿಂಗಡಿಸು_PTab = 0;   /* Pseudotable used to decode sorting results */
    ಪೂರ್ಣಾಂಕ ವಿಂಗಡಿಸು_ಹೊರಹೋಗು = 0;    /* Output register from the sorter */
    ಪೂರ್ಣಾಂಕ ಕ್ರಮ_By_Grp = 0; /* True if the GROUP BY and ORDER BY are the same */

    /* Remove any and all aliases between the result set and the
    ** GROUP BY clause.
    */
    ಆದರೆ( p_ಗುಂಪು_By ){
      ಪೂರ್ಣಾಂಕ k;                        /* Loop counter */
      ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;  /* For looping over expression in a list */

      ಗಾಗಿ(k=p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಂಶ=p->pEList->a; k>0; k--, p_ಅಂಶ++){
        p_ಅಂಶ->u.x.iAlias = 0;
      }
      ಗಾಗಿ(k=p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಂಶ=p_ಗುಂಪು_By->a; k>0; k--, p_ಅಂಶ++){
        p_ಅಂಶ->u.x.iAlias = 0;
      }
      ಪ್ರತಿಜ್ಞಾ( 66==sqlite3LogEst(100) );
      ಆದರೆ( p->n_Select_ಅಡ್ಡಸಾಲು>66 ) p->n_Select_ಅಡ್ಡಸಾಲು = 66;

      /* If there is both a GROUP BY and an ORDER BY clause and they are
      ** identical, then it may be possible to disable the ORDER BY clause
      ** on the grounds that the GROUP BY will cause elements to come out
      ** in the correct order. It also may not - the GROUP BY might use a
      ** database index that causes rows to be grouped together as required
      ** but not actually sorted. Either way, record the fact that the
      ** ORDER BY and GROUP BY clauses are the same by setting the orderByGrp
      ** variable.  */
      ಆದರೆ( sqlite3_Copy_ವಿಂಗಡಿಸು_ಕ್ರಮ(p_ಗುಂಪು_By, s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By)
       && sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Compare(p_ಗುಂಪು_By, s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By, -1)==0
      ){
        ಕ್ರಮ_By_Grp = 1;
      }
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಪ್ರತಿಜ್ಞಾ( 0==sqlite3LogEst(1) );
      p->n_Select_ಅಡ್ಡಸಾಲು = 0;
    }

    /* Create a label to jump to when we want to abort the query */
    addr_ಕೊನೆ = sqlite3VdbeMakeLabel(pParse);

    /* Convert TK_COLUMN nodes into TK_AGG_COLUMN and make entries in
    ** sAggInfo for all TK_AGG_FUNCTION nodes in expressions of the
    ** SELECT statement.
    */
    pAggInfo = sqlite3DbMallocZero(db, ಗಾತ್ರ(*pAggInfo) );
    ಆದರೆ( pAggInfo ){
      sqlite3_Parser_ಕೂಡಿಸು_Cleanup(pParse, agginfoFree, pAggInfo);
      testcase( pParse->earlyCleanup );
    }
    ಆದರೆ( db->mallocFailed ){
      ಹೋಗು select_ಕೊನೆ;
    }
    pAggInfo->selId = p->selId;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
    pAggInfo->pSelect = p;
#ಆದರೆ_ಅಂತ್ಯ
    memset(&sNC, 0, ಗಾತ್ರ(sNC));
    sNC.pParse = pParse;
    sNC.pSrcList = pTabList;
    sNC.uNC.pAggInfo = pAggInfo;
    VVA_ONLY( sNC.ncFlags = NC_UAggInfo; )
    pAggInfo->n_ವಿಂಗಡಣೆ_ಸ್ತಂಭ = p_ಗುಂಪು_By ? p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
    pAggInfo->p_ಗುಂಪು_By = p_ಗುಂಪು_By;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(&sNC, pEList);
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Agg_List(&sNC, s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By);
    ಆದರೆ( pHaving ){
      ಆದರೆ( p_ಗುಂಪು_By ){
        ಪ್ರತಿಜ್ಞಾ( pWhere==p->pWhere );
        ಪ್ರತಿಜ್ಞಾ( pHaving==p->pHaving );
        ಪ್ರತಿಜ್ಞಾ( p_ಗುಂಪು_By==p->p_ಗುಂಪು_By );
        having_ಗಮ್ಯ_Where(pParse, p);
        pWhere = p->pWhere;
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Analyze_Aggregates(&sNC, pHaving);
    }
    pAggInfo->n_ಸಂಚಯಕ = pAggInfo->n_ಸ್ತಂಭ;
    ಆದರೆ( p->p_ಗುಂಪು_By==0 && p->pHaving==0 && pAggInfo->nFunc==1 ){
      minMaxFlag = minMaxQuery(db, pAggInfo->aFunc[0].pFExpr, &p_Min_Max_ಕ್ರಮ_By);
    }ಇಲ್ಲದಿದ್ದರೆ{
      minMaxFlag = WHERE_ORDERBY_ಸಾಮಾನ್ಯ;
    }
    analyze_Agg_Func_ವಾದಗಳು(pAggInfo, &sNC);
    ಆದರೆ( db->mallocFailed ) ಹೋಗು select_ಕೊನೆ;
#ಆದರೆ TREETRACE_ENABLED
    ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("After aggregate analysis %p:\n", pAggInfo));
      sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
      ಆದರೆ( minMaxFlag ){
        sqlite3DebugPrintf("MIN/MAX Optimization (0x%02x) adds:\n", minMaxFlag);
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(0, p_Min_Max_ಕ್ರಮ_By, 0, "ORDERBY");
      }
      printAggInfo(pAggInfo);
    }
#ಆದರೆ_ಅಂತ್ಯ


    /* Processing for aggregates with GROUP BY is very different and
    ** much more complex than aggregates without a GROUP BY.  tag-select-0810
    */
    ಆದರೆ( p_ಗುಂಪು_By ){
      ಕೀಲಿ_Info *p_ಕೀಲಿ_Info;  /* Keying information for the group by clause */
      ಪೂರ್ಣಾಂಕ addr1;          /* A-vs-B comparison jump */
      ಪೂರ್ಣಾಂಕ addr_Output_ಅಡ್ಡಸಾಲು;  /* Start of subroutine that outputs a result row */
      ಪೂರ್ಣಾಂಕ reg_Output_ಅಡ್ಡಸಾಲು;   /* Return address register for output subroutine */
      ಪೂರ್ಣಾಂಕ addr_ಗಣ_Abort;   /* Set the abort flag and return */
      ಪೂರ್ಣಾಂಕ addr_ತುದಿ_ದ_Loop;  /* Top of the input loop */
      ಪೂರ್ಣಾಂಕ addr_ವಿಂಗಡಣೆ_Idx; /* The OP_OpenEphemeral for the sorting index */
      ಪೂರ್ಣಾಂಕ addrReset;      /* Subroutine for resetting the accumulator */
      ಪೂರ್ಣಾಂಕ regReset;       /* Return address register for reset subroutine */
      ಅಭಿವ್ಯಕ್ತಿ_List *pDistinct = 0;
      u16 distFlag = 0;
      ಪೂರ್ಣಾಂಕ eDist = WHERE_DISTINCT_NOOP;

      ಆದರೆ( pAggInfo->nFunc==1
       && pAggInfo->aFunc[0].iDistinct>=0
       && ALWAYS(pAggInfo->aFunc[0].pFExpr!=0)
       && ALWAYS(ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pAggInfo->aFunc[0].pFExpr))
       && pAggInfo->aFunc[0].pFExpr->x.pList!=0
      ){
        ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pAggInfo->aFunc[0].pFExpr->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
        p_ಅಭಿವ್ಯಕ್ತಿ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
        pDistinct = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Dup(db, p_ಗುಂಪು_By, 0);
        pDistinct = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, pDistinct, p_ಅಭಿವ್ಯಕ್ತಿ);
        distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
      }

      /* If there is a GROUP BY clause we might need a sorting index to
      ** implement it.  Allocate that sorting index now.  If it turns out
      ** that we do not need it after all, the OP_SorterOpen instruction
      ** will be converted into a Noop. 
      */
      pAggInfo->ವಿಂಗಡಣೆ_Idx = pParse->nTab++;
      p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_From_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, p_ಗುಂಪು_By,
                                            0, pAggInfo->n_ಸ್ತಂಭ);
      addr_ವಿಂಗಡಣೆ_Idx = sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_SorterOpen,
          pAggInfo->ವಿಂಗಡಣೆ_Idx, pAggInfo->n_ವಿಂಗಡಣೆ_ಸ್ತಂಭ,
          0, (ಅಕ್ಷರ*)p_ಕೀಲಿ_Info, P4_KEYINFO);

      /* Initialize memory locations used by GROUP BY aggregate processing
      */
      iUseFlag = ++pParse->nMem;
      iAbortFlag = ++pParse->nMem;
      reg_Output_ಅಡ್ಡಸಾಲು = ++pParse->nMem;
      addr_Output_ಅಡ್ಡಸಾಲು = sqlite3VdbeMakeLabel(pParse);
      regReset = ++pParse->nMem;
      addrReset = sqlite3VdbeMakeLabel(pParse);
      iAMem = pParse->nMem + 1;
      pParse->nMem += p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ;
      iBMem = pParse->nMem + 1;
      pParse->nMem += p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ;
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, iAbortFlag);
      VdbeComment((v, "clear abort flag"));
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Null, 0, iAMem, iAMem+p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ-1);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Null_ರಿಜಿಸ್ಟರ್_Range(pParse, iAMem, p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ);

      /* Begin a loop that will extract all source rows in GROUP BY order.
      ** This might involve two separate loops with an OP_Sort in between, or
      ** it might be a single loop that uses an index to extract information
      ** in the right order to begin with.
      */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, regReset, addrReset);
      TREETRACE(0x2,pParse,p,("WhereBegin\n"));
      pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_ಗುಂಪು_By, pDistinct,
          p, (sDistinct.isTnct==2 ? WHERE_DISTINCTBY : WHERE_GROUPBY)
          |  (ಕ್ರಮ_By_Grp ? WHERE_SORTBYGROUP : 0) | distFlag, 0
      );
      ಆದರೆ( pWInfo==0 ){
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pDistinct);
        ಹೋಗು select_ಕೊನೆ;
      }
      ಆದರೆ( pParse->pIdxEpr ){
        optimize_Aggregate_Use_ದ_ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ(pParse, p, pAggInfo, &sNC);
      }
      ನಿಯೋಜಿಸು_Aggregate_Registers(pParse, pAggInfo);
      eDist = sqlite3WhereIsDistinct(pWInfo);
      TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
      ಆದರೆ( sqlite3WhereIsOrdered(pWInfo)==p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ ){
        /* The optimizer is able to deliver rows in group by order so
        ** we do not have to sort.  The OP_OpenEphemeral table will be
        ** cancelled later because we still need to use the pKeyInfo
        */
        ಗುಂಪು_By_ವಿಂಗಡಿಸು = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* Rows are coming out in undetermined order.  We have to push
        ** each row into a sorting index, terminate the first loop,
        ** then loop over the sorting index in order to get the output
        ** in sorted order
        */
        ಪೂರ್ಣಾಂಕ reg_ತಳ;
        ಪೂರ್ಣಾಂಕ reg_ದಾಖಲೆ;
        ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು;
        ಪೂರ್ಣಾಂಕ n_ಗುಂಪು_By;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_STMT_SCANSTATUS
        ಪೂರ್ಣಾಂಕ addrExp;              /* Address of OP_Explain instruction */
#ಆದರೆ_ಅಂತ್ಯ
        ExplainQueryPlan2(addrExp, (pParse, 0, "USE TEMP B-TREE FOR %s",
            (sDistinct.isTnct && (p->selFlags&SF_Distinct)==0) ?
                    "DISTINCT" : "GROUP BY"
        ));

        ಗುಂಪು_By_ವಿಂಗಡಿಸು = 1;
        n_ಗುಂಪು_By = p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ;
        n_ಲಂಬಸಾಲು = n_ಗುಂಪು_By;
        j = n_ಗುಂಪು_By;
        ಗಾಗಿ(i=0; i<pAggInfo->n_ಸ್ತಂಭ; i++){
          ಆದರೆ( pAggInfo->a_ಲಂಬಸಾಲು[i].i_Sorter_ಸ್ತಂಭ>=j ){
            n_ಲಂಬಸಾಲು++;
            j++;
          }
        }
        reg_ತಳ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Range(pParse, n_ಲಂಬಸಾಲು);
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, p_ಗುಂಪು_By, reg_ತಳ, 0, 0);
        j = n_ಗುಂಪು_By;
        pAggInfo->ನೇರ_ಬಹುಲಕ = 1;
        ಗಾಗಿ(i=0; i<pAggInfo->n_ಸ್ತಂಭ; i++){
          ರಚನೆ Agg_Info_ಲಂಬಸಾಲು *p_ಲಂಬಸಾಲು = &pAggInfo->a_ಲಂಬಸಾಲು[i];
          ಆದರೆ( p_ಲಂಬಸಾಲು->i_Sorter_ಸ್ತಂಭ>=j ){
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಲಂಬಸಾಲು->pCExpr, j + reg_ತಳ);
            j++;
          }
        }
        pAggInfo->ನೇರ_ಬಹುಲಕ = 0;
        reg_ದಾಖಲೆ = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
        sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Counters(v, addrExp, 0, sqlite3VdbeCurrentAddr(v));
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Make_ದಾಖಲೆ, reg_ತಳ, n_ಲಂಬಸಾಲು, reg_ದಾಖಲೆ);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Sorter_ಸೇರಿಸು, pAggInfo->ವಿಂಗಡಣೆ_Idx, reg_ದಾಖಲೆ);
        sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExp, sqlite3VdbeCurrentAddr(v)-2, -1);
        sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Reg(pParse, reg_ದಾಖಲೆ);
        sqlite3_Release_ತಾತ್ಕಾಲಿಕ_Range(pParse, reg_ತಳ, n_ಲಂಬಸಾಲು);
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_ಕೊನೆ(pWInfo);
        pAggInfo->ವಿಂಗಡಣೆ_Idx_PTab = ವಿಂಗಡಿಸು_PTab = pParse->nTab++;
        ವಿಂಗಡಿಸು_ಹೊರಹೋಗು = sqlite3_Get_ತಾತ್ಕಾಲಿಕ_Reg(pParse);
        sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Counters(v, addrExp, sqlite3VdbeCurrentAddr(v), 0);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_OpenPseudo, ವಿಂಗಡಿಸು_PTab, ವಿಂಗಡಿಸು_ಹೊರಹೋಗು, n_ಲಂಬಸಾಲು);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Sorter_ವಿಂಗಡಿಸು, pAggInfo->ವಿಂಗಡಣೆ_Idx, addr_ಕೊನೆ);
        VdbeComment((v, "GROUP BY sort")); VdbeCoverage(v);
        pAggInfo->use_ವಿಂಗಡಣೆ_Idx = 1;
        sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExp, -1, ವಿಂಗಡಿಸು_PTab);
        sqlite3_Vdbe_ಪರಿಶೀಲನೆ_ಸ್ಥಿತಿ_Range(v, addrExp, -1, pAggInfo->ವಿಂಗಡಣೆ_Idx);
      }

      /* If there are entries in pAgggInfo->aFunc[] that contain subexpressions
      ** that are indexed (and that were previously identified and tagged
      ** in optimizeAggregateUseOfIndexedExpr()) then those subexpressions
      ** must now be converted into a TK_AGG_COLUMN node so that the value
      ** is correctly pulled from the index rather than being recomputed. */
      ಆದರೆ( pParse->pIdxEpr ){
        aggregate_Convert_ಸೂಚ್ಯಂಕಿತ_ಅಭಿವ್ಯಕ್ತಿ_Ref_ಗಮ್ಯ_ಸ್ತಂಭ(pAggInfo);
#ಆದರೆ TREETRACE_ENABLED
        ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x20 ){
          TREETRACE(0x20, pParse, p,
             ("AggInfo function expressions converted to reference index\n"));
          sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
          printAggInfo(pAggInfo);
        }
#ಆದರೆ_ಅಂತ್ಯ
      }

      /* If the index or temporary table used by the GROUP BY sort
      ** will naturally deliver rows in the order required by the ORDER BY
      ** clause, cancel the ephemeral table open coded earlier.
      **
      ** This is an optimization - the correct answer should result regardless.
      ** Use the SQLITE_GroupByOrder flag with SQLITE_TESTCTRL_OPTIMIZER to
      ** disable this optimization for testing purposes.  */
      ಆದರೆ( ಕ್ರಮ_By_Grp && OptimizationEnabled(db, SQLITE_ಗುಂಪು_By_ಕ್ರಮ)
       && (ಗುಂಪು_By_ವಿಂಗಡಿಸು || sqlite3WhereIsSorted(pWInfo))
      ){
        s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By = 0;
        sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, s_ವಿಂಗಡಿಸು.addr_ವಿಂಗಡಿಸು_ಸೂಚ್ಯಂಕ);
      }

      /* Evaluate the current GROUP BY terms and store in b0, b1, b2...
      ** (b0 is memory location iBMem+0, b1 is iBMem+1, and so forth)
      ** Then compare the current GROUP BY terms against the GROUP BY terms
      ** from the previous row currently stored in a0, a1, a2...
      */
      addr_ತುದಿ_ದ_Loop = sqlite3VdbeCurrentAddr(v);
      ಆದರೆ( ಗುಂಪು_By_ವಿಂಗಡಿಸು ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Sorter_ದತ್ತಾಂಶ, pAggInfo->ವಿಂಗಡಣೆ_Idx,
                          ವಿಂಗಡಿಸು_ಹೊರಹೋಗು, ವಿಂಗಡಿಸು_PTab);
      }
      ಗಾಗಿ(j=0; j<p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
        ಪೂರ್ಣಾಂಕ i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = p_ಗುಂಪು_By->a[j].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು;
        
        ಆದರೆ( ಗುಂಪು_By_ವಿಂಗಡಿಸು ){
          sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_ಸ್ತಂಭ, ವಿಂಗಡಿಸು_PTab, j, iBMem+j);
        }ಇಲ್ಲದಿದ್ದರೆ{
          pAggInfo->ನೇರ_ಬಹುಲಕ = 1;
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ(pParse, p_ಗುಂಪು_By->a[j].p_ಅಭಿವ್ಯಕ್ತಿ, iBMem+j);
        }

        ಆದರೆ( i_ಕ್ರಮ_By_ಲಂಬಸಾಲು ){
          ಅಭಿವ್ಯಕ್ತಿ *pX = p->pEList->a[i_ಕ್ರಮ_By_ಲಂಬಸಾಲು-1].p_ಅಭಿವ್ಯಕ್ತಿ;
          ಅಭಿವ್ಯಕ್ತಿ *p_ತಳ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pX);
          ಆಗ( ALWAYS(p_ತಳ!=0) && p_ತಳ->op==TK_IF_NULL_ಅಡ್ಡಸಾಲು ){
            pX = p_ತಳ->p_ಎಡ;
            p_ತಳ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pX);
          }
          ಆದರೆ( ALWAYS(p_ತಳ!=0)
           && p_ತಳ->op!=TK_AGG_ಸ್ತಂಭ
           && p_ತಳ->op!=TK_ರಿಜಿಸ್ಟರ್
          ){
            sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಗಮ್ಯ_ರಿಜಿಸ್ಟರ್(pX, iAMem+j);
          }
        }
      }
      sqlite3_Vdbe_ಕೂಡಿಸು_Op4(v, OP_Compare, iAMem, iBMem, p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ,
                          (ಅಕ್ಷರ*)sqlite3_ಕೀಲಿ_Info_Ref(p_ಕೀಲಿ_Info), P4_KEYINFO);
      addr1 = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op3(v, OP_Jump, addr1+1, 0, addr1+1); VdbeCoverage(v);

      /* Generate code that runs whenever the GROUP BY changes.
      ** Changes in the GROUP BY are detected by the previous code
      ** block.  If there were no changes, this block is skipped.
      **
      ** This code copies current group by terms in b0,b1,b2,...
      ** over to a0,a1,a2.  It then calls the output subroutine
      ** and resets the aggregate accumulator registers in preparation
      ** for the next GROUP BY batch.
      */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, reg_Output_ಅಡ್ಡಸಾಲು, addr_Output_ಅಡ್ಡಸಾಲು);
      VdbeComment((v, "output one row of %d", p->selId));
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸಂಕೇತ_Move(pParse, iBMem, iAMem, p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IfPos, iAbortFlag, addr_ಕೊನೆ); VdbeCoverage(v);
      VdbeComment((v, "check abort flag"));
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, regReset, addrReset);
      VdbeComment((v, "reset accumulator %d", p->selId));

      /* Update the aggregate accumulators based on the content of
      ** the current row
      */
      sqlite3_Vdbe_Jump_ಇಲ್ಲಿ(v, addr1);
      update_ಸಂಚಯಕ(pParse, iUseFlag, pAggInfo, eDist);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, iUseFlag);
      VdbeComment((v, "indicate data in accumulator %d", p->selId));

      /* End of the loop
      */
      ಆದರೆ( ಗುಂಪು_By_ವಿಂಗಡಿಸು ){
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Sorter_ಮುಂದಿನ, pAggInfo->ವಿಂಗಡಣೆ_Idx,addr_ತುದಿ_ದ_Loop);
        VdbeCoverage(v);
      }ಇಲ್ಲದಿದ್ದರೆ{
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_ಕೊನೆ(pWInfo);
        sqlite3_Vdbe_Change_ಗಮ್ಯ_Noop(v, addr_ವಿಂಗಡಣೆ_Idx);
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pDistinct);

      /* Output the final row of result
      */
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Gosub, reg_Output_ಅಡ್ಡಸಾಲು, addr_Output_ಅಡ್ಡಸಾಲು);
      VdbeComment((v, "output final row of %d", p->selId));

      /* Jump over the subroutines
      */
      sqlite3VdbeGoto(v, addr_ಕೊನೆ);

      /* Generate a subroutine that outputs a single row of the result
      ** set.  This subroutine first looks at the iUseFlag.  If iUseFlag
      ** is less than or equal to zero, the subroutine is a no-op.  If
      ** the processing calls for the query to abort, this subroutine
      ** increments the iAbortFlag memory location before returning in
      ** order to signal the caller to abort.
      */
      addr_ಗಣ_Abort = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, iAbortFlag);
      VdbeComment((v, "set abort flag"));
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Return, reg_Output_ಅಡ್ಡಸಾಲು);
      sqlite3VdbeResolveLabel(v, addr_Output_ಅಡ್ಡಸಾಲು);
      addr_Output_ಅಡ್ಡಸಾಲು = sqlite3VdbeCurrentAddr(v);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_IfPos, iUseFlag, addr_Output_ಅಡ್ಡಸಾಲು+2);
      VdbeCoverage(v);
      VdbeComment((v, "Groupby result generator entry point %d", p->selId));
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Return, reg_Output_ಅಡ್ಡಸಾಲು);
      finalizeAggFunctions(pParse, pAggInfo);
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pHaving, addr_Output_ಅಡ್ಡಸಾಲು+1, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, &s_ವಿಂಗಡಿಸು,
                      &sDistinct, pDest,
                      addr_Output_ಅಡ್ಡಸಾಲು+1, addr_ಗಣ_Abort);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Return, reg_Output_ಅಡ್ಡಸಾಲು);
      VdbeComment((v, "end groupby result generator %d", p->selId));

      /* Generate a subroutine that will reset the group-by accumulator
      */
      sqlite3VdbeResolveLabel(v, addrReset);
      reset_ಸಂಚಯಕ(pParse, pAggInfo);
      sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, iUseFlag);
      VdbeComment((v, "indicate accumulator %d empty", p->selId));
      sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Return, regReset);

      ಆದರೆ( distFlag!=0 && eDist!=WHERE_DISTINCT_NOOP ){
        ರಚನೆ AggInfo_func *pF = &pAggInfo->aFunc[0];
        fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
      }
    } /* endif pGroupBy.  Begin aggregate queries without GROUP BY: */
    ಇಲ್ಲದಿದ್ದರೆ {
      /* Aggregate functions without GROUP BY. tag-select-0820 */
      ಕೋಷ್ಟಕ *pTab;
      ಆದರೆ( (pTab = is_Simple_ಎಣಿಕೆ(p, pAggInfo))!=0 ){
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
        ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iDb = sqlite3_Schema_ಗಮ್ಯ_ಸೂಚ್ಯಂಕ(pParse->db, pTab->pSchema);
        ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ iCsr = pParse->nTab++;     /* Cursor to scan b-tree */
        ಸೂಚ್ಯಂಕ *pIdx;                         /* Iterator variable */
        ಕೀಲಿ_Info *p_ಕೀಲಿ_Info = 0;               /* Keyinfo for scanned index */
        ಸೂಚ್ಯಂಕ *p_ಉತ್ತಮ = 0;                    /* Best index found so far */
        Pgno i_ಬೇರು = pTab->tnum;             /* Root page of scanned b-tree */

        sqlite3_ಸಂಕೇತ_Verify_Schema(pParse, iDb);
        sqlite3_ಕೋಷ್ಟಕ_ಬೀಗ(pParse, iDb, pTab->tnum, 0, pTab->zName);

        /* Search for the index that has the lowest scan cost.
        **
        ** (2011-04-15) Do not do a full scan of an unordered index.
        **
        ** (2013-10-03) Do not count the entries in a partial index.
        **
        ** In practice the KeyInfo structure will not be used. It is only
        ** passed to keep OP_OpenRead happy.
        */
        ಆದರೆ( !HasRowid(pTab) ) p_ಉತ್ತಮ = sqlite3_Primary_ಕೀಲಿ_ಸೂಚ್ಯಂಕ(pTab);
        ಆದರೆ( !p->pSrc->a[0].fg.not_ಸೂಚ್ಯಂಕಿತ ){
          ಗಾಗಿ(pIdx=pTab->p_ಸೂಚ್ಯಂಕ; pIdx; pIdx=pIdx->p_ಮುಂದಿನ){
            ಆದರೆ( pIdx->bUnordered==0
             && pIdx->sz_Idx_ಅಡ್ಡಸಾಲು<pTab->sz_Tab_ಅಡ್ಡಸಾಲು
             && pIdx->pPartIdxWhere==0
             && (!p_ಉತ್ತಮ || pIdx->sz_Idx_ಅಡ್ಡಸಾಲು<p_ಉತ್ತಮ->sz_Idx_ಅಡ್ಡಸಾಲು)
            ){
              p_ಉತ್ತಮ = pIdx;
            }
          }
        }
        ಆದರೆ( p_ಉತ್ತಮ ){
          i_ಬೇರು = p_ಉತ್ತಮ->tnum;
          p_ಕೀಲಿ_Info = sqlite3_ಕೀಲಿ_Info_ದ_ಸೂಚ್ಯಂಕ(pParse, p_ಉತ್ತಮ);
        }

        /* Open a read-only cursor, execute the OP_Count, close the cursor. */
        sqlite3_Vdbe_ಕೂಡಿಸು_Op4_Int(v, OP_OpenRead, iCsr, (ಪೂರ್ಣಾಂಕ)i_ಬೇರು, iDb, 1);
        ಆದರೆ( p_ಕೀಲಿ_Info ){
          sqlite3VdbeChangeP4(v, -1, (ಅಕ್ಷರ *)p_ಕೀಲಿ_Info, P4_KEYINFO);
        }
        ನಿಯೋಜಿಸು_Aggregate_Registers(pParse, pAggInfo);
        sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_ಎಣಿಕೆ, iCsr, AggInfoFuncReg(pAggInfo,0));
        sqlite3_Vdbe_ಕೂಡಿಸು_Op1(v, OP_Close, iCsr);
        explain_Simple_ಎಣಿಕೆ(pParse, pTab, p_ಉತ್ತಮ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        /* The general case of an aggregate query without GROUP BY
        ** tag-select-0822 */
        ಪೂರ್ಣಾಂಕ regAcc = 0;           /* "populate accumulators" flag */
        ಅಭಿವ್ಯಕ್ತಿ_List *pDistinct = 0;
        u16 distFlag = 0;
        ಪೂರ್ಣಾಂಕ eDist;

        /* If there are accumulator registers but no min() or max() functions
        ** without FILTER clauses, allocate register regAcc. Register regAcc
        ** will contain 0 the first time the inner loop runs, and 1 thereafter.
        ** The code generated by updateAccumulator() uses this to ensure
        ** that the accumulator registers are (a) updated only once if
        ** there are no min() or max functions or (b) always updated for the
        ** first row visited by the aggregate, so that they are updated at
        ** least once even if the FILTER clause means the min() or max()
        ** function visits zero rows.  */
        ಆದರೆ( pAggInfo->n_ಸಂಚಯಕ ){
          ಗಾಗಿ(i=0; i<pAggInfo->nFunc; i++){
            ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pAggInfo->aFunc[i].pFExpr, EP_WinFunc) ){
              ಮುಂದುವರಿ;
            }
            ಆದರೆ( pAggInfo->aFunc[i].pFunc->funcFlags&SQLITE_FUNC_NEEDCOLL ){
              ನಿಲ್ಲಿಸು;
            }
          }
          ಆದರೆ( i==pAggInfo->nFunc ){
            regAcc = ++pParse->nMem;
            sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 0, regAcc);
          }
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pAggInfo->nFunc==1 && pAggInfo->aFunc[0].iDistinct>=0 ){
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pAggInfo->aFunc[0].pFExpr) );
          pDistinct = pAggInfo->aFunc[0].pFExpr->x.pList;
          distFlag = pDistinct ? (WHERE_WANT_DISTINCT|WHERE_AGG_DISTINCT) : 0;
        }
        ನಿಯೋಜಿಸು_Aggregate_Registers(pParse, pAggInfo);

        /* This case runs if the aggregate has no GROUP BY clause.  The
        ** processing is much simpler since there is only a single row
        ** of output.
        */
        ಪ್ರತಿಜ್ಞಾ( p->p_ಗುಂಪು_By==0 );
        reset_ಸಂಚಯಕ(pParse, pAggInfo);

        /* If this query is a candidate for the min/max optimization, then
        ** minMaxFlag will have been previously set to either
        ** WHERE_ORDERBY_MIN or WHERE_ORDERBY_MAX and pMinMaxOrderBy will
        ** be an appropriate ORDER BY expression for the optimization.
        */
        ಪ್ರತಿಜ್ಞಾ( minMaxFlag==WHERE_ORDERBY_ಸಾಮಾನ್ಯ || p_Min_Max_ಕ್ರಮ_By!=0 );
        ಪ್ರತಿಜ್ಞಾ( p_Min_Max_ಕ್ರಮ_By==0 || p_Min_Max_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ==1 );

        TREETRACE(0x2,pParse,p,("WhereBegin\n"));
        pWInfo = sqlite3WhereBegin(pParse, pTabList, pWhere, p_Min_Max_ಕ್ರಮ_By,
                                   pDistinct, p, minMaxFlag|distFlag, 0);
        ಆದರೆ( pWInfo==0 ){
          ಹೋಗು select_ಕೊನೆ;
        }
        TREETRACE(0x2,pParse,p,("WhereBegin returns\n"));
        eDist = sqlite3WhereIsDistinct(pWInfo);
        update_ಸಂಚಯಕ(pParse, regAcc, pAggInfo, eDist);
        ಆದರೆ( eDist!=WHERE_DISTINCT_NOOP ){
          ರಚನೆ AggInfo_func *pF = pAggInfo->aFunc;
          ಆದರೆ( pF ){
            fixDistinctOpenEph(pParse, eDist, pF->iDistinct, pF->iDistAddr);
          }
        }

        ಆದರೆ( regAcc ) sqlite3_Vdbe_ಕೂಡಿಸು_Op2(v, OP_Integer, 1, regAcc);
        ಆದರೆ( minMaxFlag ){
          sqlite3_Where_Min_Max_Opt_Early_ಹೊರಹೋಗು(v, pWInfo);
        }
        TREETRACE(0x2,pParse,p,("WhereEnd\n"));
        sqlite3_Where_ಕೊನೆ(pWInfo);
        finalizeAggFunctions(pParse, pAggInfo);
      }

      s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By = 0;
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_If_False(pParse, pHaving, addr_ಕೊನೆ, SQLITE_JUMPIFNULL);
      selectInnerLoop(pParse, p, -1, 0, 0,
                      pDest, addr_ಕೊನೆ, addr_ಕೊನೆ);
    }
    sqlite3VdbeResolveLabel(v, addr_ಕೊನೆ);

  } /* endif aggregate query */

  ಆದರೆ( sDistinct.eTnctType==WHERE_DISTINCT_UNORDERED ){
    explain_ತಾತ್ಕಾಲಿಕ_ಕೋಷ್ಟಕ(pParse, "DISTINCT");
  }

  /* If there is an ORDER BY clause, then we need to sort the results
  ** and send them to the callback one by one.  tag-select-0900
  */
  ಆದರೆ( s_ವಿಂಗಡಿಸು.p_ಕ್ರಮ_By ){
    ಪ್ರತಿಜ್ಞಾ( p->pEList==pEList );
    generate_ವಿಂಗಡಿಸು_ಬಾಲ(pParse, p, &s_ವಿಂಗಡಿಸು, pEList->n_ಅಭಿವ್ಯಕ್ತಿ, pDest);
  }

  /* Jump here to skip this query
  */
  sqlite3VdbeResolveLabel(v, i_ಕೊನೆ);

  /* The SELECT has been coded. If there is an error in the Parse structure,
  ** set the return code to 1. Otherwise 0. */
  rc = (pParse->nErr>0);

  /* Control jumps to here if an error is encountered above, or upon
  ** successful coding of the SELECT.
  */
select_ಕೊನೆ:
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || db->mallocFailed==1 );
  ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 || pParse->nErr!=0 );
  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, p_Min_Max_ಕ್ರಮ_By);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG
  /* Internal self-checks.  tag-select-1000 */
  ಆದರೆ( pAggInfo && !db->mallocFailed ){
#ಆದರೆ TREETRACE_ENABLED
    ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x20 ){
      TREETRACE(0x20,pParse,p,("Finished with AggInfo\n"));
      printAggInfo(pAggInfo);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಗಾಗಿ(i=0; i<pAggInfo->n_ಸ್ತಂಭ; i++){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pAggInfo->a_ಲಂಬಸಾಲು[i].pCExpr;
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಮುಂದುವರಿ;
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==pAggInfo );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg==i );
    }
    ಗಾಗಿ(i=0; i<pAggInfo->nFunc; i++){
      ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pAggInfo->aFunc[i].pFExpr;
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo==pAggInfo );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->iAgg==i );
    }
  }
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ TREETRACE_ENABLED
  TREETRACE(0x1,pParse,p,("end processing\n"));
  ಆದರೆ( (sqlite3_ವೃಕ್ಷ_Trace & 0x40000)!=0 && Explain_Query_Plan_ಪೋಷಕ(pParse)==0 ){
    sqlite3_ವೃಕ್ಷ_View_Select(0, p, 0);
  }
#ಆದರೆ_ಅಂತ್ಯ
  Explain_Query_Plan_ತೆಗೆ(pParse);
  ಹಿಂತಿರುಗಿ rc;
}
