/*
** 2008 August 18
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains routines used for walking the parser tree and
** resolve all identifiers by associating them with a particular
** table and column.
*/
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#define EXCLUDED_TABLE_ಸಂಖ್ಯೆ  2

/*
** Walk the expression tree pExpr and increase the aggregate function
** depth (the Expr.op2 field) by N on every TK_AGG_FUNCTION node.
** This needs to occur when copying a TK_AGG_FUNCTION node from an
** outer query into an inner subquery.
**
** incrAggFunctionDepth(pExpr,n) is the main routine.  incrAggDepth(..)
** is a helper function - a callback for the tree walker.
**
** See also the sqlite3WindowExtraAggFuncDepth() routine in window.c
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ incrAggDepth(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION ) p_ಅಭಿವ್ಯಕ್ತಿ->op2 += pWalker->u.n;
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}
ಸ್ಥಿರ ಶೂನ್ಯ incrAggFunctionDepth(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಙ್ಕ N){
  ಯದಿ( N>0 ){
    Walker w;
    memset(&w, 0, ಪರಿಮಾಣ(w));
    w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = incrAggDepth;
    w.u.n = N;
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
  }
}

/*
** Turn the pExpr expression into an alias for the iCol-th column of the
** result set in pEList.
**
** If the reference is followed by a COLLATE operator, then make sure
** the COLLATE operator is preserved.  For example:
**
**     SELECT a+b, c+d FROM t1 ORDER BY 1 COLLATE nocase;
**
** Should be transformed into:
**
**     SELECT a+b, c+d FROM t1 ORDER BY (a+b) COLLATE nocase;
**
** The nSubquery parameter specifies how many levels of subquery the
** alias is removed from the original expression.  The usual value is
** zero but it might be more if the alias is contained within a subquery
** of the original expression.  The Expr.op2 field of TK_AGG_FUNCTION
** structures must be increased by the nSubquery amount.
*/
ಸ್ಥಿರ ಶೂನ್ಯ resolveAlias(
  Parse *pParse,         /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,      /* A result set */
  ಪೂರ್ಣಾಙ್ಕ iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,           /* Transform this into an alias to the result set */
  ಪೂರ್ಣಾಙ್ಕ nSubquery          /* Number of subqueries that the label is moving */
){
  ಅಭಿವ್ಯಕ್ತಿ *pOrig;           /* The iCol-th column of the result set */
  ಅಭಿವ್ಯಕ್ತಿ *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  ಪ್ರತಿಜ್ಞಾ( iCol>=0 && iCol<pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
  pOrig = pEList->a[iCol].p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( pOrig!=0 );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced|EP_TokenOnly) );
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo ) ಪ್ರತ್ಯಾಗಮ;
  db = pParse->db;
  pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pOrig, 0);
  ಯದಿ( db->mallocFailed ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDup);
    pDup = 0;
  }ಅನ್ಯಥಾ{
    ಅಭಿವ್ಯಕ್ತಿ temp;
    incrAggFunctionDepth(pDup, nSubquery);
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Add_Collate_String(pParse, pDup, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
    }
    memcpy(&temp, pDup, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
    memcpy(pDup, p_ಅಭಿವ್ಯಕ್ತಿ, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
    memcpy(p_ಅಭಿವ್ಯಕ್ತಿ, &temp, ಪರಿಮಾಣ(ಅಭಿವ್ಯಕ್ತಿ));
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
      ಯದಿ( ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin!=0) ){
        p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin->pOwner = p_ಅಭಿವ್ಯಕ್ತಿ;
      }
    }
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Deferred_Delete(pParse, pDup);
  }
}

