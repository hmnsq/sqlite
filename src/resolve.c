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
#అన్తర్భావ "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#నిర్వచించు EXCLUDED_పట్టిక_సంఖ్య  2

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
స్థిర పూర్ణాంకం incr_Agg_లోతు(Walker *pWalker, సమాసం *p_సమాసం){
  ఒకవేళ( p_సమాసం->op==TK_AGG_FUNCTION ) p_సమాసం->op2 += pWalker->u.n;
  తిరిగి WRC_Continue;
}
స్థిర శూన్య incr_Agg_Function_లోతు(సమాసం *p_సమాసం, పూర్ణాంకం N){
  ఒకవేళ( N>0 ){
    Walker w;
    memset(&w, 0, పరిమాణం(w));
    w.x_సమాసం_Callback = incr_Agg_లోతు;
    w.u.n = N;
    sqlite3_Walk_సమాసం(&w, p_సమాసం);
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
స్థిర శూన్య resolveAlias(
  Parse *pParse,         /* Parsing context */
  సమాసం_List *pEList,      /* A result set */
  పూర్ణాంకం i_నిలువు,              /* A column in the result set.  0..pEList->nExpr-1 */
  సమాసం *p_సమాసం,           /* Transform this into an alias to the result set */
  పూర్ణాంకం nSubquery          /* Number of subqueries that the label is moving */
){
  సమాసం *pOrig;           /* The iCol-th column of the result set */
  సమాసం *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  ప్రతిజ్ఞా( i_నిలువు>=0 && i_నిలువు<pEList->n_సమాసం );
  pOrig = pEList->a[i_నిలువు].p_సమాసం;
  ప్రతిజ్ఞా( pOrig!=0 );
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Reduced|EP_TokenOnly) );
  ఒకవేళ( p_సమాసం->pAggInfo ) తిరిగి;
  db = pParse->db;
  pDup = sqlite3_సమాసం_Dup(db, pOrig, 0);
  ఒకవేళ( db->mallocFailed ){
    sqlite3_సమాసం_Delete(db, pDup);
    pDup = 0;
  }లేకపోతే{
    సమాసం తాత్కాలిక;
    incr_Agg_Function_లోతు(pDup, nSubquery);
    ఒకవేళ( p_సమాసం->op==TK_COLLATE ){
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      pDup = sqlite3_సమాసం_కూడు_Collate_String(pParse, pDup, p_సమాసం->u.zToken);
    }
    జ్ఞాపకం_నకలు(&తాత్కాలిక, pDup, పరిమాణం(సమాసం));
    జ్ఞాపకం_నకలు(pDup, p_సమాసం, పరిమాణం(సమాసం));
    జ్ఞాపకం_నకలు(p_సమాసం, &తాత్కాలిక, పరిమాణం(సమాసం));
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
      ఒకవేళ( ALWAYS(p_సమాసం->y.pWin!=0) ){
        p_సమాసం->y.pWin->pOwner = p_సమాసం;
      }
    }
    sqlite3_సమాసం_Deferred_Delete(pParse, pDup);
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
పూర్ణాంకం sqlite3_సరిపోలిక_EName(
  స్థిరరాశి నిర్మాణం సమాసం_List_అంశం *p_అంశం,
  స్థిరరాశి అక్షరం *z_నిలువు,
  స్థిరరాశి అక్షరం *zTab,
  స్థిరరాశి అక్షరం *zDb,
  పూర్ణాంకం *pbRowid
){
  పూర్ణాంకం n;
  స్థిరరాశి అక్షరం *z_పరిధి;
  పూర్ణాంకం eEName = p_అంశం->fg.eEName;
  ఒకవేళ( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    తిరిగి 0;
  }
  ప్రతిజ్ఞా( pbRowid==0 || *pbRowid==0 );
  z_పరిధి = p_అంశం->zEName;
  కొరకు(n=0; ALWAYS(z_పరిధి[n]) && z_పరిధి[n]!='.'; n++){}
  ఒకవేళ( zDb && (sqlite3StrNICmp(z_పరిధి, zDb, n)!=0 || zDb[n]!=0) ){
    తిరిగి 0;
  }
  z_పరిధి += n+1;
  కొరకు(n=0; ALWAYS(z_పరిధి[n]) && z_పరిధి[n]!='.'; n++){}
  ఒకవేళ( zTab && (sqlite3StrNICmp(z_పరిధి, zTab, n)!=0 || zTab[n]!=0) ){
    తిరిగి 0;
  }
  z_పరిధి += n+1;
  ఒకవేళ( z_నిలువు ){
    ఒకవేళ( eEName==ENAME_TAB && sqlite3StrICmp(z_పరిధి, z_నిలువు)!=0 ) తిరిగి 0;
    ఒకవేళ( eEName==ENAME_ROWID && sqlite3IsRowid(z_నిలువు)==0 ) తిరిగి 0;
  }
  ఒకవేళ( eEName==ENAME_ROWID ) *pbRowid = 1;
  తిరిగి 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
స్థిర పూర్ణాంకం are_Double_Quoted_అక్షరశ్రేణులు_Enabled(sqlite3 *db, NameContext *p_శిఖరం_NC){
  ఒకవేళ( db->init.busy ) తిరిగి 1;  /* Always support for legacy schemas */
  ఒకవేళ( p_శిఖరం_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    ఒకవేళ( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      తిరిగి 1;
    }
    తిరిగి (db->flags & SQLITE_DqsDDL)!=0;
  }లేకపోతే{
    /* Currently parsing a DML statement */
    తిరిగి (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_సమాసం_నిలువు_వాడిన(సమాసం *p_సమాసం){
  పూర్ణాంకం n;
  పట్టిక *pExTab;

  n = p_సమాసం->i_స్తంభం;
  ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
  pExTab = p_సమాసం->y.pTab;
  ప్రతిజ్ఞా( pExTab!=0 );
  ప్రతిజ్ఞా( n < pExTab->n_నిలువు );
  ఒకవేళ( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->a_నిలువు[n].నిలువు_Flags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->n_నిలువు==BMS-1 );
    testcase( pExTab->n_నిలువు==BMS );
    తిరిగి pExTab->n_నిలువు>=BMS ? ALLBITS : MASKBIT(pExTab->n_నిలువు)-1;
  }లేకపోతే{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    ఒకవేళ( n>=BMS ) n = BMS-1;
    తిరిగి ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
స్థిర శూన్య extendFJMatch(
  Parse *pParse,          /* Parsing context */
  సమాసం_List **ppList,      /* ExprList to extend */
  Src_అంశం *p_సరిపోలిక,        /* Source table containing the column */
  i16 i_స్తంభం             /* The column number */
){
  సమాసం *pNew = sqlite3_సమాసం_Alloc(pParse->db, TK_స్తంభం, 0, 0);
  ఒకవేళ( pNew ){
    pNew->i_పట్టిక = p_సరిపోలిక->iCursor;
    pNew->i_స్తంభం = i_స్తంభం;
    pNew->y.pTab = p_సరిపోలిక->pSTab;
    ప్రతిజ్ఞా( (p_సరిపోలిక->fg.jointype & (JT_ఎడమ|JT_LTORJ))!=0 );
    సమాసం_సమితి_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_సమాసం_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
స్థిర SQLITE_NOINLINE పూర్ణాంకం is_Valid_Schema_పట్టిక_Name(
  స్థిరరాశి అక్షరం *zTab,         /* Name as it appears in the SQL */
  పట్టిక *pTab,              /* The schema table we are trying to match */
  స్థిరరాశి అక్షరం *zDb           /* non-NULL if a database qualifier is present */
){
  స్థిరరాశి అక్షరం *zLegacy;
  ప్రతిజ్ఞా( pTab!=0 );
  ప్రతిజ్ఞా( pTab->tnum==1 );
  ఒకవేళ( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) తిరిగి 0;
  zLegacy = pTab->zName;
  ఒకవేళ( strcmp(zLegacy+7, &LEGACY_తాత్కాలిక_SCHEMA_పట్టిక[7])==0 ){
    ఒకవేళ( sqlite3StrICmp(zTab+7, &PREFERRED_తాత్కాలిక_SCHEMA_పట్టిక[7])==0 ){
      తిరిగి 1;
    }
    ఒకవేళ( zDb==0 ) తిరిగి 0;
    ఒకవేళ( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_పట్టిక[7])==0 ) తిరిగి 1;
    ఒకవేళ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_పట్టిక[7])==0 ) తిరిగి 1;
  }లేకపోతే{
    ఒకవేళ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_పట్టిక[7])==0 ) తిరిగి 1;
  }
  తిరిగి 0;
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
స్థిర పూర్ణాంకం lookupName(
  Parse *pParse,       /* The parsing context */
  స్థిరరాశి అక్షరం *zDb,     /* Name of the database containing table, or NULL */
  స్థిరరాశి అక్షరం *zTab,    /* Name of table containing column, or NULL */
  స్థిరరాశి సమాసం *p_కుడి,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  సమాసం *p_సమాసం          /* Make this EXPR node point to the selected column */
){
  పూర్ణాంకం i, j;                         /* Loop counters */
  పూర్ణాంకం cnt = 0;                      /* Number of matching column names */
  పూర్ణాంకం cntTab = 0;                   /* Number of potential "rowid" matches */
  పూర్ణాంకం nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  Src_అంశం *p_అంశం;                   /* Use for looping over pSrcList items */
  Src_అంశం *p_సరిపోలిక = 0;              /* The matching pSrcList item */
  NameContext *p_శిఖరం_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  పూర్ణాంకం e_New_సమాసం_Op = TK_స్తంభం;       /* New value for pExpr->op on success */
  పట్టిక *pTab = 0;                  /* Table holding the row */
  సమాసం_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  స్థిరరాశి అక్షరం *z_నిలువు = p_కుడి->u.zToken;

  ప్రతిజ్ఞా( pNC );     /* the name context cannot be NULL. */
  ప్రతిజ్ఞా( z_నిలువు );    /* The Z in X.Y.Z cannot be NULL */
  ప్రతిజ్ఞా( zDb==0 || zTab!=0 );
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_సమాసం->i_పట్టిక = -1;
  సమాసం_సమితి_VVAProperty(p_సమాసం, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  ఒకవేళ( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    ఒకవేళ( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }లేకపోతే{
      కొరకు(i=0; i<db->nDb; i++){
        ప్రతిజ్ఞా( db->aDb[i].zDbSName );
        ఒకవేళ( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          ఆపు;
        }
      }
      ఒకవేళ( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  ప్రతిజ్ఞా( pNC && cnt==0 );
  చేయి{
    సమాసం_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    ఒకవేళ( pSrcList ){
      కొరకు(i=0, p_అంశం=pSrcList->a; i<pSrcList->nSrc; i++, p_అంశం++){
        pTab = p_అంశం->pSTab;
        ప్రతిజ్ఞా( pTab!=0 && pTab->zName!=0 );
        ప్రతిజ్ఞా( pTab->n_నిలువు>0 || pParse->nErr );
        ప్రతిజ్ఞా( (పూర్ణాంకం)p_అంశం->fg.isNestedFrom == IsNestedFrom(p_అంశం));
        ఒకవేళ( p_అంశం->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          పూర్ణాంకం తగిలింది = 0;
          Select *pSel;
          ప్రతిజ్ఞా( p_అంశం->fg.isSubquery );
          ప్రతిజ్ఞా( p_అంశం->u4.pSubq!=0 );
          pSel = p_అంశం->u4.pSubq->pSelect;
          ప్రతిజ్ఞా( pSel!=0 );
          pEList = pSel->pEList;
          ప్రతిజ్ఞా( pEList!=0 );
          ప్రతిజ్ఞా( pEList->n_సమాసం==pTab->n_నిలువు );
          కొరకు(j=0; j<pEList->n_సమాసం; j++){
            పూర్ణాంకం bRowid = 0;       /* True if possible rowid match */
            ఒకవేళ( !sqlite3_సరిపోలిక_EName(&pEList->a[j], z_నిలువు, zTab, zDb, &bRowid) ){
              కొనసాగు;
            }
            ఒకవేళ( bRowid==0 ){
              ఒకవేళ( cnt>0 ){
                ఒకవేళ( p_అంశం->fg.isUsing==0
                 || sqlite3_Id_List_సూచీ(p_అంశం->u3.pUsing, z_నిలువు)<0
                 || p_సరిపోలిక==p_అంశం
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_సమాసం_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }లేకపోతే
                ఒకవేళ( (p_అంశం->fg.jointype & JT_కుడి)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  కొనసాగు;
                }లేకపోతే
                ఒకవేళ( (p_అంశం->fg.jointype & JT_ఎడమ)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_సమాసం_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }లేకపోతే{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, p_సరిపోలిక, p_సమాసం->i_స్తంభం);
                }
              }
              cnt++;
              తగిలింది = 1;
            }లేకపోతే ఒకవేళ( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              కొనసాగు;
            }
            cntTab++;
            p_సరిపోలిక = p_అంశం;
            p_సమాసం->i_స్తంభం = j;
            pEList->a[j].fg.b_వాడిన = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            ప్రతిజ్ఞా( bRowid==0 || pEList->a[j].fg.b_Using_పదం==0 );
            ఒకవేళ( pEList->a[j].fg.b_Using_పదం ) ఆపు;
          }
          ఒకవేళ( తగిలింది || zTab==0 ) కొనసాగు;
        }
        ప్రతిజ్ఞా( zDb==0 || zTab!=0 );
        ఒకవేళ( zTab ){
          ఒకవేళ( zDb ){
            ఒకవేళ( pTab->pSchema!=pSchema ) కొనసాగు;
            ఒకవేళ( pSchema==0 && strcmp(zDb,"*")!=0 ) కొనసాగు;
          }
          ఒకవేళ( p_అంశం->zAlias!=0 ){
            ఒకవేళ( sqlite3StrICmp(zTab, p_అంశం->zAlias)!=0 ){
              కొనసాగు;
            }
          }లేకపోతే ఒకవేళ( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            ఒకవేళ( pTab->tnum!=1 ) కొనసాగు;
            ఒకవేళ( !is_Valid_Schema_పట్టిక_Name(zTab, pTab, zDb) ) కొనసాగు;
          }
          ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
          ఒకవేళ( లోపల_RENAME_OBJECT && p_అంశం->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (శూన్య*)&p_సమాసం->y.pTab);
          }
        }
        j = sqlite3_స్తంభం_సూచీ(pTab, z_నిలువు);
        ఒకవేళ( j>=0 ){
          ఒకవేళ( cnt>0 ){
            ఒకవేళ( p_అంశం->fg.isUsing==0
             || sqlite3_Id_List_సూచీ(p_అంశం->u3.pUsing, z_నిలువు)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_సమాసం_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }లేకపోతే
            ఒకవేళ( (p_అంశం->fg.jointype & JT_కుడి)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              కొనసాగు;
            }లేకపోతే
            ఒకవేళ( (p_అంశం->fg.jointype & JT_ఎడమ)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_సమాసం_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }లేకపోతే{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, p_సరిపోలిక, p_సమాసం->i_స్తంభం);
            }
          }
          cnt++;
          p_సరిపోలిక = p_అంశం;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_సమాసం->i_స్తంభం = j==pTab->iPKey ? -1 : (i16)j;
          ఒకవేళ( p_అంశం->fg.isNestedFrom ){
            sqlite3_Src_అంశం_స్తంభం_వాడిన(p_అంశం, j);
          }
        }
        ఒకవేళ( 0==cnt && VisibleRowid(pTab) ){
          /* pTab is a potential ROWID match.  Keep track of it and match
          ** the ROWID later if that seems appropriate.  (Search for "cntTab"
          ** to find related code.)  Only allow a ROWID match if there is
          ** a single ROWID match candidate.
          */
#నిర్వచితమైతే SQLITE_ALLOW_ROWID_లోపల_VIEW
          /* In SQLITE_ALLOW_ROWID_IN_VIEW mode, allow a ROWID match
          ** if there is a single VIEW candidate or if there is a single
          ** non-VIEW candidate plus multiple VIEW candidates.  In other
          ** words non-VIEW candidate terms take precedence over VIEWs.
          */
          ఒకవేళ( cntTab==0
           || (cntTab==1
               && p_సరిపోలిక!=0
               && ALWAYS(p_సరిపోలిక->pSTab!=0)
               && (p_సరిపోలిక->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            p_సరిపోలిక = p_అంశం;
          }లేకపోతే{
            cntTab++;
          }
#లేకపోతే
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          p_సరిపోలిక = p_అంశం;
#అయితే_ముగింపు
        }
      }
      ఒకవేళ( p_సరిపోలిక ){
        p_సమాసం->i_పట్టిక = p_సరిపోలిక->iCursor;
        ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
        p_సమాసం->y.pTab = p_సరిపోలిక->pSTab;
        ఒకవేళ( (p_సరిపోలిక->fg.jointype & (JT_ఎడమ|JT_LTORJ))!=0 ){
          సమాసం_సమితి_Property(p_సమాసం, EP_CanBeNull);
        }
        pSchema = p_సమాసం->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#ఒకవేళ !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    ఒకవేళ( cnt==0 && zDb==0 ){
      pTab = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
      ఒకవేళ( pParse->pTriggerTab!=0 ){
        పూర్ణాంకం op = pParse->eTriggerOp;
        ప్రతిజ్ఞా( op==TK_DELETE || op==TK_UPDATE || op==TK_చొప్పించు );
        ఒకవేళ( pParse->bReturning ){
          ఒకవేళ( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || is_Valid_Schema_పట్టిక_Name(zTab, pParse->pTriggerTab, 0))
          ){
            p_సమాసం->i_పట్టిక = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }లేకపోతే ఒకవేళ( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_సమాసం->i_పట్టిక = 1;
          pTab = pParse->pTriggerTab;
        }లేకపోతే ఒకవేళ( op!=TK_చొప్పించు && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_సమాసం->i_పట్టిక = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#అయితే_ముగింపు /* SQLITE_OMIT_TRIGGER */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UPSERT
      ఒకవేళ( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        ఒకవేళ( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_సమాసం->i_పట్టిక = EXCLUDED_పట్టిక_సంఖ్య;
          ఒకవేళ( లోపల_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (శూన్య*)&p_సమాసం->y.pTab);
          }
        }
      }
#అయితే_ముగింపు /* SQLITE_OMIT_UPSERT */

      ఒకవేళ( pTab ){
        పూర్ణాంకం i_నిలువు;
        pSchema = pTab->pSchema;
        cntTab++;
        i_నిలువు = sqlite3_స్తంభం_సూచీ(pTab, z_నిలువు);
        ఒకవేళ( i_నిలువు>=0 ){
          ఒకవేళ( pTab->iPKey==i_నిలువు ) i_నిలువు = -1;
        }లేకపోతే{
          ఒకవేళ( sqlite3IsRowid(z_నిలువు) && VisibleRowid(pTab) ){
            i_నిలువు = -1;
          }లేకపోతే{
            i_నిలువు = pTab->n_నిలువు;
          }
        }
        ఒకవేళ( i_నిలువు<pTab->n_నిలువు ){
          cnt++;
          p_సరిపోలిక = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_UPSERT
          ఒకవేళ( p_సమాసం->i_పట్టిక==EXCLUDED_పట్టిక_సంఖ్య ){
            testcase( i_నిలువు==(-1) );
            ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
            ఒకవేళ( లోపల_RENAME_OBJECT ){
              p_సమాసం->i_స్తంభం = i_నిలువు;
              p_సమాసం->y.pTab = pTab;
              e_New_సమాసం_Op = TK_స్తంభం;
            }లేకపోతే{
              p_సమాసం->i_పట్టిక = pNC->uNC.pUpsert->reg_దత్తాంశం +
                 sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i_నిలువు);
              e_New_సమాసం_Op = TK_రిజిస్టరు;
            }
          }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_UPSERT */
          {
            ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
            p_సమాసం->y.pTab = pTab;
            ఒకవేళ( pParse->bReturning ){
              e_New_సమాసం_Op = TK_రిజిస్టరు;
              p_సమాసం->op2 = TK_స్తంభం;
              p_సమాసం->i_స్తంభం = i_నిలువు;
              p_సమాసం->i_పట్టిక = pNC->uNC.i_పునాది_Reg + (pTab->n_నిలువు+1)*p_సమాసం->i_పట్టిక +
                 sqlite3_పట్టిక_స్తంభం_గమ్యం_Storage(pTab, i_నిలువు) + 1;
            }లేకపోతే{
              p_సమాసం->i_స్తంభం = (i16)i_నిలువు;
              e_New_సమాసం_Op = TK_TRIGGER;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
              ఒకవేళ( i_నిలువు<0 ){
                p_సమాసం->aff_సమాసం = SQLITE_AFF_INTEGER;
              }లేకపోతే ఒకవేళ( p_సమాసం->i_పట్టిక==0 ){
                testcase( i_నిలువు==31 );
                testcase( i_నిలువు==32 );
                pParse->oldmask |= (i_నిలువు>=32 ? 0xffffffff : (((u32)1)<<i_నిలువు));
              }లేకపోతే{
                testcase( i_నిలువు==31 );
                testcase( i_నిలువు==32 );
                pParse->newmask |= (i_నిలువు>=32 ? 0xffffffff : (((u32)1)<<i_నిలువు));
              }
#అయితే_ముగింపు /* SQLITE_OMIT_TRIGGER */
            }
          }
        }
      }
    }
#అయితే_ముగింపు /* !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT) */

    /*
    ** Perhaps the name is a reference to the ROWID
    */
    ఒకవేళ( cnt==0
     && cntTab>=1
     && p_సరిపోలిక
     && (pNC->ncFlags & (NC_Idx_సమాసం|NC_Gen_నిలువు))==0
     && sqlite3IsRowid(z_నిలువు)
     && ALWAYS(VisibleRowid(p_సరిపోలిక->pSTab) || p_సరిపోలిక->fg.isNestedFrom)
    ){
      cnt = cntTab;
#ఒకవేళ SQLITE_ALLOW_ROWID_లోపల_VIEW+0==2
      ఒకవేళ( p_సరిపోలిక->pSTab!=0 && IsView(p_సరిపోలిక->pSTab) ){
        e_New_సమాసం_Op = TK_NULL;
      }
#అయితే_ముగింపు
      ఒకవేళ( p_సరిపోలిక->fg.isNestedFrom==0 ) p_సమాసం->i_స్తంభం = -1;
      p_సమాసం->aff_సమాసం = SQLITE_AFF_INTEGER;
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
    ఒకవేళ( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      ప్రతిజ్ఞా( pEList!=0 );
      కొరకు(j=0; j<pEList->n_సమాసం; j++){
        అక్షరం *zAs = pEList->a[j].zEName;
        ఒకవేళ( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, z_నిలువు)==0
        ){
          సమాసం *pOrig;
          ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ==0 && p_సమాసం->p_కుడి==0 );
          ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం)==0 || p_సమాసం->x.pList==0 );
          ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం)==0 || p_సమాసం->x.pSelect==0 );
          pOrig = pEList->a[j].p_సమాసం;
          ఒకవేళ( (pNC->ncFlags&NC_AllowAgg)==0 && సమాసం_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_సందేశం(pParse, "misuse of aliased aggregate %s", zAs);
            తిరిగి WRC_Abort;
          }
          ఒకవేళ( సమాసం_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_శిఖరం_NC )
          ){
            sqlite3_Error_సందేశం(pParse, "misuse of aliased window function %s",zAs);
            తిరిగి WRC_Abort;
          }
          ఒకవేళ( sqlite3_సమాసం_సదిశ_కొలత(pOrig)!=1 ){
            sqlite3_Error_సందేశం(pParse, "row value misused");
            తిరిగి WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_సమాసం, nSubquery);
          cnt = 1;
          p_సరిపోలిక = 0;
          ప్రతిజ్ఞా( zTab==0 && zDb==0 );
          ఒకవేళ( లోపల_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (శూన్య*)p_సమాసం);
          }
          వెళ్ళు lookupname_చివర;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    ఒకవేళ( cnt ) ఆపు;
    pNC = pNC->p_తదుపరి;
    nSubquery++;
  }అయితే( pNC );


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
  ఒకవేళ( cnt==0 && zTab==0 ){
    ప్రతిజ్ఞా( p_సమాసం->op==TK_ID );
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం,EP_DblQuoted)
     && are_Double_Quoted_అక్షరశ్రేణులు_Enabled(db, p_శిఖరం_NC)
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
        "double-quoted string literal: \"%w\"", z_నిలువు);
#నిర్వచితమైతే SQLITE_ENABLE_NORMALIZE
      sqlite3_Vdbe_కూడు_Dblquote_Str(db, pParse->pVdbe, z_నిలువు);
#అయితే_ముగింపు
      p_సమాసం->op = TK_STRING;
      memset(&p_సమాసం->y, 0, పరిమాణం(p_సమాసం->y));
      తిరిగి WRC_Prune;
    }
    ఒకవేళ( sqlite3_సమాసం_Id_గమ్యం_True_False(p_సమాసం) ){
      తిరిగి WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  ప్రతిజ్ఞా( pFJMatch==0 || cnt>0 );
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_xIsSelect|EP_Int_విలువ) );
  ఒకవేళ( cnt!=1 ){
    స్థిరరాశి అక్షరం *zErr;
    ఒకవేళ( pFJMatch ){
      ఒకవేళ( pFJMatch->n_సమాసం==cnt-1 ){
        ఒకవేళ( సమాసం_Has_Property(p_సమాసం,EP_ఆకు) ){
          సమాసం_Clear_Property(p_సమాసం,EP_ఆకు);
        }లేకపోతే{
          sqlite3_సమాసం_Delete(db, p_సమాసం->p_ఎడమ);
          p_సమాసం->p_ఎడమ = 0;
          sqlite3_సమాసం_Delete(db, p_సమాసం->p_కుడి);
          p_సమాసం->p_కుడి = 0;
        }
        extendFJMatch(pParse, &pFJMatch, p_సరిపోలిక, p_సమాసం->i_స్తంభం);
        p_సమాసం->op = TK_FUNCTION;
        p_సమాసం->u.zToken = "coalesce";
        p_సమాసం->x.pList = pFJMatch;
        p_సమాసం->aff_సమాసం = SQLITE_AFF_DEFER;
        cnt = 1;
        వెళ్ళు lookupname_చివర;
      }లేకపోతే{
        sqlite3_సమాసం_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    ఒకవేళ( zDb ){
      sqlite3_Error_సందేశం(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, z_నిలువు);
    }లేకపోతే ఒకవేళ( zTab ){
      sqlite3_Error_సందేశం(pParse, "%s: %s.%s", zErr, zTab, z_నిలువు);
    }లేకపోతే ఒకవేళ( cnt==0 && సమాసం_Has_Property(p_కుడి,EP_DblQuoted) ){
      sqlite3_Error_సందేశం(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, z_నిలువు);
    }లేకపోతే{
      sqlite3_Error_సందేశం(pParse, "%s: %s", zErr, z_నిలువు);
    }
    sqlite3_రికార్డు_Error_స్థానభ్రంశం_యొక్క_సమాసం(pParse->db, p_సమాసం);
    pParse->checkSchema = 1;
    p_శిఖరం_NC->nNcErr++;
    e_New_సమాసం_Op = TK_NULL;
  }
  ప్రతిజ్ఞా( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  ఒకవేళ( !సమాసం_Has_Property(p_సమాసం,(EP_TokenOnly|EP_ఆకు)) ){
    sqlite3_సమాసం_Delete(db, p_సమాసం->p_ఎడమ);
    p_సమాసం->p_ఎడమ = 0;
    sqlite3_సమాసం_Delete(db, p_సమాసం->p_కుడి);
    p_సమాసం->p_కుడి = 0;
    సమాసం_సమితి_Property(p_సమాసం, EP_ఆకు);
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
  ఒకవేళ( p_సరిపోలిక ){
    ఒకవేళ( p_సమాసం->i_స్తంభం>=0 ){
      p_సరిపోలిక->నిలువు_వాడిన |= sqlite3_సమాసం_నిలువు_వాడిన(p_సమాసం);
    }లేకపోతే{
      p_సరిపోలిక->fg.rowid_వాడిన = 1;
    }
  }

  p_సమాసం->op = e_New_సమాసం_Op;
lookupname_చివర:
  ఒకవేళ( cnt==1 ){
    ప్రతిజ్ఞా( pNC!=0 );
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
    ఒకవేళ( db->xAuth ){
      ఒకవేళ( pFJMatch ){
        ప్రతిజ్ఞా( p_సమాసం->op==TK_FUNCTION );
        ప్రతిజ్ఞా( sqlite3_stricmp(p_సమాసం->u.zToken,"coalesce")==0 );
        ప్రతిజ్ఞా( p_సమాసం->x.pList==pFJMatch );
        ప్రతిజ్ఞా( pFJMatch->n_సమాసం>0 );
        కొరకు(i=0; i<pFJMatch->n_సమాసం; i++){
          ప్రతిజ్ఞా( pFJMatch->a[i].p_సమాసం->op==TK_స్తంభం );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_సమాసం,pSchema,pNC->pSrcList);
        }
      }లేకపోతే ఒకవేళ( p_సమాసం->op==TK_స్తంభం || p_సమాసం->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_సమాసం, pSchema, pNC->pSrcList);
      }
    } 
#అయితే_ముగింపు
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    కొరకు(;;){
      ప్రతిజ్ఞా( p_శిఖరం_NC!=0 );
      p_శిఖరం_NC->nRef++;
      ఒకవేళ( p_శిఖరం_NC==pNC ) ఆపు;
      p_శిఖరం_NC = p_శిఖరం_NC->p_తదుపరి;
    }
    తిరిగి WRC_Prune;
  } లేకపోతే {
    తిరిగి WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
సమాసం *sqlite3_Create_స్తంభం_సమాసం(sqlite3 *db, SrcList *pSrc, పూర్ణాంకం iSrc, పూర్ణాంకం i_నిలువు){
  సమాసం *p = sqlite3_సమాసం_Alloc(db, TK_స్తంభం, 0, 0);
  ఒకవేళ( p ){
    Src_అంశం *p_అంశం = &pSrc->a[iSrc];
    పట్టిక *pTab;
    ప్రతిజ్ఞా( సమాసం_Use_YTab(p) );
    pTab = p->y.pTab = p_అంశం->pSTab;
    p->i_పట్టిక = p_అంశం->iCursor;
    ఒకవేళ( p->y.pTab->iPKey==i_నిలువు ){
      p->i_స్తంభం = -1;
    }లేకపోతే{
      p->i_స్తంభం = (ynVar)i_నిలువు;
      ఒకవేళ( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->a_నిలువు[i_నిలువు].నిలువు_Flags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->n_నిలువు==63 );
        testcase( pTab->n_నిలువు==64 );
        p_అంశం->నిలువు_వాడిన = pTab->n_నిలువు>=64 ? ALLBITS : MASKBIT(pTab->n_నిలువు)-1;
      }లేకపోతే{
        testcase( i_నిలువు==BMS );
        testcase( i_నిలువు==BMS-1 );
        p_అంశం->నిలువు_వాడిన |= ((Bitmask)1)<<(i_నిలువు>=BMS ? BMS-1 : i_నిలువు);
      }
    }
  }
  తిరిగి p;
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
స్థిర శూన్య notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   స్థిరరాశి అక్షరం *z_సందేశం,    /* Type of error */
   సమాసం *p_సమాసం,         /* Invalidate this expression on error */
   సమాసం *pError         /* Associate error with this expression */
){
  స్థిరరాశి అక్షరం *z_లోపల = "partial index WHERE clauses";
  ఒకవేళ( pNC->ncFlags & NC_Idx_సమాసం )      z_లోపల = "index expressions";
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CHECK
  లేకపోతే ఒకవేళ( pNC->ncFlags & NC_IsCheck ) z_లోపల = "CHECK constraints";
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_GENERATED_COLUMNS
  లేకపోతే ఒకవేళ( pNC->ncFlags & NC_Gen_నిలువు ) z_లోపల = "generated columns";
#అయితే_ముగింపు
  sqlite3_Error_సందేశం(pParse, "%s prohibited in %s", z_సందేశం, z_లోపల);
  ఒకవేళ( p_సమాసం ) p_సమాసం->op = TK_NULL;
  sqlite3_రికార్డు_Error_స్థానభ్రంశం_యొక్క_సమాసం(pParse->db, pError);
}
#నిర్వచించు sqlite3ResolveNotValid(P,N,M,X,E,R) \
  ప్రతిజ్ఞా( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_సమాసం|NC_Gen_నిలువు))==0 ); \
  ఒకవేళ( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
స్థిర పూర్ణాంకం సమాసం_Probability(సమాసం *p){
  ద్విగుణ r = -1.0;
  ఒకవేళ( p->op!=TK_FLOAT ) తిరిగి -1;
  ప్రతిజ్ఞా( !సమాసం_Has_Property(p, EP_Int_విలువ) );
  sqlite3AtoF(p->u.zToken, &r);
  ప్రతిజ్ఞా( r>=0.0 );
  ఒకవేళ( r>1.0 ) తిరిగి -1;
  తిరిగి (పూర్ణాంకం)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
స్థిర SQLITE_NOINLINE శూన్య resolve_సమితి_సమాసం_Subtype_Arg(సమాసం_List *pList){
  పూర్ణాంకం nn, ii;
  nn = pList ? pList->n_సమాసం : 0;
  కొరకు(ii=0; ii<nn; ii++){
    సమాసం *p_సమాసం = pList->a[ii].p_సమాసం;
    అయితే( 1 /*exit-by-break*/ ){
      సమాసం_సమితి_Property(p_సమాసం, EP_SubtArg);
      ఒకవేళ( p_సమాసం->op==TK_SELECT ){
        ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం) );
        ప్రతిజ్ఞా( p_సమాసం->x.pSelect!=0 );
        resolve_సమితి_సమాసం_Subtype_Arg(p_సమాసం->x.pSelect->pEList);
        ఆపు;
      }
      ఒకవేళ( p_సమాసం->op==TK_UPLUS ){
        p_సమాసం = p_సమాసం->p_ఎడమ;
        ప్రతిజ్ఞా( p_సమాసం!=0 );
      }లేకపోతే{
        ఆపు;
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
స్థిర పూర్ణాంకం resolve_సమాసం_అడుగు(Walker *pWalker, సమాసం *p_సమాసం){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  ప్రతిజ్ఞా( pNC!=0 );
  pParse = pNC->pParse;
  ప్రతిజ్ఞా( pParse==pWalker->pParse );

#నిర్వచితం_కాకపోతే NDEBUG
  ఒకవేళ( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    పూర్ణాంకం i;
    కొరకు(i=0; i<pNC->pSrcList->nSrc; i++){
      ప్రతిజ్ఞా( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#అయితే_ముగింపు
  ఎంపిక( p_సమాసం->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    స్థితి TK_అడ్డువరుస: {
      SrcList *pSrcList = pNC->pSrcList;
      Src_అంశం *p_అంశం;
      ప్రతిజ్ఞా( pSrcList && pSrcList->nSrc>=1 );
      p_అంశం = pSrcList->a;
      p_సమాసం->op = TK_స్తంభం;
      ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
      p_సమాసం->y.pTab = p_అంశం->pSTab;
      p_సమాసం->i_పట్టిక = p_అంశం->iCursor;
      p_సమాసం->i_స్తంభం--;
      p_సమాసం->aff_సమాసం = SQLITE_AFF_INTEGER;
      ఆపు;
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
    స్థితి TK_NOTNULL:
    స్థితి TK_ISNULL: {
      పూర్ణాంకం anRef[8];
      NameContext *p;
      పూర్ణాంకం i;
      కొరకు(i=0, p=pNC; p && i<Array_కొలత(anRef); p=p->p_తదుపరి, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_సమాసం(pWalker, p_సమాసం->p_ఎడమ);
      ఒకవేళ( లోపల_RENAME_OBJECT ) తిరిగి WRC_Prune;
      ఒకవేళ( sqlite3_సమాసం_Can_Be_Null(p_సమాసం->p_ఎడమ) ){
        /* The expression can be NULL.  So the optimization does not apply */
        తిరిగి WRC_Prune;
      }

      కొరకు(i=0, p=pNC; p; p=p->p_తదుపరి, i++){
        ఒకవేళ( (p->ncFlags & NC_Where)==0 ){
          తిరిగి WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( సమాసం_Has_Property(p_సమాసం, EP_OuterON) );
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
#ఒకవేళ TREETRACE_ENABLED
      ఒకవేళ( sqlite3_వృక్షం_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_సమాసం->op==TK_NOTNULL
        );
        sqlite3_Show_సమాసం(p_సమాసం);
      }
#అయితే_ముగింపు /* TREETRACE_ENABLED */
      p_సమాసం->u.i_విలువ = (p_సమాసం->op==TK_NOTNULL);
      p_సమాసం->flags |= EP_Int_విలువ;
      p_సమాసం->op = TK_INTEGER;
      కొరకు(i=0, p=pNC; p && i<Array_కొలత(anRef); p=p->p_తదుపరి, i++){
        p->nRef = anRef[i];
      }
      sqlite3_సమాసం_Delete(pParse->db, p_సమాసం->p_ఎడమ);
      p_సమాసం->p_ఎడమ = 0;
      తిరిగి WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    స్థితి TK_ID:
    స్థితి TK_DOT: {
      స్థిరరాశి అక్షరం *z_పట్టిక;
      స్థిరరాశి అక్షరం *zDb;
      సమాసం *p_కుడి;

      ఒకవేళ( p_సమాసం->op==TK_ID ){
        zDb = 0;
        z_పట్టిక = 0;
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
        p_కుడి = p_సమాసం;
      }లేకపోతే{
        సమాసం *p_ఎడమ = p_సమాసం->p_ఎడమ;
        testcase( pNC->ncFlags & NC_Idx_సమాసం );
        testcase( pNC->ncFlags & NC_Gen_నిలువు );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_సమాసం|NC_Gen_నిలువు, 0, p_సమాసం);
        p_కుడి = p_సమాసం->p_కుడి;
        ఒకవేళ( p_కుడి->op==TK_ID ){
          zDb = 0;
        }లేకపోతే{
          ప్రతిజ్ఞా( p_కుడి->op==TK_DOT );
          ప్రతిజ్ఞా( !సమాసం_Has_Property(p_కుడి, EP_Int_విలువ) );
          zDb = p_ఎడమ->u.zToken;
          p_ఎడమ = p_కుడి->p_ఎడమ;
          p_కుడి = p_కుడి->p_కుడి;
        }
        ప్రతిజ్ఞా( సమాసం_Use_UToken(p_ఎడమ) && సమాసం_Use_UToken(p_కుడి) );
        z_పట్టిక = p_ఎడమ->u.zToken;
        ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
        ఒకవేళ( లోపల_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (శూన్య*)p_సమాసం, (శూన్య*)p_కుడి);
          sqlite3RenameTokenRemap(pParse, (శూన్య*)&p_సమాసం->y.pTab, (శూన్య*)p_ఎడమ);
        }
      }
      తిరిగి lookupName(pParse, zDb, z_పట్టిక, p_కుడి, pNC, p_సమాసం);
    }

    /* Resolve function names
    */
    స్థితి TK_FUNCTION: {
      సమాసం_List *pList;            /* The argument list */
      పూర్ణాంకం n;                      /* Number of arguments */
      పూర్ణాంకం no_such_func = 0;       /* True if no such function exists */
      పూర్ణాంకం wrong_అంకె_వాదనలు = 0;     /* True if wrong number of arguments */
      పూర్ణాంకం is_agg = 0;             /* True if is an aggregate function */
      స్థిరరాశి అక్షరం *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      పూర్ణాంకం savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      కిటికీ *pWin = (Is_కిటికీ_Func(p_సమాసం) ? p_సమాసం->y.pWin : 0);
#అయితే_ముగింపు
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_xIsSelect|EP_Int_విలువ) );
      ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ==0 || p_సమాసం->p_ఎడమ->op==TK_క్రమం );
      pList = p_సమాసం->x.pList;
      n = pList ? pList->n_సమాసం : 0;
      zId = p_సమాసం->u.zToken;
      pDef = sqlite3_కనుగొను_Function(pParse->db, zId, n, enc, 0);
      ఒకవేళ( pDef==0 ){
        pDef = sqlite3_కనుగొను_Function(pParse->db, zId, -2, enc, 0);
        ఒకవేళ( pDef==0 ){
          no_such_func = 1;
        }లేకపోతే{
          wrong_అంకె_వాదనలు = 1;
        }
      }లేకపోతే{
        is_agg = pDef->xFinalize!=0;
        ఒకవేళ( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          సమాసం_సమితి_Property(p_సమాసం, EP_Unlikely);
          ఒకవేళ( n==2 ){
            p_సమాసం->i_పట్టిక = సమాసం_Probability(pList->a[1].p_సమాసం);
            ఒకవేళ( p_సమాసం->i_పట్టిక<0 ){
              sqlite3_Error_సందేశం(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_సమాసం);
              pNC->nNcErr++;
            }
          }లేకపోతే{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_సమాసం->i_పట్టిక = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_AUTHORIZATION
        {
          పూర్ణాంకం auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          ఒకవేళ( auth!=SQLITE_OK ){
            ఒకవేళ( auth==SQLITE_DENY ){
              sqlite3_Error_సందేశం(pParse, "not authorized to use function: %#T",
                                      p_సమాసం);
              pNC->nNcErr++;
            }
            p_సమాసం->op = TK_NULL;
            తిరిగి WRC_Prune;
          }
        }
#అయితే_ముగింపు

        /* If the function may call sqlite3_value_subtype(), then set the 
        ** EP_SubtArg flag on all of its argument expressions. This prevents
        ** where.c from replacing the expression with a value read from an
        ** index on the same expression, which will not have the correct 
        ** subtype. Also set the flag if the function expression itself is
        ** an EP_SubtArg expression. In this case subtypes are required as 
        ** the function may return a value with a subtype back to its 
        ** caller using sqlite3_result_value().  */
        ఒకవేళ( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || సమాసం_Has_Property(p_సమాసం, EP_SubtArg) 
        ){
          resolve_సమితి_సమాసం_Subtype_Arg(pList);
        }

        ఒకవేళ( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          సమాసం_సమితి_Property(p_సమాసం,EP_ConstFunc);
        }
        ఒకవేళ( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_సమాసం|NC_PartIdx|NC_Gen_నిలువు, 0, p_సమాసం);
        }లేకపోతే{
          ప్రతిజ్ఞా( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_సమాసం->op2 = pNC->ncFlags & NC_SelfRef;
        }
        ఒకవేళ( (pDef->funcFlags & SQLITE_FUNC_అంతర్గత)!=0
         && pParse->nested==0
         && (pParse->db->mDbFlags & DBFLAG_అంతర్గత_Func)==0
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
        }లేకపోతే
        ఒకవేళ( (pDef->funcFlags & (SQLITE_FUNC_ప్రత్యక్ష|SQLITE_FUNC_UNSAFE))!=0
         && !లోపల_RENAME_OBJECT
        ){
          ఒకవేళ( pNC->ncFlags & NC_FromDDL ) సమాసం_సమితి_Property(p_సమాసం, EP_FromDDL);
          sqlite3_సమాసం_Function_Usable(pParse, p_సమాసం, pDef);
        }
      }

      ఒకవేళ( 0==లోపల_RENAME_OBJECT ){
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
        ప్రతిజ్ఞా( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_విలువ==0 && pDef->xInverse==0)
          || (pDef->x_విలువ && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        ఒకవేళ( pDef && pDef->x_విలువ==0 && pWin ){
          sqlite3_Error_సందేశం(pParse,
              "%#T() may not be used as a window function", p_సమాసం
          );
          pNC->nNcErr++;
        }లేకపోతే ఒకవేళ(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_కిటికీ) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          స్థిరరాశి అక్షరం *zType;
          ఒకవేళ( (pDef->funcFlags & SQLITE_FUNC_కిటికీ) || pWin ){
            zType = "window";
          }లేకపోతే{
            zType = "aggregate";
          }
          sqlite3_Error_సందేశం(pParse, "misuse of %s function %#T()",zType,p_సమాసం);
          pNC->nNcErr++;
          is_agg = 0;
        }
#లేకపోతే
        ఒకవేళ( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_సందేశం(pParse,"misuse of aggregate function %#T()",p_సమాసం);
          pNC->nNcErr++;
          is_agg = 0;
        }
#అయితే_ముగింపు
        లేకపోతే ఒకవేళ( no_such_func
              && (pParse->db->init.busy==0 ||
                  (no_such_func==2 && pParse->db->init.busy==2))
              /* Suppress "no such function" errors when reading
              ** the sqlite_schema table.  Except, do raise the error
              ** if init.busy is 2, meaning the schema parse is due
              ** to an ALTER TABLE ADD COLUMN statement, and the function
              ** is an internal-use-only function (no_such_func==2). */
#నిర్వచితమైతే SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
              && pParse->explain==0
#అయితే_ముగింపు
        ){
          sqlite3_Error_సందేశం(pParse, "no such function: %#T", p_సమాసం);
          pNC->nNcErr++;
        }లేకపోతే ఒకవేళ( wrong_అంకె_వాదనలు ){
          sqlite3_Error_సందేశం(pParse,"wrong number of arguments to function %#T()",
               p_సమాసం);
          pNC->nNcErr++;
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
        లేకపోతే ఒకవేళ( is_agg==0 && సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
          sqlite3_Error_సందేశం(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_సమాసం
          );
          pNC->nNcErr++;
        }
#అయితే_ముగింపు
        లేకపోతే ఒకవేళ( is_agg==0 && p_సమాసం->p_ఎడమ ){
          sqlite3_సమాసం_క్రమం_By_Aggregate_Error(pParse, p_సమాసం);
          pNC->nNcErr++;
        }
        ఒకవేళ( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#లేకపోతే
          pNC->ncFlags &= ~NC_AllowAgg;
#అయితే_ముగింపు
        }
      }
      లేకపోతే ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) || p_సమాసం->p_ఎడమ ){
        is_agg = 1;
      }
      sqlite3_Walk_సమాసం_List(pWalker, pList);
      ఒకవేళ( is_agg ){
        ఒకవేళ( p_సమాసం->p_ఎడమ ){
          ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ->op==TK_క్రమం );
          ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం->p_ఎడమ) );
          sqlite3_Walk_సమాసం_List(pWalker, p_సమాసం->p_ఎడమ->x.pList);
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
        ఒకవేళ( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          ప్రతిజ్ఞా( సమాసం_Use_YWin(p_సమాసం) && pWin==p_సమాసం->y.pWin );
          ఒకవేళ( లోపల_RENAME_OBJECT==0 ){
            sqlite3_కిటికీ_Update(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            ఒకవేళ( pParse->db->mallocFailed ) ఆపు;
          }
          sqlite3_Walk_సమాసం_List(pWalker, pWin->p_విభజన);
          sqlite3_Walk_సమాసం_List(pWalker, pWin->p_క్రమం_By);
          sqlite3_Walk_సమాసం(pWalker, pWin->pFilter);
          sqlite3_కిటికీ_Link(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }లేకపోతే
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_సమాసం->op = TK_AGG_FUNCTION;
          p_సమాసం->op2 = 0;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
          ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
            sqlite3_Walk_సమాసం(pWalker, p_సమాసం->y.pWin->pFilter);
          }
#అయితే_ముగింపు
          pNC2 = pNC;
          అయితే( pNC2
              && sqlite3ReferencesSrcList(pParse, p_సమాసం, pNC2->pSrcList)==0
          ){
            p_సమాసం->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->p_తదుపరి;
          }
          ప్రతిజ్ఞా( pDef!=0 || లోపల_RENAME_OBJECT );
          ఒకవేళ( pNC2 && pDef ){
            p_సమాసం->op2 += pNC2->nNestedSelect;
            ప్రతిజ్ఞా( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            ప్రతిజ్ఞా( SQLITE_FUNC_ANYORDER==NC_క్రమం_Agg );
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
      తిరిగి WRC_Prune;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    స్థితి TK_EXISTS:
    స్థితి TK_SELECT:
#అయితే_ముగింపు
    స్థితి TK_లోపల: {
      testcase( p_సమాసం->op==TK_లోపల );
      testcase( p_సమాసం->op==TK_EXISTS );
      testcase( p_సమాసం->op==TK_SELECT );
      ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
        పూర్ణాంకం nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_సమాసం );
        testcase( pNC->ncFlags & NC_Gen_నిలువు );
        ప్రతిజ్ఞా( p_సమాసం->x.pSelect );
        ఒకవేళ( p_సమాసం->op==TK_EXISTS )  pParse->bHasExists = 1;
        ఒకవేళ( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_సమాసం, p_సమాసం);
        }లేకపోతే{
          sqlite3WalkSelect(pWalker, p_సమాసం->x.pSelect);
        }
        ప్రతిజ్ఞా( pNC->nRef>=nRef );
        ఒకవేళ( nRef!=pNC->nRef ){
          సమాసం_సమితి_Property(p_సమాసం, EP_VarSelect);
          p_సమాసం->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      ఆపు;
    }
    స్థితి TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_సమాసం );
      testcase( pNC->ncFlags & NC_Gen_నిలువు );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_సమాసం|NC_Gen_నిలువు, p_సమాసం, p_సమాసం);
      ఆపు;
    }
    స్థితి TK_IS:
    స్థితి TK_ISNOT: {
      సమాసం *p_కుడి = sqlite3_సమాసం_Skip_Collate_And_Likely(p_సమాసం->p_కుడి);
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      ఒకవేళ( ALWAYS(p_కుడి) && (p_కుడి->op==TK_ID || p_కుడి->op==TK_TRUEFALSE) ){
        పూర్ణాంకం rc = resolve_సమాసం_అడుగు(pWalker, p_కుడి);
        ఒకవేళ( rc==WRC_Abort ) తిరిగి WRC_Abort;
        ఒకవేళ( p_కుడి->op==TK_TRUEFALSE ){
          p_సమాసం->op2 = p_సమాసం->op;
          p_సమాసం->op = TK_TRUTH;
          తిరిగి WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    స్థితి TK_BETWEEN:
    స్థితి TK_EQ:
    స్థితి TK_NE:
    స్థితి TK_LT:
    స్థితి TK_LE:
    స్థితి TK_GT:
    స్థితి TK_GE: {
      పూర్ణాంకం n_ఎడమ, n_కుడి;
      ఒకవేళ( pParse->db->mallocFailed ) ఆపు;
      ప్రతిజ్ఞా( p_సమాసం->p_ఎడమ!=0 );
      n_ఎడమ = sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం->p_ఎడమ);
      ఒకవేళ( p_సమాసం->op==TK_BETWEEN ){
        ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
        n_కుడి = sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం->x.pList->a[0].p_సమాసం);
        ఒకవేళ( n_కుడి==n_ఎడమ ){
          n_కుడి = sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం->x.pList->a[1].p_సమాసం);
        }
      }లేకపోతే{
        ప్రతిజ్ఞా( p_సమాసం->p_కుడి!=0 );
        n_కుడి = sqlite3_సమాసం_సదిశ_కొలత(p_సమాసం->p_కుడి);
      }
      ఒకవేళ( n_ఎడమ!=n_కుడి ){
        testcase( p_సమాసం->op==TK_EQ );
        testcase( p_సమాసం->op==TK_NE );
        testcase( p_సమాసం->op==TK_LT );
        testcase( p_సమాసం->op==TK_LE );
        testcase( p_సమాసం->op==TK_GT );
        testcase( p_సమాసం->op==TK_GE );
        testcase( p_సమాసం->op==TK_IS );
        testcase( p_సమాసం->op==TK_ISNOT );
        testcase( p_సమాసం->op==TK_BETWEEN );
        sqlite3_Error_సందేశం(pParse, "row value misused");
        sqlite3_రికార్డు_Error_స్థానభ్రంశం_యొక్క_సమాసం(pParse->db, p_సమాసం);
      }
      ఆపు;
    }
  }
  ప్రతిజ్ఞా( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  తిరిగి pParse->nErr ? WRC_Abort : WRC_Continue;
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
స్థిర పూర్ణాంకం resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  సమాసం_List *pEList,  /* List of expressions to scan */
  సమాసం *pE           /* Expression we are trying to match */
){
  పూర్ణాంకం i;             /* Loop counter */

  వాడని_PARAMETER(pParse);

  ఒకవేళ( pE->op==TK_ID ){
    స్థిరరాశి అక్షరం *z_నిలువు;
    ప్రతిజ్ఞా( !సమాసం_Has_Property(pE, EP_Int_విలువ) );
    z_నిలువు = pE->u.zToken;
    కొరకు(i=0; i<pEList->n_సమాసం; i++){
      ఒకవేళ( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, z_నిలువు)==0
      ){
        తిరిగి i+1;
      }
    }
  }
  తిరిగి 0;
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
స్థిర పూర్ణాంకం resolve_క్రమం_By_పదం_గమ్యం_సమాసం_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  సమాసం *pE           /* The specific ORDER BY term */
){
  పూర్ణాంకం i;             /* Loop counter */
  సమాసం_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  పూర్ణాంకం rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  ప్రతిజ్ఞా( sqlite3_సమాసం_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, పరిమాణం(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_సమాసం_పేర్లు(&nc, pE);
  db->suppressErr = savedSuppErr;
  ఒకవేళ( rc ) తిరిగి 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  కొరకు(i=0; i<pEList->n_సమాసం; i++){
    ఒకవేళ( sqlite3_సమాసం_Compare(0, pEList->a[i].p_సమాసం, pE, -1)<2 ){
      తిరిగి i+1;
    }
  }

  /* If no match, return 0. */
  తిరిగి 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
స్థిర శూన్య resolve_బహిర్గతం_యొక్క_Range_Error(
  Parse *pParse,         /* The error context into which to write the error */
  స్థిరరాశి అక్షరం *zType,     /* "ORDER" or "GROUP" */
  పూర్ణాంకం i,                 /* The index (1-based) of the term out of range */
  పూర్ణాంకం mx,                /* Largest permissible value of i */
  సమాసం *pError           /* Associate the error with the expression */
){
  sqlite3_Error_సందేశం(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_రికార్డు_Error_స్థానభ్రంశం_యొక్క_సమాసం(pParse->db, pError);
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
స్థిర పూర్ణాంకం resolve_Compound_క్రమం_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  పూర్ణాంకం i;
  సమాసం_List *p_క్రమం_By;
  సమాసం_List *pEList;
  sqlite3 *db;
  పూర్ణాంకం మరిన్ని_గమ్యం_Do = 1;

  p_క్రమం_By = pSelect->p_క్రమం_By;
  ఒకవేళ( p_క్రమం_By==0 ) తిరిగి 0;
  db = pParse->db;
  ఒకవేళ( p_క్రమం_By->n_సమాసం>db->a_హద్దు[SQLITE_హద్దు_స్తంభం] ){
    sqlite3_Error_సందేశం(pParse, "too many terms in ORDER BY clause");
    తిరిగి 1;
  }
  కొరకు(i=0; i<p_క్రమం_By->n_సమాసం; i++){
    p_క్రమం_By->a[i].fg.పూర్తి = 0;
  }
  pSelect->p_తదుపరి = 0;
  అయితే( pSelect->p_పూర్వ ){
    pSelect->p_పూర్వ->p_తదుపరి = pSelect;
    pSelect = pSelect->p_పూర్వ;
  }
  అయితే( pSelect && మరిన్ని_గమ్యం_Do ){
    నిర్మాణం సమాసం_List_అంశం *p_అంశం;
    మరిన్ని_గమ్యం_Do = 0;
    pEList = pSelect->pEList;
    ప్రతిజ్ఞా( pEList!=0 );
    కొరకు(i=0, p_అంశం=p_క్రమం_By->a; i<p_క్రమం_By->n_సమాసం; i++, p_అంశం++){
      పూర్ణాంకం i_నిలువు = -1;
      సమాసం *pE, *pDup;
      ఒకవేళ( p_అంశం->fg.పూర్తి ) కొనసాగు;
      pE = sqlite3_సమాసం_Skip_Collate_And_Likely(p_అంశం->p_సమాసం);
      ఒకవేళ( NEVER(pE==0) ) కొనసాగు;
      ఒకవేళ( sqlite3_సమాసం_Is_Integer(pE, &i_నిలువు, 0, 1) ){
        ఒకవేళ( i_నిలువు<=0 || i_నిలువు>pEList->n_సమాసం ){
          resolve_బహిర్గతం_యొక్క_Range_Error(pParse, "ORDER", i+1, pEList->n_సమాసం, pE);
          తిరిగి 1;
        }
      }లేకపోతే{
        i_నిలువు = resolveAsName(pParse, pEList, pE);
        ఒకవేళ( i_నిలువు==0 ){
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
          pDup = sqlite3_సమాసం_Dup(db, pE, 0);
          ఒకవేళ( !db->mallocFailed ){
            ప్రతిజ్ఞా(pDup);
            i_నిలువు = resolve_క్రమం_By_పదం_గమ్యం_సమాసం_List(pParse, pSelect, pDup);
            ఒకవేళ( లోపల_RENAME_OBJECT && i_నిలువు>0 ){
              resolve_క్రమం_By_పదం_గమ్యం_సమాసం_List(pParse, pSelect, pE);
            }
          }
          sqlite3_సమాసం_Delete(db, pDup);
        }
      }
      ఒకవేళ( i_నిలువు>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        ఒకవేళ( !లోపల_RENAME_OBJECT ){
          సమాసం *pNew = sqlite3_సమాసం_Int32(db, i_నిలువు);
          ఒకవేళ( pNew==0 ) తిరిగి 1;
          ఒకవేళ( p_అంశం->p_సమాసం==pE ){
            p_అంశం->p_సమాసం = pNew;
          }లేకపోతే{
            సమాసం *p_మాతృ = p_అంశం->p_సమాసం;
            ప్రతిజ్ఞా( p_మాతృ->op==TK_COLLATE );
            అయితే( p_మాతృ->p_ఎడమ->op==TK_COLLATE ) p_మాతృ = p_మాతృ->p_ఎడమ;
            ప్రతిజ్ఞా( p_మాతృ->p_ఎడమ==pE );
            p_మాతృ->p_ఎడమ = pNew;
          }
          sqlite3_సమాసం_Delete(db, pE);
          p_అంశం->u.x.i_క్రమం_By_నిలువు = (u16)i_నిలువు;
        }
        p_అంశం->fg.పూర్తి = 1;
      }లేకపోతే{
        మరిన్ని_గమ్యం_Do = 1;
      }
    }
    pSelect = pSelect->p_తదుపరి;
  }
  కొరకు(i=0; i<p_క్రమం_By->n_సమాసం; i++){
    ఒకవేళ( p_క్రమం_By->a[i].fg.పూర్తి==0 ){
      sqlite3_Error_సందేశం(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      తిరిగి 1;
    }
  }
  తిరిగి 0;
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
పూర్ణాంకం sqlite3_Resolve_క్రమం_సమూహం_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  సమాసం_List *p_క్రమం_By,   /* The ORDER BY or GROUP BY clause to be processed */
  స్థిరరాశి అక్షరం *zType     /* "ORDER" or "GROUP" */
){
  పూర్ణాంకం i;
  sqlite3 *db = pParse->db;
  సమాసం_List *pEList;
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;

  ఒకవేళ( p_క్రమం_By==0 || pParse->db->mallocFailed || లోపల_RENAME_OBJECT ) తిరిగి 0;
  ఒకవేళ( p_క్రమం_By->n_సమాసం>db->a_హద్దు[SQLITE_హద్దు_స్తంభం] ){
    sqlite3_Error_సందేశం(pParse, "too many terms in %s BY clause", zType);
    తిరిగి 1;
  }
  pEList = pSelect->pEList;
  ప్రతిజ్ఞా( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  కొరకు(i=0, p_అంశం=p_క్రమం_By->a; i<p_క్రమం_By->n_సమాసం; i++, p_అంశం++){
    ఒకవేళ( p_అంశం->u.x.i_క్రమం_By_నిలువు ){
      ఒకవేళ( p_అంశం->u.x.i_క్రమం_By_నిలువు>pEList->n_సమాసం ){
        resolve_బహిర్గతం_యొక్క_Range_Error(pParse, zType, i+1, pEList->n_సమాసం, 0);
        తిరిగి 1;
      }
      resolveAlias(pParse, pEList, p_అంశం->u.x.i_క్రమం_By_నిలువు-1, p_అంశం->p_సమాసం,0);
    }
  }
  తిరిగి 0;
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
స్థిర పూర్ణాంకం resolveRemoveWindowsCb(Walker *pWalker, సమాసం *p_సమాసం){
  వాడని_PARAMETER(pWalker);
  ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
    కిటికీ *pWin = p_సమాసం->y.pWin;
    sqlite3_కిటికీ_Unlink_From_Select(pWin);
  }
  తిరిగి WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
స్థిర శూన్య కిటికీ_Remove_సమాసం_From_Select(Select *pSelect, సమాసం *p_సమాసం){
  ఒకవేళ( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, పరిమాణం(Walker));
    sWalker.x_సమాసం_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_సమాసం(&sWalker, p_సమాసం);
  }
}
#లేకపోతే
# define కిటికీ_Remove_సమాసం_From_Select(a, b)
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */

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
స్థిర పూర్ణాంకం resolve_క్రమం_సమూహం_By(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  సమాసం_List *p_క్రమం_By,   /* An ORDER BY or GROUP BY clause to resolve */
  స్థిరరాశి అక్షరం *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  పూర్ణాంకం i, j;                      /* Loop counters */
  పూర్ణాంకం i_నిలువు;                      /* Column number */
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  పూర్ణాంకం n_ఫలితం;                   /* Number of terms in the result set */

  ప్రతిజ్ఞా( p_క్రమం_By!=0 );
  n_ఫలితం = pSelect->pEList->n_సమాసం;
  pParse = pNC->pParse;
  కొరకు(i=0, p_అంశం=p_క్రమం_By->a; i<p_క్రమం_By->n_సమాసం; i++, p_అంశం++){
    సమాసం *pE = p_అంశం->p_సమాసం;
    సమాసం *pE2 = sqlite3_సమాసం_Skip_Collate_And_Likely(pE);
    ఒకవేళ( NEVER(pE2==0) ) కొనసాగు;
    ఒకవేళ( zType[0]!='G' ){
      i_నిలువు = resolveAsName(pParse, pSelect->pEList, pE2);
      ఒకవేళ( i_నిలువు>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        p_అంశం->u.x.i_క్రమం_By_నిలువు = (u16)i_నిలువు;
        కొనసాగు;
      }
    }
    ఒకవేళ( sqlite3_సమాసం_Is_Integer(pE2, &i_నిలువు, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      ఒకవేళ( i_నిలువు<1 || i_నిలువు>0xffff ){
        resolve_బహిర్గతం_యొక్క_Range_Error(pParse, zType, i+1, n_ఫలితం, pE2);
        తిరిగి 1;
      }
      p_అంశం->u.x.i_క్రమం_By_నిలువు = (u16)i_నిలువు;
      కొనసాగు;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    p_అంశం->u.x.i_క్రమం_By_నిలువు = 0;
    ఒకవేళ( sqlite3_Resolve_సమాసం_పేర్లు(pNC, pE) ){
      తిరిగి 1;
    }
    కొరకు(j=0; j<pSelect->pEList->n_సమాసం; j++){
      ఒకవేళ( sqlite3_సమాసం_Compare(0, pE, pSelect->pEList->a[j].p_సమాసం, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        కిటికీ_Remove_సమాసం_From_Select(pSelect, pE);
        p_అంశం->u.x.i_క్రమం_By_నిలువు = j+1;
      }
    }
  }
  తిరిగి sqlite3_Resolve_క్రమం_సమూహం_By(pParse, pSelect, p_క్రమం_By, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
స్థిర పూర్ణాంకం resolve_Select_అడుగు(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  పూర్ణాంకం isCompound;         /* True if p is a compound select */
  పూర్ణాంకం nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  పూర్ణాంకం i;                  /* Loop counter */
  సమాసం_List *p_సమూహం_By;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  ప్రతిజ్ఞా( p!=0 );
  ఒకవేళ( p->selFlags & SF_Resolved ){
    తిరిగి WRC_Prune;
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
  ఒకవేళ( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    తిరిగి pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->p_పూర్వ!=0;
  nCompound = 0;
  pLeftmost = p;
  అయితే( p ){
    ప్రతిజ్ఞా( (p->selFlags & SF_Expanded)!=0 );
    ప్రతిజ్ఞా( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, పరిమాణం(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    ఒకవేళ( sqlite3_Resolve_సమాసం_పేర్లు(&sNC, p->p_హద్దు) ){
      తిరిగి WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    ఒకవేళ( p->selFlags & SF_Converted ){
      Select *pSub;
      ప్రతిజ్ఞా( p->pSrc->a[0].fg.isSubquery );
      ప్రతిజ్ఞా( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ప్రతిజ్ఞా( pSub!=0 );
      ప్రతిజ్ఞా( p->pSrc->nSrc==1 && p->p_క్రమం_By );
      ప్రతిజ్ఞా( pSub->p_పూర్వ && pSub->p_క్రమం_By==0 );
      pSub->p_క్రమం_By = p->p_క్రమం_By;
      p->p_క్రమం_By = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    ఒకవేళ( pOuterNC ) pOuterNC->nNestedSelect++;
    కొరకు(i=0; i<p->pSrc->nSrc; i++){
      Src_అంశం *p_అంశం = &p->pSrc->a[i];
      ప్రతిజ్ఞా( p_అంశం->zName!=0
              || p_అంశం->fg.isSubquery );  /* Test of tag-20240424-1*/
      ఒకవేళ( p_అంశం->fg.isSubquery
       && (p_అంశం->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        పూర్ణాంకం nRef = pOuterNC ? pOuterNC->nRef : 0;
        స్థిరరాశి అక్షరం *zSavedContext = pParse->zAuthContext;

        ఒకవేళ( p_అంశం->zName ) pParse->zAuthContext = p_అంశం->zName;
        sqlite3_Resolve_Select_పేర్లు(pParse, p_అంశం->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        ఒకవేళ( pParse->nErr ) తిరిగి WRC_Abort;
        ప్రతిజ్ఞా( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        ఒకవేళ( pOuterNC ){
          ప్రతిజ్ఞా( p_అంశం->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          p_అంశం->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    ఒకవేళ( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.p_తదుపరి = pOuterNC;
 
    /* Resolve names in the result set. */
    ఒకవేళ( sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, p->pEList) ) తిరిగి WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    ప్రతిజ్ఞా( (p->selFlags & SF_Aggregate)==0 );
    p_సమూహం_By = p->p_సమూహం_By;
    ఒకవేళ( p_సమూహం_By || (sNC.ncFlags & NC_HasAgg)!=0 ){
      ప్రతిజ్ఞా( NC_MinMaxAgg==SF_MinMaxAgg );
      ప్రతిజ్ఞా( NC_క్రమం_Agg==SF_క్రమం_By_Reqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_క్రమం_Agg));
    }లేకపోతే{
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
    ప్రతిజ్ఞా( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    ఒకవేళ( p->pHaving ){
      ఒకవేళ( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_సందేశం(pParse, "HAVING clause on a non-aggregate query");
        తిరిగి WRC_Abort;
      }
      ఒకవేళ( sqlite3_Resolve_సమాసం_పేర్లు(&sNC, p->pHaving) ) తిరిగి WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    ఒకవేళ( sqlite3_Resolve_సమాసం_పేర్లు(&sNC, p->pWhere) ) తిరిగి WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    కొరకు(i=0; i<p->pSrc->nSrc; i++){
      Src_అంశం *p_అంశం = &p->pSrc->a[i];
      ఒకవేళ( p_అంశం->fg.isTabFunc
       && sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, p_అంశం->u1.pFuncArg)
      ){
        తిరిగి WRC_Abort;
      }
    }

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( లోపల_RENAME_OBJECT ){
      కిటికీ *pWin;
      కొరకు(pWin=p->pWinDefn; pWin; pWin=pWin->p_తదుపరి_Win){
        ఒకవేళ( sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, pWin->p_క్రమం_By)
         || sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, pWin->p_విభజన)
        ){
          తిరిగి WRC_Abort;
        }
      }
    }
#అయితే_ముగింపు

    sNC.ncFlags |= NC_AllowAgg|NC_AllowWin;

    /* If this is a converted compound query, move the ORDER BY clause from
    ** the sub-query back to the parent query. At this point each term
    ** within the ORDER BY clause has been transformed to an integer value.
    ** These integers will be replaced by copies of the corresponding result
    ** set expressions by the call to resolveOrderGroupBy() below.  */
    ఒకవేళ( p->selFlags & SF_Converted ){
      Select *pSub;
      ప్రతిజ్ఞా( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ప్రతిజ్ఞా( pSub!=0 );
      p->p_క్రమం_By = pSub->p_క్రమం_By;
      pSub->p_క్రమం_By = 0;
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
    ఒకవేళ( p->p_క్రమం_By!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolve_క్రమం_సమూహం_By(&sNC, p, p->p_క్రమం_By, "ORDER")
    ){
      తిరిగి WRC_Abort;
    }
    ఒకవేళ( db->mallocFailed ){
      తిరిగి WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    ఒకవేళ( p_సమూహం_By ){
      నిర్మాణం సమాసం_List_అంశం *p_అంశం;
   
      ఒకవేళ( resolve_క్రమం_సమూహం_By(&sNC, p, p_సమూహం_By, "GROUP") || db->mallocFailed ){
        తిరిగి WRC_Abort;
      }
      కొరకు(i=0, p_అంశం=p_సమూహం_By->a; i<p_సమూహం_By->n_సమాసం; i++, p_అంశం++){
        ఒకవేళ( సమాసం_Has_Property(p_అంశం->p_సమాసం, EP_Agg) ){
          sqlite3_Error_సందేశం(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          తిరిగి WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    ఒకవేళ( p->p_తదుపరి && p->pEList->n_సమాసం!=p->p_తదుపరి->pEList->n_సమాసం ){
      sqlite3_Select_Wrong_అంకె_Terms_Error(pParse, p->p_తదుపరి);
      తిరిగి WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    ఒకవేళ( (p->selFlags & SF_On_గమ్యం_Where) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      ఒకవేళ( pParse->nErr ) తిరిగి WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->p_పూర్వ;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  ఒకవేళ( isCompound && resolve_Compound_క్రమం_By(pParse, pLeftmost) ){
    తిరిగి WRC_Abort;
  }

  తిరిగి WRC_Prune;
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
పూర్ణాంకం sqlite3_Resolve_సమాసం_పేర్లు(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  సమాసం *p_సమాసం             /* The expression to be analyzed. */
){
  పూర్ణాంకం savedHasAgg;
  Walker w;

  ఒకవేళ( p_సమాసం==0 ) తిరిగి SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_క్రమం_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_క్రమం_Agg);
  w.pParse = pNC->pParse;
  w.x_సమాసం_Callback = resolve_సమాసం_అడుగు;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolve_Select_అడుగు;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
  w.pParse->n_ఎత్తు += p_సమాసం->n_ఎత్తు;
  ఒకవేళ( sqlite3_సమాసం_Check_ఎత్తు(w.pParse, w.pParse->n_ఎత్తు) ){
    తిరిగి SQLITE_ERROR;
  }
#అయితే_ముగింపు
  ప్రతిజ్ఞా( p_సమాసం!=0 );
  sqlite3_Walk_సమాసం_NN(&w, p_సమాసం);
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
  w.pParse->n_ఎత్తు -= p_సమాసం->n_ఎత్తు;
#అయితే_ముగింపు
  ప్రతిజ్ఞా( EP_Agg==NC_HasAgg );
  ప్రతిజ్ఞా( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  సమాసం_సమితి_Property(p_సమాసం, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  తిరిగి pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
పూర్ణాంకం sqlite3_Resolve_సమాసం_List_పేర్లు(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  సమాసం_List *pList         /* The expression list to be analyzed. */
){
  పూర్ణాంకం i;
  పూర్ణాంకం savedHasAgg = 0;
  Walker w;
  ఒకవేళ( pList==0 ) తిరిగి SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_సమాసం_Callback = resolve_సమాసం_అడుగు;
  w.xSelectCallback = resolve_Select_అడుగు;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_క్రమం_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_క్రమం_Agg);
  కొరకు(i=0; i<pList->n_సమాసం; i++){
    సమాసం *p_సమాసం = pList->a[i].p_సమాసం;
    ఒకవేళ( p_సమాసం==0 ) కొనసాగు;
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
    w.pParse->n_ఎత్తు += p_సమాసం->n_ఎత్తు;
    ఒకవేళ( sqlite3_సమాసం_Check_ఎత్తు(w.pParse, w.pParse->n_ఎత్తు) ){
      తిరిగి SQLITE_ERROR;
    }
#అయితే_ముగింపు
    sqlite3_Walk_సమాసం_NN(&w, p_సమాసం);
#ఒకవేళ SQLITE_MAX_సమాసం_లోతు>0
    w.pParse->n_ఎత్తు -= p_సమాసం->n_ఎత్తు;
#అయితే_ముగింపు
    ప్రతిజ్ఞా( EP_Agg==NC_HasAgg );
    ప్రతిజ్ఞా( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    ఒకవేళ( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_క్రమం_Agg) ){
      సమాసం_సమితి_Property(p_సమాసం, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_క్రమం_Agg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_క్రమం_Agg);
    }
    ఒకవేళ( w.pParse->nErr>0 ) తిరిగి SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  తిరిగి SQLITE_OK;
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
శూన్య sqlite3_Resolve_Select_పేర్లు(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  ప్రతిజ్ఞా( p!=0 );
  w.x_సమాసం_Callback = resolve_సమాసం_అడుగు;
  w.xSelectCallback = resolve_Select_అడుగు;
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
పూర్ణాంకం sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  పట్టిక *pTab,     /* The table being referenced, or NULL */
  పూర్ణాంకం type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  సమాసం *p_సమాసం,     /* Expression to resolve.  May be NULL. */
  సమాసం_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  పూర్ణాంకం rc;
  సంఘం {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  ప్రతిజ్ఞా( type==0 || pTab!=0 );
  ప్రతిజ్ఞా( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_సమాసం
          || type==NC_Gen_నిలువు || pTab==0 );
  memset(&sNC, 0, పరిమాణం(sNC));
  memset(&uSrc, 0, పరిమాణం(uSrc));
  pSrc = &uSrc.sSrc;
  ఒకవేళ( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    ఒకవేళ( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  ఒకవేళ( (rc = sqlite3_Resolve_సమాసం_పేర్లు(&sNC, p_సమాసం))!=SQLITE_OK ) తిరిగి rc;
  ఒకవేళ( pList ) rc = sqlite3_Resolve_సమాసం_List_పేర్లు(&sNC, pList);
  తిరిగి rc;
}
