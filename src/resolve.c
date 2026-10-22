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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXCLUDED_ਸਾਰਣੀ_ਸੰਖਿਆ  2

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
ਸਥਿਰ ਪੂਰਨ incr_Agg_ਡੂੰਘਾਈ(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( p_ਵਿਅੰਜਕ->op==TK_AGG_FUNCTION ) p_ਵਿਅੰਜਕ->op2 += pWalker->u.n;
  ਵਾਪਸ WRC_Continue;
}
ਸਥਿਰ ਖਾਲੀ incr_Agg_Function_ਡੂੰਘਾਈ(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, ਪੂਰਨ N){
  ਜੇ( N>0 ){
    Walker w;
    memset(&w, 0, ਆਕਾਰ(w));
    w.x_ਵਿਅੰਜਕ_Callback = incr_Agg_ਡੂੰਘਾਈ;
    w.u.n = N;
    sqlite3_Walk_ਵਿਅੰਜਕ(&w, p_ਵਿਅੰਜਕ);
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
ਸਥਿਰ ਖਾਲੀ resolveAlias(
  Parse *pParse,         /* Parsing context */
  ਵਿਅੰਜਕ_List *pEList,      /* A result set */
  ਪੂਰਨ i_ਖਾਨਾ,              /* A column in the result set.  0..pEList->nExpr-1 */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,           /* Transform this into an alias to the result set */
  ਪੂਰਨ nSubquery          /* Number of subqueries that the label is moving */
){
  ਵਿਅੰਜਕ *pOrig;           /* The iCol-th column of the result set */
  ਵਿਅੰਜਕ *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  ਪ੍ਰਤਿਜ੍ਞਾ( i_ਖਾਨਾ>=0 && i_ਖਾਨਾ<pEList->n_ਵਿਅੰਜਕ );
  pOrig = pEList->a[i_ਖਾਨਾ].p_ਵਿਅੰਜਕ;
  ਪ੍ਰਤਿਜ੍ਞਾ( pOrig!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Reduced|EP_TokenOnly) );
  ਜੇ( p_ਵਿਅੰਜਕ->pAggInfo ) ਵਾਪਸ;
  db = pParse->db;
  pDup = sqlite3_ਵਿਅੰਜਕ_Dup(db, pOrig, 0);
  ਜੇ( db->mallocFailed ){
    sqlite3_ਵਿਅੰਜਕ_Delete(db, pDup);
    pDup = 0;
  }ਨਹੀਂਤਾਂ{
    ਵਿਅੰਜਕ ਆਰਜ਼ੀ;
    incr_Agg_Function_ਡੂੰਘਾਈ(pDup, nSubquery);
    ਜੇ( p_ਵਿਅੰਜਕ->op==TK_COLLATE ){
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      pDup = sqlite3_ਵਿਅੰਜਕ_ਜੋੜੋ_Collate_String(pParse, pDup, p_ਵਿਅੰਜਕ->u.zToken);
    }
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(&ਆਰਜ਼ੀ, pDup, ਆਕਾਰ(ਵਿਅੰਜਕ));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(pDup, p_ਵਿਅੰਜਕ, ਆਕਾਰ(ਵਿਅੰਜਕ));
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(p_ਵਿਅੰਜਕ, &ਆਰਜ਼ੀ, ਆਕਾਰ(ਵਿਅੰਜਕ));
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
      ਜੇ( ALWAYS(p_ਵਿਅੰਜਕ->y.pWin!=0) ){
        p_ਵਿਅੰਜਕ->y.pWin->pOwner = p_ਵਿਅੰਜਕ;
      }
    }
    sqlite3_ਵਿਅੰਜਕ_Deferred_Delete(pParse, pDup);
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
ਪੂਰਨ sqlite3_ਮੇਲ_EName(
  ਅਚਲ ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ,
  ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ,
  ਅਚਲ ਅੱਖਰ *zTab,
  ਅਚਲ ਅੱਖਰ *zDb,
  ਪੂਰਨ *pbRowid
){
  ਪੂਰਨ n;
  ਅਚਲ ਅੱਖਰ *z_ਵਿਸਤਾਰ;
  ਪੂਰਨ eEName = p_ਵਸਤੂ->fg.eEName;
  ਜੇ( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    ਵਾਪਸ 0;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pbRowid==0 || *pbRowid==0 );
  z_ਵਿਸਤਾਰ = p_ਵਸਤੂ->zEName;
  ਲਈ(n=0; ALWAYS(z_ਵਿਸਤਾਰ[n]) && z_ਵਿਸਤਾਰ[n]!='.'; n++){}
  ਜੇ( zDb && (sqlite3StrNICmp(z_ਵਿਸਤਾਰ, zDb, n)!=0 || zDb[n]!=0) ){
    ਵਾਪਸ 0;
  }
  z_ਵਿਸਤਾਰ += n+1;
  ਲਈ(n=0; ALWAYS(z_ਵਿਸਤਾਰ[n]) && z_ਵਿਸਤਾਰ[n]!='.'; n++){}
  ਜੇ( zTab && (sqlite3StrNICmp(z_ਵਿਸਤਾਰ, zTab, n)!=0 || zTab[n]!=0) ){
    ਵਾਪਸ 0;
  }
  z_ਵਿਸਤਾਰ += n+1;
  ਜੇ( z_ਖਾਨਾ ){
    ਜੇ( eEName==ENAME_TAB && sqlite3StrICmp(z_ਵਿਸਤਾਰ, z_ਖਾਨਾ)!=0 ) ਵਾਪਸ 0;
    ਜੇ( eEName==ENAME_ROWID && sqlite3IsRowid(z_ਖਾਨਾ)==0 ) ਵਾਪਸ 0;
  }
  ਜੇ( eEName==ENAME_ROWID ) *pbRowid = 1;
  ਵਾਪਸ 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
ਸਥਿਰ ਪੂਰਨ are_Double_Quoted_ਲੜੀਆਂ_Enabled(sqlite3 *db, NameContext *p_ਸਿਖਰ_NC){
  ਜੇ( db->init.busy ) ਵਾਪਸ 1;  /* Always support for legacy schemas */
  ਜੇ( p_ਸਿਖਰ_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    ਜੇ( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      ਵਾਪਸ 1;
    }
    ਵਾਪਸ (db->flags & SQLITE_DqsDDL)!=0;
  }ਨਹੀਂਤਾਂ{
    /* Currently parsing a DML statement */
    ਵਾਪਸ (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_ਵਿਅੰਜਕ_ਖਾਨਾ_ਵਰਤਿਆ(ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ n;
  ਸਾਰਣੀ *pExTab;

  n = p_ਵਿਅੰਜਕ->i_ਕਾਲਮ;
  ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
  pExTab = p_ਵਿਅੰਜਕ->y.pTab;
  ਪ੍ਰਤਿਜ੍ਞਾ( pExTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( n < pExTab->n_ਖਾਨਾ );
  ਜੇ( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->a_ਖਾਨਾ[n].ਖਾਨਾ_Flags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->n_ਖਾਨਾ==BMS-1 );
    testcase( pExTab->n_ਖਾਨਾ==BMS );
    ਵਾਪਸ pExTab->n_ਖਾਨਾ>=BMS ? ALLBITS : MASKBIT(pExTab->n_ਖਾਨਾ)-1;
  }ਨਹੀਂਤਾਂ{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    ਜੇ( n>=BMS ) n = BMS-1;
    ਵਾਪਸ ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
ਸਥਿਰ ਖਾਲੀ extendFJMatch(
  Parse *pParse,          /* Parsing context */
  ਵਿਅੰਜਕ_List **ppList,      /* ExprList to extend */
  Src_ਵਸਤੂ *p_ਮੇਲ,        /* Source table containing the column */
  i16 i_ਕਾਲਮ             /* The column number */
){
  ਵਿਅੰਜਕ *pNew = sqlite3_ਵਿਅੰਜਕ_Alloc(pParse->db, TK_ਕਾਲਮ, 0, 0);
  ਜੇ( pNew ){
    pNew->i_ਸਾਰਣੀ = p_ਮੇਲ->iCursor;
    pNew->i_ਕਾਲਮ = i_ਕਾਲਮ;
    pNew->y.pTab = p_ਮੇਲ->pSTab;
    ਪ੍ਰਤਿਜ੍ਞਾ( (p_ਮੇਲ->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ))!=0 );
    ਵਿਅੰਜਕ_ਸੈੱਟ_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_ਵਿਅੰਜਕ_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
ਸਥਿਰ SQLITE_NOINLINE ਪੂਰਨ is_Valid_Schema_ਸਾਰਣੀ_Name(
  ਅਚਲ ਅੱਖਰ *zTab,         /* Name as it appears in the SQL */
  ਸਾਰਣੀ *pTab,              /* The schema table we are trying to match */
  ਅਚਲ ਅੱਖਰ *zDb           /* non-NULL if a database qualifier is present */
){
  ਅਚਲ ਅੱਖਰ *zLegacy;
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( pTab->tnum==1 );
  ਜੇ( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) ਵਾਪਸ 0;
  zLegacy = pTab->zName;
  ਜੇ( strcmp(zLegacy+7, &LEGACY_ਆਰਜ਼ੀ_SCHEMA_ਸਾਰਣੀ[7])==0 ){
    ਜੇ( sqlite3StrICmp(zTab+7, &PREFERRED_ਆਰਜ਼ੀ_SCHEMA_ਸਾਰਣੀ[7])==0 ){
      ਵਾਪਸ 1;
    }
    ਜੇ( zDb==0 ) ਵਾਪਸ 0;
    ਜੇ( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_ਸਾਰਣੀ[7])==0 ) ਵਾਪਸ 1;
    ਜੇ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_ਸਾਰਣੀ[7])==0 ) ਵਾਪਸ 1;
  }ਨਹੀਂਤਾਂ{
    ਜੇ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_ਸਾਰਣੀ[7])==0 ) ਵਾਪਸ 1;
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ lookupName(
  Parse *pParse,       /* The parsing context */
  ਅਚਲ ਅੱਖਰ *zDb,     /* Name of the database containing table, or NULL */
  ਅਚਲ ਅੱਖਰ *zTab,    /* Name of table containing column, or NULL */
  ਅਚਲ ਵਿਅੰਜਕ *p_ਸੱਜਾ,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ          /* Make this EXPR node point to the selected column */
){
  ਪੂਰਨ i, j;                         /* Loop counters */
  ਪੂਰਨ cnt = 0;                      /* Number of matching column names */
  ਪੂਰਨ cntTab = 0;                   /* Number of potential "rowid" matches */
  ਪੂਰਨ nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  Src_ਵਸਤੂ *p_ਵਸਤੂ;                   /* Use for looping over pSrcList items */
  Src_ਵਸਤੂ *p_ਮੇਲ = 0;              /* The matching pSrcList item */
  NameContext *p_ਸਿਖਰ_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  ਪੂਰਨ e_New_ਵਿਅੰਜਕ_Op = TK_ਕਾਲਮ;       /* New value for pExpr->op on success */
  ਸਾਰਣੀ *pTab = 0;                  /* Table holding the row */
  ਵਿਅੰਜਕ_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ = p_ਸੱਜਾ->u.zToken;

  ਪ੍ਰਤਿਜ੍ਞਾ( pNC );     /* the name context cannot be NULL. */
  ਪ੍ਰਤਿਜ੍ਞਾ( z_ਖਾਨਾ );    /* The Z in X.Y.Z cannot be NULL */
  ਪ੍ਰਤਿਜ੍ਞਾ( zDb==0 || zTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = -1;
  ਵਿਅੰਜਕ_ਸੈੱਟ_VVAProperty(p_ਵਿਅੰਜਕ, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  ਜੇ( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    ਜੇ( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }ਨਹੀਂਤਾਂ{
      ਲਈ(i=0; i<db->nDb; i++){
        ਪ੍ਰਤਿਜ੍ਞਾ( db->aDb[i].zDbSName );
        ਜੇ( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          ਰੋਕੋ;
        }
      }
      ਜੇ( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  ਪ੍ਰਤਿਜ੍ਞਾ( pNC && cnt==0 );
  ਕਰੋ{
    ਵਿਅੰਜਕ_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    ਜੇ( pSrcList ){
      ਲਈ(i=0, p_ਵਸਤੂ=pSrcList->a; i<pSrcList->nSrc; i++, p_ਵਸਤੂ++){
        pTab = p_ਵਸਤੂ->pSTab;
        ਪ੍ਰਤਿਜ੍ਞਾ( pTab!=0 && pTab->zName!=0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( pTab->n_ਖਾਨਾ>0 || pParse->nErr );
        ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੂਰਨ)p_ਵਸਤੂ->fg.isNestedFrom == IsNestedFrom(p_ਵਸਤੂ));
        ਜੇ( p_ਵਸਤੂ->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          ਪੂਰਨ ਪ੍ਰਾਪਤੀ = 0;
          Select *pSel;
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.isSubquery );
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->u4.pSubq!=0 );
          pSel = p_ਵਸਤੂ->u4.pSubq->pSelect;
          ਪ੍ਰਤਿਜ੍ਞਾ( pSel!=0 );
          pEList = pSel->pEList;
          ਪ੍ਰਤਿਜ੍ਞਾ( pEList!=0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( pEList->n_ਵਿਅੰਜਕ==pTab->n_ਖਾਨਾ );
          ਲਈ(j=0; j<pEList->n_ਵਿਅੰਜਕ; j++){
            ਪੂਰਨ bRowid = 0;       /* True if possible rowid match */
            ਜੇ( !sqlite3_ਮੇਲ_EName(&pEList->a[j], z_ਖਾਨਾ, zTab, zDb, &bRowid) ){
              ਜਾਰੀ;
            }
            ਜੇ( bRowid==0 ){
              ਜੇ( cnt>0 ){
                ਜੇ( p_ਵਸਤੂ->fg.isUsing==0
                 || sqlite3_Id_List_ਸੂਚਕਾਂਕ(p_ਵਸਤੂ->u3.pUsing, z_ਖਾਨਾ)<0
                 || p_ਮੇਲ==p_ਵਸਤੂ
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ਨਹੀਂਤਾਂ
                ਜੇ( (p_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  ਜਾਰੀ;
                }ਨਹੀਂਤਾਂ
                ਜੇ( (p_ਵਸਤੂ->fg.jointype & JT_ਖੱਬਾ)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ਨਹੀਂਤਾਂ{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, p_ਮੇਲ, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ);
                }
              }
              cnt++;
              ਪ੍ਰਾਪਤੀ = 1;
            }ਨਹੀਂਤਾਂ ਜੇ( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              ਜਾਰੀ;
            }
            cntTab++;
            p_ਮੇਲ = p_ਵਸਤੂ;
            p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = j;
            pEList->a[j].fg.b_ਵਰਤਿਆ = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            ਪ੍ਰਤਿਜ੍ਞਾ( bRowid==0 || pEList->a[j].fg.b_Using_ਪਦ==0 );
            ਜੇ( pEList->a[j].fg.b_Using_ਪਦ ) ਰੋਕੋ;
          }
          ਜੇ( ਪ੍ਰਾਪਤੀ || zTab==0 ) ਜਾਰੀ;
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( zDb==0 || zTab!=0 );
        ਜੇ( zTab ){
          ਜੇ( zDb ){
            ਜੇ( pTab->pSchema!=pSchema ) ਜਾਰੀ;
            ਜੇ( pSchema==0 && strcmp(zDb,"*")!=0 ) ਜਾਰੀ;
          }
          ਜੇ( p_ਵਸਤੂ->zAlias!=0 ){
            ਜੇ( sqlite3StrICmp(zTab, p_ਵਸਤੂ->zAlias)!=0 ){
              ਜਾਰੀ;
            }
          }ਨਹੀਂਤਾਂ ਜੇ( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            ਜੇ( pTab->tnum!=1 ) ਜਾਰੀ;
            ਜੇ( !is_Valid_Schema_ਸਾਰਣੀ_Name(zTab, pTab, zDb) ) ਜਾਰੀ;
          }
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
          ਜੇ( ਵਿੱਚ_RENAME_OBJECT && p_ਵਸਤੂ->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (ਖਾਲੀ*)&p_ਵਿਅੰਜਕ->y.pTab);
          }
        }
        j = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, z_ਖਾਨਾ);
        ਜੇ( j>=0 ){
          ਜੇ( cnt>0 ){
            ਜੇ( p_ਵਸਤੂ->fg.isUsing==0
             || sqlite3_Id_List_ਸੂਚਕਾਂਕ(p_ਵਸਤੂ->u3.pUsing, z_ਖਾਨਾ)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ਨਹੀਂਤਾਂ
            ਜੇ( (p_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              ਜਾਰੀ;
            }ਨਹੀਂਤਾਂ
            ਜੇ( (p_ਵਸਤੂ->fg.jointype & JT_ਖੱਬਾ)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ਨਹੀਂਤਾਂ{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, p_ਮੇਲ, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ);
            }
          }
          cnt++;
          p_ਮੇਲ = p_ਵਸਤੂ;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = j==pTab->iPKey ? -1 : (i16)j;
          ਜੇ( p_ਵਸਤੂ->fg.isNestedFrom ){
            sqlite3_Src_ਵਸਤੂ_ਕਾਲਮ_ਵਰਤਿਆ(p_ਵਸਤੂ, j);
          }
        }
        ਜੇ( 0==cnt && VisibleRowid(pTab) ){
          /* pTab is a potential ROWID match.  Keep track of it and match
          ** the ROWID later if that seems appropriate.  (Search for "cntTab"
          ** to find related code.)  Only allow a ROWID match if there is
          ** a single ROWID match candidate.
          */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW
          /* In SQLITE_ALLOW_ROWID_IN_VIEW mode, allow a ROWID match
          ** if there is a single VIEW candidate or if there is a single
          ** non-VIEW candidate plus multiple VIEW candidates.  In other
          ** words non-VIEW candidate terms take precedence over VIEWs.
          */
          ਜੇ( cntTab==0
           || (cntTab==1
               && p_ਮੇਲ!=0
               && ALWAYS(p_ਮੇਲ->pSTab!=0)
               && (p_ਮੇਲ->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            p_ਮੇਲ = p_ਵਸਤੂ;
          }ਨਹੀਂਤਾਂ{
            cntTab++;
          }
#ਨਹੀਂਤਾਂ
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          p_ਮੇਲ = p_ਵਸਤੂ;
#ਜੇ_ਅੰਤ
        }
      }
      ਜੇ( p_ਮੇਲ ){
        p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = p_ਮੇਲ->iCursor;
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
        p_ਵਿਅੰਜਕ->y.pTab = p_ਮੇਲ->pSTab;
        ਜੇ( (p_ਮੇਲ->fg.jointype & (JT_ਖੱਬਾ|JT_LTORJ))!=0 ){
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_CanBeNull);
        }
        pSchema = p_ਵਿਅੰਜਕ->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#ਜੇ !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    ਜੇ( cnt==0 && zDb==0 ){
      pTab = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
      ਜੇ( pParse->pTriggerTab!=0 ){
        ਪੂਰਨ op = pParse->eTriggerOp;
        ਪ੍ਰਤਿਜ੍ਞਾ( op==TK_DELETE || op==TK_UPDATE || op==TK_ਪਾਓ );
        ਜੇ( pParse->bReturning ){
          ਜੇ( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || is_Valid_Schema_ਸਾਰਣੀ_Name(zTab, pParse->pTriggerTab, 0))
          ){
            p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }ਨਹੀਂਤਾਂ ਜੇ( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = 1;
          pTab = pParse->pTriggerTab;
        }ਨਹੀਂਤਾਂ ਜੇ( op!=TK_ਪਾਓ && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TRIGGER */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UPSERT
      ਜੇ( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        ਜੇ( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = EXCLUDED_ਸਾਰਣੀ_ਸੰਖਿਆ;
          ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ਖਾਲੀ*)&p_ਵਿਅੰਜਕ->y.pTab);
          }
        }
      }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UPSERT */

      ਜੇ( pTab ){
        ਪੂਰਨ i_ਖਾਨਾ;
        pSchema = pTab->pSchema;
        cntTab++;
        i_ਖਾਨਾ = sqlite3_ਕਾਲਮ_ਸੂਚਕਾਂਕ(pTab, z_ਖਾਨਾ);
        ਜੇ( i_ਖਾਨਾ>=0 ){
          ਜੇ( pTab->iPKey==i_ਖਾਨਾ ) i_ਖਾਨਾ = -1;
        }ਨਹੀਂਤਾਂ{
          ਜੇ( sqlite3IsRowid(z_ਖਾਨਾ) && VisibleRowid(pTab) ){
            i_ਖਾਨਾ = -1;
          }ਨਹੀਂਤਾਂ{
            i_ਖਾਨਾ = pTab->n_ਖਾਨਾ;
          }
        }
        ਜੇ( i_ਖਾਨਾ<pTab->n_ਖਾਨਾ ){
          cnt++;
          p_ਮੇਲ = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_UPSERT
          ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==EXCLUDED_ਸਾਰਣੀ_ਸੰਖਿਆ ){
            testcase( i_ਖਾਨਾ==(-1) );
            ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
            ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
              p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = i_ਖਾਨਾ;
              p_ਵਿਅੰਜਕ->y.pTab = pTab;
              e_New_ਵਿਅੰਜਕ_Op = TK_ਕਾਲਮ;
            }ਨਹੀਂਤਾਂ{
              p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = pNC->uNC.pUpsert->reg_ਅੰਕੜੇ +
                 sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i_ਖਾਨਾ);
              e_New_ਵਿਅੰਜਕ_Op = TK_ਰਜਿਸਟਰ;
            }
          }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_UPSERT */
          {
            ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
            p_ਵਿਅੰਜਕ->y.pTab = pTab;
            ਜੇ( pParse->bReturning ){
              e_New_ਵਿਅੰਜਕ_Op = TK_ਰਜਿਸਟਰ;
              p_ਵਿਅੰਜਕ->op2 = TK_ਕਾਲਮ;
              p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = i_ਖਾਨਾ;
              p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = pNC->uNC.i_ਅਧਾਰ_Reg + (pTab->n_ਖਾਨਾ+1)*p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ +
                 sqlite3_ਸਾਰਣੀ_ਕਾਲਮ_ਮੰਜ਼ਿਲ_Storage(pTab, i_ਖਾਨਾ) + 1;
            }ਨਹੀਂਤਾਂ{
              p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = (i16)i_ਖਾਨਾ;
              e_New_ਵਿਅੰਜਕ_Op = TK_TRIGGER;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
              ਜੇ( i_ਖਾਨਾ<0 ){
                p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ = SQLITE_AFF_INTEGER;
              }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ==0 ){
                testcase( i_ਖਾਨਾ==31 );
                testcase( i_ਖਾਨਾ==32 );
                pParse->oldmask |= (i_ਖਾਨਾ>=32 ? 0xffffffff : (((u32)1)<<i_ਖਾਨਾ));
              }ਨਹੀਂਤਾਂ{
                testcase( i_ਖਾਨਾ==31 );
                testcase( i_ਖਾਨਾ==32 );
                pParse->newmask |= (i_ਖਾਨਾ>=32 ? 0xffffffff : (((u32)1)<<i_ਖਾਨਾ));
              }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TRIGGER */
            }
          }
        }
      }
    }
#ਜੇ_ਅੰਤ /* !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT) */

    /*
    ** Perhaps the name is a reference to the ROWID
    */
    ਜੇ( cnt==0
     && cntTab>=1
     && p_ਮੇਲ
     && (pNC->ncFlags & (NC_Idx_ਵਿਅੰਜਕ|NC_Gen_ਖਾਨਾ))==0
     && sqlite3IsRowid(z_ਖਾਨਾ)
     && ALWAYS(VisibleRowid(p_ਮੇਲ->pSTab) || p_ਮੇਲ->fg.isNestedFrom)
    ){
      cnt = cntTab;
#ਜੇ SQLITE_ALLOW_ROWID_ਵਿੱਚ_VIEW+0==2
      ਜੇ( p_ਮੇਲ->pSTab!=0 && IsView(p_ਮੇਲ->pSTab) ){
        e_New_ਵਿਅੰਜਕ_Op = TK_NULL;
      }
#ਜੇ_ਅੰਤ
      ਜੇ( p_ਮੇਲ->fg.isNestedFrom==0 ) p_ਵਿਅੰਜਕ->i_ਕਾਲਮ = -1;
      p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ = SQLITE_AFF_INTEGER;
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
    ਜੇ( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      ਪ੍ਰਤਿਜ੍ਞਾ( pEList!=0 );
      ਲਈ(j=0; j<pEList->n_ਵਿਅੰਜਕ; j++){
        ਅੱਖਰ *zAs = pEList->a[j].zEName;
        ਜੇ( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, z_ਖਾਨਾ)==0
        ){
          ਵਿਅੰਜਕ *pOrig;
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ==0 && p_ਵਿਅੰਜਕ->p_ਸੱਜਾ==0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ)==0 || p_ਵਿਅੰਜਕ->x.pList==0 );
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ)==0 || p_ਵਿਅੰਜਕ->x.pSelect==0 );
          pOrig = pEList->a[j].p_ਵਿਅੰਜਕ;
          ਜੇ( (pNC->ncFlags&NC_AllowAgg)==0 && ਵਿਅੰਜਕ_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_ਸੁਨੇਹਾ(pParse, "misuse of aliased aggregate %s", zAs);
            ਵਾਪਸ WRC_Abort;
          }
          ਜੇ( ਵਿਅੰਜਕ_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_ਸਿਖਰ_NC )
          ){
            sqlite3_Error_ਸੁਨੇਹਾ(pParse, "misuse of aliased window function %s",zAs);
            ਵਾਪਸ WRC_Abort;
          }
          ਜੇ( sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(pOrig)!=1 ){
            sqlite3_Error_ਸੁਨੇਹਾ(pParse, "row value misused");
            ਵਾਪਸ WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_ਵਿਅੰਜਕ, nSubquery);
          cnt = 1;
          p_ਮੇਲ = 0;
          ਪ੍ਰਤਿਜ੍ਞਾ( zTab==0 && zDb==0 );
          ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ਖਾਲੀ*)p_ਵਿਅੰਜਕ);
          }
          ਜਾਓ lookupname_ਸਿਰਾ;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    ਜੇ( cnt ) ਰੋਕੋ;
    pNC = pNC->p_ਅਗਲਾ;
    nSubquery++;
  }ਜਦੋਂ( pNC );


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
  ਜੇ( cnt==0 && zTab==0 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_ID );
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_DblQuoted)
     && are_Double_Quoted_ਲੜੀਆਂ_Enabled(db, p_ਸਿਖਰ_NC)
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
        "double-quoted string literal: \"%w\"", z_ਖਾਨਾ);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_NORMALIZE
      sqlite3_Vdbe_ਜੋੜੋ_Dblquote_Str(db, pParse->pVdbe, z_ਖਾਨਾ);
#ਜੇ_ਅੰਤ
      p_ਵਿਅੰਜਕ->op = TK_STRING;
      memset(&p_ਵਿਅੰਜਕ->y, 0, ਆਕਾਰ(p_ਵਿਅੰਜਕ->y));
      ਵਾਪਸ WRC_Prune;
    }
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Id_ਮੰਜ਼ਿਲ_True_False(p_ਵਿਅੰਜਕ) ){
      ਵਾਪਸ WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  ਪ੍ਰਤਿਜ੍ਞਾ( pFJMatch==0 || cnt>0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_xIsSelect|EP_Int_ਮੁੱਲ) );
  ਜੇ( cnt!=1 ){
    ਅਚਲ ਅੱਖਰ *zErr;
    ਜੇ( pFJMatch ){
      ਜੇ( pFJMatch->n_ਵਿਅੰਜਕ==cnt-1 ){
        ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,EP_ਪੱਤੀ) ){
          ਵਿਅੰਜਕ_Clear_Property(p_ਵਿਅੰਜਕ,EP_ਪੱਤੀ);
        }ਨਹੀਂਤਾਂ{
          sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
          p_ਵਿਅੰਜਕ->p_ਖੱਬਾ = 0;
          sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
          p_ਵਿਅੰਜਕ->p_ਸੱਜਾ = 0;
        }
        extendFJMatch(pParse, &pFJMatch, p_ਮੇਲ, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ);
        p_ਵਿਅੰਜਕ->op = TK_FUNCTION;
        p_ਵਿਅੰਜਕ->u.zToken = "coalesce";
        p_ਵਿਅੰਜਕ->x.pList = pFJMatch;
        p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ = SQLITE_AFF_DEFER;
        cnt = 1;
        ਜਾਓ lookupname_ਸਿਰਾ;
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਵਿਅੰਜਕ_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    ਜੇ( zDb ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, z_ਖਾਨਾ);
    }ਨਹੀਂਤਾਂ ਜੇ( zTab ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s: %s.%s", zErr, zTab, z_ਖਾਨਾ);
    }ਨਹੀਂਤਾਂ ਜੇ( cnt==0 && ਵਿਅੰਜਕ_Has_Property(p_ਸੱਜਾ,EP_DblQuoted) ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, z_ਖਾਨਾ);
    }ਨਹੀਂਤਾਂ{
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s: %s", zErr, z_ਖਾਨਾ);
    }
    sqlite3_ਰਿਕਾਰਡ_Error_ਵਿੱਥ_ਦਾ_ਵਿਅੰਜਕ(pParse->db, p_ਵਿਅੰਜਕ);
    pParse->checkSchema = 1;
    p_ਸਿਖਰ_NC->nNcErr++;
    e_New_ਵਿਅੰਜਕ_Op = TK_NULL;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,(EP_TokenOnly|EP_ਪੱਤੀ)) ){
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
    p_ਵਿਅੰਜਕ->p_ਖੱਬਾ = 0;
    sqlite3_ਵਿਅੰਜਕ_Delete(db, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
    p_ਵਿਅੰਜਕ->p_ਸੱਜਾ = 0;
    ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_ਪੱਤੀ);
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
  ਜੇ( p_ਮੇਲ ){
    ਜੇ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ>=0 ){
      p_ਮੇਲ->ਖਾਨਾ_ਵਰਤਿਆ |= sqlite3_ਵਿਅੰਜਕ_ਖਾਨਾ_ਵਰਤਿਆ(p_ਵਿਅੰਜਕ);
    }ਨਹੀਂਤਾਂ{
      p_ਮੇਲ->fg.rowid_ਵਰਤਿਆ = 1;
    }
  }

  p_ਵਿਅੰਜਕ->op = e_New_ਵਿਅੰਜਕ_Op;