/*
** Subqueries store the original database, table and column names for their
** result sets in ExprList.a[].zSpan, in the form "DATABASE.TABLE.COLUMN",
** and mark the expression-list item by setting ExprList.a[].fg.eEName
** to ENAME_TAB.
**
** Check to see if the zSpan/eEName of the expression-list item passed to this
** routine matches the zDb, zTab, and zCol.  If any of zDb, zTab, and zCol are
** NULL then those fields will match anything. Return true if there is a match,
** or false otherwise.
**
** SF_NestedFrom subqueries also store an entry for the implicit rowid (or
** _rowid_, or oid) column by setting ExprList.a[].fg.eEName to ENAME_ROWID,
** and setting zSpan to "DATABASE.TABLE.<rowid-alias>". This type of pItem
** argument matches if zCol is a rowid alias. If it is not NULL, (*pbRowid)
** is set to 1 if there is this kind of match.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3MatchEName(
  ಧ್ರುವ ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem,
  ಧ್ರುವ ವರ್ಣ *zCol,
  ಧ್ರುವ ವರ್ಣ *zTab,
  ಧ್ರುವ ವರ್ಣ *zDb,
  ಪೂರ್ಣಾಙ್ಕ *pbRowid
){
  ಪೂರ್ಣಾಙ್ಕ n;
  ಧ್ರುವ ವರ್ಣ *zSpan;
  ಪೂರ್ಣಾಙ್ಕ eEName = pItem->fg.eEName;
  ಯದಿ( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  ಪ್ರತಿ(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  ಯದಿ( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  zSpan += n+1;
  ಪ್ರತಿ(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  ಯದಿ( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    ಪ್ರತ್ಯಾಗಮ 0;
  }
  zSpan += n+1;
  ಯದಿ( zCol ){
    ಯದಿ( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  }
  ಯದಿ( eEName==ENAME_ROWID ) *pbRowid = 1;
  ಪ್ರತ್ಯಾಗಮ 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *p_ತುದಿ_NC){
  ಯದಿ( db->init.busy ) ಪ್ರತ್ಯಾಗಮ 1;  /* Always support for legacy schemas */
  ಯದಿ( p_ತುದಿ_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    ಯದಿ( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪ್ರತ್ಯಾಗಮ (db->flags & SQLITE_DqsDDL)!=0;
  }ಅನ್ಯಥಾ{
    /* Currently parsing a DML statement */
    ಪ್ರತ್ಯಾಗಮ (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Col_Used(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಙ್ಕ n;
  Table *pExTab;

  n = p_ಅಭಿವ್ಯಕ್ತಿ->iColumn;
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
  pExTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
  ಪ್ರತಿಜ್ಞಾ( pExTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( n < pExTab->nCol );
  ಯದಿ( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    ಪ್ರತ್ಯಾಗಮ pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }ಅನ್ಯಥಾ{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    ಯದಿ( n>=BMS ) n = BMS-1;
    ಪ್ರತ್ಯಾಗಮ ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
ಸ್ಥಿರ ಶೂನ್ಯ extendFJMatch(
  Parse *pParse,          /* Parsing context */
  ಅಭಿವ್ಯಕ್ತಿ_List **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(pParse->db, TK_COLUMN, 0, 0);
  ಯದಿ( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    ಪ್ರತಿಜ್ಞಾ( (pMatch->fg.jointype & (JT_ಎಡ|JT_LTORJ))!=0 );
    ಅಭಿವ್ಯಕ್ತಿ_Set_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ isValidSchemaTableName(
  ಧ್ರುವ ವರ್ಣ *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  ಧ್ರುವ ವರ್ಣ *zDb           /* non-NULL if a database qualifier is present */
){
  ಧ್ರುವ ವರ್ಣ *zLegacy;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTab->tnum==1 );
  ಯದಿ( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) ಪ್ರತ್ಯಾಗಮ 0;
  zLegacy = pTab->zName;
  ಯದಿ( strcmp(zLegacy+7, &LEGACY_TEMP_SCHEMA_TABLE[7])==0 ){
    ಯದಿ( sqlite3StrICmp(zTab+7, &PREFERRED_TEMP_SCHEMA_TABLE[7])==0 ){
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಯದಿ( zDb==0 ) ಪ್ರತ್ಯಾಗಮ 0;
    ಯದಿ( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) ಪ್ರತ್ಯಾಗಮ 1;
    ಯದಿ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) ಪ್ರತ್ಯಾಗಮ 1;
  }ಅನ್ಯಥಾ{
    ಯದಿ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Given the name of a column of the form X.Y.Z or Y.Z or just Z, look up
** that name in the set of source tables in pSrcList and make the pExpr
** expression node refer back to that source column.  The following changes
** are made to pExpr:
**
**    pExpr->iDb           Set the index in db->aDb[] of the database X
**                         (even if X is implied).
**    pExpr->iTable        Set to the cursor number for the table obtained
**                         from pSrcList.
**    pExpr->y.pTab        Points to the Table structure of X.Y (even if
**                         X and/or Y are implied.)
**    pExpr->iColumn       Set to the column number within the table.
**    pExpr->op            Set to TK_COLUMN.
**    pExpr->pLeft         Any expression this points to is deleted
**    pExpr->pRight        Any expression this points to is deleted.
**
** The zDb variable is the name of the database (the "X").  This value may be
** NULL meaning that name is of the form Y.Z or Z.  Any available database
** can be used.  The zTable variable is the name of the table (the "Y").  This
** value can be NULL if zDb is also NULL.  If zTable is NULL it
** means that the form of the name is Z and that columns from any table
** can be used.
**
** If the name cannot be resolved unambiguously, leave an error message
** in pParse and return WRC_Abort.  Return WRC_Prune on success.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ lookupName(
  Parse *pParse,       /* The parsing context */
  ಧ್ರುವ ವರ್ಣ *zDb,     /* Name of the database containing table, or NULL */
  ಧ್ರುವ ವರ್ಣ *zTab,    /* Name of table containing column, or NULL */
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ          /* Make this EXPR node point to the selected column */
){
  ಪೂರ್ಣಾಙ್ಕ i, j;                         /* Loop counters */
  ಪೂರ್ಣಾಙ್ಕ cnt = 0;                      /* Number of matching column names */
  ಪೂರ್ಣಾಙ್ಕ cntTab = 0;                   /* Number of potential "rowid" matches */
  ಪೂರ್ಣಾಙ್ಕ nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *p_ತುದಿ_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  ಪೂರ್ಣಾಙ್ಕ e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  ಅಭಿವ್ಯಕ್ತಿ_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  ಧ್ರುವ ವರ್ಣ *zCol = p_ಬಲ->u.zToken;

  ಪ್ರತಿಜ್ಞಾ( pNC );     /* the name context cannot be NULL. */
  ಪ್ರತಿಜ್ಞಾ( zCol );    /* The Z in X.Y.Z cannot be NULL */
  ಪ್ರತಿಜ್ಞಾ( zDb==0 || zTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_ಅಭಿವ್ಯಕ್ತಿ->iTable = -1;
  ಅಭಿವ್ಯಕ್ತಿ_Set_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  ಯದಿ( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    ಯದಿ( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }ಅನ್ಯಥಾ{
      ಪ್ರತಿ(i=0; i<db->nDb; i++){
        ಪ್ರತಿಜ್ಞಾ( db->aDb[i].zDbSName );
        ಯದಿ( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          ಭಙ್ಗ;
        }
      }
      ಯದಿ( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  ಪ್ರತಿಜ್ಞಾ( pNC && cnt==0 );
  ಕುರು{
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    ಯದಿ( pSrcList ){
      ಪ್ರತಿ(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        ಪ್ರತಿಜ್ಞಾ( pTab!=0 && pTab->zName!=0 );
        ಪ್ರತಿಜ್ಞಾ( pTab->nCol>0 || pParse->nErr );
        ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಙ್ಕ)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        ಯದಿ( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          ಪೂರ್ಣಾಙ್ಕ hit = 0;
          Select *pSel;
          ಪ್ರತಿಜ್ಞಾ( pItem->fg.isSubquery );
          ಪ್ರತಿಜ್ಞಾ( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          ಪ್ರತಿಜ್ಞಾ( pSel!=0 );
          pEList = pSel->pEList;
          ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
          ಪ್ರತಿಜ್ಞಾ( pEList->n_ಅಭಿವ್ಯಕ್ತಿ==pTab->nCol );
          ಪ್ರತಿ(j=0; j<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
            ಪೂರ್ಣಾಙ್ಕ bRowid = 0;       /* True if possible rowid match */
            ಯದಿ( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              ಅನುವರ್ತ;
            }
            ಯದಿ( bRowid==0 ){
              ಯದಿ( cnt>0 ){
                ಯದಿ( pItem->fg.isUsing==0
                 || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ಅನ್ಯಥಾ
                ಯದಿ( (pItem->fg.jointype & JT_ಬಲ)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  ಅನುವರ್ತ;
                }ಅನ್ಯಥಾ
                ಯದಿ( (pItem->fg.jointype & JT_ಎಡ)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ಅನ್ಯಥಾ{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }ಅನ್ಯಥಾ ಯದಿ( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              ಅನುವರ್ತ;
            }
            cntTab++;
            pMatch = pItem;
            p_ಅಭಿವ್ಯಕ್ತಿ->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            ಪ್ರತಿಜ್ಞಾ( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            ಯದಿ( pEList->a[j].fg.bUsingTerm ) ಭಙ್ಗ;
          }
          ಯದಿ( hit || zTab==0 ) ಅನುವರ್ತ;
        }
        ಪ್ರತಿಜ್ಞಾ( zDb==0 || zTab!=0 );
        ಯದಿ( zTab ){
          ಯದಿ( zDb ){
            ಯದಿ( pTab->pSchema!=pSchema ) ಅನುವರ್ತ;
            ಯದಿ( pSchema==0 && strcmp(zDb,"*")!=0 ) ಅನುವರ್ತ;
          }
          ಯದಿ( pItem->zAlias!=0 ){
            ಯದಿ( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              ಅನುವರ್ತ;
            }
          }ಅನ್ಯಥಾ ಯದಿ( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            ಯದಿ( pTab->tnum!=1 ) ಅನುವರ್ತ;
            ಯದಿ( !isValidSchemaTableName(zTab, pTab, zDb) ) ಅನುವರ್ತ;
          }
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
          ಯದಿ( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        ಯದಿ( j>=0 ){
          ಯದಿ( cnt>0 ){
            ಯದಿ( pItem->fg.isUsing==0
             || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ಅನ್ಯಥಾ
            ಯದಿ( (pItem->fg.jointype & JT_ಬಲ)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              ಅನುವರ್ತ;
            }ಅನ್ಯಥಾ
            ಯದಿ( (pItem->fg.jointype & JT_ಎಡ)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ಅನ್ಯಥಾ{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_ಅಭಿವ್ಯಕ್ತಿ->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          ಯದಿ( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        ಯದಿ( 0==cnt && VisibleRowid(pTab) ){
          /* pTab is a potential ROWID match.  Keep track of it and match
          ** the ROWID later if that seems appropriate.  (Search for "cntTab"
          ** to find related code.)  Only allow a ROWID match if there is
          ** a single ROWID match candidate.
          */
#ifdef SQLITE_ALLOW_ROWID_IN_VIEW
          /* In SQLITE_ALLOW_ROWID_IN_VIEW mode, allow a ROWID match
          ** if there is a single VIEW candidate or if there is a single
          ** non-VIEW candidate plus multiple VIEW candidates.  In other
          ** words non-VIEW candidate terms take precedence over VIEWs.
          */
          ಯದಿ( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }ಅನ್ಯಥಾ{
            cntTab++;
          }
#ಅನ್ಯಥಾ
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      ಯದಿ( pMatch ){
        p_ಅಭಿವ್ಯಕ್ತಿ->iTable = pMatch->iCursor;
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
        p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab = pMatch->pSTab;
        ಯದಿ( (pMatch->fg.jointype & (JT_ಎಡ|JT_LTORJ))!=0 ){
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_CanBeNull);
        }
        pSchema = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#ಯದಿ !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    ಯದಿ( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      ಯದಿ( pParse->pTriggerTab!=0 ){
        ಪೂರ್ಣಾಙ್ಕ op = pParse->eTriggerOp;
        ಪ್ರತಿಜ್ಞಾ( op==TK_DELETE || op==TK_UPDATE || op==TK_ಸೇರಿಸು );
        ಯದಿ( pParse->bReturning ){
          ಯದಿ( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            p_ಅಭಿವ್ಯಕ್ತಿ->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }ಅನ್ಯಥಾ ಯದಿ( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_ಅಭಿವ್ಯಕ್ತಿ->iTable = 1;
          pTab = pParse->pTriggerTab;
        }ಅನ್ಯಥಾ ಯದಿ( op!=TK_ಸೇರಿಸು && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_ಅಭಿವ್ಯಕ್ತಿ->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      ಯದಿ( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        ಯದಿ( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_ಅಭಿವ್ಯಕ್ತಿ->iTable = EXCLUDED_TABLE_ಸಂಖ್ಯೆ;
          ಯದಿ( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      ಯದಿ( pTab ){
        ಪೂರ್ಣಾಙ್ಕ iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        ಯದಿ( iCol>=0 ){
          ಯದಿ( pTab->iPKey==iCol ) iCol = -1;
        }ಅನ್ಯಥಾ{
          ಯದಿ( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }ಅನ್ಯಥಾ{
            iCol = pTab->nCol;
          }
        }
        ಯದಿ( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable==EXCLUDED_TABLE_ಸಂಖ್ಯೆ ){
            testcase( iCol==(-1) );
            ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
            ಯದಿ( IN_RENAME_OBJECT ){
              p_ಅಭಿವ್ಯಕ್ತಿ->iColumn = iCol;
              p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab = pTab;
              e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_COLUMN;
            }ಅನ್ಯಥಾ{
              p_ಅಭಿವ್ಯಕ್ತಿ->iTable = pNC->uNC.pUpsert->regData +
                 sqlite3TableColumnToStorage(pTab, iCol);
              e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_REGISTER;
            }
          }ಅನ್ಯಥಾ
#endif /* SQLITE_OMIT_UPSERT */
          {
            ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
            p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab = pTab;
            ಯದಿ( pParse->bReturning ){
              e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_REGISTER;
              p_ಅಭಿವ್ಯಕ್ತಿ->op2 = TK_COLUMN;
              p_ಅಭಿವ್ಯಕ್ತಿ->iColumn = iCol;
              p_ಅಭಿವ್ಯಕ್ತಿ->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*p_ಅಭಿವ್ಯಕ್ತಿ->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }ಅನ್ಯಥಾ{
              p_ಅಭಿವ್ಯಕ್ತಿ->iColumn = (i16)iCol;
              e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              ಯದಿ( iCol<0 ){
                p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_INTEGER;
              }ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }ಅನ್ಯಥಾ{
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->newmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }
#endif /* SQLITE_OMIT_TRIGGER */
            }
          }
        }
      }
    }
#endif /* !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT) */

    /*
    ** Perhaps the name is a reference to the ROWID
    */
    ಯದಿ( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#ಯದಿ SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      ಯದಿ( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_NULL;
      }
#endif
      ಯದಿ( pMatch->fg.isNestedFrom==0 ) p_ಅಭಿವ್ಯಕ್ತಿ->iColumn = -1;
      p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_INTEGER;
    }

    /*
    ** If the input is of the form Z (not Y.Z or X.Y.Z) then the name Z
    ** might refer to an result-set alias.  This happens, for example, when
    ** we are resolving names in the WHERE clause of the following command:
    **
    **     SELECT a+b AS x FROM table WHERE x<10;
    **
    ** In cases like this, replace pExpr with a copy of the expression that
    ** forms the result set entry ("a+b" in the example) and return immediately.
    ** Note that the expression in the result set should have already been
    ** resolved by the time the WHERE clause is resolved.
    **
    ** The ability to use an output result-set column in the WHERE, GROUP BY,
    ** or HAVING clauses, or as part of a larger expression in the ORDER BY
    ** clause is not standard SQL.  This is a (goofy) SQLite extension, that
    ** is supported for backwards compatibility only. Hence, we issue a warning
    ** on sqlite3_log() whenever the capability is used.
    */
    ಯದಿ( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
      ಪ್ರತಿ(j=0; j<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
        ವರ್ಣ *zAs = pEList->a[j].zEName;
        ಯದಿ( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          ಅಭಿವ್ಯಕ್ತಿ *pOrig;
          ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0 && p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==0 );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ)==0 || p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==0 );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ)==0 || p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect==0 );
          pOrig = pEList->a[j].p_ಅಭಿವ್ಯಕ್ತಿ;
          ಯದಿ( (pNC->ncFlags&NC_AllowAgg)==0 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "misuse of aliased aggregate %s", zAs);
            ಪ್ರತ್ಯಾಗಮ WRC_Abort;
          }
          ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_ತುದಿ_NC )
          ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "misuse of aliased window function %s",zAs);
            ಪ್ರತ್ಯಾಗಮ WRC_Abort;
          }
          ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(pOrig)!=1 ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "row value misused");
            ಪ್ರತ್ಯಾಗಮ WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_ಅಭಿವ್ಯಕ್ತಿ, nSubquery);
          cnt = 1;
          pMatch = 0;
          ಪ್ರತಿಜ್ಞಾ( zTab==0 && zDb==0 );
          ಯದಿ( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
          }
          ಗಚ್ಛ lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    ಯದಿ( cnt ) ಭಙ್ಗ;
    pNC = pNC->pNext;
    nSubquery++;
  }ಯಾವತ್( pNC );


  /*
  ** If X and Y are NULL (in other words if only the column name Z is
  ** supplied) and the value of Z is enclosed in double-quotes, then
  ** Z is a string literal if it doesn't match any column names.  In that
  ** case, we need to return right away and not make any changes to
  ** pExpr.
  **
  ** Because no reference was made to outer contexts, the pNC->nRef
  ** fields are not changed in any context.
  */
  ಯದಿ( cnt==0 && zTab==0 ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID );
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, p_ತುದಿ_NC)
    ){
      /* If a double-quoted identifier does not match any known column name,
      ** then treat it as a string.
      **
      ** This hack was added in the early days of SQLite in a misguided attempt
      ** to be compatible with MySQL 3.x, which used double-quotes for strings.
      ** I now sorely regret putting in this hack. The effect of this hack is
      ** that misspelled identifier names are silently converted into strings
      ** rather than causing an error, to the frustration of countless
      ** programmers. To all those frustrated programmers, my apologies.
      **
      ** Someday, I hope to get rid of this hack. Unfortunately there is
      ** a huge amount of legacy SQL that uses it. So for now, we just
      ** issue a warning.
      */
      sqlite3_log(SQLITE_WARNING,
        "double-quoted string literal: \"%w\"", zCol);
#ifdef SQLITE_ENABLE_NORMALIZE
      sqlite3VdbeAddDblquoteStr(db, pParse->pVdbe, zCol);
#endif
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_STRING;
      memset(&p_ಅಭಿವ್ಯಕ್ತಿ->y, 0, ಪರಿಮಾಣ(p_ಅಭಿವ್ಯಕ್ತಿ->y));
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;
    }
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Id_To_True_False(p_ಅಭಿವ್ಯಕ್ತಿ) ){
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  ಪ್ರತಿಜ್ಞಾ( pFJMatch==0 || cnt>0 );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_xIsSelect|EP_Int_ಮೌಲ್ಯ) );
  ಯದಿ( cnt!=1 ){
    ಧ್ರುವ ವರ್ಣ *zErr;
    ಯದಿ( pFJMatch ){
      ಯದಿ( pFJMatch->n_ಅಭಿವ್ಯಕ್ತಿ==cnt-1 ){
        ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Leaf) ){
          ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_Leaf);
        }ಅನ್ಯಥಾ{
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
          p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = 0;
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
          p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn);
        p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_FUNCTION;
        p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken = "coalesce";
        p_ಅಭಿವ್ಯಕ್ತಿ->x.pList = pFJMatch;
        p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_DEFER;
        cnt = 1;
        ಗಚ್ಛ lookupname_end;
      }ಅನ್ಯಥಾ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    ಯದಿ( zDb ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }ಅನ್ಯಥಾ ಯದಿ( zTab ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }ಅನ್ಯಥಾ ಯದಿ( cnt==0 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ,EP_DblQuoted) ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }ಅನ್ಯಥಾ{
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3_Record_Error_Offset_Of_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
    pParse->checkSchema = 1;
    p_ತುದಿ_NC->nNcErr++;
    e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_NULL;
  }
  ಪ್ರತಿಜ್ಞಾ( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = 0;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ = 0;
    ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Leaf);
  }

  /* If a column from a table in pSrcList is referenced, then record
  ** this fact in the pSrcList.a[].colUsed bitmask.  Column 0 causes
  ** bit 0 to be set.  Column 1 sets bit 1.  And so forth.  Bit 63 is
  ** set if the 63rd or any subsequent column is used.
  **
  ** The colUsed mask is an optimization used to help determine if an
  ** index is a covering index.  The correct answer is still obtained
  ** if the mask contains extra set bits.  However, it is important to
  ** avoid setting bits beyond the maximum column number of the table.
  ** (See ticket [b92e5e8ec2cdbaa1]).
  **
  ** If a generated column is referenced, set bits for every column
  ** of the table.
  */
  ಯದಿ( pMatch ){
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iColumn>=0 ){
      pMatch->colUsed |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Col_Used(p_ಅಭಿವ್ಯಕ್ತಿ);
    }ಅನ್ಯಥಾ{
      pMatch->fg.rowidUsed = 1;
    }
  }

  p_ಅಭಿವ್ಯಕ್ತಿ->op = e_New_ಅಭಿವ್ಯಕ್ತಿ_Op;
lookupname_end:
  ಯದಿ( cnt==1 ){
    ಪ್ರತಿಜ್ಞಾ( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    ಯದಿ( db->xAuth ){
      ಯದಿ( pFJMatch ){
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
        ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,"coalesce")==0 );
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==pFJMatch );
        ಪ್ರತಿಜ್ಞಾ( pFJMatch->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
        ಪ್ರತಿ(i=0; i<pFJMatch->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
          ಪ್ರತಿಜ್ಞಾ( pFJMatch->a[i].p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_ಅಭಿವ್ಯಕ್ತಿ,pSchema,pNC->pSrcList);
        }
      }ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLUMN || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    ಪ್ರತಿ(;;){
      ಪ್ರತಿಜ್ಞಾ( p_ತುದಿ_NC!=0 );
      p_ತುದಿ_NC->nRef++;
      ಯದಿ( p_ತುದಿ_NC==pNC ) ಭಙ್ಗ;
      p_ತುದಿ_NC = p_ತುದಿ_NC->pNext;
    }
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  } ಅನ್ಯಥಾ {
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_Create_Column_ಅಭಿವ್ಯಕ್ತಿ(sqlite3 *db, SrcList *pSrc, ಪೂರ್ಣಾಙ್ಕ iSrc, ಪೂರ್ಣಾಙ್ಕ iCol){
  ಅಭಿವ್ಯಕ್ತಿ *p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_COLUMN, 0, 0);
  ಯದಿ( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    ಯದಿ( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }ಅನ್ಯಥಾ{
      p->iColumn = (ynVar)iCol;
      ಯದಿ( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }ಅನ್ಯಥಾ{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ p;
}

/*
** Report an error that an expression is not valid for some set of
** pNC->ncFlags values determined by validMask.
**
** static void notValid(
**   Parse *pParse,       // Leave error message here
**   NameContext *pNC,    // The name context
**   const char *zMsg,    // Type of error
**   int validMask,       // Set of contexts for which prohibited
**   Expr *pExpr          // Invalidate this expression on error
** ){...}
**
** As an optimization, since the conditional is almost always false
** (because errors are rare), the conditional is moved outside of the
** function call using a macro.
*/
ಸ್ಥಿರ ಶೂನ್ಯ notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   ಧ್ರುವ ವರ್ಣ *z_ಸಂದೇಶ,    /* Type of error */
   ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,         /* Invalidate this expression on error */
   ಅಭಿವ್ಯಕ್ತಿ *pError         /* Associate error with this expression */
){
  ಧ್ರುವ ವರ್ಣ *zIn = "partial index WHERE clauses";
  ಯದಿ( pNC->ncFlags & NC_Idx_ಅಭಿವ್ಯಕ್ತಿ )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  ಅನ್ಯಥಾ ಯದಿ( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  ಅನ್ಯಥಾ ಯದಿ( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3_Error_ಸಂದೇಶ(pParse, "%s prohibited in %s", z_ಸಂದೇಶ, zIn);
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ ) p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
  sqlite3_Record_Error_Offset_Of_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  ಪ್ರತಿಜ್ಞಾ( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_GenCol))==0 ); \
  ಯದಿ( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ ಅಭಿವ್ಯಕ್ತಿ_Probability(ಅಭಿವ್ಯಕ್ತಿ *p){
  ದ್ವಿಗುಣ r = -1.0;
  ಯದಿ( p->op!=TK_FLOAT ) ಪ್ರತ್ಯಾಗಮ -1;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) );
  sqlite3AtoF(p->u.zToken, &r);
  ಪ್ರತಿಜ್ಞಾ( r>=0.0 );
  ಯದಿ( r>1.0 ) ಪ್ರತ್ಯಾಗಮ -1;
  ಪ್ರತ್ಯಾಗಮ (ಪೂರ್ಣಾಙ್ಕ)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ resolve_Set_ಅಭಿವ್ಯಕ್ತಿ_Subtype_Arg(ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಪೂರ್ಣಾಙ್ಕ nn, ii;
  nn = pList ? pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
  ಪ್ರತಿ(ii=0; ii<nn; ii++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಯಾವತ್( 1 /*exit-by-break*/ ){
      ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_SubtArg);
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ){
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect!=0 );
        resolve_Set_ಅಭಿವ್ಯಕ್ತಿ_Subtype_Arg(p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList);
        ಭಙ್ಗ;
      }
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_UPLUS ){
        p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      }ಅನ್ಯಥಾ{
        ಭಙ್ಗ;
      }
    }
  }
}

/*
** This routine is callback for sqlite3WalkExpr().
**
** Resolve symbolic names into TK_COLUMN operators for the current
** node in the expression tree.  Return 0 to continue the search down
** the tree or 2 to abort the tree walk.
**
** This routine also does error checking and name resolution for
** function names.  The operator for aggregate functions is changed
** to TK_AGG_FUNCTION.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ resolve_ಅಭಿವ್ಯಕ್ತಿ_Step(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  ಪ್ರತಿಜ್ಞಾ( pNC!=0 );
  pParse = pNC->pParse;
  ಪ್ರತಿಜ್ಞಾ( pParse==pWalker->pParse );

#ifndef NDEBUG
  ಯದಿ( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    ಪೂರ್ಣಾಙ್ಕ i;
    ಪ್ರತಿ(i=0; i<pNC->pSrcList->nSrc; i++){
      ಪ್ರತಿಜ್ಞಾ( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  ವಿಕಲ್ಪ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    ಪಕ್ಷ TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      ಪ್ರತಿಜ್ಞಾ( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_COLUMN;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
      p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab = pItem->pSTab;
      p_ಅಭಿವ್ಯಕ್ತಿ->iTable = pItem->iCursor;
      p_ಅಭಿವ್ಯಕ್ತಿ->iColumn--;
      p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_INTEGER;
      ಭಙ್ಗ;
    }

    /* An optimization:  Attempt to convert
    **
    **      "expr IS NOT NULL"  -->  "TRUE"
    **      "expr IS NULL"      -->  "FALSE"
    **
    ** if we can prove that "expr" is never NULL.  Call this the
    ** "NOT NULL strength reduction optimization".
    **
    ** If this optimization occurs, also restore the NameContext ref-counts
    ** to the state they where in before the "column" LHS expression was
    ** resolved.  This prevents "column" from being counted as having been
    ** referenced, which might prevent a SELECT from being erroneously
    ** marked as correlated.
    **
    ** 2024-03-28: Beware of aggregates.  A bare column of aggregated table
    ** can still evaluate to NULL even though it is marked as NOT NULL.
    ** Example:
    **
    **       CREATE TABLE t1(a INT NOT NULL);
    **       SELECT a, a IS NULL, a IS NOT NULL, count(*) FROM t1;
    **
    ** The "a IS NULL" and "a IS NOT NULL" expressions cannot be optimized
    ** here because at the time this case is hit, we do not yet know whether
    ** or not t1 is being aggregated.  We have to assume the worst and omit
    ** the optimization.  The only time it is safe to apply this optimization
    ** is within the WHERE clause.
    */
    ಪಕ್ಷ TK_NOTNULL:
    ಪಕ್ಷ TK_ISNULL: {
      ಪೂರ್ಣಾಙ್ಕ anRef[8];
      NameContext *p;
      ಪೂರ್ಣಾಙ್ಕ i;
      ಪ್ರತಿ(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      ಯದಿ( IN_RENAME_OBJECT ) ಪ್ರತ್ಯಾಗಮ WRC_Prune;
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) ){
        /* The expression can be NULL.  So the optimization does not apply */
        ಪ್ರತ್ಯಾಗಮ WRC_Prune;
      }

      ಪ್ರತಿ(i=0, p=pNC; p; p=p->pNext, i++){
        ಯದಿ( (p->ncFlags & NC_Where)==0 ){
          ಪ್ರತ್ಯಾಗಮ WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) );
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
#ಯದಿ TREETRACE_ENABLED
      ಯದಿ( sqlite3_ವೃಕ್ಷ_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL
        );
        sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ(p_ಅಭಿವ್ಯಕ್ತಿ);
      }
#endif /* TREETRACE_ENABLED */
      p_ಅಭಿವ್ಯಕ್ತಿ->u.i_ಮೌಲ್ಯ = (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL);
      p_ಅಭಿವ್ಯಕ್ತಿ->flags |= EP_Int_ಮೌಲ್ಯ;
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_INTEGER;
      ಪ್ರತಿ(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        p->nRef = anRef[i];
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = 0;
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    ಪಕ್ಷ TK_ID:
    ಪಕ್ಷ TK_DOT: {
      ಧ್ರುವ ವರ್ಣ *zTable;
      ಧ್ರುವ ವರ್ಣ *zDb;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ;

      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
        p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ;
      }ಅನ್ಯಥಾ{
        ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        testcase( pNC->ncFlags & NC_Idx_ಅಭಿವ್ಯಕ್ತಿ );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_GenCol, 0, p_ಅಭಿವ್ಯಕ್ತಿ);
        p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        ಯದಿ( p_ಬಲ->op==TK_ID ){
          zDb = 0;
        }ಅನ್ಯಥಾ{
          ಪ್ರತಿಜ್ಞಾ( p_ಬಲ->op==TK_DOT );
          ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ, EP_Int_ಮೌಲ್ಯ) );
          zDb = p_ಎಡ->u.zToken;
          p_ಎಡ = p_ಬಲ->p_ಎಡ;
          p_ಬಲ = p_ಬಲ->p_ಬಲ;
        }
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_UToken(p_ಎಡ) && ಅಭಿವ್ಯಕ್ತಿ_Use_UToken(p_ಬಲ) );
        zTable = p_ಎಡ->u.zToken;
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
        ಯದಿ( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ, (ಶೂನ್ಯ*)p_ಬಲ);
          sqlite3RenameTokenRemap(pParse, (ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab, (ಶೂನ್ಯ*)p_ಎಡ);
        }
      }
      ಪ್ರತ್ಯಾಗಮ lookupName(pParse, zDb, zTable, p_ಬಲ, pNC, p_ಅಭಿವ್ಯಕ್ತಿ);
    }

    /* Resolve function names
    */
    ಪಕ್ಷ TK_FUNCTION: {
      ಅಭಿವ್ಯಕ್ತಿ_List *pList;            /* The argument list */
      ಪೂರ್ಣಾಙ್ಕ n;                      /* Number of arguments */
      ಪೂರ್ಣಾಙ್ಕ no_such_func = 0;       /* True if no such function exists */
      ಪೂರ್ಣಾಙ್ಕ wrong_ಅಂಕೆ_args = 0;     /* True if wrong number of arguments */
      ಪೂರ್ಣಾಙ್ಕ is_agg = 0;             /* True if is an aggregate function */
      ಧ್ರುವ ವರ್ಣ *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      ಪೂರ್ಣಾಙ್ಕ savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(p_ಅಭಿವ್ಯಕ್ತಿ) ? p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin : 0);
#endif
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_xIsSelect|EP_Int_ಮೌಲ್ಯ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0 || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ORDER );
      pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
      n = pList ? pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
      zId = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      ಯದಿ( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        ಯದಿ( pDef==0 ){
          no_such_func = 1;
        }ಅನ್ಯಥಾ{
          wrong_ಅಂಕೆ_args = 1;
        }
      }ಅನ್ಯಥಾ{
        is_agg = pDef->xFinalize!=0;
        ಯದಿ( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Unlikely);
          ಯದಿ( n==2 ){
            p_ಅಭಿವ್ಯಕ್ತಿ->iTable = ಅಭಿವ್ಯಕ್ತಿ_Probability(pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ);
            ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable<0 ){
              sqlite3_Error_ಸಂದೇಶ(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_ಅಭಿವ್ಯಕ್ತಿ);
              pNC->nNcErr++;
            }
          }ಅನ್ಯಥಾ{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_ಅಭಿವ್ಯಕ್ತಿ->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          ಪೂರ್ಣಾಙ್ಕ auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          ಯದಿ( auth!=SQLITE_OK ){
            ಯದಿ( auth==SQLITE_DENY ){
              sqlite3_Error_ಸಂದೇಶ(pParse, "not authorized to use function: %#T",
                                      p_ಅಭಿವ್ಯಕ್ತಿ);
              pNC->nNcErr++;
            }
            p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
            ಪ್ರತ್ಯಾಗಮ WRC_Prune;
          }
        }
#endif

        /* If the function may call sqlite3_value_subtype(), then set the 
        ** EP_SubtArg flag on all of its argument expressions. This prevents
        ** where.c from replacing the expression with a value read from an
        ** index on the same expression, which will not have the correct 
        ** subtype. Also set the flag if the function expression itself is
        ** an EP_SubtArg expression. In this case subtypes are required as 
        ** the function may return a value with a subtype back to its 
        ** caller using sqlite3_result_value().  */
        ಯದಿ( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_SubtArg) 
        ){
          resolve_Set_ಅಭಿವ್ಯಕ್ತಿ_Subtype_Arg(pList);
        }

        ಯದಿ( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_ConstFunc);
        }
        ಯದಿ( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_PartIdx|NC_GenCol, 0, p_ಅಭಿವ್ಯಕ್ತಿ);
        }ಅನ್ಯಥಾ{
          ಪ್ರತಿಜ್ಞಾ( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_ಅಭಿವ್ಯಕ್ತಿ->op2 = pNC->ncFlags & NC_SelfRef;
        }
        ಯದಿ( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
         && pParse->nested==0
         && (pParse->db->mDbFlags & DBFLAG_InternalFunc)==0
        ){
          /* Internal-use-only functions are disallowed unless the
          ** SQL is being compiled using sqlite3NestedParse() or
          ** the SQLITE_TESTCTRL_INTERNAL_FUNCTIONS test-control has be
          ** used to activate internal functions for testing purposes.
          **
          ** The 2 value for no_such_func means that the function is
          ** an internal-use-only function which should be treated as a
          ** non-existant function for name resolution purposes.
          */
          no_such_func = 2;
          pDef = 0;
        }ಅನ್ಯಥಾ
        ಯದಿ( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          ಯದಿ( pNC->ncFlags & NC_FromDDL ) ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function_Usable(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pDef);
        }
      }

      ಯದಿ( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        ಪ್ರತಿಜ್ಞಾ( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_ಮೌಲ್ಯ==0 && pDef->xInverse==0)
          || (pDef->x_ಮೌಲ್ಯ && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        ಯದಿ( pDef && pDef->x_ಮೌಲ್ಯ==0 && pWin ){
          sqlite3_Error_ಸಂದೇಶ(pParse,
              "%#T() may not be used as a window function", p_ಅಭಿವ್ಯಕ್ತಿ
          );
          pNC->nNcErr++;
        }ಅನ್ಯಥಾ ಯದಿ(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          ಧ್ರುವ ವರ್ಣ *zType;
          ಯದಿ( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }ಅನ್ಯಥಾ{
            zType = "aggregate";
          }
          sqlite3_Error_ಸಂದೇಶ(pParse, "misuse of %s function %#T()",zType,p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#ಅನ್ಯಥಾ
        ಯದಿ( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_ಸಂದೇಶ(pParse,"misuse of aggregate function %#T()",p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        ಅನ್ಯಥಾ ಯದಿ( no_such_func
              && (pParse->db->init.busy==0 ||
                  (no_such_func==2 && pParse->db->init.busy==2))
              /* Suppress "no such function" errors when reading
              ** the sqlite_schema table.  Except, do raise the error
              ** if init.busy is 2, meaning the schema parse is due
              ** to an ALTER TABLE ADD COLUMN statement, and the function
              ** is an internal-use-only function (no_such_func==2). */
#ifdef SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
              && pParse->explain==0
#endif
        ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "no such function: %#T", p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
        }ಅನ್ಯಥಾ ಯದಿ( wrong_ಅಂಕೆ_args ){
          sqlite3_Error_ಸಂದೇಶ(pParse,"wrong number of arguments to function %#T()",
               p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        ಅನ್ಯಥಾ ಯದಿ( is_agg==0 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
          sqlite3_Error_ಸಂದೇಶ(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_ಅಭಿವ್ಯಕ್ತಿ
          );
          pNC->nNcErr++;
        }
#endif
        ಅನ್ಯಥಾ ಯದಿ( is_agg==0 && p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Order_By_Aggregate_Error(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
        }
        ಯದಿ( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#ಅನ್ಯಥಾ
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      ಅನ್ಯಥಾ ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
        is_agg = 1;
      }
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pList);
      ಯದಿ( is_agg ){
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
          ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ORDER );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        ಯದಿ( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p_ಅಭಿವ್ಯಕ್ತಿ) && pWin==p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin );
          ಯದಿ( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            ಯದಿ( pParse->db->mallocFailed ) ಭಙ್ಗ;
          }
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pWin->pPartition);
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pWin->pOrderBy);
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }ಅನ್ಯಥಾ
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_AGG_FUNCTION;
          p_ಅಭಿವ್ಯಕ್ತಿ->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
            sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          ಯಾವತ್( pNC2
              && sqlite3ReferencesSrcList(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pNC2->pSrcList)==0
          ){
            p_ಅಭಿವ್ಯಕ್ತಿ->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }
          ಪ್ರತಿಜ್ಞಾ( pDef!=0 || IN_RENAME_OBJECT );
          ಯದಿ( pNC2 && pDef ){
            p_ಅಭಿವ್ಯಕ್ತಿ->op2 += pNC2->nNestedSelect;
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_ANYORDER==NC_OrderAgg );
            testcase( (pDef->funcFlags & SQLITE_FUNC_MINMAX)!=0 );
            testcase( (pDef->funcFlags & SQLITE_FUNC_ANYORDER)!=0 );
            pNC2->ncFlags |= NC_HasAgg
              | ((pDef->funcFlags^SQLITE_FUNC_ANYORDER)
                  & (SQLITE_FUNC_MINMAX|SQLITE_FUNC_ANYORDER));
          }
        }
        pNC->ncFlags |= savedAllowFlags;
      }
      /* FIX ME:  Compute pExpr->affinity based on the expected return
      ** type of the function
      */
      ಪ್ರತ್ಯಾಗಮ WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ಪಕ್ಷ TK_EXISTS:
    ಪಕ್ಷ TK_SELECT:
#endif
    ಪಕ್ಷ TK_IN: {
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IN );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT );
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        ಪೂರ್ಣಾಙ್ಕ nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_ಅಭಿವ್ಯಕ್ತಿ );
        testcase( pNC->ncFlags & NC_GenCol );
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect );
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS )  pParse->bHasExists = 1;
        ಯದಿ( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಭಿವ್ಯಕ್ತಿ);
        }ಅನ್ಯಥಾ{
          sqlite3WalkSelect(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect);
        }
        ಪ್ರತಿಜ್ಞಾ( pNC->nRef>=nRef );
        ಯದಿ( nRef!=pNC->nRef ){
          ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect);
          p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_ಅಭಿವ್ಯಕ್ತಿ );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_GenCol, p_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಭಿವ್ಯಕ್ತಿ);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_IS:
    ಪಕ್ಷ TK_ISNOT: {
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      ಯದಿ( ALWAYS(p_ಬಲ) && (p_ಬಲ->op==TK_ID || p_ಬಲ->op==TK_TRUEFALSE) ){
        ಪೂರ್ಣಾಙ್ಕ rc = resolve_ಅಭಿವ್ಯಕ್ತಿ_Step(pWalker, p_ಬಲ);
        ಯದಿ( rc==WRC_Abort ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
        ಯದಿ( p_ಬಲ->op==TK_TRUEFALSE ){
          p_ಅಭಿವ್ಯಕ್ತಿ->op2 = p_ಅಭಿವ್ಯಕ್ತಿ->op;
          p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_TRUTH;
          ಪ್ರತ್ಯಾಗಮ WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    ಪಕ್ಷ TK_BETWEEN:
    ಪಕ್ಷ TK_EQ:
    ಪಕ್ಷ TK_NE:
    ಪಕ್ಷ TK_LT:
    ಪಕ್ಷ TK_LE:
    ಪಕ್ಷ TK_GT:
    ಪಕ್ಷ TK_GE: {
      ಪೂರ್ಣಾಙ್ಕ n_ಎಡ, n_ಬಲ;
      ಯದಿ( pParse->db->mallocFailed ) ಭಙ್ಗ;
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ!=0 );
      n_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_BETWEEN ){
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
        n_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
        ಯದಿ( n_ಬಲ==n_ಎಡ ){
          n_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ);
        }
      }ಅನ್ಯಥಾ{
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ!=0 );
        n_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Vector_Size(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      }
      ಯದಿ( n_ಎಡ!=n_ಬಲ ){
        testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EQ );
        testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NE );
        testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LT );
        testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_LE );
        testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GT );
        testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_GE );
        testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_IS );
        testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ISNOT );
        testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_BETWEEN );
        sqlite3_Error_ಸಂದೇಶ(pParse, "row value misused");
        sqlite3_Record_Error_Offset_Of_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
      }
      ಭಙ್ಗ;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ಪ್ರತ್ಯಾಗಮ pParse->nErr ? WRC_Abort : WRC_Continue;
}

