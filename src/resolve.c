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
#include "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#define EXCLUDED_TABLE_NUMBER  2

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
static ကိန်းပြည့် incrAggDepth(Walker *pWalker, Expr *pExpr){
  အကယ်၍( pExpr->op==TK_AGG_FUNCTION ) pExpr->op2 += pWalker->u.n;
  ပြန်ပေး WRC_Continue;
}
static void incrAggFunctionDepth(Expr *pExpr, ကိန်းပြည့် N){
  အကယ်၍( N>0 ){
    Walker w;
    memset(&w, 0, parimaa_na(w));
    w.xExprCallback = incrAggDepth;
    w.u.n = N;
    sqlite3WalkExpr(&w, pExpr);
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
static void resolveAlias(
  Parse *pParse,         /* Parsing context */
  ExprList *pEList,      /* A result set */
  ကိန်းပြည့် iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  Expr *pExpr,           /* Transform this into an alias to the result set */
  ကိန်းပြည့် nSubquery          /* Number of subqueries that the label is moving */
){
  Expr *pOrig;           /* The iCol-th column of the result set */
  Expr *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  assert( iCol>=0 && iCol<pEList->nExpr );
  pOrig = pEList->a[iCol].pExpr;
  assert( pOrig!=0 );
  assert( !ExprHasProperty(pExpr, EP_Reduced|EP_TokenOnly) );
  အကယ်၍( pExpr->pAggInfo ) ပြန်ပေး;
  db = pParse->db;
  pDup = sqlite3ExprDup(db, pOrig, 0);
  အကယ်၍( db->mallocFailed ){
    sqlite3ExprDelete(db, pDup);
    pDup = 0;
  }မဟုတ်ပါက{
    Expr temp;
    incrAggFunctionDepth(pDup, nSubquery);
    အကယ်၍( pExpr->op==TK_COLLATE ){
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      pDup = sqlite3ExprAddCollateString(pParse, pDup, pExpr->u.zToken);
    }
    memcpy(&temp, pDup, parimaa_na(Expr));
    memcpy(pDup, pExpr, parimaa_na(Expr));
    memcpy(pExpr, &temp, parimaa_na(Expr));
    အကယ်၍( ExprHasProperty(pExpr, EP_WinFunc) ){
      အကယ်၍( ALWAYS(pExpr->y.pWin!=0) ){
        pExpr->y.pWin->pOwner = pExpr;
      }
    }
    sqlite3ExprDeferredDelete(pParse, pDup);
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
ကိန်းပြည့် sqlite3MatchEName(
  const sa_nracanaa ExprList_item *pItem,
  const var_na *zCol,
  const var_na *zTab,
  const var_na *zDb,
  ကိန်းပြည့် *pbRowid
){
  ကိန်းပြည့် n;
  const var_na *zSpan;
  ကိန်းပြည့် eEName = pItem->fg.eEName;
  အကယ်၍( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    ပြန်ပေး 0;
  }
  assert( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  အတွက်(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  အကယ်၍( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    ပြန်ပေး 0;
  }
  zSpan += n+1;
  အတွက်(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  အကယ်၍( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    ပြန်ပေး 0;
  }
  zSpan += n+1;
  အကယ်၍( zCol ){
    အကယ်၍( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) ပြန်ပေး 0;
    အကယ်၍( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) ပြန်ပေး 0;
  }
  အကယ်၍( eEName==ENAME_ROWID ) *pbRowid = 1;
  ပြန်ပေး 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
static ကိန်းပြည့် areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *pTopNC){
  အကယ်၍( db->init.busy ) ပြန်ပေး 1;  /* Always support for legacy schemas */
  အကယ်၍( pTopNC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    အကယ်၍( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      ပြန်ပေး 1;
    }
    ပြန်ပေး (db->flags & SQLITE_DqsDDL)!=0;
  }မဟုတ်ပါက{
    /* Currently parsing a DML statement */
    ပြန်ပေး (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3ExprColUsed(Expr *pExpr){
  ကိန်းပြည့် n;
  Table *pExTab;

  n = pExpr->iColumn;
  assert( ExprUseYTab(pExpr) );
  pExTab = pExpr->y.pTab;
  assert( pExTab!=0 );
  assert( n < pExTab->nCol );
  အကယ်၍( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    ပြန်ပေး pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }မဟုတ်ပါက{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    အကယ်၍( n>=BMS ) n = BMS-1;
    ပြန်ပေး ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
static void extendFJMatch(
  Parse *pParse,          /* Parsing context */
  ExprList **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  Expr *pNew = sqlite3ExprAlloc(pParse->db, TK_COLUMN, 0, 0);
  အကယ်၍( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    assert( (pMatch->fg.jointype & (JT_LEFT|JT_LTORJ))!=0 );
    ExprSetProperty(pNew, EP_CanBeNull);
    *ppList = sqlite3ExprListAppend(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
static SQLITE_NOINLINE ကိန်းပြည့် isValidSchemaTableName(
  const var_na *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  const var_na *zDb           /* non-NULL if a database qualifier is present */
){
  const var_na *zLegacy;
  assert( pTab!=0 );
  assert( pTab->tnum==1 );
  အကယ်၍( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) ပြန်ပေး 0;
  zLegacy = pTab->zName;
  အကယ်၍( strcmp(zLegacy+7, &LEGACY_TEMP_SCHEMA_TABLE[7])==0 ){
    အကယ်၍( sqlite3StrICmp(zTab+7, &PREFERRED_TEMP_SCHEMA_TABLE[7])==0 ){
      ပြန်ပေး 1;
    }
    အကယ်၍( zDb==0 ) ပြန်ပေး 0;
    အကယ်၍( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) ပြန်ပေး 1;
    အကယ်၍( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) ပြန်ပေး 1;
  }မဟုတ်ပါက{
    အကယ်၍( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) ပြန်ပေး 1;
  }
  ပြန်ပေး 0;
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
static ကိန်းပြည့် lookupName(
  Parse *pParse,       /* The parsing context */
  const var_na *zDb,     /* Name of the database containing table, or NULL */
  const var_na *zTab,    /* Name of table containing column, or NULL */
  const Expr *pRight,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  Expr *pExpr          /* Make this EXPR node point to the selected column */
){
  ကိန်းပြည့် i, j;                         /* Loop counters */
  ကိန်းပြည့် cnt = 0;                      /* Number of matching column names */
  ကိန်းပြည့် cntTab = 0;                   /* Number of potential "rowid" matches */
  ကိန်းပြည့် nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *pTopNC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  ကိန်းပြည့် eNewExprOp = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  ExprList *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  const var_na *zCol = pRight->u.zToken;

  assert( pNC );     /* the name context cannot be NULL. */
  assert( zCol );    /* The Z in X.Y.Z cannot be NULL */
  assert( zDb==0 || zTab!=0 );
  assert( !ExprHasProperty(pExpr, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  pExpr->iTable = -1;
  ExprSetVVAProperty(pExpr, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  အကယ်၍( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    အကယ်၍( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }မဟုတ်ပါက{
      အတွက်(i=0; i<db->nDb; i++){
        assert( db->aDb[i].zDbSName );
        အကယ်၍( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          bha_nga;
        }
      }
      အကယ်၍( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  assert( pNC && cnt==0 );
  do{
    ExprList *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    အကယ်၍( pSrcList ){
      အတွက်(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        assert( pTab!=0 && pTab->zName!=0 );
        assert( pTab->nCol>0 || pParse->nErr );
        assert( (ကိန်းပြည့်)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        အကယ်၍( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          ကိန်းပြည့် hit = 0;
          Select *pSel;
          assert( pItem->fg.isSubquery );
          assert( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          assert( pSel!=0 );
          pEList = pSel->pEList;
          assert( pEList!=0 );
          assert( pEList->nExpr==pTab->nCol );
          အတွက်(j=0; j<pEList->nExpr; j++){
            ကိန်းပြည့် bRowid = 0;       /* True if possible rowid match */
            အကယ်၍( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              continue;
            }
            အကယ်၍( bRowid==0 ){
              အကယ်၍( cnt>0 ){
                အကယ်၍( pItem->fg.isUsing==0
                 || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3ExprListDelete(db, pFJMatch);
                  pFJMatch = 0;
                }မဟုတ်ပါက
                အကယ်၍( (pItem->fg.jointype & JT_RIGHT)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  continue;
                }မဟုတ်ပါက
                အကယ်၍( (pItem->fg.jointype & JT_LEFT)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3ExprListDelete(db, pFJMatch);
                  pFJMatch = 0;
                }မဟုတ်ပါက{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, pExpr->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }မဟုတ်ပါက အကယ်၍( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              continue;
            }
            cntTab++;
            pMatch = pItem;
            pExpr->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            assert( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            အကယ်၍( pEList->a[j].fg.bUsingTerm ) bha_nga;
          }
          အကယ်၍( hit || zTab==0 ) continue;
        }
        assert( zDb==0 || zTab!=0 );
        အကယ်၍( zTab ){
          အကယ်၍( zDb ){
            အကယ်၍( pTab->pSchema!=pSchema ) continue;
            အကယ်၍( pSchema==0 && strcmp(zDb,"*")!=0 ) continue;
          }
          အကယ်၍( pItem->zAlias!=0 ){
            အကယ်၍( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              continue;
            }
          }မဟုတ်ပါက အကယ်၍( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            အကယ်၍( pTab->tnum!=1 ) continue;
            အကယ်၍( !isValidSchemaTableName(zTab, pTab, zDb) ) continue;
          }
          assert( ExprUseYTab(pExpr) );
          အကယ်၍( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (void*)&pExpr->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        အကယ်၍( j>=0 ){
          အကယ်၍( cnt>0 ){
            အကယ်၍( pItem->fg.isUsing==0
             || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3ExprListDelete(db, pFJMatch);
              pFJMatch = 0;
            }မဟုတ်ပါက
            အကယ်၍( (pItem->fg.jointype & JT_RIGHT)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              continue;
            }မဟုတ်ပါက
            အကယ်၍( (pItem->fg.jointype & JT_LEFT)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3ExprListDelete(db, pFJMatch);
              pFJMatch = 0;
            }မဟုတ်ပါက{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, pExpr->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          pExpr->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          အကယ်၍( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        အကယ်၍( 0==cnt && VisibleRowid(pTab) ){
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
          အကယ်၍( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }မဟုတ်ပါက{
            cntTab++;
          }
#မဟုတ်ပါက
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      အကယ်၍( pMatch ){
        pExpr->iTable = pMatch->iCursor;
        assert( ExprUseYTab(pExpr) );
        pExpr->y.pTab = pMatch->pSTab;
        အကယ်၍( (pMatch->fg.jointype & (JT_LEFT|JT_LTORJ))!=0 ){
          ExprSetProperty(pExpr, EP_CanBeNull);
        }
        pSchema = pExpr->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#အကယ်၍ !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    အကယ်၍( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      အကယ်၍( pParse->pTriggerTab!=0 ){
        ကိန်းပြည့် op = pParse->eTriggerOp;
        assert( op==TK_DELETE || op==TK_UPDATE || op==TK_INSERT );
        အကယ်၍( pParse->bReturning ){
          အကယ်၍( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            pExpr->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }မဟုတ်ပါက အကယ်၍( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          pExpr->iTable = 1;
          pTab = pParse->pTriggerTab;
        }မဟုတ်ပါက အကယ်၍( op!=TK_INSERT && zTab && sqlite3StrICmp("old",zTab)==0 ){
          pExpr->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      အကယ်၍( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        အကယ်၍( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          pExpr->iTable = EXCLUDED_TABLE_NUMBER;
          အကယ်၍( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (void*)&pExpr->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      အကယ်၍( pTab ){
        ကိန်းပြည့် iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        အကယ်၍( iCol>=0 ){
          အကယ်၍( pTab->iPKey==iCol ) iCol = -1;
        }မဟုတ်ပါက{
          အကယ်၍( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }မဟုတ်ပါက{
            iCol = pTab->nCol;
          }
        }
        အကယ်၍( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          အကယ်၍( pExpr->iTable==EXCLUDED_TABLE_NUMBER ){
            testcase( iCol==(-1) );
            assert( ExprUseYTab(pExpr) );
            အကယ်၍( IN_RENAME_OBJECT ){
              pExpr->iColumn = iCol;
              pExpr->y.pTab = pTab;
              eNewExprOp = TK_COLUMN;
            }မဟုတ်ပါက{
              pExpr->iTable = pNC->uNC.pUpsert->regData +
                 sqlite3TableColumnToStorage(pTab, iCol);
              eNewExprOp = TK_REGISTER;
            }
          }မဟုတ်ပါက
#endif /* SQLITE_OMIT_UPSERT */
          {
            assert( ExprUseYTab(pExpr) );
            pExpr->y.pTab = pTab;
            အကယ်၍( pParse->bReturning ){
              eNewExprOp = TK_REGISTER;
              pExpr->op2 = TK_COLUMN;
              pExpr->iColumn = iCol;
              pExpr->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*pExpr->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }မဟုတ်ပါက{
              pExpr->iColumn = (i16)iCol;
              eNewExprOp = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              အကယ်၍( iCol<0 ){
                pExpr->affExpr = SQLITE_AFF_INTEGER;
              }မဟုတ်ပါက အကယ်၍( pExpr->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }မဟုတ်ပါက{
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
    အကယ်၍( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_IdxExpr|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#အကယ်၍ SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      အကယ်၍( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        eNewExprOp = TK_NULL;
      }
#endif
      အကယ်၍( pMatch->fg.isNestedFrom==0 ) pExpr->iColumn = -1;
      pExpr->affExpr = SQLITE_AFF_INTEGER;
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
    အကယ်၍( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      assert( pEList!=0 );
      အတွက်(j=0; j<pEList->nExpr; j++){
        var_na *zAs = pEList->a[j].zEName;
        အကယ်၍( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          Expr *pOrig;
          assert( pExpr->pLeft==0 && pExpr->pRight==0 );
          assert( ExprUseXList(pExpr)==0 || pExpr->x.pList==0 );
          assert( ExprUseXSelect(pExpr)==0 || pExpr->x.pSelect==0 );
          pOrig = pEList->a[j].pExpr;
          အကယ်၍( (pNC->ncFlags&NC_AllowAgg)==0 && ExprHasProperty(pOrig, EP_Agg) ){
            sqlite3ErrorMsg(pParse, "misuse of aliased aggregate %s", zAs);
            ပြန်ပေး WRC_Abort;
          }
          အကယ်၍( ExprHasProperty(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=pTopNC )
          ){
            sqlite3ErrorMsg(pParse, "misuse of aliased window function %s",zAs);
            ပြန်ပေး WRC_Abort;
          }
          အကယ်၍( sqlite3ExprVectorSize(pOrig)!=1 ){
            sqlite3ErrorMsg(pParse, "row value misused");
            ပြန်ပေး WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, pExpr, nSubquery);
          cnt = 1;
          pMatch = 0;
          assert( zTab==0 && zDb==0 );
          အကယ်၍( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (void*)pExpr);
          }
          goto lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    အကယ်၍( cnt ) bha_nga;
    pNC = pNC->pNext;
    nSubquery++;
  }နေစဉ်( pNC );


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
  အကယ်၍( cnt==0 && zTab==0 ){
    assert( pExpr->op==TK_ID );
    အကယ်၍( ExprHasProperty(pExpr,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, pTopNC)
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
      pExpr->op = TK_STRING;
      memset(&pExpr->y, 0, parimaa_na(pExpr->y));
      ပြန်ပေး WRC_Prune;
    }
    အကယ်၍( sqlite3ExprIdToTrueFalse(pExpr) ){
      ပြန်ပေး WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  assert( pFJMatch==0 || cnt>0 );
  assert( !ExprHasProperty(pExpr, EP_xIsSelect|EP_IntValue) );
  အကယ်၍( cnt!=1 ){
    const var_na *zErr;
    အကယ်၍( pFJMatch ){
      အကယ်၍( pFJMatch->nExpr==cnt-1 ){
        အကယ်၍( ExprHasProperty(pExpr,EP_Leaf) ){
          ExprClearProperty(pExpr,EP_Leaf);
        }မဟုတ်ပါက{
          sqlite3ExprDelete(db, pExpr->pLeft);
          pExpr->pLeft = 0;
          sqlite3ExprDelete(db, pExpr->pRight);
          pExpr->pRight = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, pExpr->iColumn);
        pExpr->op = TK_FUNCTION;
        pExpr->u.zToken = "coalesce";
        pExpr->x.pList = pFJMatch;
        pExpr->affExpr = SQLITE_AFF_DEFER;
        cnt = 1;
        goto lookupname_end;
      }မဟုတ်ပါက{
        sqlite3ExprListDelete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    အကယ်၍( zDb ){
      sqlite3ErrorMsg(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }မဟုတ်ပါက အကယ်၍( zTab ){
      sqlite3ErrorMsg(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }မဟုတ်ပါက အကယ်၍( cnt==0 && ExprHasProperty(pRight,EP_DblQuoted) ){
      sqlite3ErrorMsg(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }မဟုတ်ပါက{
      sqlite3ErrorMsg(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
    pParse->checkSchema = 1;
    pTopNC->nNcErr++;
    eNewExprOp = TK_NULL;
  }
  assert( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  အကယ်၍( !ExprHasProperty(pExpr,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3ExprDelete(db, pExpr->pLeft);
    pExpr->pLeft = 0;
    sqlite3ExprDelete(db, pExpr->pRight);
    pExpr->pRight = 0;
    ExprSetProperty(pExpr, EP_Leaf);
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
  အကယ်၍( pMatch ){
    အကယ်၍( pExpr->iColumn>=0 ){
      pMatch->colUsed |= sqlite3ExprColUsed(pExpr);
    }မဟုတ်ပါက{
      pMatch->fg.rowidUsed = 1;
    }
  }

  pExpr->op = eNewExprOp;
lookupname_end:
  အကယ်၍( cnt==1 ){
    assert( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    အကယ်၍( db->xAuth ){
      အကယ်၍( pFJMatch ){
        assert( pExpr->op==TK_FUNCTION );
        assert( sqlite3_stricmp(pExpr->u.zToken,"coalesce")==0 );
        assert( pExpr->x.pList==pFJMatch );
        assert( pFJMatch->nExpr>0 );
        အတွက်(i=0; i<pFJMatch->nExpr; i++){
          assert( pFJMatch->a[i].pExpr->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].pExpr,pSchema,pNC->pSrcList);
        }
      }မဟုတ်ပါက အကယ်၍( pExpr->op==TK_COLUMN || pExpr->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, pExpr, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    အတွက်(;;){
      assert( pTopNC!=0 );
      pTopNC->nRef++;
      အကယ်၍( pTopNC==pNC ) bha_nga;
      pTopNC = pTopNC->pNext;
    }
    ပြန်ပေး WRC_Prune;
  } မဟုတ်ပါက {
    ပြန်ပေး WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
Expr *sqlite3CreateColumnExpr(sqlite3 *db, SrcList *pSrc, ကိန်းပြည့် iSrc, ကိန်းပြည့် iCol){
  Expr *p = sqlite3ExprAlloc(db, TK_COLUMN, 0, 0);
  အကယ်၍( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    assert( ExprUseYTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    အကယ်၍( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }မဟုတ်ပါက{
      p->iColumn = (ynVar)iCol;
      အကယ်၍( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }မဟုတ်ပါက{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  ပြန်ပေး p;
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
static void notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   const var_na *zMsg,    /* Type of error */
   Expr *pExpr,         /* Invalidate this expression on error */
   Expr *pError         /* Associate error with this expression */
){
  const var_na *zIn = "partial index WHERE clauses";
  အကယ်၍( pNC->ncFlags & NC_IdxExpr )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  မဟုတ်ပါက အကယ်၍( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  မဟုတ်ပါက အကယ်၍( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3ErrorMsg(pParse, "%s prohibited in %s", zMsg, zIn);
  အကယ်၍( pExpr ) pExpr->op = TK_NULL;
  sqlite3RecordErrorOffsetOfExpr(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  assert( ((X)&~(NC_IsCheck|NC_PartIdx|NC_IdxExpr|NC_GenCol))==0 ); \
  အကယ်၍( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
static ကိန်းပြည့် exprProbability(Expr *p){
  dviguu_na r = -1.0;
  အကယ်၍( p->op!=TK_FLOAT ) ပြန်ပေး -1;
  assert( !ExprHasProperty(p, EP_IntValue) );
  sqlite3AtoF(p->u.zToken, &r);
  assert( r>=0.0 );
  အကယ်၍( r>1.0 ) ပြန်ပေး -1;
  ပြန်ပေး (ကိန်းပြည့်)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
static SQLITE_NOINLINE void resolveSetExprSubtypeArg(ExprList *pList){
  ကိန်းပြည့် nn, ii;
  nn = pList ? pList->nExpr : 0;
  အတွက်(ii=0; ii<nn; ii++){
    Expr *pExpr = pList->a[ii].pExpr;
    နေစဉ်( 1 /*exit-by-break*/ ){
      ExprSetProperty(pExpr, EP_SubtArg);
      အကယ်၍( pExpr->op==TK_SELECT ){
        assert( ExprUseXSelect(pExpr) );
        assert( pExpr->x.pSelect!=0 );
        resolveSetExprSubtypeArg(pExpr->x.pSelect->pEList);
        bha_nga;
      }
      အကယ်၍( pExpr->op==TK_UPLUS ){
        pExpr = pExpr->pLeft;
        assert( pExpr!=0 );
      }မဟုတ်ပါက{
        bha_nga;
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
static ကိန်းပြည့် resolveExprStep(Walker *pWalker, Expr *pExpr){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  assert( pNC!=0 );
  pParse = pNC->pParse;
  assert( pParse==pWalker->pParse );

#ifndef NDEBUG
  အကယ်၍( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    ကိန်းပြည့် i;
    အတွက်(i=0; i<pNC->pSrcList->nSrc; i++){
      assert( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  switch( pExpr->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    pak_sa TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      assert( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      pExpr->op = TK_COLUMN;
      assert( ExprUseYTab(pExpr) );
      pExpr->y.pTab = pItem->pSTab;
      pExpr->iTable = pItem->iCursor;
      pExpr->iColumn--;
      pExpr->affExpr = SQLITE_AFF_INTEGER;
      bha_nga;
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
    pak_sa TK_NOTNULL:
    pak_sa TK_ISNULL: {
      ကိန်းပြည့် anRef[8];
      NameContext *p;
      ကိန်းပြည့် i;
      အတွက်(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        anRef[i] = p->nRef;
      }
      sqlite3WalkExpr(pWalker, pExpr->pLeft);
      အကယ်၍( IN_RENAME_OBJECT ) ပြန်ပေး WRC_Prune;
      အကယ်၍( sqlite3ExprCanBeNull(pExpr->pLeft) ){
        /* The expression can be NULL.  So the optimization does not apply */
        ပြန်ပေး WRC_Prune;
      }

      အတွက်(i=0, p=pNC; p; p=p->pNext, i++){
        အကယ်၍( (p->ncFlags & NC_Where)==0 ){
          ပြန်ပေး WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( ExprHasProperty(pExpr, EP_OuterON) );
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
#အကယ်၍ TREETRACE_ENABLED
      အကယ်၍( sqlite3TreeTrace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           pExpr->op==TK_NOTNULL
        );
        sqlite3ShowExpr(pExpr);
      }
#endif /* TREETRACE_ENABLED */
      pExpr->u.iValue = (pExpr->op==TK_NOTNULL);
      pExpr->flags |= EP_IntValue;
      pExpr->op = TK_INTEGER;
      အတွက်(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        p->nRef = anRef[i];
      }
      sqlite3ExprDelete(pParse->db, pExpr->pLeft);
      pExpr->pLeft = 0;
      ပြန်ပေး WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    pak_sa TK_ID:
    pak_sa TK_DOT: {
      const var_na *zTable;
      const var_na *zDb;
      Expr *pRight;

      အကယ်၍( pExpr->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        assert( !ExprHasProperty(pExpr, EP_IntValue) );
        pRight = pExpr;
      }မဟုတ်ပါက{
        Expr *pLeft = pExpr->pLeft;
        testcase( pNC->ncFlags & NC_IdxExpr );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_IdxExpr|NC_GenCol, 0, pExpr);
        pRight = pExpr->pRight;
        အကယ်၍( pRight->op==TK_ID ){
          zDb = 0;
        }မဟုတ်ပါက{
          assert( pRight->op==TK_DOT );
          assert( !ExprHasProperty(pRight, EP_IntValue) );
          zDb = pLeft->u.zToken;
          pLeft = pRight->pLeft;
          pRight = pRight->pRight;
        }
        assert( ExprUseUToken(pLeft) && ExprUseUToken(pRight) );
        zTable = pLeft->u.zToken;
        assert( ExprUseYTab(pExpr) );
        အကယ်၍( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (void*)pExpr, (void*)pRight);
          sqlite3RenameTokenRemap(pParse, (void*)&pExpr->y.pTab, (void*)pLeft);
        }
      }
      ပြန်ပေး lookupName(pParse, zDb, zTable, pRight, pNC, pExpr);
    }

    /* Resolve function names
    */
    pak_sa TK_FUNCTION: {
      ExprList *pList;            /* The argument list */
      ကိန်းပြည့် n;                      /* Number of arguments */
      ကိန်းပြည့် no_such_func = 0;       /* True if no such function exists */
      ကိန်းပြည့် wrong_num_args = 0;     /* True if wrong number of arguments */
      ကိန်းပြည့် is_agg = 0;             /* True if is an aggregate function */
      const var_na *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      ကိန်းပြည့် savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(pExpr) ? pExpr->y.pWin : 0);
#endif
      assert( !ExprHasProperty(pExpr, EP_xIsSelect|EP_IntValue) );
      assert( pExpr->pLeft==0 || pExpr->pLeft->op==TK_ORDER );
      pList = pExpr->x.pList;
      n = pList ? pList->nExpr : 0;
      zId = pExpr->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      အကယ်၍( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        အကယ်၍( pDef==0 ){
          no_such_func = 1;
        }မဟုတ်ပါက{
          wrong_num_args = 1;
        }
      }မဟုတ်ပါက{
        is_agg = pDef->xFinalize!=0;
        အကယ်၍( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          ExprSetProperty(pExpr, EP_Unlikely);
          အကယ်၍( n==2 ){
            pExpr->iTable = exprProbability(pList->a[1].pExpr);
            အကယ်၍( pExpr->iTable<0 ){
              sqlite3ErrorMsg(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", pExpr);
              pNC->nNcErr++;
            }
          }မဟုတ်ပါက{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            pExpr->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          ကိန်းပြည့် auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          အကယ်၍( auth!=SQLITE_OK ){
            အကယ်၍( auth==SQLITE_DENY ){
              sqlite3ErrorMsg(pParse, "not authorized to use function: %#T",
                                      pExpr);
              pNC->nNcErr++;
            }
            pExpr->op = TK_NULL;
            ပြန်ပေး WRC_Prune;
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
        အကယ်၍( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || ExprHasProperty(pExpr, EP_SubtArg) 
        ){
          resolveSetExprSubtypeArg(pList);
        }

        အကယ်၍( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          ExprSetProperty(pExpr,EP_ConstFunc);
        }
        အကယ်၍( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_IdxExpr|NC_PartIdx|NC_GenCol, 0, pExpr);
        }မဟုတ်ပါက{
          assert( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          pExpr->op2 = pNC->ncFlags & NC_SelfRef;
        }
        အကယ်၍( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
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
        }မဟုတ်ပါက
        အကယ်၍( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          အကယ်၍( pNC->ncFlags & NC_FromDDL ) ExprSetProperty(pExpr, EP_FromDDL);
          sqlite3ExprFunctionUsable(pParse, pExpr, pDef);
        }
      }

      အကယ်၍( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        assert( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->xValue==0 && pDef->xInverse==0)
          || (pDef->xValue && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        အကယ်၍( pDef && pDef->xValue==0 && pWin ){
          sqlite3ErrorMsg(pParse,
              "%#T() may not be used as a window function", pExpr
          );
          pNC->nNcErr++;
        }မဟုတ်ပါက အကယ်၍(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          const var_na *zType;
          အကယ်၍( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }မဟုတ်ပါက{
            zType = "aggregate";
          }
          sqlite3ErrorMsg(pParse, "misuse of %s function %#T()",zType,pExpr);
          pNC->nNcErr++;
          is_agg = 0;
        }
#မဟုတ်ပါက
        အကယ်၍( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3ErrorMsg(pParse,"misuse of aggregate function %#T()",pExpr);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        မဟုတ်ပါက အကယ်၍( no_such_func
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
          sqlite3ErrorMsg(pParse, "no such function: %#T", pExpr);
          pNC->nNcErr++;
        }မဟုတ်ပါက အကယ်၍( wrong_num_args ){
          sqlite3ErrorMsg(pParse,"wrong number of arguments to function %#T()",
               pExpr);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        မဟုတ်ပါက အကယ်၍( is_agg==0 && ExprHasProperty(pExpr, EP_WinFunc) ){
          sqlite3ErrorMsg(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              pExpr
          );
          pNC->nNcErr++;
        }
#endif
        မဟုတ်ပါက အကယ်၍( is_agg==0 && pExpr->pLeft ){
          sqlite3ExprOrderByAggregateError(pParse, pExpr);
          pNC->nNcErr++;
        }
        အကယ်၍( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#မဟုတ်ပါက
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      မဟုတ်ပါက အကယ်၍( ExprHasProperty(pExpr, EP_WinFunc) || pExpr->pLeft ){
        is_agg = 1;
      }
      sqlite3WalkExprList(pWalker, pList);
      အကယ်၍( is_agg ){
        အကယ်၍( pExpr->pLeft ){
          assert( pExpr->pLeft->op==TK_ORDER );
          assert( ExprUseXList(pExpr->pLeft) );
          sqlite3WalkExprList(pWalker, pExpr->pLeft->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        အကယ်၍( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          assert( ExprUseYWin(pExpr) && pWin==pExpr->y.pWin );
          အကယ်၍( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            အကယ်၍( pParse->db->mallocFailed ) bha_nga;
          }
          sqlite3WalkExprList(pWalker, pWin->pPartition);
          sqlite3WalkExprList(pWalker, pWin->pOrderBy);
          sqlite3WalkExpr(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }မဟုတ်ပါက
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          pExpr->op = TK_AGG_FUNCTION;
          pExpr->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          အကယ်၍( ExprHasProperty(pExpr, EP_WinFunc) ){
            sqlite3WalkExpr(pWalker, pExpr->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          နေစဉ်( pNC2
              && sqlite3ReferencesSrcList(pParse, pExpr, pNC2->pSrcList)==0
          ){
            pExpr->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }
          assert( pDef!=0 || IN_RENAME_OBJECT );
          အကယ်၍( pNC2 && pDef ){
            pExpr->op2 += pNC2->nNestedSelect;
            assert( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            assert( SQLITE_FUNC_ANYORDER==NC_OrderAgg );
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
      ပြန်ပေး WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    pak_sa TK_EXISTS:
    pak_sa TK_SELECT:
#endif
    pak_sa TK_IN: {
      testcase( pExpr->op==TK_IN );
      testcase( pExpr->op==TK_EXISTS );
      testcase( pExpr->op==TK_SELECT );
      အကယ်၍( ExprUseXSelect(pExpr) ){
        ကိန်းပြည့် nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_IdxExpr );
        testcase( pNC->ncFlags & NC_GenCol );
        assert( pExpr->x.pSelect );
        အကယ်၍( pExpr->op==TK_EXISTS )  pParse->bHasExists = 1;
        အကယ်၍( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", pExpr, pExpr);
        }မဟုတ်ပါက{
          sqlite3WalkSelect(pWalker, pExpr->x.pSelect);
        }
        assert( pNC->nRef>=nRef );
        အကယ်၍( nRef!=pNC->nRef ){
          ExprSetProperty(pExpr, EP_VarSelect);
          pExpr->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      bha_nga;
    }
    pak_sa TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_IdxExpr );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_IdxExpr|NC_GenCol, pExpr, pExpr);
      bha_nga;
    }
    pak_sa TK_IS:
    pak_sa TK_ISNOT: {
      Expr *pRight = sqlite3ExprSkipCollateAndLikely(pExpr->pRight);
      assert( !ExprHasProperty(pExpr, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      အကယ်၍( ALWAYS(pRight) && (pRight->op==TK_ID || pRight->op==TK_TRUEFALSE) ){
        ကိန်းပြည့် rc = resolveExprStep(pWalker, pRight);
        အကယ်၍( rc==WRC_Abort ) ပြန်ပေး WRC_Abort;
        အကယ်၍( pRight->op==TK_TRUEFALSE ){
          pExpr->op2 = pExpr->op;
          pExpr->op = TK_TRUTH;
          ပြန်ပေး WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    pak_sa TK_BETWEEN:
    pak_sa TK_EQ:
    pak_sa TK_NE:
    pak_sa TK_LT:
    pak_sa TK_LE:
    pak_sa TK_GT:
    pak_sa TK_GE: {
      ကိန်းပြည့် nLeft, nRight;
      အကယ်၍( pParse->db->mallocFailed ) bha_nga;
      assert( pExpr->pLeft!=0 );
      nLeft = sqlite3ExprVectorSize(pExpr->pLeft);
      အကယ်၍( pExpr->op==TK_BETWEEN ){
        assert( ExprUseXList(pExpr) );
        nRight = sqlite3ExprVectorSize(pExpr->x.pList->a[0].pExpr);
        အကယ်၍( nRight==nLeft ){
          nRight = sqlite3ExprVectorSize(pExpr->x.pList->a[1].pExpr);
        }
      }မဟုတ်ပါက{
        assert( pExpr->pRight!=0 );
        nRight = sqlite3ExprVectorSize(pExpr->pRight);
      }
      အကယ်၍( nLeft!=nRight ){
        testcase( pExpr->op==TK_EQ );
        testcase( pExpr->op==TK_NE );
        testcase( pExpr->op==TK_LT );
        testcase( pExpr->op==TK_LE );
        testcase( pExpr->op==TK_GT );
        testcase( pExpr->op==TK_GE );
        testcase( pExpr->op==TK_IS );
        testcase( pExpr->op==TK_ISNOT );
        testcase( pExpr->op==TK_BETWEEN );
        sqlite3ErrorMsg(pParse, "row value misused");
        sqlite3RecordErrorOffsetOfExpr(pParse->db, pExpr);
      }
      bha_nga;
    }
  }
  assert( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ပြန်ပေး pParse->nErr ? WRC_Abort : WRC_Continue;
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
static ကိန်းပြည့် resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  ExprList *pEList,  /* List of expressions to scan */
  Expr *pE           /* Expression we are trying to match */
){
  ကိန်းပြည့် i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  အကယ်၍( pE->op==TK_ID ){
    const var_na *zCol;
    assert( !ExprHasProperty(pE, EP_IntValue) );
    zCol = pE->u.zToken;
    အတွက်(i=0; i<pEList->nExpr; i++){
      အကယ်၍( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        ပြန်ပေး i+1;
      }
    }
  }
  ပြန်ပေး 0;
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
static ကိန်းပြည့် resolveOrderByTermToExprList(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  Expr *pE           /* The specific ORDER BY term */
){
  ကိန်းပြည့် i;             /* Loop counter */
  ExprList *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  ကိန်းပြည့် rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  assert( sqlite3ExprIsInteger(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, parimaa_na(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3ResolveExprNames(&nc, pE);
  db->suppressErr = savedSuppErr;
  အကယ်၍( rc ) ပြန်ပေး 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  အတွက်(i=0; i<pEList->nExpr; i++){
    အကယ်၍( sqlite3ExprCompare(0, pEList->a[i].pExpr, pE, -1)<2 ){
      ပြန်ပေး i+1;
    }
  }

  /* If no match, return 0. */
  ပြန်ပေး 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
static void resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  const var_na *zType,     /* "ORDER" or "GROUP" */
  ကိန်းပြည့် i,                 /* The index (1-based) of the term out of range */
  ကိန်းပြည့် mx,                /* Largest permissible value of i */
  Expr *pError           /* Associate the error with the expression */
){
  sqlite3ErrorMsg(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3RecordErrorOffsetOfExpr(pParse->db, pError);
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
static ကိန်းပြည့် resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  ကိန်းပြည့် i;
  ExprList *pOrderBy;
  ExprList *pEList;
  sqlite3 *db;
  ကိန်းပြည့် moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  အကယ်၍( pOrderBy==0 ) ပြန်ပေး 0;
  db = pParse->db;
  အကယ်၍( pOrderBy->nExpr>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3ErrorMsg(pParse, "too many terms in ORDER BY clause");
    ပြန်ပေး 1;
  }
  အတွက်(i=0; i<pOrderBy->nExpr; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  နေစဉ်( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  နေစဉ်( pSelect && moreToDo ){
    sa_nracanaa ExprList_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    assert( pEList!=0 );
    အတွက်(i=0, pItem=pOrderBy->a; i<pOrderBy->nExpr; i++, pItem++){
      ကိန်းပြည့် iCol = -1;
      Expr *pE, *pDup;
      အကယ်၍( pItem->fg.done ) continue;
      pE = sqlite3ExprSkipCollateAndLikely(pItem->pExpr);
      အကယ်၍( NEVER(pE==0) ) continue;
      အကယ်၍( sqlite3ExprIsInteger(pE, &iCol, 0, 1) ){
        အကယ်၍( iCol<=0 || iCol>pEList->nExpr ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->nExpr, pE);
          ပြန်ပေး 1;
        }
      }မဟုတ်ပါက{
        iCol = resolveAsName(pParse, pEList, pE);
        အကယ်၍( iCol==0 ){
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
          pDup = sqlite3ExprDup(db, pE, 0);
          အကယ်၍( !db->mallocFailed ){
            assert(pDup);
            iCol = resolveOrderByTermToExprList(pParse, pSelect, pDup);
            အကယ်၍( IN_RENAME_OBJECT && iCol>0 ){
              resolveOrderByTermToExprList(pParse, pSelect, pE);
            }
          }
          sqlite3ExprDelete(db, pDup);
        }
      }
      အကယ်၍( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        အကယ်၍( !IN_RENAME_OBJECT ){
          Expr *pNew = sqlite3ExprInt32(db, iCol);
          အကယ်၍( pNew==0 ) ပြန်ပေး 1;
          အကယ်၍( pItem->pExpr==pE ){
            pItem->pExpr = pNew;
          }မဟုတ်ပါက{
            Expr *pParent = pItem->pExpr;
            assert( pParent->op==TK_COLLATE );
            နေစဉ်( pParent->pLeft->op==TK_COLLATE ) pParent = pParent->pLeft;
            assert( pParent->pLeft==pE );
            pParent->pLeft = pNew;
          }
          sqlite3ExprDelete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }မဟုတ်ပါက{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  အတွက်(i=0; i<pOrderBy->nExpr; i++){
    အကယ်၍( pOrderBy->a[i].fg.done==0 ){
      sqlite3ErrorMsg(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      ပြန်ပေး 1;
    }
  }
  ပြန်ပေး 0;
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
ကိန်းပြည့် sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  ExprList *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  const var_na *zType     /* "ORDER" or "GROUP" */
){
  ကိန်းပြည့် i;
  sqlite3 *db = pParse->db;
  ExprList *pEList;
  sa_nracanaa ExprList_item *pItem;

  အကယ်၍( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) ပြန်ပေး 0;
  အကယ်၍( pOrderBy->nExpr>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3ErrorMsg(pParse, "too many terms in %s BY clause", zType);
    ပြန်ပေး 1;
  }
  pEList = pSelect->pEList;
  assert( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  အတွက်(i=0, pItem=pOrderBy->a; i<pOrderBy->nExpr; i++, pItem++){
    အကယ်၍( pItem->u.x.iOrderByCol ){
      အကယ်၍( pItem->u.x.iOrderByCol>pEList->nExpr ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->nExpr, 0);
        ပြန်ပေး 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->pExpr,0);
    }
  }
  ပြန်ပေး 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
static ကိန်းပြည့် resolveRemoveWindowsCb(Walker *pWalker, Expr *pExpr){
  UNUSED_PARAMETER(pWalker);
  အကယ်၍( ExprHasProperty(pExpr, EP_WinFunc) ){
    Window *pWin = pExpr->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  ပြန်ပေး WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
static void windowRemoveExprFromSelect(Select *pSelect, Expr *pExpr){
  အကယ်၍( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, parimaa_na(Walker));
    sWalker.xExprCallback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3WalkExpr(&sWalker, pExpr);
  }
}
#မဟုတ်ပါက
# define windowRemoveExprFromSelect(a, b)
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
static ကိန်းပြည့် resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  ExprList *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  const var_na *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  ကိန်းပြည့် i, j;                      /* Loop counters */
  ကိန်းပြည့် iCol;                      /* Column number */
  sa_nracanaa ExprList_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  ကိန်းပြည့် nResult;                   /* Number of terms in the result set */

  assert( pOrderBy!=0 );
  nResult = pSelect->pEList->nExpr;
  pParse = pNC->pParse;
  အတွက်(i=0, pItem=pOrderBy->a; i<pOrderBy->nExpr; i++, pItem++){
    Expr *pE = pItem->pExpr;
    Expr *pE2 = sqlite3ExprSkipCollateAndLikely(pE);
    အကယ်၍( NEVER(pE2==0) ) continue;
    အကယ်၍( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      အကယ်၍( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        continue;
      }
    }
    အကယ်၍( sqlite3ExprIsInteger(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      အကယ်၍( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult, pE2);
        ပြန်ပေး 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      continue;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    အကယ်၍( sqlite3ResolveExprNames(pNC, pE) ){
      ပြန်ပေး 1;
    }
    အတွက်(j=0; j<pSelect->pEList->nExpr; j++){
      အကယ်၍( sqlite3ExprCompare(0, pE, pSelect->pEList->a[j].pExpr, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        windowRemoveExprFromSelect(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  ပြန်ပေး sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
static ကိန်းပြည့် resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  ကိန်းပြည့် isCompound;         /* True if p is a compound select */
  ကိန်းပြည့် nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  ကိန်းပြည့် i;                  /* Loop counter */
  ExprList *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  assert( p!=0 );
  အကယ်၍( p->selFlags & SF_Resolved ){
    ပြန်ပေး WRC_Prune;
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
  အကယ်၍( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    ပြန်ပေး pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  နေစဉ်( p ){
    assert( (p->selFlags & SF_Expanded)!=0 );
    assert( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, parimaa_na(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    အကယ်၍( sqlite3ResolveExprNames(&sNC, p->pLimit) ){
      ပြန်ပေး WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    အကယ်၍( p->selFlags & SF_Converted ){
      Select *pSub;
      assert( p->pSrc->a[0].fg.isSubquery );
      assert( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      assert( pSub!=0 );
      assert( p->pSrc->nSrc==1 && p->pOrderBy );
      assert( pSub->pPrior && pSub->pOrderBy==0 );
      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    အကယ်၍( pOuterNC ) pOuterNC->nNestedSelect++;
    အတွက်(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      assert( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      အကယ်၍( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        ကိန်းပြည့် nRef = pOuterNC ? pOuterNC->nRef : 0;
        const var_na *zSavedContext = pParse->zAuthContext;

        အကယ်၍( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        အကယ်၍( pParse->nErr ) ပြန်ပေး WRC_Abort;
        assert( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        အကယ်၍( pOuterNC ){
          assert( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    အကယ်၍( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
 
    /* Resolve names in the result set. */
    အကယ်၍( sqlite3ResolveExprListNames(&sNC, p->pEList) ) ပြန်ပေး WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    assert( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    အကယ်၍( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      assert( NC_MinMaxAgg==SF_MinMaxAgg );
      assert( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }မဟုတ်ပါက{
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
    assert( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    အကယ်၍( p->pHaving ){
      အကယ်၍( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3ErrorMsg(pParse, "HAVING clause on a non-aggregate query");
        ပြန်ပေး WRC_Abort;
      }
      အကယ်၍( sqlite3ResolveExprNames(&sNC, p->pHaving) ) ပြန်ပေး WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    အကယ်၍( sqlite3ResolveExprNames(&sNC, p->pWhere) ) ပြန်ပေး WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    အတွက်(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      အကယ်၍( pItem->fg.isTabFunc
       && sqlite3ResolveExprListNames(&sNC, pItem->u1.pFuncArg)
      ){
        ပြန်ပေး WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    အကယ်၍( IN_RENAME_OBJECT ){
      Window *pWin;
      အတွက်(pWin=p->pWinDefn; pWin; pWin=pWin->pNextWin){
        အကယ်၍( sqlite3ResolveExprListNames(&sNC, pWin->pOrderBy)
         || sqlite3ResolveExprListNames(&sNC, pWin->pPartition)
        ){
          ပြန်ပေး WRC_Abort;
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
    အကယ်၍( p->selFlags & SF_Converted ){
      Select *pSub;
      assert( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      assert( pSub!=0 );
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
    အကယ်၍( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      ပြန်ပေး WRC_Abort;
    }
    အကယ်၍( db->mallocFailed ){
      ပြန်ပေး WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    အကယ်၍( pGroupBy ){
      sa_nracanaa ExprList_item *pItem;
   
      အကယ်၍( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        ပြန်ပေး WRC_Abort;
      }
      အတွက်(i=0, pItem=pGroupBy->a; i<pGroupBy->nExpr; i++, pItem++){
        အကယ်၍( ExprHasProperty(pItem->pExpr, EP_Agg) ){
          sqlite3ErrorMsg(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          ပြန်ပေး WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    အကယ်၍( p->pNext && p->pEList->nExpr!=p->pNext->pEList->nExpr ){
      sqlite3SelectWrongNumTermsError(pParse, p->pNext);
      ပြန်ပေး WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    အကယ်၍( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      အကယ်၍( pParse->nErr ) ပြန်ပေး WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  အကယ်၍( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    ပြန်ပေး WRC_Abort;
  }

  ပြန်ပေး WRC_Prune;
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
ကိန်းပြည့် sqlite3ResolveExprNames(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  Expr *pExpr             /* The expression to be analyzed. */
){
  ကိန်းပြည့် savedHasAgg;
  Walker w;

  အကယ်၍( pExpr==0 ) ပြန်ပေး SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.xExprCallback = resolveExprStep;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#အကယ်၍ SQLITE_MAX_EXPR_DEPTH>0
  w.pParse->nHeight += pExpr->nHeight;
  အကယ်၍( sqlite3ExprCheckHeight(w.pParse, w.pParse->nHeight) ){
    ပြန်ပေး SQLITE_ERROR;
  }
#endif
  assert( pExpr!=0 );
  sqlite3WalkExprNN(&w, pExpr);
#အကယ်၍ SQLITE_MAX_EXPR_DEPTH>0
  w.pParse->nHeight -= pExpr->nHeight;
#endif
  assert( EP_Agg==NC_HasAgg );
  assert( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  ExprSetProperty(pExpr, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  ပြန်ပေး pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
ကိန်းပြည့် sqlite3ResolveExprListNames(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  ExprList *pList         /* The expression list to be analyzed. */
){
  ကိန်းပြည့် i;
  ကိန်းပြည့် savedHasAgg = 0;
  Walker w;
  အကယ်၍( pList==0 ) ပြန်ပေး SQLITE_OK;
  w.pParse = pNC->pParse;
  w.xExprCallback = resolveExprStep;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  အတွက်(i=0; i<pList->nExpr; i++){
    Expr *pExpr = pList->a[i].pExpr;
    အကယ်၍( pExpr==0 ) continue;
#အကယ်၍ SQLITE_MAX_EXPR_DEPTH>0
    w.pParse->nHeight += pExpr->nHeight;
    အကယ်၍( sqlite3ExprCheckHeight(w.pParse, w.pParse->nHeight) ){
      ပြန်ပေး SQLITE_ERROR;
    }
#endif
    sqlite3WalkExprNN(&w, pExpr);
#အကယ်၍ SQLITE_MAX_EXPR_DEPTH>0
    w.pParse->nHeight -= pExpr->nHeight;
#endif
    assert( EP_Agg==NC_HasAgg );
    assert( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    အကယ်၍( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      ExprSetProperty(pExpr, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    အကယ်၍( w.pParse->nErr>0 ) ပြန်ပေး SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  ပြန်ပေး SQLITE_OK;
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
void sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  assert( p!=0 );
  w.xExprCallback = resolveExprStep;
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
ကိန်းပြည့် sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  ကိန်းပြည့် type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  Expr *pExpr,     /* Expression to resolve.  May be NULL. */
  ExprList *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  ကိန်းပြည့် rc;
  sa_ngha {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  assert( type==0 || pTab!=0 );
  assert( type==NC_IsCheck || type==NC_PartIdx || type==NC_IdxExpr
          || type==NC_GenCol || pTab==0 );
  memset(&sNC, 0, parimaa_na(sNC));
  memset(&uSrc, 0, parimaa_na(uSrc));
  pSrc = &uSrc.sSrc;
  အကယ်၍( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    အကယ်၍( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  အကယ်၍( (rc = sqlite3ResolveExprNames(&sNC, pExpr))!=SQLITE_OK ) ပြန်ပေး rc;
  အကယ်၍( pList ) rc = sqlite3ResolveExprListNames(&sNC, pList);
  ပြန်ပေး rc;
}