lookupname_ਸਿਰਾ:
  ਜੇ( cnt==1 ){
    ਪ੍ਰਤਿਜ੍ਞਾ( pNC!=0 );
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
    ਜੇ( db->xAuth ){
      ਜੇ( pFJMatch ){
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op==TK_FUNCTION );
        ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_stricmp(p_ਵਿਅੰਜਕ->u.zToken,"coalesce")==0 );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pList==pFJMatch );
        ਪ੍ਰਤਿਜ੍ਞਾ( pFJMatch->n_ਵਿਅੰਜਕ>0 );
        ਲਈ(i=0; i<pFJMatch->n_ਵਿਅੰਜਕ; i++){
          ਪ੍ਰਤਿਜ੍ਞਾ( pFJMatch->a[i].p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_ਵਿਅੰਜਕ,pSchema,pNC->pSrcList);
        }
      }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ਕਾਲਮ || p_ਵਿਅੰਜਕ->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_ਵਿਅੰਜਕ, pSchema, pNC->pSrcList);
      }
    } 
#ਜੇ_ਅੰਤ
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    ਲਈ(;;){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸਿਖਰ_NC!=0 );
      p_ਸਿਖਰ_NC->nRef++;
      ਜੇ( p_ਸਿਖਰ_NC==pNC ) ਰੋਕੋ;
      p_ਸਿਖਰ_NC = p_ਸਿਖਰ_NC->p_ਅਗਲਾ;
    }
    ਵਾਪਸ WRC_Prune;
  } ਨਹੀਂਤਾਂ {
    ਵਾਪਸ WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
ਵਿਅੰਜਕ *sqlite3_Create_ਕਾਲਮ_ਵਿਅੰਜਕ(sqlite3 *db, SrcList *pSrc, ਪੂਰਨ iSrc, ਪੂਰਨ i_ਖਾਨਾ){
  ਵਿਅੰਜਕ *p = sqlite3_ਵਿਅੰਜਕ_Alloc(db, TK_ਕਾਲਮ, 0, 0);
  ਜੇ( p ){
    Src_ਵਸਤੂ *p_ਵਸਤੂ = &pSrc->a[iSrc];
    ਸਾਰਣੀ *pTab;
    ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p) );
    pTab = p->y.pTab = p_ਵਸਤੂ->pSTab;
    p->i_ਸਾਰਣੀ = p_ਵਸਤੂ->iCursor;
    ਜੇ( p->y.pTab->iPKey==i_ਖਾਨਾ ){
      p->i_ਕਾਲਮ = -1;
    }ਨਹੀਂਤਾਂ{
      p->i_ਕਾਲਮ = (ynVar)i_ਖਾਨਾ;
      ਜੇ( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->a_ਖਾਨਾ[i_ਖਾਨਾ].ਖਾਨਾ_Flags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->n_ਖਾਨਾ==63 );
        testcase( pTab->n_ਖਾਨਾ==64 );
        p_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ = pTab->n_ਖਾਨਾ>=64 ? ALLBITS : MASKBIT(pTab->n_ਖਾਨਾ)-1;
      }ਨਹੀਂਤਾਂ{
        testcase( i_ਖਾਨਾ==BMS );
        testcase( i_ਖਾਨਾ==BMS-1 );
        p_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ |= ((Bitmask)1)<<(i_ਖਾਨਾ>=BMS ? BMS-1 : i_ਖਾਨਾ);
      }
    }
  }
  ਵਾਪਸ p;
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
ਸਥਿਰ ਖਾਲੀ notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   ਅਚਲ ਅੱਖਰ *z_ਸੁਨੇਹਾ,    /* Type of error */
   ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,         /* Invalidate this expression on error */
   ਵਿਅੰਜਕ *pError         /* Associate error with this expression */
){
  ਅਚਲ ਅੱਖਰ *z_ਵਿੱਚ = "partial index WHERE clauses";
  ਜੇ( pNC->ncFlags & NC_Idx_ਵਿਅੰਜਕ )      z_ਵਿੱਚ = "index expressions";
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CHECK
  ਨਹੀਂਤਾਂ ਜੇ( pNC->ncFlags & NC_IsCheck ) z_ਵਿੱਚ = "CHECK constraints";
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_GENERATED_COLUMNS
  ਨਹੀਂਤਾਂ ਜੇ( pNC->ncFlags & NC_Gen_ਖਾਨਾ ) z_ਵਿੱਚ = "generated columns";
#ਜੇ_ਅੰਤ
  sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%s prohibited in %s", z_ਸੁਨੇਹਾ, z_ਵਿੱਚ);
  ਜੇ( p_ਵਿਅੰਜਕ ) p_ਵਿਅੰਜਕ->op = TK_NULL;
  sqlite3_ਰਿਕਾਰਡ_Error_ਵਿੱਥ_ਦਾ_ਵਿਅੰਜਕ(pParse->db, pError);
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sqlite3ResolveNotValid(P,N,M,X,E,R) \
  ਪ੍ਰਤਿਜ੍ਞਾ( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_ਵਿਅੰਜਕ|NC_Gen_ਖਾਨਾ))==0 ); \
  ਜੇ( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
ਸਥਿਰ ਪੂਰਨ ਵਿਅੰਜਕ_Probability(ਵਿਅੰਜਕ *p){
  ਦੁੱਗਣਾ r = -1.0;
  ਜੇ( p->op!=TK_FLOAT ) ਵਾਪਸ -1;
  ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p, EP_Int_ਮੁੱਲ) );
  sqlite3AtoF(p->u.zToken, &r);
  ਪ੍ਰਤਿਜ੍ਞਾ( r>=0.0 );
  ਜੇ( r>1.0 ) ਵਾਪਸ -1;
  ਵਾਪਸ (ਪੂਰਨ)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
