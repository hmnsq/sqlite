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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#define EXCLUDED_TABLE_ସଂଖ୍ୟା  2

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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା incrAggDepth(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_AGG_FUNCTION ) p_ପଦସମଷ୍ଟି->op2 += pWalker->u.n;
  ଫେରନ୍ତୁ WRC_Continue;
}
ସ୍ଥିର ଶୂନ୍ୟ incrAggFunctionDepth(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, ପୂର୍ଣ୍ଣସଂଖ୍ୟା N){
  ଯଦି( N>0 ){
    Walker w;
    memset(&w, 0, ଆକାର(w));
    w.x_ପଦସମଷ୍ଟି_Callback = incrAggDepth;
    w.u.n = N;
    sqlite3_Walk_ପଦସମଷ୍ଟି(&w, p_ପଦସମଷ୍ଟି);
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
ସ୍ଥିର ଶୂନ୍ୟ resolveAlias(
  Parse *pParse,         /* Parsing context */
  ପଦସମଷ୍ଟି_List *pEList,      /* A result set */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,           /* Transform this into an alias to the result set */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSubquery          /* Number of subqueries that the label is moving */
){
  ପଦସମଷ୍ଟି *pOrig;           /* The iCol-th column of the result set */
  ପଦସମଷ୍ଟି *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  ପ୍ରତିଜ୍ଞା( iCol>=0 && iCol<pEList->n_ପଦସମଷ୍ଟି );
  pOrig = pEList->a[iCol].p_ପଦସମଷ୍ଟି;
  ପ୍ରତିଜ୍ଞା( pOrig!=0 );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Reduced|EP_TokenOnly) );
  ଯଦି( p_ପଦସମଷ୍ଟି->pAggInfo ) ଫେରନ୍ତୁ;
  db = pParse->db;
  pDup = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pOrig, 0);
  ଯଦି( db->mallocFailed ){
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, pDup);
    pDup = 0;
  }ଅନ୍ୟଥା{
    ପଦସମଷ୍ଟି temp;
    incrAggFunctionDepth(pDup, nSubquery);
    ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLLATE ){
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      pDup = sqlite3_ପଦସମଷ୍ଟି_Add_Collate_String(pParse, pDup, p_ପଦସମଷ୍ଟି->u.zToken);
    }
    memcpy(&temp, pDup, ଆକାର(ପଦସମଷ୍ଟି));
    memcpy(pDup, p_ପଦସମଷ୍ଟି, ଆକାର(ପଦସମଷ୍ଟି));
    memcpy(p_ପଦସମଷ୍ଟି, &temp, ଆକାର(ପଦସମଷ୍ଟି));
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
      ଯଦି( ALWAYS(p_ପଦସମଷ୍ଟି->y.pWin!=0) ){
        p_ପଦସମଷ୍ଟି->y.pWin->pOwner = p_ପଦସମଷ୍ଟି;
      }
    }
    sqlite3_ପଦସମଷ୍ଟି_Deferred_Delete(pParse, pDup);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3MatchEName(
  ଅଚଳ ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem,
  ଅଚଳ ଅକ୍ଷର *zCol,
  ଅଚଳ ଅକ୍ଷର *zTab,
  ଅଚଳ ଅକ୍ଷର *zDb,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *pbRowid
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  ଅଚଳ ଅକ୍ଷର *zSpan;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା eEName = pItem->fg.eEName;
  ଯଦି( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    ଫେରନ୍ତୁ 0;
  }
  ପ୍ରତିଜ୍ଞା( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  ପାଇଁ(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  ଯଦି( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    ଫେରନ୍ତୁ 0;
  }
  zSpan += n+1;
  ପାଇଁ(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  ଯଦି( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    ଫେରନ୍ତୁ 0;
  }
  zSpan += n+1;
  ଯଦି( zCol ){
    ଯଦି( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) ଫେରନ୍ତୁ 0;
    ଯଦି( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) ଫେରନ୍ତୁ 0;
  }
  ଯଦି( eEName==ENAME_ROWID ) *pbRowid = 1;
  ଫେରନ୍ତୁ 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *p_ଶୀର୍ଷ_NC){
  ଯଦି( db->init.busy ) ଫେରନ୍ତୁ 1;  /* Always support for legacy schemas */
  ଯଦି( p_ଶୀର୍ଷ_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    ଯଦି( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      ଫେରନ୍ତୁ 1;
    }
    ଫେରନ୍ତୁ (db->flags & SQLITE_DqsDDL)!=0;
  }ଅନ୍ୟଥା{
    /* Currently parsing a DML statement */
    ଫେରନ୍ତୁ (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_ପଦସମଷ୍ଟି_Col_Used(ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
  Table *pExTab;

  n = p_ପଦସମଷ୍ଟି->iColumn;
  ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
  pExTab = p_ପଦସମଷ୍ଟି->y.pTab;
  ପ୍ରତିଜ୍ଞା( pExTab!=0 );
  ପ୍ରତିଜ୍ଞା( n < pExTab->nCol );
  ଯଦି( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    ଫେରନ୍ତୁ pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }ଅନ୍ୟଥା{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    ଯଦି( n>=BMS ) n = BMS-1;
    ଫେରନ୍ତୁ ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
ସ୍ଥିର ଶୂନ୍ୟ extendFJMatch(
  Parse *pParse,          /* Parsing context */
  ପଦସମଷ୍ଟି_List **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  ପଦସମଷ୍ଟି *pNew = sqlite3_ପଦସମଷ୍ଟି_Alloc(pParse->db, TK_COLUMN, 0, 0);
  ଯଦି( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    ପ୍ରତିଜ୍ଞା( (pMatch->fg.jointype & (JT_ବାମ|JT_LTORJ))!=0 );
    ପଦସମଷ୍ଟି_Set_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_ପଦସମଷ୍ଟି_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
ସ୍ଥିର SQLITE_NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା isValidSchemaTableName(
  ଅଚଳ ଅକ୍ଷର *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  ଅଚଳ ଅକ୍ଷର *zDb           /* non-NULL if a database qualifier is present */
){
  ଅଚଳ ଅକ୍ଷର *zLegacy;
  ପ୍ରତିଜ୍ଞା( pTab!=0 );
  ପ୍ରତିଜ୍ଞା( pTab->tnum==1 );
  ଯଦି( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) ଫେରନ୍ତୁ 0;
  zLegacy = pTab->zName;
  ଯଦି( strcmp(zLegacy+7, &LEGACY_TEMP_SCHEMA_TABLE[7])==0 ){
    ଯଦି( sqlite3StrICmp(zTab+7, &PREFERRED_TEMP_SCHEMA_TABLE[7])==0 ){
      ଫେରନ୍ତୁ 1;
    }
    ଯଦି( zDb==0 ) ଫେରନ୍ତୁ 0;
    ଯଦି( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) ଫେରନ୍ତୁ 1;
    ଯଦି( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) ଫେରନ୍ତୁ 1;
  }ଅନ୍ୟଥା{
    ଯଦି( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) ଫେରନ୍ତୁ 1;
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା lookupName(
  Parse *pParse,       /* The parsing context */
  ଅଚଳ ଅକ୍ଷର *zDb,     /* Name of the database containing table, or NULL */
  ଅଚଳ ଅକ୍ଷର *zTab,    /* Name of table containing column, or NULL */
  ଅଚଳ ପଦସମଷ୍ଟି *p_ଡାହାଣ,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି          /* Make this EXPR node point to the selected column */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;                         /* Loop counters */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 0;                      /* Number of matching column names */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cntTab = 0;                   /* Number of potential "rowid" matches */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *p_ଶୀର୍ଷ_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା e_New_ପଦସମଷ୍ଟି_Op = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  ପଦସମଷ୍ଟି_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  ଅଚଳ ଅକ୍ଷର *zCol = p_ଡାହାଣ->u.zToken;

  ପ୍ରତିଜ୍ଞା( pNC );     /* the name context cannot be NULL. */
  ପ୍ରତିଜ୍ଞା( zCol );    /* The Z in X.Y.Z cannot be NULL */
  ପ୍ରତିଜ୍ଞା( zDb==0 || zTab!=0 );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_ପଦସମଷ୍ଟି->iTable = -1;
  ପଦସମଷ୍ଟି_Set_VVAProperty(p_ପଦସମଷ୍ଟି, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  ଯଦି( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    ଯଦି( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }ଅନ୍ୟଥା{
      ପାଇଁ(i=0; i<db->nDb; i++){
        ପ୍ରତିଜ୍ଞା( db->aDb[i].zDbSName );
        ଯଦି( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          ଭାଙ୍ଗ;
        }
      }
      ଯଦି( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  ପ୍ରତିଜ୍ଞା( pNC && cnt==0 );
  କର{
    ପଦସମଷ୍ଟି_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    ଯଦି( pSrcList ){
      ପାଇଁ(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        ପ୍ରତିଜ୍ଞା( pTab!=0 && pTab->zName!=0 );
        ପ୍ରତିଜ୍ଞା( pTab->nCol>0 || pParse->nErr );
        ପ୍ରତିଜ୍ଞା( (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        ଯଦି( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା hit = 0;
          Select *pSel;
          ପ୍ରତିଜ୍ଞା( pItem->fg.isSubquery );
          ପ୍ରତିଜ୍ଞା( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          ପ୍ରତିଜ୍ଞା( pSel!=0 );
          pEList = pSel->pEList;
          ପ୍ରତିଜ୍ଞା( pEList!=0 );
          ପ୍ରତିଜ୍ଞା( pEList->n_ପଦସମଷ୍ଟି==pTab->nCol );
          ପାଇଁ(j=0; j<pEList->n_ପଦସମଷ୍ଟି; j++){
            ପୂର୍ଣ୍ଣସଂଖ୍ୟା bRowid = 0;       /* True if possible rowid match */
            ଯଦି( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              ଜାରି;
            }
            ଯଦି( bRowid==0 ){
              ଯଦି( cnt>0 ){
                ଯଦି( pItem->fg.isUsing==0
                 || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ଅନ୍ୟଥା
                ଯଦି( (pItem->fg.jointype & JT_ଡାହାଣ)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  ଜାରି;
                }ଅନ୍ୟଥା
                ଯଦି( (pItem->fg.jointype & JT_ବାମ)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ଅନ୍ୟଥା{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, p_ପଦସମଷ୍ଟି->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }ଅନ୍ୟଥା ଯଦି( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              ଜାରି;
            }
            cntTab++;
            pMatch = pItem;
            p_ପଦସମଷ୍ଟି->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            ପ୍ରତିଜ୍ଞା( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            ଯଦି( pEList->a[j].fg.bUsingTerm ) ଭାଙ୍ଗ;
          }
          ଯଦି( hit || zTab==0 ) ଜାରି;
        }
        ପ୍ରତିଜ୍ଞା( zDb==0 || zTab!=0 );
        ଯଦି( zTab ){
          ଯଦି( zDb ){
            ଯଦି( pTab->pSchema!=pSchema ) ଜାରି;
            ଯଦି( pSchema==0 && strcmp(zDb,"*")!=0 ) ଜାରି;
          }
          ଯଦି( pItem->zAlias!=0 ){
            ଯଦି( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              ଜାରି;
            }
          }ଅନ୍ୟଥା ଯଦି( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            ଯଦି( pTab->tnum!=1 ) ଜାରି;
            ଯଦି( !isValidSchemaTableName(zTab, pTab, zDb) ) ଜାରି;
          }
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
          ଯଦି( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (ଶୂନ୍ୟ*)&p_ପଦସମଷ୍ଟି->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        ଯଦି( j>=0 ){
          ଯଦି( cnt>0 ){
            ଯଦି( pItem->fg.isUsing==0
             || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ଅନ୍ୟଥା
            ଯଦି( (pItem->fg.jointype & JT_ଡାହାଣ)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              ଜାରି;
            }ଅନ୍ୟଥା
            ଯଦି( (pItem->fg.jointype & JT_ବାମ)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ଅନ୍ୟଥା{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, p_ପଦସମଷ୍ଟି->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_ପଦସମଷ୍ଟି->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          ଯଦି( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        ଯଦି( 0==cnt && VisibleRowid(pTab) ){
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
          ଯଦି( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }ଅନ୍ୟଥା{
            cntTab++;
          }
#ଅନ୍ୟଥା
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      ଯଦି( pMatch ){
        p_ପଦସମଷ୍ଟି->iTable = pMatch->iCursor;
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
        p_ପଦସମଷ୍ଟି->y.pTab = pMatch->pSTab;
        ଯଦି( (pMatch->fg.jointype & (JT_ବାମ|JT_LTORJ))!=0 ){
          ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_CanBeNull);
        }
        pSchema = p_ପଦସମଷ୍ଟି->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#ଯଦି !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    ଯଦି( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      ଯଦି( pParse->pTriggerTab!=0 ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା op = pParse->eTriggerOp;
        ପ୍ରତିଜ୍ଞା( op==TK_DELETE || op==TK_UPDATE || op==TK_ଭର୍ତ୍ତି );
        ଯଦି( pParse->bReturning ){
          ଯଦି( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            p_ପଦସମଷ୍ଟି->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }ଅନ୍ୟଥା ଯଦି( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_ପଦସମଷ୍ଟି->iTable = 1;
          pTab = pParse->pTriggerTab;
        }ଅନ୍ୟଥା ଯଦି( op!=TK_ଭର୍ତ୍ତି && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_ପଦସମଷ୍ଟି->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      ଯଦି( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        ଯଦି( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_ପଦସମଷ୍ଟି->iTable = EXCLUDED_TABLE_ସଂଖ୍ୟା;
          ଯଦି( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ଶୂନ୍ୟ*)&p_ପଦସମଷ୍ଟି->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      ଯଦି( pTab ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        ଯଦି( iCol>=0 ){
          ଯଦି( pTab->iPKey==iCol ) iCol = -1;
        }ଅନ୍ୟଥା{
          ଯଦି( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }ଅନ୍ୟଥା{
            iCol = pTab->nCol;
          }
        }
        ଯଦି( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          ଯଦି( p_ପଦସମଷ୍ଟି->iTable==EXCLUDED_TABLE_ସଂଖ୍ୟା ){
            testcase( iCol==(-1) );
            ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
            ଯଦି( IN_RENAME_OBJECT ){
              p_ପଦସମଷ୍ଟି->iColumn = iCol;
              p_ପଦସମଷ୍ଟି->y.pTab = pTab;
              e_New_ପଦସମଷ୍ଟି_Op = TK_COLUMN;
            }ଅନ୍ୟଥା{
              p_ପଦସମଷ୍ଟି->iTable = pNC->uNC.pUpsert->regData +
                 sqlite3TableColumnToStorage(pTab, iCol);
              e_New_ପଦସମଷ୍ଟି_Op = TK_REGISTER;
            }
          }ଅନ୍ୟଥା
#endif /* SQLITE_OMIT_UPSERT */
          {
            ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
            p_ପଦସମଷ୍ଟି->y.pTab = pTab;
            ଯଦି( pParse->bReturning ){
              e_New_ପଦସମଷ୍ଟି_Op = TK_REGISTER;
              p_ପଦସମଷ୍ଟି->op2 = TK_COLUMN;
              p_ପଦସମଷ୍ଟି->iColumn = iCol;
              p_ପଦସମଷ୍ଟି->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*p_ପଦସମଷ୍ଟି->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }ଅନ୍ୟଥା{
              p_ପଦସମଷ୍ଟି->iColumn = (i16)iCol;
              e_New_ପଦସମଷ୍ଟି_Op = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              ଯଦି( iCol<0 ){
                p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି = SQLITE_AFF_INTEGER;
              }ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }ଅନ୍ୟଥା{
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
    ଯଦି( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_Idx_ପଦସମଷ୍ଟି|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#ଯଦି SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      ଯଦି( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        e_New_ପଦସମଷ୍ଟି_Op = TK_NULL;
      }
#endif
      ଯଦି( pMatch->fg.isNestedFrom==0 ) p_ପଦସମଷ୍ଟି->iColumn = -1;
      p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି = SQLITE_AFF_INTEGER;
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
    ଯଦି( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      ପ୍ରତିଜ୍ଞା( pEList!=0 );
      ପାଇଁ(j=0; j<pEList->n_ପଦସମଷ୍ଟି; j++){
        ଅକ୍ଷର *zAs = pEList->a[j].zEName;
        ଯଦି( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          ପଦସମଷ୍ଟି *pOrig;
          ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ==0 && p_ପଦସମଷ୍ଟି->p_ଡାହାଣ==0 );
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି)==0 || p_ପଦସମଷ୍ଟି->x.pList==0 );
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି)==0 || p_ପଦସମଷ୍ଟି->x.pSelect==0 );
          pOrig = pEList->a[j].p_ପଦସମଷ୍ଟି;
          ଯଦି( (pNC->ncFlags&NC_AllowAgg)==0 && ପଦସମଷ୍ଟି_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_ସନ୍ଦେଶ(pParse, "misuse of aliased aggregate %s", zAs);
            ଫେରନ୍ତୁ WRC_Abort;
          }
          ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_ଶୀର୍ଷ_NC )
          ){
            sqlite3_Error_ସନ୍ଦେଶ(pParse, "misuse of aliased window function %s",zAs);
            ଫେରନ୍ତୁ WRC_Abort;
          }
          ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Vector_Size(pOrig)!=1 ){
            sqlite3_Error_ସନ୍ଦେଶ(pParse, "row value misused");
            ଫେରନ୍ତୁ WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_ପଦସମଷ୍ଟି, nSubquery);
          cnt = 1;
          pMatch = 0;
          ପ୍ରତିଜ୍ଞା( zTab==0 && zDb==0 );
          ଯଦି( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ଶୂନ୍ୟ*)p_ପଦସମଷ୍ଟି);
          }
          ଯାଅ lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    ଯଦି( cnt ) ଭାଙ୍ଗ;
    pNC = pNC->pNext;
    nSubquery++;
  }ଯେତେବେଳେ( pNC );


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
  ଯଦି( cnt==0 && zTab==0 ){
    ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_ID );
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, p_ଶୀର୍ଷ_NC)
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
      p_ପଦସମଷ୍ଟି->op = TK_STRING;
      memset(&p_ପଦସମଷ୍ଟି->y, 0, ଆକାର(p_ପଦସମଷ୍ଟି->y));
      ଫେରନ୍ତୁ WRC_Prune;
    }
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Id_To_True_False(p_ପଦସମଷ୍ଟି) ){
      ଫେରନ୍ତୁ WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  ପ୍ରତିଜ୍ଞା( pFJMatch==0 || cnt>0 );
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_xIsSelect|EP_Int_ମୂଲ୍ୟ) );
  ଯଦି( cnt!=1 ){
    ଅଚଳ ଅକ୍ଷର *zErr;
    ଯଦି( pFJMatch ){
      ଯଦି( pFJMatch->n_ପଦସମଷ୍ଟି==cnt-1 ){
        ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,EP_Leaf) ){
          ପଦସମଷ୍ଟି_Clear_Property(p_ପଦସମଷ୍ଟି,EP_Leaf);
        }ଅନ୍ୟଥା{
          sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ପଦସମଷ୍ଟି->p_ବାମ);
          p_ପଦସମଷ୍ଟି->p_ବାମ = 0;
          sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
          p_ପଦସମଷ୍ଟି->p_ଡାହାଣ = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, p_ପଦସମଷ୍ଟି->iColumn);
        p_ପଦସମଷ୍ଟି->op = TK_FUNCTION;
        p_ପଦସମଷ୍ଟି->u.zToken = "coalesce";
        p_ପଦସମଷ୍ଟି->x.pList = pFJMatch;
        p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି = SQLITE_AFF_DEFER;
        cnt = 1;
        ଯାଅ lookupname_end;
      }ଅନ୍ୟଥା{
        sqlite3_ପଦସମଷ୍ଟି_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    ଯଦି( zDb ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }ଅନ୍ୟଥା ଯଦି( zTab ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }ଅନ୍ୟଥା ଯଦି( cnt==0 && ପଦସମଷ୍ଟି_Has_Property(p_ଡାହାଣ,EP_DblQuoted) ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }ଅନ୍ୟଥା{
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3_Record_Error_Offset_Of_ପଦସମଷ୍ଟି(pParse->db, p_ପଦସମଷ୍ଟି);
    pParse->checkSchema = 1;
    p_ଶୀର୍ଷ_NC->nNcErr++;
    e_New_ପଦସମଷ୍ଟି_Op = TK_NULL;
  }
  ପ୍ରତିଜ୍ଞା( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ପଦସମଷ୍ଟି->p_ବାମ);
    p_ପଦସମଷ୍ଟି->p_ବାମ = 0;
    sqlite3_ପଦସମଷ୍ଟି_Delete(db, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
    p_ପଦସମଷ୍ଟି->p_ଡାହାଣ = 0;
    ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_Leaf);
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
  ଯଦି( pMatch ){
    ଯଦି( p_ପଦସମଷ୍ଟି->iColumn>=0 ){
      pMatch->colUsed |= sqlite3_ପଦସମଷ୍ଟି_Col_Used(p_ପଦସମଷ୍ଟି);
    }ଅନ୍ୟଥା{
      pMatch->fg.rowidUsed = 1;
    }
  }

  p_ପଦସମଷ୍ଟି->op = e_New_ପଦସମଷ୍ଟି_Op;
lookupname_end:
  ଯଦି( cnt==1 ){
    ପ୍ରତିଜ୍ଞା( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    ଯଦି( db->xAuth ){
      ଯଦି( pFJMatch ){
        ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op==TK_FUNCTION );
        ପ୍ରତିଜ୍ଞା( sqlite3_stricmp(p_ପଦସମଷ୍ଟି->u.zToken,"coalesce")==0 );
        ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pList==pFJMatch );
        ପ୍ରତିଜ୍ଞା( pFJMatch->n_ପଦସମଷ୍ଟି>0 );
        ପାଇଁ(i=0; i<pFJMatch->n_ପଦସମଷ୍ଟି; i++){
          ପ୍ରତିଜ୍ଞା( pFJMatch->a[i].p_ପଦସମଷ୍ଟି->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_ପଦସମଷ୍ଟି,pSchema,pNC->pSrcList);
        }
      }ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_COLUMN || p_ପଦସମଷ୍ଟି->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_ପଦସମଷ୍ଟି, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    ପାଇଁ(;;){
      ପ୍ରତିଜ୍ଞା( p_ଶୀର୍ଷ_NC!=0 );
      p_ଶୀର୍ଷ_NC->nRef++;
      ଯଦି( p_ଶୀର୍ଷ_NC==pNC ) ଭାଙ୍ଗ;
      p_ଶୀର୍ଷ_NC = p_ଶୀର୍ଷ_NC->pNext;
    }
    ଫେରନ୍ତୁ WRC_Prune;
  } ଅନ୍ୟଥା {
    ଫେରନ୍ତୁ WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
ପଦସମଷ୍ଟି *sqlite3_Create_Column_ପଦସମଷ୍ଟି(sqlite3 *db, SrcList *pSrc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iSrc, ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol){
  ପଦସମଷ୍ଟି *p = sqlite3_ପଦସମଷ୍ଟି_Alloc(db, TK_COLUMN, 0, 0);
  ଯଦି( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    ଯଦି( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }ଅନ୍ୟଥା{
      p->iColumn = (ynVar)iCol;
      ଯଦି( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }ଅନ୍ୟଥା{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  ଫେରନ୍ତୁ p;
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
ସ୍ଥିର ଶୂନ୍ୟ notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   ଅଚଳ ଅକ୍ଷର *z_ସନ୍ଦେଶ,    /* Type of error */
   ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,         /* Invalidate this expression on error */
   ପଦସମଷ୍ଟି *pError         /* Associate error with this expression */
){
  ଅଚଳ ଅକ୍ଷର *zIn = "partial index WHERE clauses";
  ଯଦି( pNC->ncFlags & NC_Idx_ପଦସମଷ୍ଟି )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  ଅନ୍ୟଥା ଯଦି( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  ଅନ୍ୟଥା ଯଦି( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3_Error_ସନ୍ଦେଶ(pParse, "%s prohibited in %s", z_ସନ୍ଦେଶ, zIn);
  ଯଦି( p_ପଦସମଷ୍ଟି ) p_ପଦସମଷ୍ଟି->op = TK_NULL;
  sqlite3_Record_Error_Offset_Of_ପଦସମଷ୍ଟି(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  ପ୍ରତିଜ୍ଞା( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_ପଦସମଷ୍ଟି|NC_GenCol))==0 ); \
  ଯଦି( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା ପଦସମଷ୍ଟି_Probability(ପଦସମଷ୍ଟି *p){
  ଦ୍ୱିଗୁଣ r = -1.0;
  ଯଦି( p->op!=TK_FLOAT ) ଫେରନ୍ତୁ -1;
  ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p, EP_Int_ମୂଲ୍ୟ) );
  sqlite3AtoF(p->u.zToken, &r);
  ପ୍ରତିଜ୍ଞା( r>=0.0 );
  ଯଦି( r>1.0 ) ଫେରନ୍ତୁ -1;
  ଫେରନ୍ତୁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
ସ୍ଥିର SQLITE_NOINLINE ଶୂନ୍ୟ resolve_Set_ପଦସମଷ୍ଟି_Subtype_Arg(ପଦସମଷ୍ଟି_List *pList){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nn, ii;
  nn = pList ? pList->n_ପଦସମଷ୍ଟି : 0;
  ପାଇଁ(ii=0; ii<nn; ii++){
    ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pList->a[ii].p_ପଦସମଷ୍ଟି;
    ଯେତେବେଳେ( 1 /*exit-by-break*/ ){
      ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_SubtArg);
      ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_SELECT ){
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) );
        ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pSelect!=0 );
        resolve_Set_ପଦସମଷ୍ଟି_Subtype_Arg(p_ପଦସମଷ୍ଟି->x.pSelect->pEList);
        ଭାଙ୍ଗ;
      }
      ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_UPLUS ){
        p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->p_ବାମ;
        ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 );
      }ଅନ୍ୟଥା{
        ଭାଙ୍ଗ;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resolve_ପଦସମଷ୍ଟି_Step(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  ପ୍ରତିଜ୍ଞା( pNC!=0 );
  pParse = pNC->pParse;
  ପ୍ରତିଜ୍ଞା( pParse==pWalker->pParse );

#ifndef NDEBUG
  ଯଦି( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    ପାଇଁ(i=0; i<pNC->pSrcList->nSrc; i++){
      ପ୍ରତିଜ୍ଞା( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  ବାଛ( p_ପଦସମଷ୍ଟି->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    ସ୍ଥିତି TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      ପ୍ରତିଜ୍ଞା( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      p_ପଦସମଷ୍ଟି->op = TK_COLUMN;
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
      p_ପଦସମଷ୍ଟି->y.pTab = pItem->pSTab;
      p_ପଦସମଷ୍ଟି->iTable = pItem->iCursor;
      p_ପଦସମଷ୍ଟି->iColumn--;
      p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି = SQLITE_AFF_INTEGER;
      ଭାଙ୍ଗ;
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
    ସ୍ଥିତି TK_NOTNULL:
    ସ୍ଥିତି TK_ISNULL: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା anRef[8];
      NameContext *p;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
      ପାଇଁ(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, p_ପଦସମଷ୍ଟି->p_ବାମ);
      ଯଦି( IN_RENAME_OBJECT ) ଫେରନ୍ତୁ WRC_Prune;
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Can_Be_Null(p_ପଦସମଷ୍ଟି->p_ବାମ) ){
        /* The expression can be NULL.  So the optimization does not apply */
        ଫେରନ୍ତୁ WRC_Prune;
      }

      ପାଇଁ(i=0, p=pNC; p; p=p->pNext, i++){
        ଯଦି( (p->ncFlags & NC_Where)==0 ){
          ଫେରନ୍ତୁ WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) );
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
#ଯଦି TREETRACE_ENABLED
      ଯଦି( sqlite3_ବୃକ୍ଷ_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_ପଦସମଷ୍ଟି->op==TK_NOTNULL
        );
        sqlite3_Show_ପଦସମଷ୍ଟି(p_ପଦସମଷ୍ଟି);
      }
#endif /* TREETRACE_ENABLED */
      p_ପଦସମଷ୍ଟି->u.i_ମୂଲ୍ୟ = (p_ପଦସମଷ୍ଟି->op==TK_NOTNULL);
      p_ପଦସମଷ୍ଟି->flags |= EP_Int_ମୂଲ୍ୟ;
      p_ପଦସମଷ୍ଟି->op = TK_INTEGER;
      ପାଇଁ(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        p->nRef = anRef[i];
      }
      sqlite3_ପଦସମଷ୍ଟି_Delete(pParse->db, p_ପଦସମଷ୍ଟି->p_ବାମ);
      p_ପଦସମଷ୍ଟି->p_ବାମ = 0;
      ଫେରନ୍ତୁ WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    ସ୍ଥିତି TK_ID:
    ସ୍ଥିତି TK_DOT: {
      ଅଚଳ ଅକ୍ଷର *zTable;
      ଅଚଳ ଅକ୍ଷର *zDb;
      ପଦସମଷ୍ଟି *p_ଡାହାଣ;

      ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
        p_ଡାହାଣ = p_ପଦସମଷ୍ଟି;
      }ଅନ୍ୟଥା{
        ପଦସମଷ୍ଟି *p_ବାମ = p_ପଦସମଷ୍ଟି->p_ବାମ;
        testcase( pNC->ncFlags & NC_Idx_ପଦସମଷ୍ଟି );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_ପଦସମଷ୍ଟି|NC_GenCol, 0, p_ପଦସମଷ୍ଟି);
        p_ଡାହାଣ = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
        ଯଦି( p_ଡାହାଣ->op==TK_ID ){
          zDb = 0;
        }ଅନ୍ୟଥା{
          ପ୍ରତିଜ୍ଞା( p_ଡାହାଣ->op==TK_DOT );
          ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ଡାହାଣ, EP_Int_ମୂଲ୍ୟ) );
          zDb = p_ବାମ->u.zToken;
          p_ବାମ = p_ଡାହାଣ->p_ବାମ;
          p_ଡାହାଣ = p_ଡାହାଣ->p_ଡାହାଣ;
        }
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_UToken(p_ବାମ) && ପଦସମଷ୍ଟି_Use_UToken(p_ଡାହାଣ) );
        zTable = p_ବାମ->u.zToken;
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
        ଯଦି( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (ଶୂନ୍ୟ*)p_ପଦସମଷ୍ଟି, (ଶୂନ୍ୟ*)p_ଡାହାଣ);
          sqlite3RenameTokenRemap(pParse, (ଶୂନ୍ୟ*)&p_ପଦସମଷ୍ଟି->y.pTab, (ଶୂନ୍ୟ*)p_ବାମ);
        }
      }
      ଫେରନ୍ତୁ lookupName(pParse, zDb, zTable, p_ଡାହାଣ, pNC, p_ପଦସମଷ୍ଟି);
    }

    /* Resolve function names
    */
    ସ୍ଥିତି TK_FUNCTION: {
      ପଦସମଷ୍ଟି_List *pList;            /* The argument list */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;                      /* Number of arguments */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା no_such_func = 0;       /* True if no such function exists */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା wrong_ଅଙ୍କ_args = 0;     /* True if wrong number of arguments */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_agg = 0;             /* True if is an aggregate function */
      ଅଚଳ ଅକ୍ଷର *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(p_ପଦସମଷ୍ଟି) ? p_ପଦସମଷ୍ଟି->y.pWin : 0);
#endif
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_xIsSelect|EP_Int_ମୂଲ୍ୟ) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ==0 || p_ପଦସମଷ୍ଟି->p_ବାମ->op==TK_ORDER );
      pList = p_ପଦସମଷ୍ଟି->x.pList;
      n = pList ? pList->n_ପଦସମଷ୍ଟି : 0;
      zId = p_ପଦସମଷ୍ଟି->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      ଯଦି( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        ଯଦି( pDef==0 ){
          no_such_func = 1;
        }ଅନ୍ୟଥା{
          wrong_ଅଙ୍କ_args = 1;
        }
      }ଅନ୍ୟଥା{
        is_agg = pDef->xFinalize!=0;
        ଯଦି( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_Unlikely);
          ଯଦି( n==2 ){
            p_ପଦସମଷ୍ଟି->iTable = ପଦସମଷ୍ଟି_Probability(pList->a[1].p_ପଦସମଷ୍ଟି);
            ଯଦି( p_ପଦସମଷ୍ଟି->iTable<0 ){
              sqlite3_Error_ସନ୍ଦେଶ(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_ପଦସମଷ୍ଟି);
              pNC->nNcErr++;
            }
          }ଅନ୍ୟଥା{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_ପଦସମଷ୍ଟି->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          ପୂର୍ଣ୍ଣସଂଖ୍ୟା auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          ଯଦି( auth!=SQLITE_OK ){
            ଯଦି( auth==SQLITE_DENY ){
              sqlite3_Error_ସନ୍ଦେଶ(pParse, "not authorized to use function: %#T",
                                      p_ପଦସମଷ୍ଟି);
              pNC->nNcErr++;
            }
            p_ପଦସମଷ୍ଟି->op = TK_NULL;
            ଫେରନ୍ତୁ WRC_Prune;
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
        ଯଦି( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_SubtArg) 
        ){
          resolve_Set_ପଦସମଷ୍ଟି_Subtype_Arg(pList);
        }

        ଯଦି( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି,EP_ConstFunc);
        }
        ଯଦି( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_ପଦସମଷ୍ଟି|NC_PartIdx|NC_GenCol, 0, p_ପଦସମଷ୍ଟି);
        }ଅନ୍ୟଥା{
          ପ୍ରତିଜ୍ଞା( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_ପଦସମଷ୍ଟି->op2 = pNC->ncFlags & NC_SelfRef;
        }
        ଯଦି( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
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
        }ଅନ୍ୟଥା
        ଯଦି( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          ଯଦି( pNC->ncFlags & NC_FromDDL ) ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_FromDDL);
          sqlite3_ପଦସମଷ୍ଟି_Function_Usable(pParse, p_ପଦସମଷ୍ଟି, pDef);
        }
      }

      ଯଦି( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        ପ୍ରତିଜ୍ଞା( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_ମୂଲ୍ୟ==0 && pDef->xInverse==0)
          || (pDef->x_ମୂଲ୍ୟ && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        ଯଦି( pDef && pDef->x_ମୂଲ୍ୟ==0 && pWin ){
          sqlite3_Error_ସନ୍ଦେଶ(pParse,
              "%#T() may not be used as a window function", p_ପଦସମଷ୍ଟି
          );
          pNC->nNcErr++;
        }ଅନ୍ୟଥା ଯଦି(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          ଅଚଳ ଅକ୍ଷର *zType;
          ଯଦି( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }ଅନ୍ୟଥା{
            zType = "aggregate";
          }
          sqlite3_Error_ସନ୍ଦେଶ(pParse, "misuse of %s function %#T()",zType,p_ପଦସମଷ୍ଟି);
          pNC->nNcErr++;
          is_agg = 0;
        }
#ଅନ୍ୟଥା
        ଯଦି( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_ସନ୍ଦେଶ(pParse,"misuse of aggregate function %#T()",p_ପଦସମଷ୍ଟି);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        ଅନ୍ୟଥା ଯଦି( no_such_func
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
          sqlite3_Error_ସନ୍ଦେଶ(pParse, "no such function: %#T", p_ପଦସମଷ୍ଟି);
          pNC->nNcErr++;
        }ଅନ୍ୟଥା ଯଦି( wrong_ଅଙ୍କ_args ){
          sqlite3_Error_ସନ୍ଦେଶ(pParse,"wrong number of arguments to function %#T()",
               p_ପଦସମଷ୍ଟି);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        ଅନ୍ୟଥା ଯଦି( is_agg==0 && ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
          sqlite3_Error_ସନ୍ଦେଶ(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_ପଦସମଷ୍ଟି
          );
          pNC->nNcErr++;
        }
#endif
        ଅନ୍ୟଥା ଯଦି( is_agg==0 && p_ପଦସମଷ୍ଟି->p_ବାମ ){
          sqlite3_ପଦସମଷ୍ଟି_Order_By_Aggregate_Error(pParse, p_ପଦସମଷ୍ଟି);
          pNC->nNcErr++;
        }
        ଯଦି( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#ଅନ୍ୟଥା
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      ଅନ୍ୟଥା ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) || p_ପଦସମଷ୍ଟି->p_ବାମ ){
        is_agg = 1;
      }
      sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pList);
      ଯଦି( is_agg ){
        ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ ){
          ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ->op==TK_ORDER );
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି->p_ବାମ) );
          sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, p_ପଦସମଷ୍ଟି->p_ବାମ->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        ଯଦି( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YWin(p_ପଦସମଷ୍ଟି) && pWin==p_ପଦସମଷ୍ଟି->y.pWin );
          ଯଦି( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            ଯଦି( pParse->db->mallocFailed ) ଭାଙ୍ଗ;
          }
          sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pWin->pPartition);
          sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pWin->pOrderBy);
          sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }ଅନ୍ୟଥା
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_ପଦସମଷ୍ଟି->op = TK_AGG_FUNCTION;
          p_ପଦସମଷ୍ଟି->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
            sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, p_ପଦସମଷ୍ଟି->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          ଯେତେବେଳେ( pNC2
              && sqlite3ReferencesSrcList(pParse, p_ପଦସମଷ୍ଟି, pNC2->pSrcList)==0
          ){
            p_ପଦସମଷ୍ଟି->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }
          ପ୍ରତିଜ୍ଞା( pDef!=0 || IN_RENAME_OBJECT );
          ଯଦି( pNC2 && pDef ){
            p_ପଦସମଷ୍ଟି->op2 += pNC2->nNestedSelect;
            ପ୍ରତିଜ୍ଞା( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            ପ୍ରତିଜ୍ଞା( SQLITE_FUNC_ANYORDER==NC_OrderAgg );
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
      ଫେରନ୍ତୁ WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ସ୍ଥିତି TK_EXISTS:
    ସ୍ଥିତି TK_SELECT:
#endif
    ସ୍ଥିତି TK_IN: {
      testcase( p_ପଦସମଷ୍ଟି->op==TK_IN );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_EXISTS );
      testcase( p_ପଦସମଷ୍ଟି->op==TK_SELECT );
      ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_ପଦସମଷ୍ଟି );
        testcase( pNC->ncFlags & NC_GenCol );
        ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pSelect );
        ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_EXISTS )  pParse->bHasExists = 1;
        ଯଦି( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_ପଦସମଷ୍ଟି, p_ପଦସମଷ୍ଟି);
        }ଅନ୍ୟଥା{
          sqlite3WalkSelect(pWalker, p_ପଦସମଷ୍ଟି->x.pSelect);
        }
        ପ୍ରତିଜ୍ଞା( pNC->nRef>=nRef );
        ଯଦି( nRef!=pNC->nRef ){
          ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, EP_VarSelect);
          p_ପଦସମଷ୍ଟି->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_ପଦସମଷ୍ଟି );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_ପଦସମଷ୍ଟି|NC_GenCol, p_ପଦସମଷ୍ଟି, p_ପଦସମଷ୍ଟି);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_IS:
    ସ୍ଥିତି TK_ISNOT: {
      ପଦସମଷ୍ଟି *p_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      ଯଦି( ALWAYS(p_ଡାହାଣ) && (p_ଡାହାଣ->op==TK_ID || p_ଡାହାଣ->op==TK_TRUEFALSE) ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = resolve_ପଦସମଷ୍ଟି_Step(pWalker, p_ଡାହାଣ);
        ଯଦି( rc==WRC_Abort ) ଫେରନ୍ତୁ WRC_Abort;
        ଯଦି( p_ଡାହାଣ->op==TK_TRUEFALSE ){
          p_ପଦସମଷ୍ଟି->op2 = p_ପଦସମଷ୍ଟି->op;
          p_ପଦସମଷ୍ଟି->op = TK_TRUTH;
          ଫେରନ୍ତୁ WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    ସ୍ଥିତି TK_BETWEEN:
    ସ୍ଥିତି TK_EQ:
    ସ୍ଥିତି TK_NE:
    ସ୍ଥିତି TK_LT:
    ସ୍ଥିତି TK_LE:
    ସ୍ଥିତି TK_GT:
    ସ୍ଥିତି TK_GE: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_ବାମ, n_ଡାହାଣ;
      ଯଦି( pParse->db->mallocFailed ) ଭାଙ୍ଗ;
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ବାମ!=0 );
      n_ବାମ = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି->p_ବାମ);
      ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_BETWEEN ){
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
        n_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି->x.pList->a[0].p_ପଦସମଷ୍ଟି);
        ଯଦି( n_ଡାହାଣ==n_ବାମ ){
          n_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି->x.pList->a[1].p_ପଦସମଷ୍ଟି);
        }
      }ଅନ୍ୟଥା{
        ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ଡାହାଣ!=0 );
        n_ଡାହାଣ = sqlite3_ପଦସମଷ୍ଟି_Vector_Size(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
      }
      ଯଦି( n_ବାମ!=n_ଡାହାଣ ){
        testcase( p_ପଦସମଷ୍ଟି->op==TK_EQ );
        testcase( p_ପଦସମଷ୍ଟି->op==TK_NE );
        testcase( p_ପଦସମଷ୍ଟି->op==TK_LT );
        testcase( p_ପଦସମଷ୍ଟି->op==TK_LE );
        testcase( p_ପଦସମଷ୍ଟି->op==TK_GT );
        testcase( p_ପଦସମଷ୍ଟି->op==TK_GE );
        testcase( p_ପଦସମଷ୍ଟି->op==TK_IS );
        testcase( p_ପଦସମଷ୍ଟି->op==TK_ISNOT );
        testcase( p_ପଦସମଷ୍ଟି->op==TK_BETWEEN );
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "row value misused");
        sqlite3_Record_Error_Offset_Of_ପଦସମଷ୍ଟି(pParse->db, p_ପଦସମଷ୍ଟି);
      }
      ଭାଙ୍ଗ;
    }
  }
  ପ୍ରତିଜ୍ଞା( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ଫେରନ୍ତୁ pParse->nErr ? WRC_Abort : WRC_Continue;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  ପଦସମଷ୍ଟି_List *pEList,  /* List of expressions to scan */
  ପଦସମଷ୍ଟି *pE           /* Expression we are trying to match */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  ଯଦି( pE->op==TK_ID ){
    ଅଚଳ ଅକ୍ଷର *zCol;
    ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(pE, EP_Int_ମୂଲ୍ୟ) );
    zCol = pE->u.zToken;
    ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
      ଯଦି( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        ଫେରନ୍ତୁ i+1;
      }
    }
  }
  ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resolve_Order_By_Term_To_ପଦସମଷ୍ଟି_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  ପଦସମଷ୍ଟି *pE           /* The specific ORDER BY term */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;             /* Loop counter */
  ପଦସମଷ୍ଟି_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  ପ୍ରତିଜ୍ଞା( sqlite3_ପଦସମଷ୍ଟି_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, ଆକାର(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&nc, pE);
  db->suppressErr = savedSuppErr;
  ଯଦି( rc ) ଫେରନ୍ତୁ 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  ପାଇଁ(i=0; i<pEList->n_ପଦସମଷ୍ଟି; i++){
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(0, pEList->a[i].p_ପଦସମଷ୍ଟି, pE, -1)<2 ){
      ଫେରନ୍ତୁ i+1;
    }
  }

  /* If no match, return 0. */
  ଫେରନ୍ତୁ 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
ସ୍ଥିର ଶୂନ୍ୟ resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  ଅଚଳ ଅକ୍ଷର *zType,     /* "ORDER" or "GROUP" */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i,                 /* The index (1-based) of the term out of range */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା mx,                /* Largest permissible value of i */
  ପଦସମଷ୍ଟି *pError           /* Associate the error with the expression */
){
  sqlite3_Error_ସନ୍ଦେଶ(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_Record_Error_Offset_Of_ପଦସମଷ୍ଟି(pParse->db, pError);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପଦସମଷ୍ଟି_List *pOrderBy;
  ପଦସମଷ୍ଟି_List *pEList;
  sqlite3 *db;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  ଯଦି( pOrderBy==0 ) ଫେରନ୍ତୁ 0;
  db = pParse->db;
  ଯଦି( pOrderBy->n_ପଦସମଷ୍ଟି>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many terms in ORDER BY clause");
    ଫେରନ୍ତୁ 1;
  }
  ପାଇଁ(i=0; i<pOrderBy->n_ପଦସମଷ୍ଟି; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  ଯେତେବେଳେ( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  ଯେତେବେଳେ( pSelect && moreToDo ){
    ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    ପ୍ରତିଜ୍ଞା( pEList!=0 );
    ପାଇଁ(i=0, pItem=pOrderBy->a; i<pOrderBy->n_ପଦସମଷ୍ଟି; i++, pItem++){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol = -1;
      ପଦସମଷ୍ଟି *pE, *pDup;
      ଯଦି( pItem->fg.done ) ଜାରି;
      pE = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(pItem->p_ପଦସମଷ୍ଟି);
      ଯଦି( NEVER(pE==0) ) ଜାରି;
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Integer(pE, &iCol, 0, 1) ){
        ଯଦି( iCol<=0 || iCol>pEList->n_ପଦସମଷ୍ଟି ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->n_ପଦସମଷ୍ଟି, pE);
          ଫେରନ୍ତୁ 1;
        }
      }ଅନ୍ୟଥା{
        iCol = resolveAsName(pParse, pEList, pE);
        ଯଦି( iCol==0 ){
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
          pDup = sqlite3_ପଦସମଷ୍ଟି_Dup(db, pE, 0);
          ଯଦି( !db->mallocFailed ){
            ପ୍ରତିଜ୍ଞା(pDup);
            iCol = resolve_Order_By_Term_To_ପଦସମଷ୍ଟି_List(pParse, pSelect, pDup);
            ଯଦି( IN_RENAME_OBJECT && iCol>0 ){
              resolve_Order_By_Term_To_ପଦସମଷ୍ଟି_List(pParse, pSelect, pE);
            }
          }
          sqlite3_ପଦସମଷ୍ଟି_Delete(db, pDup);
        }
      }
      ଯଦି( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        ଯଦି( !IN_RENAME_OBJECT ){
          ପଦସମଷ୍ଟି *pNew = sqlite3_ପଦସମଷ୍ଟି_Int32(db, iCol);
          ଯଦି( pNew==0 ) ଫେରନ୍ତୁ 1;
          ଯଦି( pItem->p_ପଦସମଷ୍ଟି==pE ){
            pItem->p_ପଦସମଷ୍ଟି = pNew;
          }ଅନ୍ୟଥା{
            ପଦସମଷ୍ଟି *pParent = pItem->p_ପଦସମଷ୍ଟି;
            ପ୍ରତିଜ୍ଞା( pParent->op==TK_COLLATE );
            ଯେତେବେଳେ( pParent->p_ବାମ->op==TK_COLLATE ) pParent = pParent->p_ବାମ;
            ପ୍ରତିଜ୍ଞା( pParent->p_ବାମ==pE );
            pParent->p_ବାମ = pNew;
          }
          sqlite3_ପଦସମଷ୍ଟି_Delete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }ଅନ୍ୟଥା{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  ପାଇଁ(i=0; i<pOrderBy->n_ପଦସମଷ୍ଟି; i++){
    ଯଦି( pOrderBy->a[i].fg.done==0 ){
      sqlite3_Error_ସନ୍ଦେଶ(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      ଫେରନ୍ତୁ 1;
    }
  }
  ଫେରନ୍ତୁ 0;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  ପଦସମଷ୍ଟି_List *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  ଅଚଳ ଅକ୍ଷର *zType     /* "ORDER" or "GROUP" */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  sqlite3 *db = pParse->db;
  ପଦସମଷ୍ଟି_List *pEList;
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;

  ଯଦି( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) ଫେରନ୍ତୁ 0;
  ଯଦି( pOrderBy->n_ପଦସମଷ୍ଟି>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ସନ୍ଦେଶ(pParse, "too many terms in %s BY clause", zType);
    ଫେରନ୍ତୁ 1;
  }
  pEList = pSelect->pEList;
  ପ୍ରତିଜ୍ଞା( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  ପାଇଁ(i=0, pItem=pOrderBy->a; i<pOrderBy->n_ପଦସମଷ୍ଟି; i++, pItem++){
    ଯଦି( pItem->u.x.iOrderByCol ){
      ଯଦି( pItem->u.x.iOrderByCol>pEList->n_ପଦସମଷ୍ଟି ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->n_ପଦସମଷ୍ଟି, 0);
        ଫେରନ୍ତୁ 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->p_ପଦସମଷ୍ଟି,0);
    }
  }
  ଫେରନ୍ତୁ 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resolveRemoveWindowsCb(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  UNUSED_PARAMETER(pWalker);
  ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
    Window *pWin = p_ପଦସମଷ୍ଟି->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
ସ୍ଥିର ଶୂନ୍ୟ window_Remove_ପଦସମଷ୍ଟି_From_Select(Select *pSelect, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଯଦି( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, ଆକାର(Walker));
    sWalker.x_ପଦସମଷ୍ଟି_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_ପଦସମଷ୍ଟି(&sWalker, p_ପଦସମଷ୍ଟି);
  }
}
#ଅନ୍ୟଥା
# define window_Remove_ପଦସମଷ୍ଟି_From_Select(a, b)
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  ପଦସମଷ୍ଟି_List *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  ଅଚଳ ଅକ୍ଷର *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;                      /* Loop counters */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା iCol;                      /* Column number */
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nResult;                   /* Number of terms in the result set */

  ପ୍ରତିଜ୍ଞା( pOrderBy!=0 );
  nResult = pSelect->pEList->n_ପଦସମଷ୍ଟି;
  pParse = pNC->pParse;
  ପାଇଁ(i=0, pItem=pOrderBy->a; i<pOrderBy->n_ପଦସମଷ୍ଟି; i++, pItem++){
    ପଦସମଷ୍ଟି *pE = pItem->p_ପଦସମଷ୍ଟି;
    ପଦସମଷ୍ଟି *pE2 = sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(pE);
    ଯଦି( NEVER(pE2==0) ) ଜାରି;
    ଯଦି( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      ଯଦି( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        ଜାରି;
      }
    }
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Is_Integer(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      ଯଦି( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult, pE2);
        ଫେରନ୍ତୁ 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      ଜାରି;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    ଯଦି( sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(pNC, pE) ){
      ଫେରନ୍ତୁ 1;
    }
    ପାଇଁ(j=0; j<pSelect->pEList->n_ପଦସମଷ୍ଟି; j++){
      ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Compare(0, pE, pSelect->pEList->a[j].p_ପଦସମଷ୍ଟି, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        window_Remove_ପଦସମଷ୍ଟି_From_Select(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  ଫେରନ୍ତୁ sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା isCompound;         /* True if p is a compound select */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;                  /* Loop counter */
  ପଦସମଷ୍ଟି_List *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  ପ୍ରତିଜ୍ଞା( p!=0 );
  ଯଦି( p->selFlags & SF_Resolved ){
    ଫେରନ୍ତୁ WRC_Prune;
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
  ଯଦି( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    ଫେରନ୍ତୁ pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  ଯେତେବେଳେ( p ){
    ପ୍ରତିଜ୍ଞା( (p->selFlags & SF_Expanded)!=0 );
    ପ୍ରତିଜ୍ଞା( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, ଆକାର(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    ଯଦି( sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, p->pLimit) ){
      ଫେରନ୍ତୁ WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    ଯଦି( p->selFlags & SF_Converted ){
      Select *pSub;
      ପ୍ରତିଜ୍ଞା( p->pSrc->a[0].fg.isSubquery );
      ପ୍ରତିଜ୍ଞା( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ପ୍ରତିଜ୍ଞା( pSub!=0 );
      ପ୍ରତିଜ୍ଞା( p->pSrc->nSrc==1 && p->pOrderBy );
      ପ୍ରତିଜ୍ଞା( pSub->pPrior && pSub->pOrderBy==0 );
      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    ଯଦି( pOuterNC ) pOuterNC->nNestedSelect++;
    ପାଇଁ(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      ପ୍ରତିଜ୍ଞା( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      ଯଦି( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା nRef = pOuterNC ? pOuterNC->nRef : 0;
        ଅଚଳ ଅକ୍ଷର *zSavedContext = pParse->zAuthContext;

        ଯଦି( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ WRC_Abort;
        ପ୍ରତିଜ୍ଞା( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        ଯଦି( pOuterNC ){
          ପ୍ରତିଜ୍ଞା( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    ଯଦି( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
 
    /* Resolve names in the result set. */
    ଯଦି( sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, p->pEList) ) ଫେରନ୍ତୁ WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    ପ୍ରତିଜ୍ଞା( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    ଯଦି( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      ପ୍ରତିଜ୍ଞା( NC_MinMaxAgg==SF_MinMaxAgg );
      ପ୍ରତିଜ୍ଞା( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }ଅନ୍ୟଥା{
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
    ପ୍ରତିଜ୍ଞା( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    ଯଦି( p->pHaving ){
      ଯଦି( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_ସନ୍ଦେଶ(pParse, "HAVING clause on a non-aggregate query");
        ଫେରନ୍ତୁ WRC_Abort;
      }
      ଯଦି( sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, p->pHaving) ) ଫେରନ୍ତୁ WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    ଯଦି( sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, p->pWhere) ) ଫେରନ୍ତୁ WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    ପାଇଁ(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      ଯଦି( pItem->fg.isTabFunc
       && sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, pItem->u1.pFuncArg)
      ){
        ଫେରନ୍ତୁ WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( IN_RENAME_OBJECT ){
      Window *pWin;
      ପାଇଁ(pWin=p->pWinDefn; pWin; pWin=pWin->pNextWin){
        ଯଦି( sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, pWin->pOrderBy)
         || sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, pWin->pPartition)
        ){
          ଫେରନ୍ତୁ WRC_Abort;
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
    ଯଦି( p->selFlags & SF_Converted ){
      Select *pSub;
      ପ୍ରତିଜ୍ଞା( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ପ୍ରତିଜ୍ଞା( pSub!=0 );
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
    ଯଦି( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      ଫେରନ୍ତୁ WRC_Abort;
    }
    ଯଦି( db->mallocFailed ){
      ଫେରନ୍ତୁ WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    ଯଦି( pGroupBy ){
      ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
   
      ଯଦି( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        ଫେରନ୍ତୁ WRC_Abort;
      }
      ପାଇଁ(i=0, pItem=pGroupBy->a; i<pGroupBy->n_ପଦସମଷ୍ଟି; i++, pItem++){
        ଯଦି( ପଦସମଷ୍ଟି_Has_Property(pItem->p_ପଦସମଷ୍ଟି, EP_Agg) ){
          sqlite3_Error_ସନ୍ଦେଶ(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          ଫେରନ୍ତୁ WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    ଯଦି( p->pNext && p->pEList->n_ପଦସମଷ୍ଟି!=p->pNext->pEList->n_ପଦସମଷ୍ଟି ){
      sqlite3_Select_Wrong_ଅଙ୍କ_Terms_Error(pParse, p->pNext);
      ଫେରନ୍ତୁ WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    ଯଦି( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      ଯଦି( pParse->nErr ) ଫେରନ୍ତୁ WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  ଯଦି( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    ଫେରନ୍ତୁ WRC_Abort;
  }

  ଫେରନ୍ତୁ WRC_Prune;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି             /* The expression to be analyzed. */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା savedHasAgg;
  Walker w;

  ଯଦି( p_ପଦସମଷ୍ଟି==0 ) ଫେରନ୍ତୁ SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.x_ପଦସମଷ୍ଟି_Callback = resolve_ପଦସମଷ୍ଟି_Step;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
  w.pParse->nHeight += p_ପଦସମଷ୍ଟି->nHeight;
  ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Check_Height(w.pParse, w.pParse->nHeight) ){
    ଫେରନ୍ତୁ SQLITE_ERROR;
  }
#endif
  ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି!=0 );
  sqlite3_Walk_ପଦସମଷ୍ଟି_NN(&w, p_ପଦସମଷ୍ଟି);
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
  w.pParse->nHeight -= p_ପଦସମଷ୍ଟି->nHeight;
#endif
  ପ୍ରତିଜ୍ଞା( EP_Agg==NC_HasAgg );
  ପ୍ରତିଜ୍ଞା( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  ଫେରନ୍ତୁ pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  ପଦସମଷ୍ଟି_List *pList         /* The expression list to be analyzed. */
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା savedHasAgg = 0;
  Walker w;
  ଯଦି( pList==0 ) ଫେରନ୍ତୁ SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_ପଦସମଷ୍ଟି_Callback = resolve_ପଦସମଷ୍ଟି_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  ପାଇଁ(i=0; i<pList->n_ପଦସମଷ୍ଟି; i++){
    ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି = pList->a[i].p_ପଦସମଷ୍ଟି;
    ଯଦି( p_ପଦସମଷ୍ଟି==0 ) ଜାରି;
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
    w.pParse->nHeight += p_ପଦସମଷ୍ଟି->nHeight;
    ଯଦି( sqlite3_ପଦସମଷ୍ଟି_Check_Height(w.pParse, w.pParse->nHeight) ){
      ଫେରନ୍ତୁ SQLITE_ERROR;
    }
#endif
    sqlite3_Walk_ପଦସମଷ୍ଟି_NN(&w, p_ପଦସମଷ୍ଟି);
#ଯଦି SQLITE_MAX_ପଦସମଷ୍ଟି_DEPTH>0
    w.pParse->nHeight -= p_ପଦସମଷ୍ଟି->nHeight;
#endif
    ପ୍ରତିଜ୍ଞା( EP_Agg==NC_HasAgg );
    ପ୍ରତିଜ୍ଞା( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    ଯଦି( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      ପଦସମଷ୍ଟି_Set_Property(p_ପଦସମଷ୍ଟି, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    ଯଦି( w.pParse->nErr>0 ) ଫେରନ୍ତୁ SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  ଫେରନ୍ତୁ SQLITE_OK;
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
ଶୂନ୍ୟ sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  ପ୍ରତିଜ୍ଞା( p!=0 );
  w.x_ପଦସମଷ୍ଟି_Callback = resolve_ପଦସମଷ୍ଟି_Step;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,     /* Expression to resolve.  May be NULL. */
  ପଦସମଷ୍ଟି_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ସଂଘ {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  ପ୍ରତିଜ୍ଞା( type==0 || pTab!=0 );
  ପ୍ରତିଜ୍ଞା( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_ପଦସମଷ୍ଟି
          || type==NC_GenCol || pTab==0 );
  memset(&sNC, 0, ଆକାର(sNC));
  memset(&uSrc, 0, ଆକାର(uSrc));
  pSrc = &uSrc.sSrc;
  ଯଦି( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    ଯଦି( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  ଯଦି( (rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_Names(&sNC, p_ପଦସମଷ୍ଟି))!=SQLITE_OK ) ଫେରନ୍ତୁ rc;
  ଯଦି( pList ) rc = sqlite3_Resolve_ପଦସମଷ୍ଟି_List_Names(&sNC, pList);
  ଫେରନ୍ତୁ rc;
}
