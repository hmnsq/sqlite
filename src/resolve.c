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
#ವ್ಯಾಖ್ಯಾನಿಸು EXCLUDED_ಕೋಷ್ಟಕ_ಸಂಖ್ಯೆ  2

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ incr_Agg_ಆಳ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION ) p_ಅಭಿವ್ಯಕ್ತಿ->op2 += pWalker->u.n;
  ಹಿಂತಿರುಗಿ WRC_Continue;
}
ಸ್ಥಿರ ಶೂನ್ಯ incr_Agg_Function_ಆಳ(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, ಪೂರ್ಣಾಂಕ N){
  ಆದರೆ( N>0 ){
    Walker w;
    memset(&w, 0, ಗಾತ್ರ(w));
    w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = incr_Agg_ಆಳ;
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
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು,              /* A column in the result set.  0..pEList->nExpr-1 */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,           /* Transform this into an alias to the result set */
  ಪೂರ್ಣಾಂಕ nSubquery          /* Number of subqueries that the label is moving */
){
  ಅಭಿವ್ಯಕ್ತಿ *pOrig;           /* The iCol-th column of the result set */
  ಅಭಿವ್ಯಕ್ತಿ *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  ಪ್ರತಿಜ್ಞಾ( i_ಲಂಬಸಾಲು>=0 && i_ಲಂಬಸಾಲು<pEList->n_ಅಭಿವ್ಯಕ್ತಿ );
  pOrig = pEList->a[i_ಲಂಬಸಾಲು].p_ಅಭಿವ್ಯಕ್ತಿ;
  ಪ್ರತಿಜ್ಞಾ( pOrig!=0 );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced|EP_TokenOnly) );
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo ) ಹಿಂತಿರುಗಿ;
  db = pParse->db;
  pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Dup(db, pOrig, 0);
  ಆದರೆ( db->mallocFailed ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDup);
    pDup = 0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಅಭಿವ್ಯಕ್ತಿ ತಾತ್ಕಾಲಿಕ;
    incr_Agg_Function_ಆಳ(pDup, nSubquery);
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_COLLATE ){
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      pDup = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕೂಡಿಸು_Collate_String(pParse, pDup, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
    }
    ಸ್ಮರಣೆ_ನಕಲು(&ತಾತ್ಕಾಲಿಕ, pDup, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
    ಸ್ಮರಣೆ_ನಕಲು(pDup, p_ಅಭಿವ್ಯಕ್ತಿ, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
    ಸ್ಮರಣೆ_ನಕಲು(p_ಅಭಿವ್ಯಕ್ತಿ, &ತಾತ್ಕಾಲಿಕ, ಗಾತ್ರ(ಅಭಿವ್ಯಕ್ತಿ));
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
      ಆದರೆ( ALWAYS(p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin!=0) ){
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
ಪೂರ್ಣಾಂಕ sqlite3_ಹೊಂದಿಕೆ_EName(
  ಸ್ಥಿರಾಂಕ ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,
  ಪೂರ್ಣಾಂಕ *pbRowid
){
  ಪೂರ್ಣಾಂಕ n;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಹರವು;
  ಪೂರ್ಣಾಂಕ eEName = p_ಅಂಶ->fg.eEName;
  ಆದರೆ( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  ಪ್ರತಿಜ್ಞಾ( pbRowid==0 || *pbRowid==0 );
  z_ಹರವು = p_ಅಂಶ->zEName;
  ಗಾಗಿ(n=0; ALWAYS(z_ಹರವು[n]) && z_ಹರವು[n]!='.'; n++){}
  ಆದರೆ( zDb && (sqlite3StrNICmp(z_ಹರವು, zDb, n)!=0 || zDb[n]!=0) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  z_ಹರವು += n+1;
  ಗಾಗಿ(n=0; ALWAYS(z_ಹರವು[n]) && z_ಹರವು[n]!='.'; n++){}
  ಆದರೆ( zTab && (sqlite3StrNICmp(z_ಹರವು, zTab, n)!=0 || zTab[n]!=0) ){
    ಹಿಂತಿರುಗಿ 0;
  }
  z_ಹರವು += n+1;
  ಆದರೆ( z_ಲಂಬಸಾಲು ){
    ಆದರೆ( eEName==ENAME_TAB && sqlite3StrICmp(z_ಹರವು, z_ಲಂಬಸಾಲು)!=0 ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( eEName==ENAME_ROWID && sqlite3IsRowid(z_ಲಂಬಸಾಲು)==0 ) ಹಿಂತಿರುಗಿ 0;
  }
  ಆದರೆ( eEName==ENAME_ROWID ) *pbRowid = 1;
  ಹಿಂತಿರುಗಿ 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ are_Double_Quoted_ಅಕ್ಷರಸರಣಿಗಳು_Enabled(sqlite3 *db, NameContext *p_ತುದಿ_NC){
  ಆದರೆ( db->init.busy ) ಹಿಂತಿರುಗಿ 1;  /* Always support for legacy schemas */
  ಆದರೆ( p_ತುದಿ_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    ಆದರೆ( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      ಹಿಂತಿರುಗಿ 1;
    }
    ಹಿಂತಿರುಗಿ (db->flags & SQLITE_DqsDDL)!=0;
  }ಇಲ್ಲದಿದ್ದರೆ{
    /* Currently parsing a DML statement */
    ಹಿಂತಿರುಗಿ (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಲಂಬಸಾಲು_ಬಳಸಿದ(ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ n;
  ಕೋಷ್ಟಕ *pExTab;

  n = p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ;
  ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
  pExTab = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab;
  ಪ್ರತಿಜ್ಞಾ( pExTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( n < pExTab->n_ಲಂಬಸಾಲು );
  ಆದರೆ( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->a_ಲಂಬಸಾಲು[n].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->n_ಲಂಬಸಾಲು==BMS-1 );
    testcase( pExTab->n_ಲಂಬಸಾಲು==BMS );
    ಹಿಂತಿರುಗಿ pExTab->n_ಲಂಬಸಾಲು>=BMS ? ALLBITS : MASKBIT(pExTab->n_ಲಂಬಸಾಲು)-1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    ಆದರೆ( n>=BMS ) n = BMS-1;
    ಹಿಂತಿರುಗಿ ((Bitmask)1)<<n;
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
  Src_ಅಂಶ *p_ಹೊಂದಿಕೆ,        /* Source table containing the column */
  i16 i_ಸ್ತಂಭ             /* The column number */
){
  ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(pParse->db, TK_ಸ್ತಂಭ, 0, 0);
  ಆದರೆ( pNew ){
    pNew->i_ಕೋಷ್ಟಕ = p_ಹೊಂದಿಕೆ->iCursor;
    pNew->i_ಸ್ತಂಭ = i_ಸ್ತಂಭ;
    pNew->y.pTab = p_ಹೊಂದಿಕೆ->pSTab;
    ಪ್ರತಿಜ್ಞಾ( (p_ಹೊಂದಿಕೆ->fg.jointype & (JT_ಎಡ|JT_LTORJ))!=0 );
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ is_Valid_Schema_ಕೋಷ್ಟಕ_Name(
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab,         /* Name as it appears in the SQL */
  ಕೋಷ್ಟಕ *pTab,              /* The schema table we are trying to match */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb           /* non-NULL if a database qualifier is present */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLegacy;
  ಪ್ರತಿಜ್ಞಾ( pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( pTab->tnum==1 );
  ಆದರೆ( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) ಹಿಂತಿರುಗಿ 0;
  zLegacy = pTab->zName;
  ಆದರೆ( strcmp(zLegacy+7, &LEGACY_ತಾತ್ಕಾಲಿಕ_SCHEMA_ಕೋಷ್ಟಕ[7])==0 ){
    ಆದರೆ( sqlite3StrICmp(zTab+7, &PREFERRED_ತಾತ್ಕಾಲಿಕ_SCHEMA_ಕೋಷ್ಟಕ[7])==0 ){
      ಹಿಂತಿರುಗಿ 1;
    }
    ಆದರೆ( zDb==0 ) ಹಿಂತಿರುಗಿ 0;
    ಆದರೆ( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_ಕೋಷ್ಟಕ[7])==0 ) ಹಿಂತಿರುಗಿ 1;
    ಆದರೆ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_ಕೋಷ್ಟಕ[7])==0 ) ಹಿಂತಿರುಗಿ 1;
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಆದರೆ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_ಕೋಷ್ಟಕ[7])==0 ) ಹಿಂತಿರುಗಿ 1;
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ lookupName(
  Parse *pParse,       /* The parsing context */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb,     /* Name of the database containing table, or NULL */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zTab,    /* Name of table containing column, or NULL */
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ          /* Make this EXPR node point to the selected column */
){
  ಪೂರ್ಣಾಂಕ i, j;                         /* Loop counters */
  ಪೂರ್ಣಾಂಕ cnt = 0;                      /* Number of matching column names */
  ಪೂರ್ಣಾಂಕ cntTab = 0;                   /* Number of potential "rowid" matches */
  ಪೂರ್ಣಾಂಕ nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  Src_ಅಂಶ *p_ಅಂಶ;                   /* Use for looping over pSrcList items */
  Src_ಅಂಶ *p_ಹೊಂದಿಕೆ = 0;              /* The matching pSrcList item */
  NameContext *p_ತುದಿ_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  ಪೂರ್ಣಾಂಕ e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_ಸ್ತಂಭ;       /* New value for pExpr->op on success */
  ಕೋಷ್ಟಕ *pTab = 0;                  /* Table holding the row */
  ಅಭಿವ್ಯಕ್ತಿ_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು = p_ಬಲ->u.zToken;

  ಪ್ರತಿಜ್ಞಾ( pNC );     /* the name context cannot be NULL. */
  ಪ್ರತಿಜ್ಞಾ( z_ಲಂಬಸಾಲು );    /* The Z in X.Y.Z cannot be NULL */
  ಪ್ರತಿಜ್ಞಾ( zDb==0 || zTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = -1;
  ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  ಆದರೆ( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    ಆದರೆ( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }ಇಲ್ಲದಿದ್ದರೆ{
      ಗಾಗಿ(i=0; i<db->nDb; i++){
        ಪ್ರತಿಜ್ಞಾ( db->aDb[i].zDbSName );
        ಆದರೆ( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          ನಿಲ್ಲಿಸು;
        }
      }
      ಆದರೆ( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  ಪ್ರತಿಜ್ಞಾ( pNC && cnt==0 );
  ಮಾಡು{
    ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    ಆದರೆ( pSrcList ){
      ಗಾಗಿ(i=0, p_ಅಂಶ=pSrcList->a; i<pSrcList->nSrc; i++, p_ಅಂಶ++){
        pTab = p_ಅಂಶ->pSTab;
        ಪ್ರತಿಜ್ಞಾ( pTab!=0 && pTab->zName!=0 );
        ಪ್ರತಿಜ್ಞಾ( pTab->n_ಲಂಬಸಾಲು>0 || pParse->nErr );
        ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಂಕ)p_ಅಂಶ->fg.isNestedFrom == IsNestedFrom(p_ಅಂಶ));
        ಆದರೆ( p_ಅಂಶ->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          ಪೂರ್ಣಾಂಕ ದೊರಕಿತು = 0;
          Select *pSel;
          ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.isSubquery );
          ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->u4.pSubq!=0 );
          pSel = p_ಅಂಶ->u4.pSubq->pSelect;
          ಪ್ರತಿಜ್ಞಾ( pSel!=0 );
          pEList = pSel->pEList;
          ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
          ಪ್ರತಿಜ್ಞಾ( pEList->n_ಅಭಿವ್ಯಕ್ತಿ==pTab->n_ಲಂಬಸಾಲು );
          ಗಾಗಿ(j=0; j<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
            ಪೂರ್ಣಾಂಕ bRowid = 0;       /* True if possible rowid match */
            ಆದರೆ( !sqlite3_ಹೊಂದಿಕೆ_EName(&pEList->a[j], z_ಲಂಬಸಾಲು, zTab, zDb, &bRowid) ){
              ಮುಂದುವರಿ;
            }
            ಆದರೆ( bRowid==0 ){
              ಆದರೆ( cnt>0 ){
                ಆದರೆ( p_ಅಂಶ->fg.isUsing==0
                 || sqlite3_Id_List_ಸೂಚ್ಯಂಕ(p_ಅಂಶ->u3.pUsing, z_ಲಂಬಸಾಲು)<0
                 || p_ಹೊಂದಿಕೆ==p_ಅಂಶ
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ಇಲ್ಲದಿದ್ದರೆ
                ಆದರೆ( (p_ಅಂಶ->fg.jointype & JT_ಬಲ)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  ಮುಂದುವರಿ;
                }ಇಲ್ಲದಿದ್ದರೆ
                ಆದರೆ( (p_ಅಂಶ->fg.jointype & JT_ಎಡ)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ಇಲ್ಲದಿದ್ದರೆ{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, p_ಹೊಂದಿಕೆ, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ);
                }
              }
              cnt++;
              ದೊರಕಿತು = 1;
            }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              ಮುಂದುವರಿ;
            }
            cntTab++;
            p_ಹೊಂದಿಕೆ = p_ಅಂಶ;
            p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = j;
            pEList->a[j].fg.b_ಬಳಸಿದ = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            ಪ್ರತಿಜ್ಞಾ( bRowid==0 || pEList->a[j].fg.b_Using_ಪದ==0 );
            ಆದರೆ( pEList->a[j].fg.b_Using_ಪದ ) ನಿಲ್ಲಿಸು;
          }
          ಆದರೆ( ದೊರಕಿತು || zTab==0 ) ಮುಂದುವರಿ;
        }
        ಪ್ರತಿಜ್ಞಾ( zDb==0 || zTab!=0 );
        ಆದರೆ( zTab ){
          ಆದರೆ( zDb ){
            ಆದರೆ( pTab->pSchema!=pSchema ) ಮುಂದುವರಿ;
            ಆದರೆ( pSchema==0 && strcmp(zDb,"*")!=0 ) ಮುಂದುವರಿ;
          }
          ಆದರೆ( p_ಅಂಶ->zAlias!=0 ){
            ಆದರೆ( sqlite3StrICmp(zTab, p_ಅಂಶ->zAlias)!=0 ){
              ಮುಂದುವರಿ;
            }
          }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            ಆದರೆ( pTab->tnum!=1 ) ಮುಂದುವರಿ;
            ಆದರೆ( !is_Valid_Schema_ಕೋಷ್ಟಕ_Name(zTab, pTab, zDb) ) ಮುಂದುವರಿ;
          }
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
          ಆದರೆ( ಒಳ_RENAME_OBJECT && p_ಅಂಶ->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab);
          }
        }
        j = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, z_ಲಂಬಸಾಲು);
        ಆದರೆ( j>=0 ){
          ಆದರೆ( cnt>0 ){
            ಆದರೆ( p_ಅಂಶ->fg.isUsing==0
             || sqlite3_Id_List_ಸೂಚ್ಯಂಕ(p_ಅಂಶ->u3.pUsing, z_ಲಂಬಸಾಲು)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ಇಲ್ಲದಿದ್ದರೆ
            ಆದರೆ( (p_ಅಂಶ->fg.jointype & JT_ಬಲ)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              ಮುಂದುವರಿ;
            }ಇಲ್ಲದಿದ್ದರೆ
            ಆದರೆ( (p_ಅಂಶ->fg.jointype & JT_ಎಡ)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ಇಲ್ಲದಿದ್ದರೆ{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, p_ಹೊಂದಿಕೆ, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ);
            }
          }
          cnt++;
          p_ಹೊಂದಿಕೆ = p_ಅಂಶ;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = j==pTab->iPKey ? -1 : (i16)j;
          ಆದರೆ( p_ಅಂಶ->fg.isNestedFrom ){
            sqlite3_Src_ಅಂಶ_ಸ್ತಂಭ_ಬಳಸಿದ(p_ಅಂಶ, j);
          }
        }
        ಆದರೆ( 0==cnt && VisibleRowid(pTab) ){
          /* pTab is a potential ROWID match.  Keep track of it and match
          ** the ROWID later if that seems appropriate.  (Search for "cntTab"
          ** to find related code.)  Only allow a ROWID match if there is
          ** a single ROWID match candidate.
          */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW
          /* In SQLITE_ALLOW_ROWID_IN_VIEW mode, allow a ROWID match
          ** if there is a single VIEW candidate or if there is a single
          ** non-VIEW candidate plus multiple VIEW candidates.  In other
          ** words non-VIEW candidate terms take precedence over VIEWs.
          */
          ಆದರೆ( cntTab==0
           || (cntTab==1
               && p_ಹೊಂದಿಕೆ!=0
               && ALWAYS(p_ಹೊಂದಿಕೆ->pSTab!=0)
               && (p_ಹೊಂದಿಕೆ->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            p_ಹೊಂದಿಕೆ = p_ಅಂಶ;
          }ಇಲ್ಲದಿದ್ದರೆ{
            cntTab++;
          }
#ಇಲ್ಲದಿದ್ದರೆ
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          p_ಹೊಂದಿಕೆ = p_ಅಂಶ;
#ಆದರೆ_ಅಂತ್ಯ
        }
      }
      ಆದರೆ( p_ಹೊಂದಿಕೆ ){
        p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = p_ಹೊಂದಿಕೆ->iCursor;
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
        p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab = p_ಹೊಂದಿಕೆ->pSTab;
        ಆದರೆ( (p_ಹೊಂದಿಕೆ->fg.jointype & (JT_ಎಡ|JT_LTORJ))!=0 ){
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_CanBeNull);
        }
        pSchema = p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#ಆದರೆ !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    ಆದರೆ( cnt==0 && zDb==0 ){
      pTab = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
      ಆದರೆ( pParse->pTriggerTab!=0 ){
        ಪೂರ್ಣಾಂಕ op = pParse->eTriggerOp;
        ಪ್ರತಿಜ್ಞಾ( op==TK_DELETE || op==TK_UPDATE || op==TK_ಸೇರಿಸು );
        ಆದರೆ( pParse->bReturning ){
          ಆದರೆ( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || is_Valid_Schema_ಕೋಷ್ಟಕ_Name(zTab, pParse->pTriggerTab, 0))
          ){
            p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = 1;
          pTab = pParse->pTriggerTab;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( op!=TK_ಸೇರಿಸು && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TRIGGER */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UPSERT
      ಆದರೆ( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        ಆದರೆ( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = EXCLUDED_ಕೋಷ್ಟಕ_ಸಂಖ್ಯೆ;
          ಆದರೆ( ಒಳ_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab);
          }
        }
      }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UPSERT */

      ಆದರೆ( pTab ){
        ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;
        pSchema = pTab->pSchema;
        cntTab++;
        i_ಲಂಬಸಾಲು = sqlite3_ಸ್ತಂಭ_ಸೂಚ್ಯಂಕ(pTab, z_ಲಂಬಸಾಲು);
        ಆದರೆ( i_ಲಂಬಸಾಲು>=0 ){
          ಆದರೆ( pTab->iPKey==i_ಲಂಬಸಾಲು ) i_ಲಂಬಸಾಲು = -1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಆದರೆ( sqlite3IsRowid(z_ಲಂಬಸಾಲು) && VisibleRowid(pTab) ){
            i_ಲಂಬಸಾಲು = -1;
          }ಇಲ್ಲದಿದ್ದರೆ{
            i_ಲಂಬಸಾಲು = pTab->n_ಲಂಬಸಾಲು;
          }
        }
        ಆದರೆ( i_ಲಂಬಸಾಲು<pTab->n_ಲಂಬಸಾಲು ){
          cnt++;
          p_ಹೊಂದಿಕೆ = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_UPSERT
          ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==EXCLUDED_ಕೋಷ್ಟಕ_ಸಂಖ್ಯೆ ){
            testcase( i_ಲಂಬಸಾಲು==(-1) );
            ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
            ಆದರೆ( ಒಳ_RENAME_OBJECT ){
              p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = i_ಲಂಬಸಾಲು;
              p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab = pTab;
              e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_ಸ್ತಂಭ;
            }ಇಲ್ಲದಿದ್ದರೆ{
              p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = pNC->uNC.pUpsert->reg_ದತ್ತಾಂಶ +
                 sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i_ಲಂಬಸಾಲು);
              e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_ರಿಜಿಸ್ಟರ್;
            }
          }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_UPSERT */
          {
            ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
            p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab = pTab;
            ಆದರೆ( pParse->bReturning ){
              e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_ರಿಜಿಸ್ಟರ್;
              p_ಅಭಿವ್ಯಕ್ತಿ->op2 = TK_ಸ್ತಂಭ;
              p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = i_ಲಂಬಸಾಲು;
              p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = pNC->uNC.i_ತಳ_Reg + (pTab->n_ಲಂಬಸಾಲು+1)*p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ +
                 sqlite3_ಕೋಷ್ಟಕ_ಸ್ತಂಭ_ಗಮ್ಯ_Storage(pTab, i_ಲಂಬಸಾಲು) + 1;
            }ಇಲ್ಲದಿದ್ದರೆ{
              p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = (i16)i_ಲಂಬಸಾಲು;
              e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_TRIGGER;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
              ಆದರೆ( i_ಲಂಬಸಾಲು<0 ){
                p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_INTEGER;
              }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ==0 ){
                testcase( i_ಲಂಬಸಾಲು==31 );
                testcase( i_ಲಂಬಸಾಲು==32 );
                pParse->oldmask |= (i_ಲಂಬಸಾಲು>=32 ? 0xffffffff : (((u32)1)<<i_ಲಂಬಸಾಲು));
              }ಇಲ್ಲದಿದ್ದರೆ{
                testcase( i_ಲಂಬಸಾಲು==31 );
                testcase( i_ಲಂಬಸಾಲು==32 );
                pParse->newmask |= (i_ಲಂಬಸಾಲು>=32 ? 0xffffffff : (((u32)1)<<i_ಲಂಬಸಾಲು));
              }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TRIGGER */
            }
          }
        }
      }
    }
#ಆದರೆ_ಅಂತ್ಯ /* !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT) */

    /*
    ** Perhaps the name is a reference to the ROWID
    */
    ಆದರೆ( cnt==0
     && cntTab>=1
     && p_ಹೊಂದಿಕೆ
     && (pNC->ncFlags & (NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_Gen_ಲಂಬಸಾಲು))==0
     && sqlite3IsRowid(z_ಲಂಬಸಾಲು)
     && ALWAYS(VisibleRowid(p_ಹೊಂದಿಕೆ->pSTab) || p_ಹೊಂದಿಕೆ->fg.isNestedFrom)
    ){
      cnt = cntTab;
#ಆದರೆ SQLITE_ALLOW_ROWID_ಒಳ_VIEW+0==2
      ಆದರೆ( p_ಹೊಂದಿಕೆ->pSTab!=0 && IsView(p_ಹೊಂದಿಕೆ->pSTab) ){
        e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_NULL;
      }
#ಆದರೆ_ಅಂತ್ಯ
      ಆದರೆ( p_ಹೊಂದಿಕೆ->fg.isNestedFrom==0 ) p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ = -1;
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
    ಆದರೆ( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
      ಗಾಗಿ(j=0; j<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
        ಅಕ್ಷರ *zAs = pEList->a[j].zEName;
        ಆದರೆ( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, z_ಲಂಬಸಾಲು)==0
        ){
          ಅಭಿವ್ಯಕ್ತಿ *pOrig;
          ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0 && p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==0 );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ)==0 || p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==0 );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ)==0 || p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect==0 );
          pOrig = pEList->a[j].p_ಅಭಿವ್ಯಕ್ತಿ;
          ಆದರೆ( (pNC->ncFlags&NC_AllowAgg)==0 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "misuse of aliased aggregate %s", zAs);
            ಹಿಂತಿರುಗಿ WRC_Abort;
          }
          ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_ತುದಿ_NC )
          ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "misuse of aliased window function %s",zAs);
            ಹಿಂತಿರುಗಿ WRC_Abort;
          }
          ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(pOrig)!=1 ){
            sqlite3_Error_ಸಂದೇಶ(pParse, "row value misused");
            ಹಿಂತಿರುಗಿ WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_ಅಭಿವ್ಯಕ್ತಿ, nSubquery);
          cnt = 1;
          p_ಹೊಂದಿಕೆ = 0;
          ಪ್ರತಿಜ್ಞಾ( zTab==0 && zDb==0 );
          ಆದರೆ( ಒಳ_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ);
          }
          ಹೋಗು lookupname_ಕೊನೆ;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    ಆದರೆ( cnt ) ನಿಲ್ಲಿಸು;
    pNC = pNC->p_ಮುಂದಿನ;
    nSubquery++;
  }ಆಗ( pNC );


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
  ಆದರೆ( cnt==0 && zTab==0 ){
    ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID );
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_DblQuoted)
     && are_Double_Quoted_ಅಕ್ಷರಸರಣಿಗಳು_Enabled(db, p_ತುದಿ_NC)
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
        "double-quoted string literal: \"%w\"", z_ಲಂಬಸಾಲು);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_NORMALIZE
      sqlite3_Vdbe_ಕೂಡಿಸು_Dblquote_Str(db, pParse->pVdbe, z_ಲಂಬಸಾಲು);
#ಆದರೆ_ಅಂತ್ಯ
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_STRING;
      memset(&p_ಅಭಿವ್ಯಕ್ತಿ->y, 0, ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ->y));
      ಹಿಂತಿರುಗಿ WRC_Prune;
    }
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Id_ಗಮ್ಯ_True_False(p_ಅಭಿವ್ಯಕ್ತಿ) ){
      ಹಿಂತಿರುಗಿ WRC_Prune;
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
  ಆದರೆ( cnt!=1 ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zErr;
    ಆದರೆ( pFJMatch ){
      ಆದರೆ( pFJMatch->n_ಅಭಿವ್ಯಕ್ತಿ==cnt-1 ){
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_ಎಲೆ) ){
          ಅಭಿವ್ಯಕ್ತಿ_Clear_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_ಎಲೆ);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
          p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = 0;
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
          p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ = 0;
        }
        extendFJMatch(pParse, &pFJMatch, p_ಹೊಂದಿಕೆ, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ);
        p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_FUNCTION;
        p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken = "coalesce";
        p_ಅಭಿವ್ಯಕ್ತಿ->x.pList = pFJMatch;
        p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_DEFER;
        cnt = 1;
        ಹೋಗು lookupname_ಕೊನೆ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ಅಭಿವ್ಯಕ್ತಿ_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    ಆದರೆ( zDb ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, z_ಲಂಬಸಾಲು);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( zTab ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s: %s.%s", zErr, zTab, z_ಲಂಬಸಾಲು);
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( cnt==0 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ,EP_DblQuoted) ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, z_ಲಂಬಸಾಲು);
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_Error_ಸಂದೇಶ(pParse, "%s: %s", zErr, z_ಲಂಬಸಾಲು);
    }
    sqlite3_ದಾಖಲೆ_Error_ಸ್ಥಾನಾಂತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
    pParse->checkSchema = 1;
    p_ತುದಿ_NC->nNcErr++;
    e_New_ಅಭಿವ್ಯಕ್ತಿ_Op = TK_NULL;
  }
  ಪ್ರತಿಜ್ಞಾ( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,(EP_TokenOnly|EP_ಎಲೆ)) ){
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = 0;
    sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
    p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ = 0;
    ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_ಎಲೆ);
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
  ಆದರೆ( p_ಹೊಂದಿಕೆ ){
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ>=0 ){
      p_ಹೊಂದಿಕೆ->ಲಂಬಸಾಲು_ಬಳಸಿದ |= sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಲಂಬಸಾಲು_ಬಳಸಿದ(p_ಅಭಿವ್ಯಕ್ತಿ);
    }ಇಲ್ಲದಿದ್ದರೆ{
      p_ಹೊಂದಿಕೆ->fg.rowid_ಬಳಸಿದ = 1;
    }
  }

  p_ಅಭಿವ್ಯಕ್ತಿ->op = e_New_ಅಭಿವ್ಯಕ್ತಿ_Op;