ਸਥਿਰ SQLITE_NOINLINE ਖਾਲੀ resolve_ਸੈੱਟ_ਵਿਅੰਜਕ_Subtype_Arg(ਵਿਅੰਜਕ_List *pList){
  ਪੂਰਨ nn, ii;
  nn = pList ? pList->n_ਵਿਅੰਜਕ : 0;
  ਲਈ(ii=0; ii<nn; ii++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pList->a[ii].p_ਵਿਅੰਜਕ;
    ਜਦੋਂ( 1 /*exit-by-break*/ ){
      ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_SubtArg);
      ਜੇ( p_ਵਿਅੰਜਕ->op==TK_SELECT ){
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pSelect!=0 );
        resolve_ਸੈੱਟ_ਵਿਅੰਜਕ_Subtype_Arg(p_ਵਿਅੰਜਕ->x.pSelect->pEList);
        ਰੋਕੋ;
      }
      ਜੇ( p_ਵਿਅੰਜਕ->op==TK_UPLUS ){
        p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
      }ਨਹੀਂਤਾਂ{
        ਰੋਕੋ;
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
ਸਥਿਰ ਪੂਰਨ resolve_ਵਿਅੰਜਕ_ਕਦਮ(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  ਪ੍ਰਤਿਜ੍ਞਾ( pNC!=0 );
  pParse = pNC->pParse;
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse==pWalker->pParse );

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  ਜੇ( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    ਪੂਰਨ i;
    ਲਈ(i=0; i<pNC->pSrcList->nSrc; i++){
      ਪ੍ਰਤਿਜ੍ਞਾ( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#ਜੇ_ਅੰਤ
  ਚੋਣ( p_ਵਿਅੰਜਕ->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    ਹਾਲਤ TK_ਪੰਕਤੀ: {
      SrcList *pSrcList = pNC->pSrcList;
      Src_ਵਸਤੂ *p_ਵਸਤੂ;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSrcList && pSrcList->nSrc>=1 );
      p_ਵਸਤੂ = pSrcList->a;
      p_ਵਿਅੰਜਕ->op = TK_ਕਾਲਮ;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
      p_ਵਿਅੰਜਕ->y.pTab = p_ਵਸਤੂ->pSTab;
      p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = p_ਵਸਤੂ->iCursor;
      p_ਵਿਅੰਜਕ->i_ਕਾਲਮ--;
      p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ = SQLITE_AFF_INTEGER;
      ਰੋਕੋ;
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
    ਹਾਲਤ TK_NOTNULL:
    ਹਾਲਤ TK_ISNULL: {
      ਪੂਰਨ anRef[8];
      NameContext *p;
      ਪੂਰਨ i;
      ਲਈ(i=0, p=pNC; p && i<Array_ਮਾਪ(anRef); p=p->p_ਅਗਲਾ, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
      ਜੇ( ਵਿੱਚ_RENAME_OBJECT ) ਵਾਪਸ WRC_Prune;
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Can_Be_Null(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) ){
        /* The expression can be NULL.  So the optimization does not apply */
        ਵਾਪਸ WRC_Prune;
      }

      ਲਈ(i=0, p=pNC; p; p=p->p_ਅਗਲਾ, i++){
        ਜੇ( (p->ncFlags & NC_Where)==0 ){
          ਵਾਪਸ WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) );
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
#ਜੇ TREETRACE_ENABLED
      ਜੇ( sqlite3_ਰੁੱਖ_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_ਵਿਅੰਜਕ->op==TK_NOTNULL
        );
        sqlite3_Show_ਵਿਅੰਜਕ(p_ਵਿਅੰਜਕ);
      }
#ਜੇ_ਅੰਤ /* TREETRACE_ENABLED */
      p_ਵਿਅੰਜਕ->u.i_ਮੁੱਲ = (p_ਵਿਅੰਜਕ->op==TK_NOTNULL);
      p_ਵਿਅੰਜਕ->flags |= EP_Int_ਮੁੱਲ;
      p_ਵਿਅੰਜਕ->op = TK_INTEGER;
      ਲਈ(i=0, p=pNC; p && i<Array_ਮਾਪ(anRef); p=p->p_ਅਗਲਾ, i++){
        p->nRef = anRef[i];
      }
      sqlite3_ਵਿਅੰਜਕ_Delete(pParse->db, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
      p_ਵਿਅੰਜਕ->p_ਖੱਬਾ = 0;
      ਵਾਪਸ WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    ਹਾਲਤ TK_ID:
    ਹਾਲਤ TK_DOT: {
      ਅਚਲ ਅੱਖਰ *z_ਸਾਰਣੀ;
      ਅਚਲ ਅੱਖਰ *zDb;
      ਵਿਅੰਜਕ *p_ਸੱਜਾ;

      ਜੇ( p_ਵਿਅੰਜਕ->op==TK_ID ){
        zDb = 0;
        z_ਸਾਰਣੀ = 0;
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
        p_ਸੱਜਾ = p_ਵਿਅੰਜਕ;
      }ਨਹੀਂਤਾਂ{
        ਵਿਅੰਜਕ *p_ਖੱਬਾ = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
        testcase( pNC->ncFlags & NC_Idx_ਵਿਅੰਜਕ );
        testcase( pNC->ncFlags & NC_Gen_ਖਾਨਾ );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_ਵਿਅੰਜਕ|NC_Gen_ਖਾਨਾ, 0, p_ਵਿਅੰਜਕ);
        p_ਸੱਜਾ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        ਜੇ( p_ਸੱਜਾ->op==TK_ID ){
          zDb = 0;
        }ਨਹੀਂਤਾਂ{
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਸੱਜਾ->op==TK_DOT );
          ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਸੱਜਾ, EP_Int_ਮੁੱਲ) );
          zDb = p_ਖੱਬਾ->u.zToken;
          p_ਖੱਬਾ = p_ਸੱਜਾ->p_ਖੱਬਾ;
          p_ਸੱਜਾ = p_ਸੱਜਾ->p_ਸੱਜਾ;
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_UToken(p_ਖੱਬਾ) && ਵਿਅੰਜਕ_Use_UToken(p_ਸੱਜਾ) );
        z_ਸਾਰਣੀ = p_ਖੱਬਾ->u.zToken;
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
        ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (ਖਾਲੀ*)p_ਵਿਅੰਜਕ, (ਖਾਲੀ*)p_ਸੱਜਾ);
          sqlite3RenameTokenRemap(pParse, (ਖਾਲੀ*)&p_ਵਿਅੰਜਕ->y.pTab, (ਖਾਲੀ*)p_ਖੱਬਾ);
        }
      }
      ਵਾਪਸ lookupName(pParse, zDb, z_ਸਾਰਣੀ, p_ਸੱਜਾ, pNC, p_ਵਿਅੰਜਕ);
    }

    /* Resolve function names
    */
    ਹਾਲਤ TK_FUNCTION: {
      ਵਿਅੰਜਕ_List *pList;            /* The argument list */
      ਪੂਰਨ n;                      /* Number of arguments */
      ਪੂਰਨ no_such_func = 0;       /* True if no such function exists */
      ਪੂਰਨ wrong_ਅੰਕ_ਦਲੀਲਾਂ = 0;     /* True if wrong number of arguments */
      ਪੂਰਨ is_agg = 0;             /* True if is an aggregate function */
      ਅਚਲ ਅੱਖਰ *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      ਪੂਰਨ savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਖਿੜਕੀ *pWin = (Is_ਖਿੜਕੀ_Func(p_ਵਿਅੰਜਕ) ? p_ਵਿਅੰਜਕ->y.pWin : 0);
#ਜੇ_ਅੰਤ
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_xIsSelect|EP_Int_ਮੁੱਲ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ==0 || p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->op==TK_ਕ੍ਰਮ );
      pList = p_ਵਿਅੰਜਕ->x.pList;
      n = pList ? pList->n_ਵਿਅੰਜਕ : 0;
      zId = p_ਵਿਅੰਜਕ->u.zToken;
      pDef = sqlite3_ਲੱਭੋ_Function(pParse->db, zId, n, enc, 0);
      ਜੇ( pDef==0 ){
        pDef = sqlite3_ਲੱਭੋ_Function(pParse->db, zId, -2, enc, 0);
        ਜੇ( pDef==0 ){
          no_such_func = 1;
        }ਨਹੀਂਤਾਂ{
          wrong_ਅੰਕ_ਦਲੀਲਾਂ = 1;
        }
      }ਨਹੀਂਤਾਂ{
        is_agg = pDef->xFinalize!=0;
        ਜੇ( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_Unlikely);
          ਜੇ( n==2 ){
            p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = ਵਿਅੰਜਕ_Probability(pList->a[1].p_ਵਿਅੰਜਕ);
            ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ<0 ){
              sqlite3_Error_ਸੁਨੇਹਾ(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_ਵਿਅੰਜਕ);
              pNC->nNcErr++;
            }
          }ਨਹੀਂਤਾਂ{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_AUTHORIZATION
        {
          ਪੂਰਨ auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          ਜੇ( auth!=SQLITE_OK ){
            ਜੇ( auth==SQLITE_DENY ){
              sqlite3_Error_ਸੁਨੇਹਾ(pParse, "not authorized to use function: %#T",
                                      p_ਵਿਅੰਜਕ);
              pNC->nNcErr++;
            }
            p_ਵਿਅੰਜਕ->op = TK_NULL;
            ਵਾਪਸ WRC_Prune;
          }
        }
#ਜੇ_ਅੰਤ

        /* If the function may call sqlite3_value_subtype(), then set the 
        ** EP_SubtArg flag on all of its argument expressions. This prevents
        ** where.c from replacing the expression with a value read from an
        ** index on the same expression, which will not have the correct 
        ** subtype. Also set the flag if the function expression itself is
        ** an EP_SubtArg expression. In this case subtypes are required as 
        ** the function may return a value with a subtype back to its 
        ** caller using sqlite3_result_value().  */
        ਜੇ( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_SubtArg) 
        ){
          resolve_ਸੈੱਟ_ਵਿਅੰਜਕ_Subtype_Arg(pList);
        }

        ਜੇ( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ,EP_ConstFunc);
        }
        ਜੇ( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_ਵਿਅੰਜਕ|NC_PartIdx|NC_Gen_ਖਾਨਾ, 0, p_ਵਿਅੰਜਕ);
        }ਨਹੀਂਤਾਂ{
          ਪ੍ਰਤਿਜ੍ਞਾ( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_ਵਿਅੰਜਕ->op2 = pNC->ncFlags & NC_SelfRef;
        }
        ਜੇ( (pDef->funcFlags & SQLITE_FUNC_ਅੰਦਰੂਨੀ)!=0
         && pParse->nested==0
         && (pParse->db->mDbFlags & DBFLAG_ਅੰਦਰੂਨੀ_Func)==0
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
        }ਨਹੀਂਤਾਂ
        ਜੇ( (pDef->funcFlags & (SQLITE_FUNC_ਸਿੱਧਾ|SQLITE_FUNC_UNSAFE))!=0
         && !ਵਿੱਚ_RENAME_OBJECT
        ){
          ਜੇ( pNC->ncFlags & NC_FromDDL ) ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_FromDDL);
          sqlite3_ਵਿਅੰਜਕ_Function_Usable(pParse, p_ਵਿਅੰਜਕ, pDef);
        }
      }

      ਜੇ( 0==ਵਿੱਚ_RENAME_OBJECT ){
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
        ਪ੍ਰਤਿਜ੍ਞਾ( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_ਮੁੱਲ==0 && pDef->xInverse==0)
          || (pDef->x_ਮੁੱਲ && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        ਜੇ( pDef && pDef->x_ਮੁੱਲ==0 && pWin ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse,
              "%#T() may not be used as a window function", p_ਵਿਅੰਜਕ
          );
          pNC->nNcErr++;
        }ਨਹੀਂਤਾਂ ਜੇ(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_ਖਿੜਕੀ) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          ਅਚਲ ਅੱਖਰ *zType;
          ਜੇ( (pDef->funcFlags & SQLITE_FUNC_ਖਿੜਕੀ) || pWin ){
            zType = "window";
          }ਨਹੀਂਤਾਂ{
            zType = "aggregate";
          }
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "misuse of %s function %#T()",zType,p_ਵਿਅੰਜਕ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#ਨਹੀਂਤਾਂ
        ਜੇ( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse,"misuse of aggregate function %#T()",p_ਵਿਅੰਜਕ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#ਜੇ_ਅੰਤ
        ਨਹੀਂਤਾਂ ਜੇ( no_such_func
              && (pParse->db->init.busy==0 ||
                  (no_such_func==2 && pParse->db->init.busy==2))
              /* Suppress "no such function" errors when reading
              ** the sqlite_schema table.  Except, do raise the error
              ** if init.busy is 2, meaning the schema parse is due
              ** to an ALTER TABLE ADD COLUMN statement, and the function
              ** is an internal-use-only function (no_such_func==2). */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
              && pParse->explain==0
#ਜੇ_ਅੰਤ
        ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "no such function: %#T", p_ਵਿਅੰਜਕ);
          pNC->nNcErr++;
        }ਨਹੀਂਤਾਂ ਜੇ( wrong_ਅੰਕ_ਦਲੀਲਾਂ ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse,"wrong number of arguments to function %#T()",
               p_ਵਿਅੰਜਕ);
          pNC->nNcErr++;
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
        ਨਹੀਂਤਾਂ ਜੇ( is_agg==0 && ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_ਵਿਅੰਜਕ
          );
          pNC->nNcErr++;
        }