/*
** pEList is a list of expressions which are really the result set of the
** a SELECT statement.  pE is a term in an ORDER BY or GROUP BY clause.
** This routine checks to see if pE is a simple identifier which corresponds
** to the AS-name of one of the terms of the expression list.  If it is,
** this routine return an integer between 1 and N where N is the number of
** elements in pEList, corresponding to the matching entry.  If there is
** no match, or if pE is not a simple identifier, then this routine
** return 0.
**
** pEList has been resolved.  pE has not.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,  /* List of expressions to scan */
  ಅಭಿವ್ಯಕ್ತಿ *pE           /* Expression we are trying to match */
){
  ಪೂರ್ಣಾಙ್ಕ i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  ಯದಿ( pE->op==TK_ID ){
    ಧ್ರುವ ವರ್ಣ *zCol;
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pE, EP_Int_ಮೌಲ್ಯ) );
    zCol = pE->u.zToken;
    ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಯದಿ( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        ಪ್ರತ್ಯಾಗಮ i+1;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** pE is a pointer to an expression which is a single term in the
** ORDER BY of a compound SELECT.  The expression has not been
** name resolved.
**
** At the point this routine is called, we already know that the
** ORDER BY term is not an integer index into the result set.  That
** case is handled by the calling routine.
**
** Attempt to match pE against result set columns in the left-most
** SELECT statement.  Return the index i of the matching column,
** as an indication to the caller that it should sort by the i-th column.
** The left-most column is 1.  In other words, the value returned is the
** same integer value that would be used in the SQL statement to indicate
** the column.
**
** If there is no match, return 0.  Return -1 if an error occurs.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ resolve_Order_By_Term_To_ಅಭಿವ್ಯಕ್ತಿ_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  ಅಭಿವ್ಯಕ್ತಿ *pE           /* The specific ORDER BY term */
){
  ಪೂರ್ಣಾಙ್ಕ i;             /* Loop counter */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  ಪೂರ್ಣಾಙ್ಕ rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  ಪ್ರತಿಜ್ಞಾ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, ಪರಿಮಾಣ(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&nc, pE);
  db->suppressErr = savedSuppErr;
  ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  ಪ್ರತಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, pE, -1)<2 ){
      ಪ್ರತ್ಯಾಗಮ i+1;
    }
  }

  /* If no match, return 0. */
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  ಧ್ರುವ ವರ್ಣ *zType,     /* "ORDER" or "GROUP" */
  ಪೂರ್ಣಾಙ್ಕ i,                 /* The index (1-based) of the term out of range */
  ಪೂರ್ಣಾಙ್ಕ mx,                /* Largest permissible value of i */
  ಅಭಿವ್ಯಕ್ತಿ *pError           /* Associate the error with the expression */
){
  sqlite3_Error_ಸಂದೇಶ(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_Record_Error_Offset_Of_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, pError);
}