lookupname_ಕೊನೆ:
  ಆದರೆ( cnt==1 ){
    ಪ್ರತಿಜ್ಞಾ( pNC!=0 );
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
    ಆದರೆ( db->xAuth ){
      ಆದರೆ( pFJMatch ){
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_FUNCTION );
        ಪ್ರತಿಜ್ಞಾ( sqlite3_stricmp(p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken,"coalesce")==0 );
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==pFJMatch );
        ಪ್ರತಿಜ್ಞಾ( pFJMatch->n_ಅಭಿವ್ಯಕ್ತಿ>0 );
        ಗಾಗಿ(i=0; i<pFJMatch->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
          ಪ್ರತಿಜ್ಞಾ( pFJMatch->a[i].p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_ಅಭಿವ್ಯಕ್ತಿ,pSchema,pNC->pSrcList);
        }
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಸ್ತಂಭ || p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pSchema, pNC->pSrcList);
      }
    } 
#ಆದರೆ_ಅಂತ್ಯ
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    ಗಾಗಿ(;;){
      ಪ್ರತಿಜ್ಞಾ( p_ತುದಿ_NC!=0 );
      p_ತುದಿ_NC->nRef++;
      ಆದರೆ( p_ತುದಿ_NC==pNC ) ನಿಲ್ಲಿಸು;
      p_ತುದಿ_NC = p_ತುದಿ_NC->p_ಮುಂದಿನ;
    }
    ಹಿಂತಿರುಗಿ WRC_Prune;
  } ಇಲ್ಲದಿದ್ದರೆ {
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
ಅಭಿವ್ಯಕ್ತಿ *sqlite3_Create_ಸ್ತಂಭ_ಅಭಿವ್ಯಕ್ತಿ(sqlite3 *db, SrcList *pSrc, ಪೂರ್ಣಾಂಕ iSrc, ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು){
  ಅಭಿವ್ಯಕ್ತಿ *p = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Alloc(db, TK_ಸ್ತಂಭ, 0, 0);
  ಆದರೆ( p ){
    Src_ಅಂಶ *p_ಅಂಶ = &pSrc->a[iSrc];
    ಕೋಷ್ಟಕ *pTab;
    ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p) );
    pTab = p->y.pTab = p_ಅಂಶ->pSTab;
    p->i_ಕೋಷ್ಟಕ = p_ಅಂಶ->iCursor;
    ಆದರೆ( p->y.pTab->iPKey==i_ಲಂಬಸಾಲು ){
      p->i_ಸ್ತಂಭ = -1;
    }ಇಲ್ಲದಿದ್ದರೆ{
      p->i_ಸ್ತಂಭ = (ynVar)i_ಲಂಬಸಾಲು;
      ಆದರೆ( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->a_ಲಂಬಸಾಲು[i_ಲಂಬಸಾಲು].ಲಂಬಸಾಲು_Flags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->n_ಲಂಬಸಾಲು==63 );
        testcase( pTab->n_ಲಂಬಸಾಲು==64 );
        p_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ = pTab->n_ಲಂಬಸಾಲು>=64 ? ALLBITS : MASKBIT(pTab->n_ಲಂಬಸಾಲು)-1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        testcase( i_ಲಂಬಸಾಲು==BMS );
        testcase( i_ಲಂಬಸಾಲು==BMS-1 );
        p_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ |= ((Bitmask)1)<<(i_ಲಂಬಸಾಲು>=BMS ? BMS-1 : i_ಲಂಬಸಾಲು);
      }
    }
  }
  ಹಿಂತಿರುಗಿ p;
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
   ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಸಂದೇಶ,    /* Type of error */
   ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,         /* Invalidate this expression on error */
   ಅಭಿವ್ಯಕ್ತಿ *pError         /* Associate error with this expression */
){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಒಳ = "partial index WHERE clauses";
  ಆದರೆ( pNC->ncFlags & NC_Idx_ಅಭಿವ್ಯಕ್ತಿ )      z_ಒಳ = "index expressions";
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CHECK
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pNC->ncFlags & NC_IsCheck ) z_ಒಳ = "CHECK constraints";
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_GENERATED_COLUMNS
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( pNC->ncFlags & NC_Gen_ಲಂಬಸಾಲು ) z_ಒಳ = "generated columns";
#ಆದರೆ_ಅಂತ್ಯ
  sqlite3_Error_ಸಂದೇಶ(pParse, "%s prohibited in %s", z_ಸಂದೇಶ, z_ಒಳ);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ ) p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
  sqlite3_ದಾಖಲೆ_Error_ಸ್ಥಾನಾಂತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, pError);
}
#ವ್ಯಾಖ್ಯಾನಿಸು sqlite3ResolveNotValid(P,N,M,X,E,R) \
  ಪ್ರತಿಜ್ಞಾ( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_Gen_ಲಂಬಸಾಲು))==0 ); \
  ಆದರೆ( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_Probability(ಅಭಿವ್ಯಕ್ತಿ *p){
  ದ್ವಿಗುಣ r = -1.0;
  ಆದರೆ( p->op!=TK_FLOAT ) ಹಿಂತಿರುಗಿ -1;
  ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p, EP_Int_ಮೌಲ್ಯ) );
  sqlite3AtoF(p->u.zToken, &r);
  ಪ್ರತಿಜ್ಞಾ( r>=0.0 );
  ಆದರೆ( r>1.0 ) ಹಿಂತಿರುಗಿ -1;
  ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