#ਜੇ_ਅੰਤ
        ਨਹੀਂਤਾਂ ਜੇ( is_agg==0 && p_ਵਿਅੰਜਕ->p_ਖੱਬਾ ){
          sqlite3_ਵਿਅੰਜਕ_ਕ੍ਰਮ_By_Aggregate_Error(pParse, p_ਵਿਅੰਜਕ);
          pNC->nNcErr++;
        }
        ਜੇ( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#ਨਹੀਂਤਾਂ
          pNC->ncFlags &= ~NC_AllowAgg;
#ਜੇ_ਅੰਤ
        }
      }
      ਨਹੀਂਤਾਂ ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) || p_ਵਿਅੰਜਕ->p_ਖੱਬਾ ){
        is_agg = 1;
      }
      sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, pList);
      ਜੇ( is_agg ){
        ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ ){
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->op==TK_ਕ੍ਰਮ );
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) );
          sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->x.pList);
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
        ਜੇ( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YWin(p_ਵਿਅੰਜਕ) && pWin==p_ਵਿਅੰਜਕ->y.pWin );
          ਜੇ( ਵਿੱਚ_RENAME_OBJECT==0 ){
            sqlite3_ਖਿੜਕੀ_Update(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            ਜੇ( pParse->db->mallocFailed ) ਰੋਕੋ;
          }
          sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, pWin->p_ਹਿੱਸੇ_ਕਰੋ);
          sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, pWin->p_ਕ੍ਰਮ_By);
          sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pWin->pFilter);
          sqlite3_ਖਿੜਕੀ_Link(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_ਵਿਅੰਜਕ->op = TK_AGG_FUNCTION;
          p_ਵਿਅੰਜਕ->op2 = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
          ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
            sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, p_ਵਿਅੰਜਕ->y.pWin->pFilter);
          }