/*
** Analyze the ORDER BY clause in a compound SELECT statement.   Modify
** each term of the ORDER BY clause is a constant integer between 1
** and N where N is the number of columns in the compound SELECT.
**
** ORDER BY terms that are already an integer between 1 and N are
** unmodified.  ORDER BY terms that are integers outside the range of
** 1 through N generate an error.  ORDER BY terms that are expressions
** are matched against result set expressions of compound SELECT
** beginning with the left-most SELECT and working toward the right.
** At the first match, the ORDER BY expression is transformed into
** the integer column number.
**
** Return the number of errors seen.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  sqlite3 *db;
  ಪೂರ್ಣಾಙ್ಕ moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  ಯದಿ( pOrderBy==0 ) ಪ್ರತ್ಯಾಗಮ 0;
  db = pParse->db;
  ಯದಿ( pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many terms in ORDER BY clause");
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  ಪ್ರತಿ(i=0; i<pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  ಯಾವತ್( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  ಯಾವತ್( pSelect && moreToDo ){
    ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
    ಪ್ರತಿ(i=0, pItem=pOrderBy->a; i<pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++, pItem++){
      ಪೂರ್ಣಾಙ್ಕ iCol = -1;
      ಅಭಿವ್ಯಕ್ತಿ *pE, *pDup;
      ಯದಿ( pItem->fg.done ) ಅನುವರ್ತ;
      pE = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pItem->p_ಅಭಿವ್ಯಕ್ತಿ);
      ಯದಿ( NEVER(pE==0) ) ಅನುವರ್ತ;
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(pE, &iCol, 0, 1) ){
        ಯದಿ( iCol<=0 || iCol>pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->n_ಅಭಿವ್ಯಕ್ತಿ, pE);
          ಪ್ರತ್ಯಾಗಮ 1;
        }
      }ಅನ್ಯಥಾ{
        iCol = resolveAsName(pParse, pEList, pE);
        ಯದಿ( iCol==0 ){
          /* Now test if expression pE matches one of the values returned
          ** by pSelect. In the usual case this is done by duplicating the
          ** expression, resolving any symbols in it, and then comparing
          ** it against each expression returned by the SELECT statement.
          ** Once the comparisons are finished, the duplicate expression
          ** is deleted.
          **
          ** If this is running as part of an ALTER TABLE operation and
          ** the symbols resolve successfully, also resolve the symbols in the
          ** actual expression. This allows the code in alter.c to modify
          ** column references within the ORDER BY expression as required.  */
          pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pE, 0);
          ಯದಿ( !db->mallocFailed ){
            ಪ್ರತಿಜ್ಞಾ(pDup);
            iCol = resolve_Order_By_Term_To_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSelect, pDup);
            ಯದಿ( IN_RENAME_OBJECT && iCol>0 ){
              resolve_Order_By_Term_To_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSelect, pE);
            }
          }
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDup);
        }
      }
      ಯದಿ( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        ಯದಿ( !IN_RENAME_OBJECT ){
          ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, iCol);
          ಯದಿ( pNew==0 ) ಪ್ರತ್ಯಾಗಮ 1;
          ಯದಿ( pItem->p_ಅಭಿವ್ಯಕ್ತಿ==pE ){
            pItem->p_ಅಭಿವ್ಯಕ್ತಿ = pNew;
          }ಅನ್ಯಥಾ{
            ಅಭಿವ್ಯಕ್ತಿ *pParent = pItem->p_ಅಭಿವ್ಯಕ್ತಿ;
            ಪ್ರತಿಜ್ಞಾ( pParent->op==TK_COLLATE );
            ಯಾವತ್( pParent->p_ಎಡ->op==TK_COLLATE ) pParent = pParent->p_ಎಡ;
            ಪ್ರತಿಜ್ಞಾ( pParent->p_ಎಡ==pE );
            pParent->p_ಎಡ = pNew;
          }
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }ಅನ್ಯಥಾ{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  ಪ್ರತಿ(i=0; i<pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಯದಿ( pOrderBy->a[i].fg.done==0 ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      ಪ್ರತ್ಯಾಗಮ 1;
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

/*
** Check every term in the ORDER BY or GROUP BY clause pOrderBy of
** the SELECT statement pSelect.  If any term is reference to a
** result set expression (as determined by the ExprList.a.u.x.iOrderByCol
** field) then convert that term into a copy of the corresponding result set
** column.
**
** If any errors are detected, add an error message to pParse and
** return non-zero.  Return zero if no errors are seen.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  ಧ್ರುವ ವರ್ಣ *zType     /* "ORDER" or "GROUP" */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;

  ಯದಿ( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) ಪ್ರತ್ಯಾಗಮ 0;
  ಯದಿ( pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many terms in %s BY clause", zType);
    ಪ್ರತ್ಯಾಗಮ 1;
  }
  pEList = pSelect->pEList;
  ಪ್ರತಿಜ್ಞಾ( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  ಪ್ರತಿ(i=0, pItem=pOrderBy->a; i<pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++, pItem++){
    ಯದಿ( pItem->u.x.iOrderByCol ){
      ಯದಿ( pItem->u.x.iOrderByCol>pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->n_ಅಭಿವ್ಯಕ್ತಿ, 0);
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->p_ಅಭಿವ್ಯಕ್ತಿ,0);
    }
  }
  ಪ್ರತ್ಯಾಗಮ 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ resolveRemoveWindowsCb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  UNUSED_PARAMETER(pWalker);
  ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
    Window *pWin = p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
ಸ್ಥಿರ ಶೂನ್ಯ window_Remove_ಅಭಿವ್ಯಕ್ತಿ_From_Select(Select *pSelect, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಯದಿ( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, ಪರಿಮಾಣ(Walker));
    sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
  }
}
#ಅನ್ಯಥಾ
# define window_Remove_ಅಭಿವ್ಯಕ್ತಿ_From_Select(a, b)
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** pOrderBy is an ORDER BY or GROUP BY clause in SELECT statement pSelect.
** The Name context of the SELECT statement is pNC.  zType is either
** "ORDER" or "GROUP" depending on which type of clause pOrderBy is.
**
** This routine resolves each term of the clause into an expression.
** If the order-by term is an integer I between 1 and N (where N is the
** number of columns in the result set of the SELECT) then the expression
** in the resolution is a copy of the I-th result-set expression.  If
** the order-by term is an identifier that corresponds to the AS-name of
** a result-set expression, then the term resolves to a copy of the
** result-set expression.  Otherwise, the expression is resolved in
** the usual way - using sqlite3ResolveExprNames().
**
** This routine returns the number of errors.  If errors occur, then
** an appropriate error message might be left in pParse.  (OOM errors
** excepted.)
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  ಧ್ರುವ ವರ್ಣ *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  ಪೂರ್ಣಾಙ್ಕ i, j;                      /* Loop counters */
  ಪೂರ್ಣಾಙ್ಕ iCol;                      /* Column number */
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  ಪೂರ್ಣಾಙ್ಕ nResult;                   /* Number of terms in the result set */

  ಪ್ರತಿಜ್ಞಾ( pOrderBy!=0 );
  nResult = pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
  pParse = pNC->pParse;
  ಪ್ರತಿ(i=0, pItem=pOrderBy->a; i<pOrderBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++, pItem++){
    ಅಭಿವ್ಯಕ್ತಿ *pE = pItem->p_ಅಭಿವ್ಯಕ್ತಿ;
    ಅಭಿವ್ಯಕ್ತಿ *pE2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pE);
    ಯದಿ( NEVER(pE2==0) ) ಅನುವರ್ತ;
    ಯದಿ( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      ಯದಿ( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        ಅನುವರ್ತ;
      }
    }
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      ಯದಿ( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult, pE2);
        ಪ್ರತ್ಯಾಗಮ 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      ಅನುವರ್ತ;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    ಯದಿ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(pNC, pE) ){
      ಪ್ರತ್ಯಾಗಮ 1;
    }
    ಪ್ರತಿ(j=0; j<pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
      ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, pE, pSelect->pEList->a[j].p_ಅಭಿವ್ಯಕ್ತಿ, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        window_Remove_ಅಭಿವ್ಯಕ್ತಿ_From_Select(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  ಪೂರ್ಣಾಙ್ಕ isCompound;         /* True if p is a compound select */
  ಪೂರ್ಣಾಙ್ಕ nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  ಪೂರ್ಣಾಙ್ಕ i;                  /* Loop counter */
  ಅಭಿವ್ಯಕ್ತಿ_List *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಯದಿ( p->selFlags & SF_Resolved ){
    ಪ್ರತ್ಯಾಗಮ WRC_Prune;
  }
  pOuterNC = pWalker->u.pNC;
  pParse = pWalker->pParse;
  db = pParse->db;

  /* Normally sqlite3SelectExpand() will be called first and will have
  ** already expanded this SELECT.  However, if this is a subquery within
  ** an expression, sqlite3ResolveExprNames() will be called without a
  ** prior call to sqlite3SelectExpand().  When that happens, let
  ** sqlite3SelectPrep() do all of the processing for this SELECT.
  ** sqlite3SelectPrep() will invoke both sqlite3SelectExpand() and
  ** this routine in the correct order.
  */
  ಯದಿ( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    ಪ್ರತ್ಯಾಗಮ pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  ಯಾವತ್( p ){
    ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Expanded)!=0 );
    ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    ಯದಿ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, p->pLimit) ){
      ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    ಯದಿ( p->selFlags & SF_Converted ){
      Select *pSub;
      ಪ್ರತಿಜ್ಞಾ( p->pSrc->a[0].fg.isSubquery );
      ಪ್ರತಿಜ್ಞಾ( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ಪ್ರತಿಜ್ಞಾ( pSub!=0 );
      ಪ್ರತಿಜ್ಞಾ( p->pSrc->nSrc==1 && p->pOrderBy );
      ಪ್ರತಿಜ್ಞಾ( pSub->pPrior && pSub->pOrderBy==0 );
      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    ಯದಿ( pOuterNC ) pOuterNC->nNestedSelect++;
    ಪ್ರತಿ(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      ಪ್ರತಿಜ್ಞಾ( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      ಯದಿ( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        ಪೂರ್ಣಾಙ್ಕ nRef = pOuterNC ? pOuterNC->nRef : 0;
        ಧ್ರುವ ವರ್ಣ *zSavedContext = pParse->zAuthContext;

        ಯದಿ( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
        ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        ಯದಿ( pOuterNC ){
          ಪ್ರತಿಜ್ಞಾ( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    ಯದಿ( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
 
    /* Resolve names in the result set. */
    ಯದಿ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, p->pEList) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    ಯದಿ( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      ಪ್ರತಿಜ್ಞಾ( NC_MinMaxAgg==SF_MinMaxAgg );
      ಪ್ರತಿಜ್ಞಾ( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }ಅನ್ಯಥಾ{
      sNC.ncFlags &= ~NC_AllowAgg;
    }
 
    /* Add the output column list to the name-context before parsing the
    ** other expressions in the SELECT statement. This is so that
    ** expressions in the WHERE clause (etc.) can refer to expressions by
    ** aliases in the result set.
    **
    ** Minor point: If this is the case, then the expression will be
    ** re-evaluated for each reference to it.
    */
    ಪ್ರತಿಜ್ಞಾ( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    ಯದಿ( p->pHaving ){
      ಯದಿ( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "HAVING clause on a non-aggregate query");
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
      ಯದಿ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, p->pHaving) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    ಯದಿ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, p->pWhere) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    ಪ್ರತಿ(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      ಯದಿ( pItem->fg.isTabFunc
       && sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, pItem->u1.pFuncArg)
      ){
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( IN_RENAME_OBJECT ){
      Window *pWin;
      ಪ್ರತಿ(pWin=p->pWinDefn; pWin; pWin=pWin->pNextWin){
        ಯದಿ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, pWin->pOrderBy)
         || sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, pWin->pPartition)
        ){
          ಪ್ರತ್ಯಾಗಮ WRC_Abort;
        }
      }
    }
#endif

    sNC.ncFlags |= NC_AllowAgg|NC_AllowWin;

    /* If this is a converted compound query, move the ORDER BY clause from
    ** the sub-query back to the parent query. At this point each term
    ** within the ORDER BY clause has been transformed to an integer value.
    ** These integers will be replaced by copies of the corresponding result
    ** set expressions by the call to resolveOrderGroupBy() below.  */
    ಯದಿ( p->selFlags & SF_Converted ){
      Select *pSub;
      ಪ್ರತಿಜ್ಞಾ( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ಪ್ರತಿಜ್ಞಾ( pSub!=0 );
      p->pOrderBy = pSub->pOrderBy;
      pSub->pOrderBy = 0;
    }

    /* Process the ORDER BY clause for singleton SELECT statements.
    ** The ORDER BY clause for compounds SELECT statements is handled
    ** below, after all of the result-sets for all of the elements of
    ** the compound have been resolved.
    **
    ** If there is an ORDER BY clause on a term of a compound-select other
    ** than the right-most term, then that is a syntax error.  But the error
    ** is not detected until much later, and so we need to go ahead and
    ** resolve those symbols on the incorrect ORDER BY for consistency.
    */
    ಯದಿ( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }
    ಯದಿ( db->mallocFailed ){
      ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    ಯದಿ( pGroupBy ){
      ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
   
      ಯದಿ( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
      ಪ್ರತಿ(i=0, pItem=pGroupBy->a; i<pGroupBy->n_ಅಭಿವ್ಯಕ್ತಿ; i++, pItem++){
        ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pItem->p_ಅಭಿವ್ಯಕ್ತಿ, EP_Agg) ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          ಪ್ರತ್ಯಾಗಮ WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    ಯದಿ( p->pNext && p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=p->pNext->pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
      sqlite3_Select_Wrong_ಅಂಕೆ_Terms_Error(pParse, p->pNext);
      ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    ಯದಿ( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      ಯದಿ( pParse->nErr ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  ಯದಿ( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  }

  ಪ್ರತ್ಯಾಗಮ WRC_Prune;
}

/*
** This routine walks an expression tree and resolves references to
** table columns and result-set columns.  At the same time, do error
** checking on function usage and set a flag if any aggregate functions
** are seen.
**
** To resolve table columns references we look for nodes (or subtrees) of the
** form X.Y.Z or Y.Z or just Z where
**
**      X:   The name of a database.  Ex:  "main" or "temp" or
**           the symbolic name assigned to an ATTACH-ed database.
**
**      Y:   The name of a table in a FROM clause.  Or in a trigger
**           one of the special names "old" or "new".
**
**      Z:   The name of a column in table Y.
**
** The node at the root of the subtree is modified as follows:
**
**    Expr.op        Changed to TK_COLUMN
**    Expr.pTab      Points to the Table object for X.Y
**    Expr.iColumn   The column index in X.Y.  -1 for the rowid.
**    Expr.iTable    The VDBE cursor number for X.Y
**
**
** To resolve result-set references, look for expression nodes of the
** form Z (with no X and Y prefix) where the Z matches the right-hand
** size of an AS clause in the result-set of a SELECT.  The Z expression
** is replaced by a copy of the left-hand side of the result-set expression.
** Table-name and function resolution occurs on the substituted expression
** tree.  For example, in:
**
**      SELECT a+b AS x, c+d AS y FROM t1 ORDER BY x;
**
** The "x" term of the order by is replaced by "a+b" to render:
**
**      SELECT a+b AS x, c+d AS y FROM t1 ORDER BY a+b;
**
** Function calls are checked to make sure that the function is
** defined and that the correct number of arguments are specified.
** If the function is an aggregate function, then the NC_HasAgg flag is
** set and the opcode is changed from TK_FUNCTION to TK_AGG_FUNCTION.
** If an expression contains aggregate functions then the EP_Agg
** property on the expression is set.
**
** An error message is left in pParse if anything is amiss.  The number
** if errors is returned.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ             /* The expression to be analyzed. */
){
  ಪೂರ್ಣಾಙ್ಕ savedHasAgg;
  Walker w;

  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = resolve_ಅಭಿವ್ಯಕ್ತಿ_Step;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
  w.pParse->nHeight += p_ಅಭಿವ್ಯಕ್ತಿ->nHeight;
  ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_Height(w.pParse, w.pParse->nHeight) ){
    ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }
#endif
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
  w.pParse->nHeight -= p_ಅಭಿವ್ಯಕ್ತಿ->nHeight;
#endif
  ಪ್ರತಿಜ್ಞಾ( EP_Agg==NC_HasAgg );
  ಪ್ರತಿಜ್ಞಾ( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  ಪ್ರತ್ಯಾಗಮ pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList         /* The expression list to be analyzed. */
){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಪೂರ್ಣಾಙ್ಕ savedHasAgg = 0;
  Walker w;
  ಯದಿ( pList==0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = resolve_ಅಭಿವ್ಯಕ್ತಿ_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  ಪ್ರತಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಅನುವರ್ತ;
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
    w.pParse->nHeight += p_ಅಭಿವ್ಯಕ್ತಿ->nHeight;
    ಯದಿ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_Height(w.pParse, w.pParse->nHeight) ){
      ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
    }
#endif
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
#ಯದಿ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_DEPTH>0
    w.pParse->nHeight -= p_ಅಭಿವ್ಯಕ್ತಿ->nHeight;
#endif
    ಪ್ರತಿಜ್ಞಾ( EP_Agg==NC_HasAgg );
    ಪ್ರತಿಜ್ಞಾ( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    ಯದಿ( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      ಅಭಿವ್ಯಕ್ತಿ_Set_Property(p_ಅಭಿವ್ಯಕ್ತಿ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    ಯದಿ( w.pParse->nErr>0 ) ಪ್ರತ್ಯಾಗಮ SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  ಪ್ರತ್ಯಾಗಮ SQLITE_OK;
}

/*
** Resolve all names in all expressions of a SELECT and in all
** descendants of the SELECT, including compounds off of p->pPrior,
** subqueries in expressions, and subqueries used as FROM clause
** terms.
**
** See sqlite3ResolveExprNames() for a description of the kinds of
** transformations that occur.
**
** All SELECT statements should have been expanded using
** sqlite3SelectExpand() prior to invoking this routine.
*/
ಶೂನ್ಯ sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = resolve_ಅಭಿವ್ಯಕ್ತಿ_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.pParse = pParse;
  w.u.pNC = pOuterNC;
  sqlite3WalkSelect(&w, p);
}

/*
** Resolve names in expressions that can only reference a single table
** or which cannot reference any tables at all.  Examples:
**
**                                                    "type" flag
**                                                    ------------
**    (1)   CHECK constraints                         NC_IsCheck
**    (2)   WHERE clauses on partial indices          NC_PartIdx
**    (3)   Expressions in indexes on expressions     NC_IdxExpr
**    (4)   Expression arguments to VACUUM INTO.      0
**    (5)   GENERATED ALWAYS as expressions           NC_GenCol
**
** In all cases except (4), the Expr.iTable value for Expr.op==TK_COLUMN
** nodes of the expression is set to -1 and the Expr.iColumn value is
** set to the column number.  In case (4), TK_COLUMN nodes cause an error.
**
** Any errors cause an error message to be set in pParse.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  ಪೂರ್ಣಾಙ್ಕ type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,     /* Expression to resolve.  May be NULL. */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಸಙ್ಘ {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  ಪ್ರತಿಜ್ಞಾ( type==0 || pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_ಅಭಿವ್ಯಕ್ತಿ
          || type==NC_GenCol || pTab==0 );
  memset(&sNC, 0, ಪರಿಮಾಣ(sNC));
  memset(&uSrc, 0, ಪರಿಮಾಣ(uSrc));
  pSrc = &uSrc.sSrc;
  ಯದಿ( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    ಯದಿ( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  ಯದಿ( (rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_Names(&sNC, p_ಅಭಿವ್ಯಕ್ತಿ))!=SQLITE_OK ) ಪ್ರತ್ಯಾಗಮ rc;
  ಯದಿ( pList ) rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_Names(&sNC, pList);
  ಪ್ರತ್ಯಾಗಮ rc;
}