ಸ್ಥಿರ SQLITE_NOINLINE ಶೂನ್ಯ resolve_ಗಣ_ಅಭಿವ್ಯಕ್ತಿ_Subtype_Arg(ಅಭಿವ್ಯಕ್ತಿ_List *pList){
  ಪೂರ್ಣಾಂಕ nn, ii;
  nn = pList ? pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
  ಗಾಗಿ(ii=0; ii<nn; ii++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pList->a[ii].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಆಗ( 1 /*exit-by-break*/ ){
      ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_SubtArg);
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT ){
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect!=0 );
        resolve_ಗಣ_ಅಭಿವ್ಯಕ್ತಿ_Subtype_Arg(p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->pEList);
        ನಿಲ್ಲಿಸು;
      }
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_UPLUS ){
        p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
      }ಇಲ್ಲದಿದ್ದರೆ{
        ನಿಲ್ಲಿಸು;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಜ್ಜೆ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  ಪ್ರತಿಜ್ಞಾ( pNC!=0 );
  pParse = pNC->pParse;
  ಪ್ರತಿಜ್ಞಾ( pParse==pWalker->pParse );

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  ಆದರೆ( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    ಪೂರ್ಣಾಂಕ i;
    ಗಾಗಿ(i=0; i<pNC->pSrcList->nSrc; i++){
      ಪ್ರತಿಜ್ಞಾ( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    ಸ್ಥಿತಿ TK_ಅಡ್ಡಸಾಲು: {
      SrcList *pSrcList = pNC->pSrcList;
      Src_ಅಂಶ *p_ಅಂಶ;
      ಪ್ರತಿಜ್ಞಾ( pSrcList && pSrcList->nSrc>=1 );
      p_ಅಂಶ = pSrcList->a;
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_ಸ್ತಂಭ;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
      p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab = p_ಅಂಶ->pSTab;
      p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = p_ಅಂಶ->iCursor;
      p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ--;
      p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ = SQLITE_AFF_INTEGER;
      ನಿಲ್ಲಿಸು;
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
    ಸ್ಥಿತಿ TK_NOTNULL:
    ಸ್ಥಿತಿ TK_ISNULL: {
      ಪೂರ್ಣಾಂಕ anRef[8];
      NameContext *p;
      ಪೂರ್ಣಾಂಕ i;
      ಗಾಗಿ(i=0, p=pNC; p && i<Array_ಗಾತ್ರ(anRef); p=p->p_ಮುಂದಿನ, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      ಆದರೆ( ಒಳ_RENAME_OBJECT ) ಹಿಂತಿರುಗಿ WRC_Prune;
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Can_Be_Null(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) ){
        /* The expression can be NULL.  So the optimization does not apply */
        ಹಿಂತಿರುಗಿ WRC_Prune;
      }

      ಗಾಗಿ(i=0, p=pNC; p; p=p->p_ಮುಂದಿನ, i++){
        ಆದರೆ( (p->ncFlags & NC_Where)==0 ){
          ಹಿಂತಿರುಗಿ WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) );
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
#ಆದರೆ TREETRACE_ENABLED
      ಆದರೆ( sqlite3_ವೃಕ್ಷ_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL
        );
        sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ(p_ಅಭಿವ್ಯಕ್ತಿ);
      }
#ಆದರೆ_ಅಂತ್ಯ /* TREETRACE_ENABLED */
      p_ಅಭಿವ್ಯಕ್ತಿ->u.i_ಮೌಲ್ಯ = (p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_NOTNULL);
      p_ಅಭಿವ್ಯಕ್ತಿ->flags |= EP_Int_ಮೌಲ್ಯ;
      p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_INTEGER;
      ಗಾಗಿ(i=0, p=pNC; p && i<Array_ಗಾತ್ರ(anRef); p=p->p_ಮುಂದಿನ, i++){
        p->nRef = anRef[i];
      }
      sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ = 0;
      ಹಿಂತಿರುಗಿ WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    ಸ್ಥಿತಿ TK_ID:
    ಸ್ಥಿತಿ TK_DOT: {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಕೋಷ್ಟಕ;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zDb;
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ;

      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ID ){
        zDb = 0;
        z_ಕೋಷ್ಟಕ = 0;
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
        p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಅಭಿವ್ಯಕ್ತಿ *p_ಎಡ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
        testcase( pNC->ncFlags & NC_Idx_ಅಭಿವ್ಯಕ್ತಿ );
        testcase( pNC->ncFlags & NC_Gen_ಲಂಬಸಾಲು );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_Gen_ಲಂಬಸಾಲು, 0, p_ಅಭಿವ್ಯಕ್ತಿ);
        p_ಬಲ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        ಆದರೆ( p_ಬಲ->op==TK_ID ){
          zDb = 0;
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪ್ರತಿಜ್ಞಾ( p_ಬಲ->op==TK_DOT );
          ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಬಲ, EP_Int_ಮೌಲ್ಯ) );
          zDb = p_ಎಡ->u.zToken;
          p_ಎಡ = p_ಬಲ->p_ಎಡ;
          p_ಬಲ = p_ಬಲ->p_ಬಲ;
        }
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_UToken(p_ಎಡ) && ಅಭಿವ್ಯಕ್ತಿ_Use_UToken(p_ಬಲ) );
        z_ಕೋಷ್ಟಕ = p_ಎಡ->u.zToken;
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
        ಆದರೆ( ಒಳ_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (ಶೂನ್ಯ*)p_ಅಭಿವ್ಯಕ್ತಿ, (ಶೂನ್ಯ*)p_ಬಲ);
          sqlite3RenameTokenRemap(pParse, (ಶೂನ್ಯ*)&p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab, (ಶೂನ್ಯ*)p_ಎಡ);
        }
      }
      ಹಿಂತಿರುಗಿ lookupName(pParse, zDb, z_ಕೋಷ್ಟಕ, p_ಬಲ, pNC, p_ಅಭಿವ್ಯಕ್ತಿ);
    }

    /* Resolve function names
    */
    ಸ್ಥಿತಿ TK_FUNCTION: {
      ಅಭಿವ್ಯಕ್ತಿ_List *pList;            /* The argument list */
      ಪೂರ್ಣಾಂಕ n;                      /* Number of arguments */
      ಪೂರ್ಣಾಂಕ no_such_func = 0;       /* True if no such function exists */
      ಪೂರ್ಣಾಂಕ wrong_ಅಂಕೆ_ವಾದಗಳು = 0;     /* True if wrong number of arguments */
      ಪೂರ್ಣಾಂಕ is_agg = 0;             /* True if is an aggregate function */
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      ಪೂರ್ಣಾಂಕ savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಕಿಟಕಿ *pWin = (Is_ಕಿಟಕಿ_Func(p_ಅಭಿವ್ಯಕ್ತಿ) ? p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin : 0);
#ಆದರೆ_ಅಂತ್ಯ
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_xIsSelect|EP_Int_ಮೌಲ್ಯ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ==0 || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ಕ್ರಮ );
      pList = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
      n = pList ? pList->n_ಅಭಿವ್ಯಕ್ತಿ : 0;
      zId = p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken;
      pDef = sqlite3_ಕಂಡುಹಿಡಿ_Function(pParse->db, zId, n, enc, 0);
      ಆದರೆ( pDef==0 ){
        pDef = sqlite3_ಕಂಡುಹಿಡಿ_Function(pParse->db, zId, -2, enc, 0);
        ಆದರೆ( pDef==0 ){
          no_such_func = 1;
        }ಇಲ್ಲದಿದ್ದರೆ{
          wrong_ಅಂಕೆ_ವಾದಗಳು = 1;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        is_agg = pDef->xFinalize!=0;
        ಆದರೆ( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Unlikely);
          ಆದರೆ( n==2 ){
            p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = ಅಭಿವ್ಯಕ್ತಿ_Probability(pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ);
            ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ<0 ){
              sqlite3_Error_ಸಂದೇಶ(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_ಅಭಿವ್ಯಕ್ತಿ);
              pNC->nNcErr++;
            }
          }ಇಲ್ಲದಿದ್ದರೆ{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_AUTHORIZATION
        {
          ಪೂರ್ಣಾಂಕ auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          ಆದರೆ( auth!=SQLITE_OK ){
            ಆದರೆ( auth==SQLITE_DENY ){
              sqlite3_Error_ಸಂದೇಶ(pParse, "not authorized to use function: %#T",
                                      p_ಅಭಿವ್ಯಕ್ತಿ);
              pNC->nNcErr++;
            }
            p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_NULL;
            ಹಿಂತಿರುಗಿ WRC_Prune;
          }
        }
#ಆದರೆ_ಅಂತ್ಯ

        /* If the function may call sqlite3_value_subtype(), then set the 
        ** EP_SubtArg flag on all of its argument expressions. This prevents
        ** where.c from replacing the expression with a value read from an
        ** index on the same expression, which will not have the correct 
        ** subtype. Also set the flag if the function expression itself is
        ** an EP_SubtArg expression. In this case subtypes are required as 
        ** the function may return a value with a subtype back to its 
        ** caller using sqlite3_result_value().  */
        ಆದರೆ( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_SubtArg) 
        ){
          resolve_ಗಣ_ಅಭಿವ್ಯಕ್ತಿ_Subtype_Arg(pList);
        }

        ಆದರೆ( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ,EP_ConstFunc);
        }
        ಆದರೆ( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_PartIdx|NC_Gen_ಲಂಬಸಾಲು, 0, p_ಅಭಿವ್ಯಕ್ತಿ);
        }ಇಲ್ಲದಿದ್ದರೆ{
          ಪ್ರತಿಜ್ಞಾ( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_ಅಭಿವ್ಯಕ್ತಿ->op2 = pNC->ncFlags & NC_SelfRef;
        }
        ಆದರೆ( (pDef->funcFlags & SQLITE_FUNC_ಆಂತರಿಕ)!=0
         && pParse->nested==0
         && (pParse->db->mDbFlags & DBFLAG_ಆಂತರಿಕ_Func)==0
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
        }ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ( (pDef->funcFlags & (SQLITE_FUNC_ನೇರ|SQLITE_FUNC_UNSAFE))!=0
         && !ಒಳ_RENAME_OBJECT
        ){
          ಆದರೆ( pNC->ncFlags & NC_FromDDL ) ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL);
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Function_Usable(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pDef);
        }
      }

      ಆದರೆ( 0==ಒಳ_RENAME_OBJECT ){
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
        ಪ್ರತಿಜ್ಞಾ( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_ಮೌಲ್ಯ==0 && pDef->xInverse==0)
          || (pDef->x_ಮೌಲ್ಯ && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        ಆದರೆ( pDef && pDef->x_ಮೌಲ್ಯ==0 && pWin ){
          sqlite3_Error_ಸಂದೇಶ(pParse,
              "%#T() may not be used as a window function", p_ಅಭಿವ್ಯಕ್ತಿ
          );
          pNC->nNcErr++;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_ಕಿಟಕಿ) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType;
          ಆದರೆ( (pDef->funcFlags & SQLITE_FUNC_ಕಿಟಕಿ) || pWin ){
            zType = "window";
          }ಇಲ್ಲದಿದ್ದರೆ{
            zType = "aggregate";
          }
          sqlite3_Error_ಸಂದೇಶ(pParse, "misuse of %s function %#T()",zType,p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#ಇಲ್ಲದಿದ್ದರೆ
        ಆದರೆ( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_ಸಂದೇಶ(pParse,"misuse of aggregate function %#T()",p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#ಆದರೆ_ಅಂತ್ಯ
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( no_such_func
              && (pParse->db->init.busy==0 ||
                  (no_such_func==2 && pParse->db->init.busy==2))
              /* Suppress "no such function" errors when reading
              ** the sqlite_schema table.  Except, do raise the error
              ** if init.busy is 2, meaning the schema parse is due
              ** to an ALTER TABLE ADD COLUMN statement, and the function
              ** is an internal-use-only function (no_such_func==2). */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
              && pParse->explain==0
#ಆದರೆ_ಅಂತ್ಯ
        ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "no such function: %#T", p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( wrong_ಅಂಕೆ_ವಾದಗಳು ){
          sqlite3_Error_ಸಂದೇಶ(pParse,"wrong number of arguments to function %#T()",
               p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( is_agg==0 && ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
          sqlite3_Error_ಸಂದೇಶ(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_ಅಭಿವ್ಯಕ್ತಿ
          );
          pNC->nNcErr++;
        }
#ಆದರೆ_ಅಂತ್ಯ
        ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( is_agg==0 && p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಕ್ರಮ_By_Aggregate_Error(pParse, p_ಅಭಿವ್ಯಕ್ತಿ);
          pNC->nNcErr++;
        }
        ಆದರೆ( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#ಇಲ್ಲದಿದ್ದರೆ
          pNC->ncFlags &= ~NC_AllowAgg;
#ಆದರೆ_ಅಂತ್ಯ
        }
      }
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
        is_agg = 1;
      }
      sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pList);
      ಆದರೆ( is_agg ){
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
          ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->op==TK_ಕ್ರಮ );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pList);
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
        ಆದರೆ( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YWin(p_ಅಭಿವ್ಯಕ್ತಿ) && pWin==p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin );
          ಆದರೆ( ಒಳ_RENAME_OBJECT==0 ){
            sqlite3_ಕಿಟಕಿ_Update(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            ಆದರೆ( pParse->db->mallocFailed ) ನಿಲ್ಲಿಸು;
          }
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pWin->p_ವಿಭಜನೆ);
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pWin->p_ಕ್ರಮ_By);
          sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pWin->pFilter);
          sqlite3_ಕಿಟಕಿ_Link(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_AGG_FUNCTION;
          p_ಅಭಿವ್ಯಕ್ತಿ->op2 = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
          ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
            sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin->pFilter);
          }
#ಆದರೆ_ಅಂತ್ಯ
          pNC2 = pNC;
          ಆಗ( pNC2
              && sqlite3ReferencesSrcList(pParse, p_ಅಭಿವ್ಯಕ್ತಿ, pNC2->pSrcList)==0
          ){
            p_ಅಭಿವ್ಯಕ್ತಿ->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->p_ಮುಂದಿನ;
          }
          ಪ್ರತಿಜ್ಞಾ( pDef!=0 || ಒಳ_RENAME_OBJECT );
          ಆದರೆ( pNC2 && pDef ){
            p_ಅಭಿವ್ಯಕ್ತಿ->op2 += pNC2->nNestedSelect;
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            ಪ್ರತಿಜ್ಞಾ( SQLITE_FUNC_ANYORDER==NC_ಕ್ರಮ_Agg );
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
      ಹಿಂತಿರುಗಿ WRC_Prune;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    ಸ್ಥಿತಿ TK_EXISTS:
    ಸ್ಥಿತಿ TK_SELECT:
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ TK_ಒಳ: {
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_ಒಳ );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS );
      testcase( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_SELECT );
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        ಪೂರ್ಣಾಂಕ nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_ಅಭಿವ್ಯಕ್ತಿ );
        testcase( pNC->ncFlags & NC_Gen_ಲಂಬಸಾಲು );
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect );
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_EXISTS )  pParse->bHasExists = 1;
        ಆದರೆ( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಭಿವ್ಯಕ್ತಿ);
        }ಇಲ್ಲದಿದ್ದರೆ{
          sqlite3WalkSelect(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect);
        }
        ಪ್ರತಿಜ್ಞಾ( pNC->nRef>=nRef );
        ಆದರೆ( nRef!=pNC->nRef ){
          ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_VarSelect);
          p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_ಅಭಿವ್ಯಕ್ತಿ );
      testcase( pNC->ncFlags & NC_Gen_ಲಂಬಸಾಲು );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_ಅಭಿವ್ಯಕ್ತಿ|NC_Gen_ಲಂಬಸಾಲು, p_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಭಿವ್ಯಕ್ತಿ);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_IS:
    ಸ್ಥಿತಿ TK_ISNOT: {
      ಅಭಿವ್ಯಕ್ತಿ *p_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      ಆದರೆ( ALWAYS(p_ಬಲ) && (p_ಬಲ->op==TK_ID || p_ಬಲ->op==TK_TRUEFALSE) ){
        ಪೂರ್ಣಾಂಕ rc = resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಜ್ಜೆ(pWalker, p_ಬಲ);
        ಆದರೆ( rc==WRC_Abort ) ಹಿಂತಿರುಗಿ WRC_Abort;
        ಆದರೆ( p_ಬಲ->op==TK_TRUEFALSE ){
          p_ಅಭಿವ್ಯಕ್ತಿ->op2 = p_ಅಭಿವ್ಯಕ್ತಿ->op;
          p_ಅಭಿವ್ಯಕ್ತಿ->op = TK_TRUTH;
          ಹಿಂತಿರುಗಿ WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    ಸ್ಥಿತಿ TK_BETWEEN:
    ಸ್ಥಿತಿ TK_EQ:
    ಸ್ಥಿತಿ TK_NE:
    ಸ್ಥಿತಿ TK_LT:
    ಸ್ಥಿತಿ TK_LE:
    ಸ್ಥಿತಿ TK_GT:
    ಸ್ಥಿತಿ TK_GE: {
      ಪೂರ್ಣಾಂಕ n_ಎಡ, n_ಬಲ;
      ಆದರೆ( pParse->db->mallocFailed ) ನಿಲ್ಲಿಸು;
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ!=0 );
      n_ಎಡ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ);
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_BETWEEN ){
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
        n_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ);
        ಆದರೆ( n_ಬಲ==n_ಎಡ ){
          n_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ);
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ!=0 );
        n_ಬಲ = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_ಸದಿಶ_ಗಾತ್ರ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      }
      ಆದರೆ( n_ಎಡ!=n_ಬಲ ){
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
        sqlite3_ದಾಖಲೆ_Error_ಸ್ಥಾನಾಂತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, p_ಅಭಿವ್ಯಕ್ತಿ);
      }
      ನಿಲ್ಲಿಸು;
    }
  }
  ಪ್ರತಿಜ್ಞಾ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ಹಿಂತಿರುಗಿ pParse->nErr ? WRC_Abort : WRC_Continue;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList,  /* List of expressions to scan */
  ಅಭಿವ್ಯಕ್ತಿ *pE           /* Expression we are trying to match */
){
  ಪೂರ್ಣಾಂಕ i;             /* Loop counter */

  ಬಳಸದ_PARAMETER(pParse);

  ಆದರೆ( pE->op==TK_ID ){
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z_ಲಂಬಸಾಲು;
    ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(pE, EP_Int_ಮೌಲ್ಯ) );
    z_ಲಂಬಸಾಲು = pE->u.zToken;
    ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಆದರೆ( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, z_ಲಂಬಸಾಲು)==0
      ){
        ಹಿಂತಿರುಗಿ i+1;
      }
    }
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resolve_ಕ್ರಮ_By_ಪದ_ಗಮ್ಯ_ಅಭಿವ್ಯಕ್ತಿ_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  ಅಭಿವ್ಯಕ್ತಿ *pE           /* The specific ORDER BY term */
){
  ಪೂರ್ಣಾಂಕ i;             /* Loop counter */
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  ಪೂರ್ಣಾಂಕ rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  ಪ್ರತಿಜ್ಞಾ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, ಗಾತ್ರ(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&nc, pE);
  db->suppressErr = savedSuppErr;
  ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  ಗಾಗಿ(i=0; i<pEList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, pEList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, pE, -1)<2 ){
      ಹಿಂತಿರುಗಿ i+1;
    }
  }

  /* If no match, return 0. */
  ಹಿಂತಿರುಗಿ 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