#ਜੇ_ਅੰਤ
          pNC2 = pNC;
          ਜਦੋਂ( pNC2
              && sqlite3ReferencesSrcList(pParse, p_ਵਿਅੰਜਕ, pNC2->pSrcList)==0
          ){
            p_ਵਿਅੰਜਕ->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->p_ਅਗਲਾ;
          }
          ਪ੍ਰਤਿਜ੍ਞਾ( pDef!=0 || ਵਿੱਚ_RENAME_OBJECT );
          ਜੇ( pNC2 && pDef ){
            p_ਵਿਅੰਜਕ->op2 += pNC2->nNestedSelect;
            ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            ਪ੍ਰਤਿਜ੍ਞਾ( SQLITE_FUNC_ANYORDER==NC_ਕ੍ਰਮ_Agg );
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
      ਵਾਪਸ WRC_Prune;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    ਹਾਲਤ TK_EXISTS:
    ਹਾਲਤ TK_SELECT:
#ਜੇ_ਅੰਤ
    ਹਾਲਤ TK_ਵਿੱਚ: {
      testcase( p_ਵਿਅੰਜਕ->op==TK_ਵਿੱਚ );
      testcase( p_ਵਿਅੰਜਕ->op==TK_EXISTS );
      testcase( p_ਵਿਅੰਜਕ->op==TK_SELECT );
      ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
        ਪੂਰਨ nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_ਵਿਅੰਜਕ );
        testcase( pNC->ncFlags & NC_Gen_ਖਾਨਾ );
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pSelect );
        ਜੇ( p_ਵਿਅੰਜਕ->op==TK_EXISTS )  pParse->bHasExists = 1;
        ਜੇ( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_ਵਿਅੰਜਕ, p_ਵਿਅੰਜਕ);
        }ਨਹੀਂਤਾਂ{
          sqlite3WalkSelect(pWalker, p_ਵਿਅੰਜਕ->x.pSelect);
        }
        ਪ੍ਰਤਿਜ੍ਞਾ( pNC->nRef>=nRef );
        ਜੇ( nRef!=pNC->nRef ){
          ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, EP_VarSelect);
          p_ਵਿਅੰਜਕ->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_ਵਿਅੰਜਕ );
      testcase( pNC->ncFlags & NC_Gen_ਖਾਨਾ );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_ਵਿਅੰਜਕ|NC_Gen_ਖਾਨਾ, p_ਵਿਅੰਜਕ, p_ਵਿਅੰਜਕ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_IS:
    ਹਾਲਤ TK_ISNOT: {
      ਵਿਅੰਜਕ *p_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      ਜੇ( ALWAYS(p_ਸੱਜਾ) && (p_ਸੱਜਾ->op==TK_ID || p_ਸੱਜਾ->op==TK_TRUEFALSE) ){
        ਪੂਰਨ rc = resolve_ਵਿਅੰਜਕ_ਕਦਮ(pWalker, p_ਸੱਜਾ);
        ਜੇ( rc==WRC_Abort ) ਵਾਪਸ WRC_Abort;
        ਜੇ( p_ਸੱਜਾ->op==TK_TRUEFALSE ){
          p_ਵਿਅੰਜਕ->op2 = p_ਵਿਅੰਜਕ->op;
          p_ਵਿਅੰਜਕ->op = TK_TRUTH;
          ਵਾਪਸ WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    ਹਾਲਤ TK_BETWEEN:
    ਹਾਲਤ TK_EQ:
    ਹਾਲਤ TK_NE:
    ਹਾਲਤ TK_LT:
    ਹਾਲਤ TK_LE:
    ਹਾਲਤ TK_GT:
    ਹਾਲਤ TK_GE: {
      ਪੂਰਨ n_ਖੱਬਾ, n_ਸੱਜਾ;
      ਜੇ( pParse->db->mallocFailed ) ਰੋਕੋ;
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ!=0 );
      n_ਖੱਬਾ = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ);
      ਜੇ( p_ਵਿਅੰਜਕ->op==TK_BETWEEN ){
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
        n_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ->x.pList->a[0].p_ਵਿਅੰਜਕ);
        ਜੇ( n_ਸੱਜਾ==n_ਖੱਬਾ ){
          n_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ->x.pList->a[1].p_ਵਿਅੰਜਕ);
        }
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਸੱਜਾ!=0 );
        n_ਸੱਜਾ = sqlite3_ਵਿਅੰਜਕ_ਸਦਿਸ਼_ਮਾਪ(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
      }
      ਜੇ( n_ਖੱਬਾ!=n_ਸੱਜਾ ){
        testcase( p_ਵਿਅੰਜਕ->op==TK_EQ );
        testcase( p_ਵਿਅੰਜਕ->op==TK_NE );
        testcase( p_ਵਿਅੰਜਕ->op==TK_LT );
        testcase( p_ਵਿਅੰਜਕ->op==TK_LE );
        testcase( p_ਵਿਅੰਜਕ->op==TK_GT );
        testcase( p_ਵਿਅੰਜਕ->op==TK_GE );
        testcase( p_ਵਿਅੰਜਕ->op==TK_IS );
        testcase( p_ਵਿਅੰਜਕ->op==TK_ISNOT );
        testcase( p_ਵਿਅੰਜਕ->op==TK_BETWEEN );
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "row value misused");
        sqlite3_ਰਿਕਾਰਡ_Error_ਵਿੱਥ_ਦਾ_ਵਿਅੰਜਕ(pParse->db, p_ਵਿਅੰਜਕ);
      }
      ਰੋਕੋ;
    }
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ਵਾਪਸ pParse->nErr ? WRC_Abort : WRC_Continue;
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
ਸਥਿਰ ਪੂਰਨ resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  ਵਿਅੰਜਕ_List *pEList,  /* List of expressions to scan */
  ਵਿਅੰਜਕ *pE           /* Expression we are trying to match */
){
  ਪੂਰਨ i;             /* Loop counter */

  ਅਣਵਰਤਿਆ_PARAMETER(pParse);

  ਜੇ( pE->op==TK_ID ){
    ਅਚਲ ਅੱਖਰ *z_ਖਾਨਾ;
    ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(pE, EP_Int_ਮੁੱਲ) );
    z_ਖਾਨਾ = pE->u.zToken;
    ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
      ਜੇ( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, z_ਖਾਨਾ)==0
      ){
        ਵਾਪਸ i+1;
      }
    }
  }
  ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ resolve_ਕ੍ਰਮ_By_ਪਦ_ਮੰਜ਼ਿਲ_ਵਿਅੰਜਕ_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  ਵਿਅੰਜਕ *pE           /* The specific ORDER BY term */
){
  ਪੂਰਨ i;             /* Loop counter */
  ਵਿਅੰਜਕ_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  ਪੂਰਨ rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਵਿਅੰਜਕ_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, ਆਕਾਰ(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&nc, pE);
  db->suppressErr = savedSuppErr;
  ਜੇ( rc ) ਵਾਪਸ 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  ਲਈ(i=0; i<pEList->n_ਵਿਅੰਜਕ; i++){
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0, pEList->a[i].p_ਵਿਅੰਜਕ, pE, -1)<2 ){
      ਵਾਪਸ i+1;
    }
  }

  /* If no match, return 0. */
  ਵਾਪਸ 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