ಸ್ಥಿರ ಶೂನ್ಯ resolve_ಹೊರಹೋಗು_ದ_Range_Error(
  Parse *pParse,         /* The error context into which to write the error */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType,     /* "ORDER" or "GROUP" */
  ಪೂರ್ಣಾಂಕ i,                 /* The index (1-based) of the term out of range */
  ಪೂರ್ಣಾಂಕ mx,                /* Largest permissible value of i */
  ಅಭಿವ್ಯಕ್ತಿ *pError           /* Associate the error with the expression */
){
  sqlite3_Error_ಸಂದೇಶ(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_ದಾಖಲೆ_Error_ಸ್ಥಾನಾಂತರ_ದ_ಅಭಿವ್ಯಕ್ತಿ(pParse->db, pError);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resolve_Compound_ಕ್ರಮ_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  ಪೂರ್ಣಾಂಕ i;
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  sqlite3 *db;
  ಪೂರ್ಣಾಂಕ ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Do = 1;

  p_ಕ್ರಮ_By = pSelect->p_ಕ್ರಮ_By;
  ಆದರೆ( p_ಕ್ರಮ_By==0 ) ಹಿಂತಿರುಗಿ 0;
  db = pParse->db;
  ಆದರೆ( p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಸ್ತಂಭ] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many terms in ORDER BY clause");
    ಹಿಂತಿರುಗಿ 1;
  }
  ಗಾಗಿ(i=0; i<p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    p_ಕ್ರಮ_By->a[i].fg.ಮುಗಿದ = 0;
  }
  pSelect->p_ಮುಂದಿನ = 0;
  ಆಗ( pSelect->p_ಪೂರ್ವ ){
    pSelect->p_ಪೂರ್ವ->p_ಮುಂದಿನ = pSelect;
    pSelect = pSelect->p_ಪೂರ್ವ;
  }
  ಆಗ( pSelect && ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Do ){
    ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
    ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Do = 0;
    pEList = pSelect->pEList;
    ಪ್ರತಿಜ್ಞಾ( pEList!=0 );
    ಗಾಗಿ(i=0, p_ಅಂಶ=p_ಕ್ರಮ_By->a; i<p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++, p_ಅಂಶ++){
      ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು = -1;
      ಅಭಿವ್ಯಕ್ತಿ *pE, *pDup;
      ಆದರೆ( p_ಅಂಶ->fg.ಮುಗಿದ ) ಮುಂದುವರಿ;
      pE = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ);
      ಆದರೆ( NEVER(pE==0) ) ಮುಂದುವರಿ;
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(pE, &i_ಲಂಬಸಾಲು, 0, 1) ){
        ಆದರೆ( i_ಲಂಬಸಾಲು<=0 || i_ಲಂಬಸಾಲು>pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
          resolve_ಹೊರಹೋಗು_ದ_Range_Error(pParse, "ORDER", i+1, pEList->n_ಅಭಿವ್ಯಕ್ತಿ, pE);
          ಹಿಂತಿರುಗಿ 1;
        }
      }ಇಲ್ಲದಿದ್ದರೆ{
        i_ಲಂಬಸಾಲು = resolveAsName(pParse, pEList, pE);
        ಆದರೆ( i_ಲಂಬಸಾಲು==0 ){
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
          ಆದರೆ( !db->mallocFailed ){
            ಪ್ರತಿಜ್ಞಾ(pDup);
            i_ಲಂಬಸಾಲು = resolve_ಕ್ರಮ_By_ಪದ_ಗಮ್ಯ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSelect, pDup);
            ಆದರೆ( ಒಳ_RENAME_OBJECT && i_ಲಂಬಸಾಲು>0 ){
              resolve_ಕ್ರಮ_By_ಪದ_ಗಮ್ಯ_ಅಭಿವ್ಯಕ್ತಿ_List(pParse, pSelect, pE);
            }
          }
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pDup);
        }
      }
      ಆದರೆ( i_ಲಂಬಸಾಲು>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        ಆದರೆ( !ಒಳ_RENAME_OBJECT ){
          ಅಭಿವ್ಯಕ್ತಿ *pNew = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Int32(db, i_ಲಂಬಸಾಲು);
          ಆದರೆ( pNew==0 ) ಹಿಂತಿರುಗಿ 1;
          ಆದರೆ( p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ==pE ){
            p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ = pNew;
          }ಇಲ್ಲದಿದ್ದರೆ{
            ಅಭಿವ್ಯಕ್ತಿ *p_ಪೋಷಕ = p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ;
            ಪ್ರತಿಜ್ಞಾ( p_ಪೋಷಕ->op==TK_COLLATE );
            ಆಗ( p_ಪೋಷಕ->p_ಎಡ->op==TK_COLLATE ) p_ಪೋಷಕ = p_ಪೋಷಕ->p_ಎಡ;
            ಪ್ರತಿಜ್ಞಾ( p_ಪೋಷಕ->p_ಎಡ==pE );
            p_ಪೋಷಕ->p_ಎಡ = pNew;
          }
          sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Delete(db, pE);
          p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = (u16)i_ಲಂಬಸಾಲು;
        }
        p_ಅಂಶ->fg.ಮುಗಿದ = 1;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Do = 1;
      }
    }
    pSelect = pSelect->p_ಮುಂದಿನ;
  }
  ಗಾಗಿ(i=0; i<p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಆದರೆ( p_ಕ್ರಮ_By->a[i].fg.ಮುಗಿದ==0 ){
      sqlite3_Error_ಸಂದೇಶ(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      ಹಿಂತಿರುಗಿ 1;
    }
  }
  ಹಿಂತಿರುಗಿ 0;
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
ಪೂರ್ಣಾಂಕ sqlite3_Resolve_ಕ್ರಮ_ಗುಂಪು_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,   /* The ORDER BY or GROUP BY clause to be processed */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType     /* "ORDER" or "GROUP" */
){
  ಪೂರ್ಣಾಂಕ i;
  sqlite3 *db = pParse->db;
  ಅಭಿವ್ಯಕ್ತಿ_List *pEList;
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;

  ಆದರೆ( p_ಕ್ರಮ_By==0 || pParse->db->mallocFailed || ಒಳ_RENAME_OBJECT ) ಹಿಂತಿರುಗಿ 0;
  ಆದರೆ( p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ>db->a_ಎಲ್ಲೆ[SQLITE_ಎಲ್ಲೆ_ಸ್ತಂಭ] ){
    sqlite3_Error_ಸಂದೇಶ(pParse, "too many terms in %s BY clause", zType);
    ಹಿಂತಿರುಗಿ 1;
  }
  pEList = pSelect->pEList;
  ಪ್ರತಿಜ್ಞಾ( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  ಗಾಗಿ(i=0, p_ಅಂಶ=p_ಕ್ರಮ_By->a; i<p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++, p_ಅಂಶ++){
    ಆದರೆ( p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು ){
      ಆದರೆ( p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು>pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
        resolve_ಹೊರಹೋಗು_ದ_Range_Error(pParse, zType, i+1, pEList->n_ಅಭಿವ್ಯಕ್ತಿ, 0);
        ಹಿಂತಿರುಗಿ 1;
      }
      resolveAlias(pParse, pEList, p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು-1, p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ,0);
    }
  }
  ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resolveRemoveWindowsCb(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಬಳಸದ_PARAMETER(pWalker);
  ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
    ಕಿಟಕಿ *pWin = p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin;
    sqlite3_ಕಿಟಕಿ_Unlink_From_Select(pWin);
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
ಸ್ಥಿರ ಶೂನ್ಯ ಕಿಟಕಿ_Remove_ಅಭಿವ್ಯಕ್ತಿ_From_Select(Select *pSelect, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಆದರೆ( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, ಗಾತ್ರ(Walker));
    sWalker.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(&sWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
  }
}
#ಇಲ್ಲದಿದ್ದರೆ
# define ಕಿಟಕಿ_Remove_ಅಭಿವ್ಯಕ್ತಿ_From_Select(a, b)
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resolve_ಕ್ರಮ_ಗುಂಪು_By(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,   /* An ORDER BY or GROUP BY clause to resolve */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  ಪೂರ್ಣಾಂಕ i, j;                      /* Loop counters */
  ಪೂರ್ಣಾಂಕ i_ಲಂಬಸಾಲು;                      /* Column number */
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  ಪೂರ್ಣಾಂಕ n_ಫಲಿತಾಂಶ;                   /* Number of terms in the result set */

  ಪ್ರತಿಜ್ಞಾ( p_ಕ್ರಮ_By!=0 );
  n_ಫಲಿತಾಂಶ = pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ;
  pParse = pNC->pParse;
  ಗಾಗಿ(i=0, p_ಅಂಶ=p_ಕ್ರಮ_By->a; i<p_ಕ್ರಮ_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++, p_ಅಂಶ++){
    ಅಭಿವ್ಯಕ್ತಿ *pE = p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ;
    ಅಭಿವ್ಯಕ್ತಿ *pE2 = sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(pE);
    ಆದರೆ( NEVER(pE2==0) ) ಮುಂದುವರಿ;
    ಆದರೆ( zType[0]!='G' ){
      i_ಲಂಬಸಾಲು = resolveAsName(pParse, pSelect->pEList, pE2);
      ಆದರೆ( i_ಲಂಬಸಾಲು>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = (u16)i_ಲಂಬಸಾಲು;
        ಮುಂದುವರಿ;
      }
    }
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Is_Integer(pE2, &i_ಲಂಬಸಾಲು, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      ಆದರೆ( i_ಲಂಬಸಾಲು<1 || i_ಲಂಬಸಾಲು>0xffff ){
        resolve_ಹೊರಹೋಗು_ದ_Range_Error(pParse, zType, i+1, n_ಫಲಿತಾಂಶ, pE2);
        ಹಿಂತಿರುಗಿ 1;
      }
      p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = (u16)i_ಲಂಬಸಾಲು;
      ಮುಂದುವರಿ;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = 0;
    ಆದರೆ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(pNC, pE) ){
      ಹಿಂತಿರುಗಿ 1;
    }
    ಗಾಗಿ(j=0; j<pSelect->pEList->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
      ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Compare(0, pE, pSelect->pEList->a[j].p_ಅಭಿವ್ಯಕ್ತಿ, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        ಕಿಟಕಿ_Remove_ಅಭಿವ್ಯಕ್ತಿ_From_Select(pSelect, pE);
        p_ಅಂಶ->u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು = j+1;
      }
    }
  }
  ಹಿಂತಿರುಗಿ sqlite3_Resolve_ಕ್ರಮ_ಗುಂಪು_By(pParse, pSelect, p_ಕ್ರಮ_By, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ resolve_Select_ಹೆಜ್ಜೆ(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  ಪೂರ್ಣಾಂಕ isCompound;         /* True if p is a compound select */
  ಪೂರ್ಣಾಂಕ nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  ಪೂರ್ಣಾಂಕ i;                  /* Loop counter */
  ಅಭಿವ್ಯಕ್ತಿ_List *p_ಗುಂಪು_By;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  ಆದರೆ( p->selFlags & SF_Resolved ){
    ಹಿಂತಿರುಗಿ WRC_Prune;
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
  ಆದರೆ( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    ಹಿಂತಿರುಗಿ pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->p_ಪೂರ್ವ!=0;
  nCompound = 0;
  pLeftmost = p;
  ಆಗ( p ){
    ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Expanded)!=0 );
    ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, ಗಾತ್ರ(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    ಆದರೆ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, p->p_ಎಲ್ಲೆ) ){
      ಹಿಂತಿರುಗಿ WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    ಆದರೆ( p->selFlags & SF_Converted ){
      Select *pSub;
      ಪ್ರತಿಜ್ಞಾ( p->pSrc->a[0].fg.isSubquery );
      ಪ್ರತಿಜ್ಞಾ( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ಪ್ರತಿಜ್ಞಾ( pSub!=0 );
      ಪ್ರತಿಜ್ಞಾ( p->pSrc->nSrc==1 && p->p_ಕ್ರಮ_By );
      ಪ್ರತಿಜ್ಞಾ( pSub->p_ಪೂರ್ವ && pSub->p_ಕ್ರಮ_By==0 );
      pSub->p_ಕ್ರಮ_By = p->p_ಕ್ರಮ_By;
      p->p_ಕ್ರಮ_By = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    ಆದರೆ( pOuterNC ) pOuterNC->nNestedSelect++;
    ಗಾಗಿ(i=0; i<p->pSrc->nSrc; i++){
      Src_ಅಂಶ *p_ಅಂಶ = &p->pSrc->a[i];
      ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->zName!=0
              || p_ಅಂಶ->fg.isSubquery );  /* Test of tag-20240424-1*/
      ಆದರೆ( p_ಅಂಶ->fg.isSubquery
       && (p_ಅಂಶ->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        ಪೂರ್ಣಾಂಕ nRef = pOuterNC ? pOuterNC->nRef : 0;
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zSavedContext = pParse->zAuthContext;

        ಆದರೆ( p_ಅಂಶ->zName ) pParse->zAuthContext = p_ಅಂಶ->zName;
        sqlite3_Resolve_Select_ಹೆಸರುಗಳು(pParse, p_ಅಂಶ->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ WRC_Abort;
        ಪ್ರತಿಜ್ಞಾ( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        ಆದರೆ( pOuterNC ){
          ಪ್ರತಿಜ್ಞಾ( p_ಅಂಶ->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          p_ಅಂಶ->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    ಆದರೆ( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.p_ಮುಂದಿನ = pOuterNC;
 
    /* Resolve names in the result set. */
    ಆದರೆ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, p->pEList) ) ಹಿಂತಿರುಗಿ WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    ಪ್ರತಿಜ್ಞಾ( (p->selFlags & SF_Aggregate)==0 );
    p_ಗುಂಪು_By = p->p_ಗುಂಪು_By;
    ಆದರೆ( p_ಗುಂಪು_By || (sNC.ncFlags & NC_HasAgg)!=0 ){
      ಪ್ರತಿಜ್ಞಾ( NC_MinMaxAgg==SF_MinMaxAgg );
      ಪ್ರತಿಜ್ಞಾ( NC_ಕ್ರಮ_Agg==SF_ಕ್ರಮ_By_Reqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_ಕ್ರಮ_Agg));
    }ಇಲ್ಲದಿದ್ದರೆ{
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
    ಆದರೆ( p->pHaving ){
      ಆದರೆ( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_ಸಂದೇಶ(pParse, "HAVING clause on a non-aggregate query");
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
      ಆದರೆ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, p->pHaving) ) ಹಿಂತಿರುಗಿ WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    ಆದರೆ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, p->pWhere) ) ಹಿಂತಿರುಗಿ WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    ಗಾಗಿ(i=0; i<p->pSrc->nSrc; i++){
      Src_ಅಂಶ *p_ಅಂಶ = &p->pSrc->a[i];
      ಆದರೆ( p_ಅಂಶ->fg.isTabFunc
       && sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, p_ಅಂಶ->u1.pFuncArg)
      ){
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
    }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( ಒಳ_RENAME_OBJECT ){
      ಕಿಟಕಿ *pWin;
      ಗಾಗಿ(pWin=p->pWinDefn; pWin; pWin=pWin->p_ಮುಂದಿನ_Win){
        ಆದರೆ( sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, pWin->p_ಕ್ರಮ_By)
         || sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, pWin->p_ವಿಭಜನೆ)
        ){
          ಹಿಂತಿರುಗಿ WRC_Abort;
        }
      }
    }
#ಆದರೆ_ಅಂತ್ಯ

    sNC.ncFlags |= NC_AllowAgg|NC_AllowWin;

    /* If this is a converted compound query, move the ORDER BY clause from
    ** the sub-query back to the parent query. At this point each term
    ** within the ORDER BY clause has been transformed to an integer value.
    ** These integers will be replaced by copies of the corresponding result
    ** set expressions by the call to resolveOrderGroupBy() below.  */
    ಆದರೆ( p->selFlags & SF_Converted ){
      Select *pSub;
      ಪ್ರತಿಜ್ಞಾ( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ಪ್ರತಿಜ್ಞಾ( pSub!=0 );
      p->p_ಕ್ರಮ_By = pSub->p_ಕ್ರಮ_By;
      pSub->p_ಕ್ರಮ_By = 0;
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
    ಆದರೆ( p->p_ಕ್ರಮ_By!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolve_ಕ್ರಮ_ಗುಂಪು_By(&sNC, p, p->p_ಕ್ರಮ_By, "ORDER")
    ){
      ಹಿಂತಿರುಗಿ WRC_Abort;
    }
    ಆದರೆ( db->mallocFailed ){
      ಹಿಂತಿರುಗಿ WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    ಆದರೆ( p_ಗುಂಪು_By ){
      ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
   
      ಆದರೆ( resolve_ಕ್ರಮ_ಗುಂಪು_By(&sNC, p, p_ಗುಂಪು_By, "GROUP") || db->mallocFailed ){
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
      ಗಾಗಿ(i=0, p_ಅಂಶ=p_ಗುಂಪು_By->a; i<p_ಗುಂಪು_By->n_ಅಭಿವ್ಯಕ್ತಿ; i++, p_ಅಂಶ++){
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ, EP_Agg) ){
          sqlite3_Error_ಸಂದೇಶ(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          ಹಿಂತಿರುಗಿ WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    ಆದರೆ( p->p_ಮುಂದಿನ && p->pEList->n_ಅಭಿವ್ಯಕ್ತಿ!=p->p_ಮುಂದಿನ->pEList->n_ಅಭಿವ್ಯಕ್ತಿ ){
      sqlite3_Select_Wrong_ಅಂಕೆ_Terms_Error(pParse, p->p_ಮುಂದಿನ);
      ಹಿಂತಿರುಗಿ WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    ಆದರೆ( (p->selFlags & SF_On_ಗಮ್ಯ_Where) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      ಆದರೆ( pParse->nErr ) ಹಿಂತಿರುಗಿ WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->p_ಪೂರ್ವ;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  ಆದರೆ( isCompound && resolve_Compound_ಕ್ರಮ_By(pParse, pLeftmost) ){
    ಹಿಂತಿರುಗಿ WRC_Abort;
  }

  ಹಿಂತಿರುಗಿ WRC_Prune;
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
ಪೂರ್ಣಾಂಕ sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ             /* The expression to be analyzed. */
){
  ಪೂರ್ಣಾಂಕ savedHasAgg;
  Walker w;

  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ಕ್ರಮ_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ಕ್ರಮ_Agg);
  w.pParse = pNC->pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಜ್ಜೆ;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolve_Select_ಹೆಜ್ಜೆ;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
  w.pParse->n_ಎತ್ತರ += p_ಅಭಿವ್ಯಕ್ತಿ->n_ಎತ್ತರ;
  ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಎತ್ತರ(w.pParse, w.pParse->n_ಎತ್ತರ) ){
    ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ!=0 );
  sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
  w.pParse->n_ಎತ್ತರ -= p_ಅಭಿವ್ಯಕ್ತಿ->n_ಎತ್ತರ;
#ಆದರೆ_ಅಂತ್ಯ
  ಪ್ರತಿಜ್ಞಾ( EP_Agg==NC_HasAgg );
  ಪ್ರತಿಜ್ಞಾ( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  ಹಿಂತಿರುಗಿ pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList         /* The expression list to be analyzed. */
){
  ಪೂರ್ಣಾಂಕ i;
  ಪೂರ್ಣಾಂಕ savedHasAgg = 0;
  Walker w;
  ಆದರೆ( pList==0 ) ಹಿಂತಿರುಗಿ SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಜ್ಜೆ;
  w.xSelectCallback = resolve_Select_ಹೆಜ್ಜೆ;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ಕ್ರಮ_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ಕ್ರಮ_Agg);
  ಗಾಗಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
    ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ = pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ;
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ) ಮುಂದುವರಿ;
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
    w.pParse->n_ಎತ್ತರ += p_ಅಭಿವ್ಯಕ್ತಿ->n_ಎತ್ತರ;
    ಆದರೆ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Check_ಎತ್ತರ(w.pParse, w.pParse->n_ಎತ್ತರ) ){
      ಹಿಂತಿರುಗಿ SQLITE_ERROR;
    }
#ಆದರೆ_ಅಂತ್ಯ
    sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(&w, p_ಅಭಿವ್ಯಕ್ತಿ);
#ಆದರೆ SQLITE_MAX_ಅಭಿವ್ಯಕ್ತಿ_ಆಳ>0
    w.pParse->n_ಎತ್ತರ -= p_ಅಭಿವ್ಯಕ್ತಿ->n_ಎತ್ತರ;
#ಆದರೆ_ಅಂತ್ಯ
    ಪ್ರತಿಜ್ಞಾ( EP_Agg==NC_HasAgg );
    ಪ್ರತಿಜ್ಞಾ( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    ಆದರೆ( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ಕ್ರಮ_Agg) ){
      ಅಭಿವ್ಯಕ್ತಿ_ಗಣ_Property(p_ಅಭಿವ್ಯಕ್ತಿ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ಕ್ರಮ_Agg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ಕ್ರಮ_Agg);
    }
    ಆದರೆ( w.pParse->nErr>0 ) ಹಿಂತಿರುಗಿ SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  ಹಿಂತಿರುಗಿ SQLITE_OK;
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
ಶೂನ್ಯ sqlite3_Resolve_Select_ಹೆಸರುಗಳು(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  ಪ್ರತಿಜ್ಞಾ( p!=0 );
  w.x_ಅಭಿವ್ಯಕ್ತಿ_Callback = resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಜ್ಜೆ;
  w.xSelectCallback = resolve_Select_ಹೆಜ್ಜೆ;
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
ಪೂರ್ಣಾಂಕ sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  ಕೋಷ್ಟಕ *pTab,     /* The table being referenced, or NULL */
  ಪೂರ್ಣಾಂಕ type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,     /* Expression to resolve.  May be NULL. */
  ಅಭಿವ್ಯಕ್ತಿ_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  ಪೂರ್ಣಾಂಕ rc;
  ಒಕ್ಕೂಟ {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  ಪ್ರತಿಜ್ಞಾ( type==0 || pTab!=0 );
  ಪ್ರತಿಜ್ಞಾ( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_ಅಭಿವ್ಯಕ್ತಿ
          || type==NC_Gen_ಲಂಬಸಾಲು || pTab==0 );
  memset(&sNC, 0, ಗಾತ್ರ(sNC));
  memset(&uSrc, 0, ಗಾತ್ರ(uSrc));
  pSrc = &uSrc.sSrc;
  ಆದರೆ( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    ಆದರೆ( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  ಆದರೆ( (rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_ಹೆಸರುಗಳು(&sNC, p_ಅಭಿವ್ಯಕ್ತಿ))!=SQLITE_OK ) ಹಿಂತಿರುಗಿ rc;
  ಆದರೆ( pList ) rc = sqlite3_Resolve_ಅಭಿವ್ಯಕ್ತಿ_List_ಹೆಸರುಗಳು(&sNC, pList);
  ಹಿಂತಿರುಗಿ rc;
}