ਸਥਿਰ ਖਾਲੀ resolve_ਬਾਹਰਲਾ_ਦਾ_Range_Error(
  Parse *pParse,         /* The error context into which to write the error */
  ਅਚਲ ਅੱਖਰ *zType,     /* "ORDER" or "GROUP" */
  ਪੂਰਨ i,                 /* The index (1-based) of the term out of range */
  ਪੂਰਨ mx,                /* Largest permissible value of i */
  ਵਿਅੰਜਕ *pError           /* Associate the error with the expression */
){
  sqlite3_Error_ਸੁਨੇਹਾ(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_ਰਿਕਾਰਡ_Error_ਵਿੱਥ_ਦਾ_ਵਿਅੰਜਕ(pParse->db, pError);
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
ਸਥਿਰ ਪੂਰਨ resolve_Compound_ਕ੍ਰਮ_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  ਪੂਰਨ i;
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By;
  ਵਿਅੰਜਕ_List *pEList;
  sqlite3 *db;
  ਪੂਰਨ ਹੋਰ_ਮੰਜ਼ਿਲ_Do = 1;

  p_ਕ੍ਰਮ_By = pSelect->p_ਕ੍ਰਮ_By;
  ਜੇ( p_ਕ੍ਰਮ_By==0 ) ਵਾਪਸ 0;
  db = pParse->db;
  ਜੇ( p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ>db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਲਮ] ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many terms in ORDER BY clause");
    ਵਾਪਸ 1;
  }
  ਲਈ(i=0; i<p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; i++){
    p_ਕ੍ਰਮ_By->a[i].fg.ਪੂਰਾ = 0;
  }
  pSelect->p_ਅਗਲਾ = 0;
  ਜਦੋਂ( pSelect->p_ਪੂਰਵ ){
    pSelect->p_ਪੂਰਵ->p_ਅਗਲਾ = pSelect;
    pSelect = pSelect->p_ਪੂਰਵ;
  }
  ਜਦੋਂ( pSelect && ਹੋਰ_ਮੰਜ਼ਿਲ_Do ){
    ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
    ਹੋਰ_ਮੰਜ਼ਿਲ_Do = 0;
    pEList = pSelect->pEList;
    ਪ੍ਰਤਿਜ੍ਞਾ( pEList!=0 );
    ਲਈ(i=0, p_ਵਸਤੂ=p_ਕ੍ਰਮ_By->a; i<p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; i++, p_ਵਸਤੂ++){
      ਪੂਰਨ i_ਖਾਨਾ = -1;
      ਵਿਅੰਜਕ *pE, *pDup;
      ਜੇ( p_ਵਸਤੂ->fg.ਪੂਰਾ ) ਜਾਰੀ;
      pE = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p_ਵਸਤੂ->p_ਵਿਅੰਜਕ);
      ਜੇ( NEVER(pE==0) ) ਜਾਰੀ;
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Integer(pE, &i_ਖਾਨਾ, 0, 1) ){
        ਜੇ( i_ਖਾਨਾ<=0 || i_ਖਾਨਾ>pEList->n_ਵਿਅੰਜਕ ){
          resolve_ਬਾਹਰਲਾ_ਦਾ_Range_Error(pParse, "ORDER", i+1, pEList->n_ਵਿਅੰਜਕ, pE);
          ਵਾਪਸ 1;
        }
      }ਨਹੀਂਤਾਂ{
        i_ਖਾਨਾ = resolveAsName(pParse, pEList, pE);
        ਜੇ( i_ਖਾਨਾ==0 ){
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
          pDup = sqlite3_ਵਿਅੰਜਕ_Dup(db, pE, 0);
          ਜੇ( !db->mallocFailed ){
            ਪ੍ਰਤਿਜ੍ਞਾ(pDup);
            i_ਖਾਨਾ = resolve_ਕ੍ਰਮ_By_ਪਦ_ਮੰਜ਼ਿਲ_ਵਿਅੰਜਕ_List(pParse, pSelect, pDup);
            ਜੇ( ਵਿੱਚ_RENAME_OBJECT && i_ਖਾਨਾ>0 ){
              resolve_ਕ੍ਰਮ_By_ਪਦ_ਮੰਜ਼ਿਲ_ਵਿਅੰਜਕ_List(pParse, pSelect, pE);
            }
          }
          sqlite3_ਵਿਅੰਜਕ_Delete(db, pDup);
        }
      }
      ਜੇ( i_ਖਾਨਾ>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        ਜੇ( !ਵਿੱਚ_RENAME_OBJECT ){
          ਵਿਅੰਜਕ *pNew = sqlite3_ਵਿਅੰਜਕ_Int32(db, i_ਖਾਨਾ);
          ਜੇ( pNew==0 ) ਵਾਪਸ 1;
          ਜੇ( p_ਵਸਤੂ->p_ਵਿਅੰਜਕ==pE ){
            p_ਵਸਤੂ->p_ਵਿਅੰਜਕ = pNew;
          }ਨਹੀਂਤਾਂ{
            ਵਿਅੰਜਕ *p_ਮਾਪਾ = p_ਵਸਤੂ->p_ਵਿਅੰਜਕ;
            ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮਾਪਾ->op==TK_COLLATE );
            ਜਦੋਂ( p_ਮਾਪਾ->p_ਖੱਬਾ->op==TK_COLLATE ) p_ਮਾਪਾ = p_ਮਾਪਾ->p_ਖੱਬਾ;
            ਪ੍ਰਤਿਜ੍ਞਾ( p_ਮਾਪਾ->p_ਖੱਬਾ==pE );
            p_ਮਾਪਾ->p_ਖੱਬਾ = pNew;
          }
          sqlite3_ਵਿਅੰਜਕ_Delete(db, pE);
          p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = (u16)i_ਖਾਨਾ;
        }
        p_ਵਸਤੂ->fg.ਪੂਰਾ = 1;
      }ਨਹੀਂਤਾਂ{
        ਹੋਰ_ਮੰਜ਼ਿਲ_Do = 1;
      }
    }
    pSelect = pSelect->p_ਅਗਲਾ;
  }
  ਲਈ(i=0; i<p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; i++){
    ਜੇ( p_ਕ੍ਰਮ_By->a[i].fg.ਪੂਰਾ==0 ){
      sqlite3_Error_ਸੁਨੇਹਾ(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      ਵਾਪਸ 1;
    }
  }
  ਵਾਪਸ 0;
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
ਪੂਰਨ sqlite3_Resolve_ਕ੍ਰਮ_ਸਮੂਹ_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,   /* The ORDER BY or GROUP BY clause to be processed */
  ਅਚਲ ਅੱਖਰ *zType     /* "ORDER" or "GROUP" */
){
  ਪੂਰਨ i;
  sqlite3 *db = pParse->db;
  ਵਿਅੰਜਕ_List *pEList;
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;

  ਜੇ( p_ਕ੍ਰਮ_By==0 || pParse->db->mallocFailed || ਵਿੱਚ_RENAME_OBJECT ) ਵਾਪਸ 0;
  ਜੇ( p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ>db->a_ਹੱਦਬੰਦੀ[SQLITE_ਹੱਦਬੰਦੀ_ਕਾਲਮ] ){
    sqlite3_Error_ਸੁਨੇਹਾ(pParse, "too many terms in %s BY clause", zType);
    ਵਾਪਸ 1;
  }
  pEList = pSelect->pEList;
  ਪ੍ਰਤਿਜ੍ਞਾ( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  ਲਈ(i=0, p_ਵਸਤੂ=p_ਕ੍ਰਮ_By->a; i<p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; i++, p_ਵਸਤੂ++){
    ਜੇ( p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ ){
      ਜੇ( p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ>pEList->n_ਵਿਅੰਜਕ ){
        resolve_ਬਾਹਰਲਾ_ਦਾ_Range_Error(pParse, zType, i+1, pEList->n_ਵਿਅੰਜਕ, 0);
        ਵਾਪਸ 1;
      }
      resolveAlias(pParse, pEList, p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ-1, p_ਵਸਤੂ->p_ਵਿਅੰਜਕ,0);
    }
  }
  ਵਾਪਸ 0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
ਸਥਿਰ ਪੂਰਨ resolveRemoveWindowsCb(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਅਣਵਰਤਿਆ_PARAMETER(pWalker);
  ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
    ਖਿੜਕੀ *pWin = p_ਵਿਅੰਜਕ->y.pWin;
    sqlite3_ਖਿੜਕੀ_Unlink_From_Select(pWin);
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
ਸਥਿਰ ਖਾਲੀ ਖਿੜਕੀ_Remove_ਵਿਅੰਜਕ_From_Select(Select *pSelect, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਜੇ( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, ਆਕਾਰ(Walker));
    sWalker.x_ਵਿਅੰਜਕ_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_ਵਿਅੰਜਕ(&sWalker, p_ਵਿਅੰਜਕ);
  }
}
#ਨਹੀਂਤਾਂ
# define ਖਿੜਕੀ_Remove_ਵਿਅੰਜਕ_From_Select(a, b)
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */

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
ਸਥਿਰ ਪੂਰਨ resolve_ਕ੍ਰਮ_ਸਮੂਹ_By(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,   /* An ORDER BY or GROUP BY clause to resolve */
  ਅਚਲ ਅੱਖਰ *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  ਪੂਰਨ i, j;                      /* Loop counters */
  ਪੂਰਨ i_ਖਾਨਾ;                      /* Column number */
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  ਪੂਰਨ n_ਨਤੀਜਾ;                   /* Number of terms in the result set */

  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਕ੍ਰਮ_By!=0 );
  n_ਨਤੀਜਾ = pSelect->pEList->n_ਵਿਅੰਜਕ;
  pParse = pNC->pParse;
  ਲਈ(i=0, p_ਵਸਤੂ=p_ਕ੍ਰਮ_By->a; i<p_ਕ੍ਰਮ_By->n_ਵਿਅੰਜਕ; i++, p_ਵਸਤੂ++){
    ਵਿਅੰਜਕ *pE = p_ਵਸਤੂ->p_ਵਿਅੰਜਕ;
    ਵਿਅੰਜਕ *pE2 = sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(pE);
    ਜੇ( NEVER(pE2==0) ) ਜਾਰੀ;
    ਜੇ( zType[0]!='G' ){
      i_ਖਾਨਾ = resolveAsName(pParse, pSelect->pEList, pE2);
      ਜੇ( i_ਖਾਨਾ>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = (u16)i_ਖਾਨਾ;
        ਜਾਰੀ;
      }
    }
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Is_Integer(pE2, &i_ਖਾਨਾ, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      ਜੇ( i_ਖਾਨਾ<1 || i_ਖਾਨਾ>0xffff ){
        resolve_ਬਾਹਰਲਾ_ਦਾ_Range_Error(pParse, zType, i+1, n_ਨਤੀਜਾ, pE2);
        ਵਾਪਸ 1;
      }
      p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = (u16)i_ਖਾਨਾ;
      ਜਾਰੀ;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = 0;
    ਜੇ( sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(pNC, pE) ){
      ਵਾਪਸ 1;
    }
    ਲਈ(j=0; j<pSelect->pEList->n_ਵਿਅੰਜਕ; j++){
      ਜੇ( sqlite3_ਵਿਅੰਜਕ_Compare(0, pE, pSelect->pEList->a[j].p_ਵਿਅੰਜਕ, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        ਖਿੜਕੀ_Remove_ਵਿਅੰਜਕ_From_Select(pSelect, pE);
        p_ਵਸਤੂ->u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ = j+1;
      }
    }
  }
  ਵਾਪਸ sqlite3_Resolve_ਕ੍ਰਮ_ਸਮੂਹ_By(pParse, pSelect, p_ਕ੍ਰਮ_By, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
ਸਥਿਰ ਪੂਰਨ resolve_Select_ਕਦਮ(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  ਪੂਰਨ isCompound;         /* True if p is a compound select */
  ਪੂਰਨ nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  ਪੂਰਨ i;                  /* Loop counter */
  ਵਿਅੰਜਕ_List *p_ਸਮੂਹ_By;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  ਜੇ( p->selFlags & SF_Resolved ){
    ਵਾਪਸ WRC_Prune;
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
  ਜੇ( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    ਵਾਪਸ pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->p_ਪੂਰਵ!=0;
  nCompound = 0;
  pLeftmost = p;
  ਜਦੋਂ( p ){
    ਪ੍ਰਤਿਜ੍ਞਾ( (p->selFlags & SF_Expanded)!=0 );
    ਪ੍ਰਤਿਜ੍ਞਾ( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, ਆਕਾਰ(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    ਜੇ( sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, p->p_ਹੱਦਬੰਦੀ) ){
      ਵਾਪਸ WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    ਜੇ( p->selFlags & SF_Converted ){
      Select *pSub;
      ਪ੍ਰਤਿਜ੍ਞਾ( p->pSrc->a[0].fg.isSubquery );
      ਪ੍ਰਤਿਜ੍ਞਾ( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSub!=0 );
      ਪ੍ਰਤਿਜ੍ਞਾ( p->pSrc->nSrc==1 && p->p_ਕ੍ਰਮ_By );
      ਪ੍ਰਤਿਜ੍ਞਾ( pSub->p_ਪੂਰਵ && pSub->p_ਕ੍ਰਮ_By==0 );
      pSub->p_ਕ੍ਰਮ_By = p->p_ਕ੍ਰਮ_By;
      p->p_ਕ੍ਰਮ_By = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    ਜੇ( pOuterNC ) pOuterNC->nNestedSelect++;
    ਲਈ(i=0; i<p->pSrc->nSrc; i++){
      Src_ਵਸਤੂ *p_ਵਸਤੂ = &p->pSrc->a[i];
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->zName!=0
              || p_ਵਸਤੂ->fg.isSubquery );  /* Test of tag-20240424-1*/
      ਜੇ( p_ਵਸਤੂ->fg.isSubquery
       && (p_ਵਸਤੂ->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        ਪੂਰਨ nRef = pOuterNC ? pOuterNC->nRef : 0;
        ਅਚਲ ਅੱਖਰ *zSavedContext = pParse->zAuthContext;

        ਜੇ( p_ਵਸਤੂ->zName ) pParse->zAuthContext = p_ਵਸਤੂ->zName;
        sqlite3_Resolve_Select_ਨਾਂ(pParse, p_ਵਸਤੂ->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        ਜੇ( pParse->nErr ) ਵਾਪਸ WRC_Abort;
        ਪ੍ਰਤਿਜ੍ਞਾ( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        ਜੇ( pOuterNC ){
          ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਸਤੂ->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          p_ਵਸਤੂ->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    ਜੇ( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.p_ਅਗਲਾ = pOuterNC;
 
    /* Resolve names in the result set. */
    ਜੇ( sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, p->pEList) ) ਵਾਪਸ WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    ਪ੍ਰਤਿਜ੍ਞਾ( (p->selFlags & SF_Aggregate)==0 );
    p_ਸਮੂਹ_By = p->p_ਸਮੂਹ_By;
    ਜੇ( p_ਸਮੂਹ_By || (sNC.ncFlags & NC_HasAgg)!=0 ){
      ਪ੍ਰਤਿਜ੍ਞਾ( NC_MinMaxAgg==SF_MinMaxAgg );
      ਪ੍ਰਤਿਜ੍ਞਾ( NC_ਕ੍ਰਮ_Agg==SF_ਕ੍ਰਮ_By_Reqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_ਕ੍ਰਮ_Agg));
    }ਨਹੀਂਤਾਂ{
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
    ਪ੍ਰਤਿਜ੍ਞਾ( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    ਜੇ( p->pHaving ){
      ਜੇ( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_ਸੁਨੇਹਾ(pParse, "HAVING clause on a non-aggregate query");
        ਵਾਪਸ WRC_Abort;
      }
      ਜੇ( sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, p->pHaving) ) ਵਾਪਸ WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    ਜੇ( sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, p->pWhere) ) ਵਾਪਸ WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    ਲਈ(i=0; i<p->pSrc->nSrc; i++){
      Src_ਵਸਤੂ *p_ਵਸਤੂ = &p->pSrc->a[i];
      ਜੇ( p_ਵਸਤੂ->fg.isTabFunc
       && sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, p_ਵਸਤੂ->u1.pFuncArg)
      ){
        ਵਾਪਸ WRC_Abort;
      }
    }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( ਵਿੱਚ_RENAME_OBJECT ){
      ਖਿੜਕੀ *pWin;
      ਲਈ(pWin=p->pWinDefn; pWin; pWin=pWin->p_ਅਗਲਾ_Win){
        ਜੇ( sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, pWin->p_ਕ੍ਰਮ_By)
         || sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, pWin->p_ਹਿੱਸੇ_ਕਰੋ)
        ){
          ਵਾਪਸ WRC_Abort;
        }
      }
    }
#ਜੇ_ਅੰਤ

    sNC.ncFlags |= NC_AllowAgg|NC_AllowWin;

    /* If this is a converted compound query, move the ORDER BY clause from
    ** the sub-query back to the parent query. At this point each term
    ** within the ORDER BY clause has been transformed to an integer value.
    ** These integers will be replaced by copies of the corresponding result
    ** set expressions by the call to resolveOrderGroupBy() below.  */
    ਜੇ( p->selFlags & SF_Converted ){
      Select *pSub;
      ਪ੍ਰਤਿਜ੍ਞਾ( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ਪ੍ਰਤਿਜ੍ਞਾ( pSub!=0 );
      p->p_ਕ੍ਰਮ_By = pSub->p_ਕ੍ਰਮ_By;
      pSub->p_ਕ੍ਰਮ_By = 0;
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
    ਜੇ( p->p_ਕ੍ਰਮ_By!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolve_ਕ੍ਰਮ_ਸਮੂਹ_By(&sNC, p, p->p_ਕ੍ਰਮ_By, "ORDER")
    ){
      ਵਾਪਸ WRC_Abort;
    }
    ਜੇ( db->mallocFailed ){
      ਵਾਪਸ WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    ਜੇ( p_ਸਮੂਹ_By ){
      ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
   
      ਜੇ( resolve_ਕ੍ਰਮ_ਸਮੂਹ_By(&sNC, p, p_ਸਮੂਹ_By, "GROUP") || db->mallocFailed ){
        ਵਾਪਸ WRC_Abort;
      }
      ਲਈ(i=0, p_ਵਸਤੂ=p_ਸਮੂਹ_By->a; i<p_ਸਮੂਹ_By->n_ਵਿਅੰਜਕ; i++, p_ਵਸਤੂ++){
        ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਸਤੂ->p_ਵਿਅੰਜਕ, EP_Agg) ){
          sqlite3_Error_ਸੁਨੇਹਾ(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          ਵਾਪਸ WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    ਜੇ( p->p_ਅਗਲਾ && p->pEList->n_ਵਿਅੰਜਕ!=p->p_ਅਗਲਾ->pEList->n_ਵਿਅੰਜਕ ){
      sqlite3_Select_Wrong_ਅੰਕ_Terms_Error(pParse, p->p_ਅਗਲਾ);
      ਵਾਪਸ WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    ਜੇ( (p->selFlags & SF_On_ਮੰਜ਼ਿਲ_Where) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      ਜੇ( pParse->nErr ) ਵਾਪਸ WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->p_ਪੂਰਵ;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  ਜੇ( isCompound && resolve_Compound_ਕ੍ਰਮ_By(pParse, pLeftmost) ){
    ਵਾਪਸ WRC_Abort;
  }

  ਵਾਪਸ WRC_Prune;
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
ਪੂਰਨ sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ             /* The expression to be analyzed. */
){
  ਪੂਰਨ savedHasAgg;
  Walker w;

  ਜੇ( p_ਵਿਅੰਜਕ==0 ) ਵਾਪਸ SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ਕ੍ਰਮ_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ਕ੍ਰਮ_Agg);
  w.pParse = pNC->pParse;
  w.x_ਵਿਅੰਜਕ_Callback = resolve_ਵਿਅੰਜਕ_ਕਦਮ;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolve_Select_ਕਦਮ;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
  w.pParse->n_ਉਚਾਈ += p_ਵਿਅੰਜਕ->n_ਉਚਾਈ;
  ਜੇ( sqlite3_ਵਿਅੰਜਕ_Check_ਉਚਾਈ(w.pParse, w.pParse->n_ਉਚਾਈ) ){
    ਵਾਪਸ SQLITE_ERROR;
  }
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ!=0 );
  sqlite3_Walk_ਵਿਅੰਜਕ_NN(&w, p_ਵਿਅੰਜਕ);
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
  w.pParse->n_ਉਚਾਈ -= p_ਵਿਅੰਜਕ->n_ਉਚਾਈ;
#ਜੇ_ਅੰਤ
  ਪ੍ਰਤਿਜ੍ਞਾ( EP_Agg==NC_HasAgg );
  ਪ੍ਰਤਿਜ੍ਞਾ( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  ਵਾਪਸ pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
ਪੂਰਨ sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  ਵਿਅੰਜਕ_List *pList         /* The expression list to be analyzed. */
){
  ਪੂਰਨ i;
  ਪੂਰਨ savedHasAgg = 0;
  Walker w;
  ਜੇ( pList==0 ) ਵਾਪਸ SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_ਵਿਅੰਜਕ_Callback = resolve_ਵਿਅੰਜਕ_ਕਦਮ;
  w.xSelectCallback = resolve_Select_ਕਦਮ;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ਕ੍ਰਮ_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ਕ੍ਰਮ_Agg);
  ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
    ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ = pList->a[i].p_ਵਿਅੰਜਕ;
    ਜੇ( p_ਵਿਅੰਜਕ==0 ) ਜਾਰੀ;
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
    w.pParse->n_ਉਚਾਈ += p_ਵਿਅੰਜਕ->n_ਉਚਾਈ;
    ਜੇ( sqlite3_ਵਿਅੰਜਕ_Check_ਉਚਾਈ(w.pParse, w.pParse->n_ਉਚਾਈ) ){
      ਵਾਪਸ SQLITE_ERROR;
    }
#ਜੇ_ਅੰਤ
    sqlite3_Walk_ਵਿਅੰਜਕ_NN(&w, p_ਵਿਅੰਜਕ);
#ਜੇ SQLITE_MAX_ਵਿਅੰਜਕ_ਡੂੰਘਾਈ>0
    w.pParse->n_ਉਚਾਈ -= p_ਵਿਅੰਜਕ->n_ਉਚਾਈ;
#ਜੇ_ਅੰਤ
    ਪ੍ਰਤਿਜ੍ਞਾ( EP_Agg==NC_HasAgg );
    ਪ੍ਰਤਿਜ੍ਞਾ( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    ਜੇ( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ਕ੍ਰਮ_Agg) ){
      ਵਿਅੰਜਕ_ਸੈੱਟ_Property(p_ਵਿਅੰਜਕ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ਕ੍ਰਮ_Agg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ਕ੍ਰਮ_Agg);
    }
    ਜੇ( w.pParse->nErr>0 ) ਵਾਪਸ SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  ਵਾਪਸ SQLITE_OK;
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
ਖਾਲੀ sqlite3_Resolve_Select_ਨਾਂ(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  ਪ੍ਰਤਿਜ੍ਞਾ( p!=0 );
  w.x_ਵਿਅੰਜਕ_Callback = resolve_ਵਿਅੰਜਕ_ਕਦਮ;
  w.xSelectCallback = resolve_Select_ਕਦਮ;
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
ਪੂਰਨ sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  ਸਾਰਣੀ *pTab,     /* The table being referenced, or NULL */
  ਪੂਰਨ type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,     /* Expression to resolve.  May be NULL. */
  ਵਿਅੰਜਕ_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  ਪੂਰਨ rc;
  ਸੰਘ {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  ਪ੍ਰਤਿਜ੍ਞਾ( type==0 || pTab!=0 );
  ਪ੍ਰਤਿਜ੍ਞਾ( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_ਵਿਅੰਜਕ
          || type==NC_Gen_ਖਾਨਾ || pTab==0 );
  memset(&sNC, 0, ਆਕਾਰ(sNC));
  memset(&uSrc, 0, ਆਕਾਰ(uSrc));
  pSrc = &uSrc.sSrc;
  ਜੇ( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    ਜੇ( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  ਜੇ( (rc = sqlite3_Resolve_ਵਿਅੰਜਕ_ਨਾਂ(&sNC, p_ਵਿਅੰਜਕ))!=SQLITE_OK ) ਵਾਪਸ rc;
  ਜੇ( pList ) rc = sqlite3_Resolve_ਵਿਅੰਜਕ_List_ਨਾਂ(&sNC, pList);
  ਵਾਪਸ rc;
}
