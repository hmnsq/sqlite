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
#define EXCLUDED_TABLE_नुमेरो  2

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
static int incrAggDepth(Walker *pWalker, पाहायाग *p_पाहायाग){
  if( p_पाहायाग->op==TK_AGG_FUNCTION ) p_पाहायाग->op2 += pWalker->u.n;
  return WRC_Continue;
}
static void incrAggFunctionDepth(पाहायाग *p_पाहायाग, int N){
  if( N>0 ){
    Walker w;
    memset(&w, 0, sizeof(w));
    w.x_पाहायाग_Callback = incrAggDepth;
    w.u.n = N;
    sqlite3_Walk_पाहायाग(&w, p_पाहायाग);
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
  पाहायाग_List *pEList,      /* A result set */
  int iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  पाहायाग *p_पाहायाग,           /* Transform this into an alias to the result set */
  int nSubquery          /* Number of subqueries that the label is moving */
){
  पाहायाग *pOrig;           /* The iCol-th column of the result set */
  पाहायाग *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  assert( iCol>=0 && iCol<pEList->n_पाहायाग );
  pOrig = pEList->a[iCol].p_पाहायाग;
  assert( pOrig!=0 );
  assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Reduced|EP_TokenOnly) );
  if( p_पाहायाग->pAggInfo ) return;
  db = pParse->db;
  pDup = sqlite3_पाहायाग_Dup(db, pOrig, 0);
  if( db->mallocFailed ){
    sqlite3_पाहायाग_Delete(db, pDup);
    pDup = 0;
  }else{
    पाहायाग temp;
    incrAggFunctionDepth(pDup, nSubquery);
    if( p_पाहायाग->op==TK_COLLATE ){
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      pDup = sqlite3_पाहायाग_Add_Collate_String(pParse, pDup, p_पाहायाग->u.zToken);
    }
    memcpy(&temp, pDup, sizeof(पाहायाग));
    memcpy(pDup, p_पाहायाग, sizeof(पाहायाग));
    memcpy(p_पाहायाग, &temp, sizeof(पाहायाग));
    if( पाहायाग_Has_Property(p_पाहायाग, EP_WinFunc) ){
      if( ALWAYS(p_पाहायाग->y.pWin!=0) ){
        p_पाहायाग->y.pWin->pOwner = p_पाहायाग;
      }
    }
    sqlite3_पाहायाग_Deferred_Delete(pParse, pDup);
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
int sqlite3MatchEName(
  const struct पाहायाग_List_item *pItem,
  const char *zCol,
  const char *zTab,
  const char *zDb,
  int *pbRowid
){
  int n;
  const char *zSpan;
  int eEName = pItem->fg.eEName;
  if( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    return 0;
  }
  assert( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  for(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  if( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    return 0;
  }
  zSpan += n+1;
  for(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  if( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    return 0;
  }
  zSpan += n+1;
  if( zCol ){
    if( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) return 0;
    if( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) return 0;
  }
  if( eEName==ENAME_ROWID ) *pbRowid = 1;
  return 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
static int areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *p_इताअस_NC){
  if( db->init.busy ) return 1;  /* Always support for legacy schemas */
  if( p_इताअस_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    if( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      return 1;
    }
    return (db->flags & SQLITE_DqsDDL)!=0;
  }else{
    /* Currently parsing a DML statement */
    return (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_पाहायाग_Col_Used(पाहायाग *p_पाहायाग){
  int n;
  Table *pExTab;

  n = p_पाहायाग->iColumn;
  assert( पाहायाग_Use_YTab(p_पाहायाग) );
  pExTab = p_पाहायाग->y.pTab;
  assert( pExTab!=0 );
  assert( n < pExTab->nCol );
  if( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    return pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }else{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    if( n>=BMS ) n = BMS-1;
    return ((Bitmask)1)<<n;
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
  पाहायाग_List **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  पाहायाग *pNew = sqlite3_पाहायाग_Alloc(pParse->db, TK_COLUMN, 0, 0);
  if( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    assert( (pMatch->fg.jointype & (JT_कालिवा|JT_LTORJ))!=0 );
    पाहायाग_Set_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_पाहायाग_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
static SQLITE_NOINLINE int isValidSchemaTableName(
  const char *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  const char *zDb           /* non-NULL if a database qualifier is present */
){
  const char *zLegacy;
  assert( pTab!=0 );
  assert( pTab->tnum==1 );
  if( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) return 0;
  zLegacy = pTab->zName;
  if( strcmp(zLegacy+7, &LEGACY_TEMP_SCHEMA_TABLE[7])==0 ){
    if( sqlite3StrICmp(zTab+7, &PREFERRED_TEMP_SCHEMA_TABLE[7])==0 ){
      return 1;
    }
    if( zDb==0 ) return 0;
    if( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) return 1;
    if( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) return 1;
  }else{
    if( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) return 1;
  }
  return 0;
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
static int lookupName(
  Parse *pParse,       /* The parsing context */
  const char *zDb,     /* Name of the database containing table, or NULL */
  const char *zTab,    /* Name of table containing column, or NULL */
  const पाहायाग *p_कानान,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  पाहायाग *p_पाहायाग          /* Make this EXPR node point to the selected column */
){
  int i, j;                         /* Loop counters */
  int cnt = 0;                      /* Number of matching column names */
  int cntTab = 0;                   /* Number of potential "rowid" matches */
  int nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *p_इताअस_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  int e_New_पाहायाग_Op = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  पाहायाग_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  const char *zCol = p_कानान->u.zToken;

  assert( pNC );     /* the name context cannot be NULL. */
  assert( zCol );    /* The Z in X.Y.Z cannot be NULL */
  assert( zDb==0 || zTab!=0 );
  assert( !पाहायाग_Has_Property(p_पाहायाग, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_पाहायाग->iTable = -1;
  पाहायाग_Set_VVAProperty(p_पाहायाग, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  if( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    if( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }else{
      for(i=0; i<db->nDb; i++){
        assert( db->aDb[i].zDbSName );
        if( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          break;
        }
      }
      if( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
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
    पाहायाग_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    if( pSrcList ){
      for(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        assert( pTab!=0 && pTab->zName!=0 );
        assert( pTab->nCol>0 || pParse->nErr );
        assert( (int)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        if( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          int hit = 0;
          Select *pSel;
          assert( pItem->fg.isSubquery );
          assert( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          assert( pSel!=0 );
          pEList = pSel->pEList;
          assert( pEList!=0 );
          assert( pEList->n_पाहायाग==pTab->nCol );
          for(j=0; j<pEList->n_पाहायाग; j++){
            int bRowid = 0;       /* True if possible rowid match */
            if( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              continue;
            }
            if( bRowid==0 ){
              if( cnt>0 ){
                if( pItem->fg.isUsing==0
                 || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_पाहायाग_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }else
                if( (pItem->fg.jointype & JT_कानान)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  continue;
                }else
                if( (pItem->fg.jointype & JT_कालिवा)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_पाहायाग_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }else{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, p_पाहायाग->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }else if( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              continue;
            }
            cntTab++;
            pMatch = pItem;
            p_पाहायाग->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            assert( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            if( pEList->a[j].fg.bUsingTerm ) break;
          }
          if( hit || zTab==0 ) continue;
        }
        assert( zDb==0 || zTab!=0 );
        if( zTab ){
          if( zDb ){
            if( pTab->pSchema!=pSchema ) continue;
            if( pSchema==0 && strcmp(zDb,"*")!=0 ) continue;
          }
          if( pItem->zAlias!=0 ){
            if( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              continue;
            }
          }else if( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            if( pTab->tnum!=1 ) continue;
            if( !isValidSchemaTableName(zTab, pTab, zDb) ) continue;
          }
          assert( पाहायाग_Use_YTab(p_पाहायाग) );
          if( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (void*)&p_पाहायाग->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        if( j>=0 ){
          if( cnt>0 ){
            if( pItem->fg.isUsing==0
             || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_पाहायाग_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }else
            if( (pItem->fg.jointype & JT_कानान)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              continue;
            }else
            if( (pItem->fg.jointype & JT_कालिवा)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_पाहायाग_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }else{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, p_पाहायाग->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_पाहायाग->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          if( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        if( 0==cnt && VisibleRowid(pTab) ){
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
          if( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }else{
            cntTab++;
          }
#else
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      if( pMatch ){
        p_पाहायाग->iTable = pMatch->iCursor;
        assert( पाहायाग_Use_YTab(p_पाहायाग) );
        p_पाहायाग->y.pTab = pMatch->pSTab;
        if( (pMatch->fg.jointype & (JT_कालिवा|JT_LTORJ))!=0 ){
          पाहायाग_Set_Property(p_पाहायाग, EP_CanBeNull);
        }
        pSchema = p_पाहायाग->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#if !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    if( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      if( pParse->pTriggerTab!=0 ){
        int op = pParse->eTriggerOp;
        assert( op==TK_DELETE || op==TK_UPDATE || op==TK_इसिंगित );
        if( pParse->bReturning ){
          if( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            p_पाहायाग->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }else if( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_पाहायाग->iTable = 1;
          pTab = pParse->pTriggerTab;
        }else if( op!=TK_इसिंगित && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_पाहायाग->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      if( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        if( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_पाहायाग->iTable = EXCLUDED_TABLE_नुमेरो;
          if( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (void*)&p_पाहायाग->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      if( pTab ){
        int iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        if( iCol>=0 ){
          if( pTab->iPKey==iCol ) iCol = -1;
        }else{
          if( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }else{
            iCol = pTab->nCol;
          }
        }
        if( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          if( p_पाहायाग->iTable==EXCLUDED_TABLE_नुमेरो ){
            testcase( iCol==(-1) );
            assert( पाहायाग_Use_YTab(p_पाहायाग) );
            if( IN_RENAME_OBJECT ){
              p_पाहायाग->iColumn = iCol;
              p_पाहायाग->y.pTab = pTab;
              e_New_पाहायाग_Op = TK_COLUMN;
            }else{
              p_पाहायाग->iTable = pNC->uNC.pUpsert->regData +
                 sqlite3TableColumnToStorage(pTab, iCol);
              e_New_पाहायाग_Op = TK_REGISTER;
            }
          }else
#endif /* SQLITE_OMIT_UPSERT */
          {
            assert( पाहायाग_Use_YTab(p_पाहायाग) );
            p_पाहायाग->y.pTab = pTab;
            if( pParse->bReturning ){
              e_New_पाहायाग_Op = TK_REGISTER;
              p_पाहायाग->op2 = TK_COLUMN;
              p_पाहायाग->iColumn = iCol;
              p_पाहायाग->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*p_पाहायाग->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }else{
              p_पाहायाग->iColumn = (i16)iCol;
              e_New_पाहायाग_Op = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              if( iCol<0 ){
                p_पाहायाग->aff_पाहायाग = SQLITE_AFF_INTEGER;
              }else if( p_पाहायाग->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }else{
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
    if( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_Idx_पाहायाग|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#if SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      if( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        e_New_पाहायाग_Op = TK_NULL;
      }
#endif
      if( pMatch->fg.isNestedFrom==0 ) p_पाहायाग->iColumn = -1;
      p_पाहायाग->aff_पाहायाग = SQLITE_AFF_INTEGER;
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
    if( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      assert( pEList!=0 );
      for(j=0; j<pEList->n_पाहायाग; j++){
        char *zAs = pEList->a[j].zEName;
        if( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          पाहायाग *pOrig;
          assert( p_पाहायाग->p_कालिवा==0 && p_पाहायाग->p_कानान==0 );
          assert( पाहायाग_Use_XList(p_पाहायाग)==0 || p_पाहायाग->x.pList==0 );
          assert( पाहायाग_Use_XSelect(p_पाहायाग)==0 || p_पाहायाग->x.pSelect==0 );
          pOrig = pEList->a[j].p_पाहायाग;
          if( (pNC->ncFlags&NC_AllowAgg)==0 && पाहायाग_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_मेनसाहे(pParse, "misuse of aliased aggregate %s", zAs);
            return WRC_Abort;
          }
          if( पाहायाग_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_इताअस_NC )
          ){
            sqlite3_Error_मेनसाहे(pParse, "misuse of aliased window function %s",zAs);
            return WRC_Abort;
          }
          if( sqlite3_पाहायाग_Vector_Size(pOrig)!=1 ){
            sqlite3_Error_मेनसाहे(pParse, "row value misused");
            return WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_पाहायाग, nSubquery);
          cnt = 1;
          pMatch = 0;
          assert( zTab==0 && zDb==0 );
          if( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (void*)p_पाहायाग);
          }
          goto lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    if( cnt ) break;
    pNC = pNC->pNext;
    nSubquery++;
  }while( pNC );


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
  if( cnt==0 && zTab==0 ){
    assert( p_पाहायाग->op==TK_ID );
    if( पाहायाग_Has_Property(p_पाहायाग,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, p_इताअस_NC)
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
      p_पाहायाग->op = TK_STRING;
      memset(&p_पाहायाग->y, 0, sizeof(p_पाहायाग->y));
      return WRC_Prune;
    }
    if( sqlite3_पाहायाग_Id_To_True_False(p_पाहायाग) ){
      return WRC_Prune;
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
  assert( !पाहायाग_Has_Property(p_पाहायाग, EP_xIsSelect|EP_Int_हालागा) );
  if( cnt!=1 ){
    const char *zErr;
    if( pFJMatch ){
      if( pFJMatch->n_पाहायाग==cnt-1 ){
        if( पाहायाग_Has_Property(p_पाहायाग,EP_Leaf) ){
          पाहायाग_Clear_Property(p_पाहायाग,EP_Leaf);
        }else{
          sqlite3_पाहायाग_Delete(db, p_पाहायाग->p_कालिवा);
          p_पाहायाग->p_कालिवा = 0;
          sqlite3_पाहायाग_Delete(db, p_पाहायाग->p_कानान);
          p_पाहायाग->p_कानान = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, p_पाहायाग->iColumn);
        p_पाहायाग->op = TK_FUNCTION;
        p_पाहायाग->u.zToken = "coalesce";
        p_पाहायाग->x.pList = pFJMatch;
        p_पाहायाग->aff_पाहायाग = SQLITE_AFF_DEFER;
        cnt = 1;
        goto lookupname_end;
      }else{
        sqlite3_पाहायाग_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    if( zDb ){
      sqlite3_Error_मेनसाहे(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }else if( zTab ){
      sqlite3_Error_मेनसाहे(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }else if( cnt==0 && पाहायाग_Has_Property(p_कानान,EP_DblQuoted) ){
      sqlite3_Error_मेनसाहे(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }else{
      sqlite3_Error_मेनसाहे(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3_Record_Error_Offset_Of_पाहायाग(pParse->db, p_पाहायाग);
    pParse->checkSchema = 1;
    p_इताअस_NC->nNcErr++;
    e_New_पाहायाग_Op = TK_NULL;
  }
  assert( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  if( !पाहायाग_Has_Property(p_पाहायाग,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3_पाहायाग_Delete(db, p_पाहायाग->p_कालिवा);
    p_पाहायाग->p_कालिवा = 0;
    sqlite3_पाहायाग_Delete(db, p_पाहायाग->p_कानान);
    p_पाहायाग->p_कानान = 0;
    पाहायाग_Set_Property(p_पाहायाग, EP_Leaf);
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
  if( pMatch ){
    if( p_पाहायाग->iColumn>=0 ){
      pMatch->colUsed |= sqlite3_पाहायाग_Col_Used(p_पाहायाग);
    }else{
      pMatch->fg.rowidUsed = 1;
    }
  }

  p_पाहायाग->op = e_New_पाहायाग_Op;
lookupname_end:
  if( cnt==1 ){
    assert( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    if( db->xAuth ){
      if( pFJMatch ){
        assert( p_पाहायाग->op==TK_FUNCTION );
        assert( sqlite3_stricmp(p_पाहायाग->u.zToken,"coalesce")==0 );
        assert( p_पाहायाग->x.pList==pFJMatch );
        assert( pFJMatch->n_पाहायाग>0 );
        for(i=0; i<pFJMatch->n_पाहायाग; i++){
          assert( pFJMatch->a[i].p_पाहायाग->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_पाहायाग,pSchema,pNC->pSrcList);
        }
      }else if( p_पाहायाग->op==TK_COLUMN || p_पाहायाग->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_पाहायाग, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    for(;;){
      assert( p_इताअस_NC!=0 );
      p_इताअस_NC->nRef++;
      if( p_इताअस_NC==pNC ) break;
      p_इताअस_NC = p_इताअस_NC->pNext;
    }
    return WRC_Prune;
  } else {
    return WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
पाहायाग *sqlite3_Create_Column_पाहायाग(sqlite3 *db, SrcList *pSrc, int iSrc, int iCol){
  पाहायाग *p = sqlite3_पाहायाग_Alloc(db, TK_COLUMN, 0, 0);
  if( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    assert( पाहायाग_Use_YTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    if( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }else{
      p->iColumn = (ynVar)iCol;
      if( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }else{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  return p;
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
   const char *z_मेनसाहे,    /* Type of error */
   पाहायाग *p_पाहायाग,         /* Invalidate this expression on error */
   पाहायाग *pError         /* Associate error with this expression */
){
  const char *zIn = "partial index WHERE clauses";
  if( pNC->ncFlags & NC_Idx_पाहायाग )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  else if( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  else if( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3_Error_मेनसाहे(pParse, "%s prohibited in %s", z_मेनसाहे, zIn);
  if( p_पाहायाग ) p_पाहायाग->op = TK_NULL;
  sqlite3_Record_Error_Offset_Of_पाहायाग(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  assert( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_पाहायाग|NC_GenCol))==0 ); \
  if( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
static int पाहायाग_Probability(पाहायाग *p){
  double r = -1.0;
  if( p->op!=TK_FLOAT ) return -1;
  assert( !पाहायाग_Has_Property(p, EP_Int_हालागा) );
  sqlite3AtoF(p->u.zToken, &r);
  assert( r>=0.0 );
  if( r>1.0 ) return -1;
  return (int)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
static SQLITE_NOINLINE void resolve_Set_पाहायाग_Subtype_Arg(पाहायाग_List *pList){
  int nn, ii;
  nn = pList ? pList->n_पाहायाग : 0;
  for(ii=0; ii<nn; ii++){
    पाहायाग *p_पाहायाग = pList->a[ii].p_पाहायाग;
    while( 1 /*exit-by-break*/ ){
      पाहायाग_Set_Property(p_पाहायाग, EP_SubtArg);
      if( p_पाहायाग->op==TK_SELECT ){
        assert( पाहायाग_Use_XSelect(p_पाहायाग) );
        assert( p_पाहायाग->x.pSelect!=0 );
        resolve_Set_पाहायाग_Subtype_Arg(p_पाहायाग->x.pSelect->pEList);
        break;
      }
      if( p_पाहायाग->op==TK_UPLUS ){
        p_पाहायाग = p_पाहायाग->p_कालिवा;
        assert( p_पाहायाग!=0 );
      }else{
        break;
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
static int resolve_पाहायाग_Step(Walker *pWalker, पाहायाग *p_पाहायाग){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  assert( pNC!=0 );
  pParse = pNC->pParse;
  assert( pParse==pWalker->pParse );

#ifndef NDEBUG
  if( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    int i;
    for(i=0; i<pNC->pSrcList->nSrc; i++){
      assert( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  switch( p_पाहायाग->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    case TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      assert( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      p_पाहायाग->op = TK_COLUMN;
      assert( पाहायाग_Use_YTab(p_पाहायाग) );
      p_पाहायाग->y.pTab = pItem->pSTab;
      p_पाहायाग->iTable = pItem->iCursor;
      p_पाहायाग->iColumn--;
      p_पाहायाग->aff_पाहायाग = SQLITE_AFF_INTEGER;
      break;
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
    case TK_NOTNULL:
    case TK_ISNULL: {
      int anRef[8];
      NameContext *p;
      int i;
      for(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_पाहायाग(pWalker, p_पाहायाग->p_कालिवा);
      if( IN_RENAME_OBJECT ) return WRC_Prune;
      if( sqlite3_पाहायाग_Can_Be_Null(p_पाहायाग->p_कालिवा) ){
        /* The expression can be NULL.  So the optimization does not apply */
        return WRC_Prune;
      }

      for(i=0, p=pNC; p; p=p->pNext, i++){
        if( (p->ncFlags & NC_Where)==0 ){
          return WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( पाहायाग_Has_Property(p_पाहायाग, EP_OuterON) );
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
#if TREETRACE_ENABLED
      if( sqlite3_पुनो_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_पाहायाग->op==TK_NOTNULL
        );
        sqlite3_Show_पाहायाग(p_पाहायाग);
      }
#endif /* TREETRACE_ENABLED */
      p_पाहायाग->u.i_हालागा = (p_पाहायाग->op==TK_NOTNULL);
      p_पाहायाग->flags |= EP_Int_हालागा;
      p_पाहायाग->op = TK_INTEGER;
      for(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        p->nRef = anRef[i];
      }
      sqlite3_पाहायाग_Delete(pParse->db, p_पाहायाग->p_कालिवा);
      p_पाहायाग->p_कालिवा = 0;
      return WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    case TK_ID:
    case TK_DOT: {
      const char *zTable;
      const char *zDb;
      पाहायाग *p_कानान;

      if( p_पाहायाग->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
        p_कानान = p_पाहायाग;
      }else{
        पाहायाग *p_कालिवा = p_पाहायाग->p_कालिवा;
        testcase( pNC->ncFlags & NC_Idx_पाहायाग );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_पाहायाग|NC_GenCol, 0, p_पाहायाग);
        p_कानान = p_पाहायाग->p_कानान;
        if( p_कानान->op==TK_ID ){
          zDb = 0;
        }else{
          assert( p_कानान->op==TK_DOT );
          assert( !पाहायाग_Has_Property(p_कानान, EP_Int_हालागा) );
          zDb = p_कालिवा->u.zToken;
          p_कालिवा = p_कानान->p_कालिवा;
          p_कानान = p_कानान->p_कानान;
        }
        assert( पाहायाग_Use_UToken(p_कालिवा) && पाहायाग_Use_UToken(p_कानान) );
        zTable = p_कालिवा->u.zToken;
        assert( पाहायाग_Use_YTab(p_पाहायाग) );
        if( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (void*)p_पाहायाग, (void*)p_कानान);
          sqlite3RenameTokenRemap(pParse, (void*)&p_पाहायाग->y.pTab, (void*)p_कालिवा);
        }
      }
      return lookupName(pParse, zDb, zTable, p_कानान, pNC, p_पाहायाग);
    }

    /* Resolve function names
    */
    case TK_FUNCTION: {
      पाहायाग_List *pList;            /* The argument list */
      int n;                      /* Number of arguments */
      int no_such_func = 0;       /* True if no such function exists */
      int wrong_नुमेरो_मगा_कातविरान = 0;     /* True if wrong number of arguments */
      int is_agg = 0;             /* True if is an aggregate function */
      const char *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      int savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(p_पाहायाग) ? p_पाहायाग->y.pWin : 0);
#endif
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_xIsSelect|EP_Int_हालागा) );
      assert( p_पाहायाग->p_कालिवा==0 || p_पाहायाग->p_कालिवा->op==TK_ORDER );
      pList = p_पाहायाग->x.pList;
      n = pList ? pList->n_पाहायाग : 0;
      zId = p_पाहायाग->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      if( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        if( pDef==0 ){
          no_such_func = 1;
        }else{
          wrong_नुमेरो_मगा_कातविरान = 1;
        }
      }else{
        is_agg = pDef->xFinalize!=0;
        if( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          पाहायाग_Set_Property(p_पाहायाग, EP_Unlikely);
          if( n==2 ){
            p_पाहायाग->iTable = पाहायाग_Probability(pList->a[1].p_पाहायाग);
            if( p_पाहायाग->iTable<0 ){
              sqlite3_Error_मेनसाहे(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_पाहायाग);
              pNC->nNcErr++;
            }
          }else{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_पाहायाग->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          int auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          if( auth!=SQLITE_OK ){
            if( auth==SQLITE_DENY ){
              sqlite3_Error_मेनसाहे(pParse, "not authorized to use function: %#T",
                                      p_पाहायाग);
              pNC->nNcErr++;
            }
            p_पाहायाग->op = TK_NULL;
            return WRC_Prune;
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
        if( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || पाहायाग_Has_Property(p_पाहायाग, EP_SubtArg) 
        ){
          resolve_Set_पाहायाग_Subtype_Arg(pList);
        }

        if( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          पाहायाग_Set_Property(p_पाहायाग,EP_ConstFunc);
        }
        if( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_पाहायाग|NC_PartIdx|NC_GenCol, 0, p_पाहायाग);
        }else{
          assert( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_पाहायाग->op2 = pNC->ncFlags & NC_SelfRef;
        }
        if( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
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
        }else
        if( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          if( pNC->ncFlags & NC_FromDDL ) पाहायाग_Set_Property(p_पाहायाग, EP_FromDDL);
          sqlite3_पाहायाग_Function_Usable(pParse, p_पाहायाग, pDef);
        }
      }

      if( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        assert( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_हालागा==0 && pDef->xInverse==0)
          || (pDef->x_हालागा && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        if( pDef && pDef->x_हालागा==0 && pWin ){
          sqlite3_Error_मेनसाहे(pParse,
              "%#T() may not be used as a window function", p_पाहायाग
          );
          pNC->nNcErr++;
        }else if(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          const char *zType;
          if( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }else{
            zType = "aggregate";
          }
          sqlite3_Error_मेनसाहे(pParse, "misuse of %s function %#T()",zType,p_पाहायाग);
          pNC->nNcErr++;
          is_agg = 0;
        }
#else
        if( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_मेनसाहे(pParse,"misuse of aggregate function %#T()",p_पाहायाग);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        else if( no_such_func
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
          sqlite3_Error_मेनसाहे(pParse, "no such function: %#T", p_पाहायाग);
          pNC->nNcErr++;
        }else if( wrong_नुमेरो_मगा_कातविरान ){
          sqlite3_Error_मेनसाहे(pParse,"wrong number of arguments to function %#T()",
               p_पाहायाग);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        else if( is_agg==0 && पाहायाग_Has_Property(p_पाहायाग, EP_WinFunc) ){
          sqlite3_Error_मेनसाहे(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_पाहायाग
          );
          pNC->nNcErr++;
        }
#endif
        else if( is_agg==0 && p_पाहायाग->p_कालिवा ){
          sqlite3_पाहायाग_Order_By_Aggregate_Error(pParse, p_पाहायाग);
          pNC->nNcErr++;
        }
        if( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#else
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      else if( पाहायाग_Has_Property(p_पाहायाग, EP_WinFunc) || p_पाहायाग->p_कालिवा ){
        is_agg = 1;
      }
      sqlite3_Walk_पाहायाग_List(pWalker, pList);
      if( is_agg ){
        if( p_पाहायाग->p_कालिवा ){
          assert( p_पाहायाग->p_कालिवा->op==TK_ORDER );
          assert( पाहायाग_Use_XList(p_पाहायाग->p_कालिवा) );
          sqlite3_Walk_पाहायाग_List(pWalker, p_पाहायाग->p_कालिवा->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        if( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          assert( पाहायाग_Use_YWin(p_पाहायाग) && pWin==p_पाहायाग->y.pWin );
          if( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            if( pParse->db->mallocFailed ) break;
          }
          sqlite3_Walk_पाहायाग_List(pWalker, pWin->pPartition);
          sqlite3_Walk_पाहायाग_List(pWalker, pWin->pOrderBy);
          sqlite3_Walk_पाहायाग(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }else
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_पाहायाग->op = TK_AGG_FUNCTION;
          p_पाहायाग->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          if( पाहायाग_Has_Property(p_पाहायाग, EP_WinFunc) ){
            sqlite3_Walk_पाहायाग(pWalker, p_पाहायाग->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          while( pNC2
              && sqlite3ReferencesSrcList(pParse, p_पाहायाग, pNC2->pSrcList)==0
          ){
            p_पाहायाग->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }
          assert( pDef!=0 || IN_RENAME_OBJECT );
          if( pNC2 && pDef ){
            p_पाहायाग->op2 += pNC2->nNestedSelect;
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
      return WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_EXISTS:
    case TK_SELECT:
#endif
    case TK_IN: {
      testcase( p_पाहायाग->op==TK_IN );
      testcase( p_पाहायाग->op==TK_EXISTS );
      testcase( p_पाहायाग->op==TK_SELECT );
      if( पाहायाग_Use_XSelect(p_पाहायाग) ){
        int nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_पाहायाग );
        testcase( pNC->ncFlags & NC_GenCol );
        assert( p_पाहायाग->x.pSelect );
        if( p_पाहायाग->op==TK_EXISTS )  pParse->bHasExists = 1;
        if( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_पाहायाग, p_पाहायाग);
        }else{
          sqlite3WalkSelect(pWalker, p_पाहायाग->x.pSelect);
        }
        assert( pNC->nRef>=nRef );
        if( nRef!=pNC->nRef ){
          पाहायाग_Set_Property(p_पाहायाग, EP_VarSelect);
          p_पाहायाग->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      break;
    }
    case TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_पाहायाग );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_पाहायाग|NC_GenCol, p_पाहायाग, p_पाहायाग);
      break;
    }
    case TK_IS:
    case TK_ISNOT: {
      पाहायाग *p_कानान = sqlite3_पाहायाग_Skip_Collate_And_Likely(p_पाहायाग->p_कानान);
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      if( ALWAYS(p_कानान) && (p_कानान->op==TK_ID || p_कानान->op==TK_TRUEFALSE) ){
        int rc = resolve_पाहायाग_Step(pWalker, p_कानान);
        if( rc==WRC_Abort ) return WRC_Abort;
        if( p_कानान->op==TK_TRUEFALSE ){
          p_पाहायाग->op2 = p_पाहायाग->op;
          p_पाहायाग->op = TK_TRUTH;
          return WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    case TK_BETWEEN:
    case TK_EQ:
    case TK_NE:
    case TK_LT:
    case TK_LE:
    case TK_GT:
    case TK_GE: {
      int n_कालिवा, n_कानान;
      if( pParse->db->mallocFailed ) break;
      assert( p_पाहायाग->p_कालिवा!=0 );
      n_कालिवा = sqlite3_पाहायाग_Vector_Size(p_पाहायाग->p_कालिवा);
      if( p_पाहायाग->op==TK_BETWEEN ){
        assert( पाहायाग_Use_XList(p_पाहायाग) );
        n_कानान = sqlite3_पाहायाग_Vector_Size(p_पाहायाग->x.pList->a[0].p_पाहायाग);
        if( n_कानान==n_कालिवा ){
          n_कानान = sqlite3_पाहायाग_Vector_Size(p_पाहायाग->x.pList->a[1].p_पाहायाग);
        }
      }else{
        assert( p_पाहायाग->p_कानान!=0 );
        n_कानान = sqlite3_पाहायाग_Vector_Size(p_पाहायाग->p_कानान);
      }
      if( n_कालिवा!=n_कानान ){
        testcase( p_पाहायाग->op==TK_EQ );
        testcase( p_पाहायाग->op==TK_NE );
        testcase( p_पाहायाग->op==TK_LT );
        testcase( p_पाहायाग->op==TK_LE );
        testcase( p_पाहायाग->op==TK_GT );
        testcase( p_पाहायाग->op==TK_GE );
        testcase( p_पाहायाग->op==TK_IS );
        testcase( p_पाहायाग->op==TK_ISNOT );
        testcase( p_पाहायाग->op==TK_BETWEEN );
        sqlite3_Error_मेनसाहे(pParse, "row value misused");
        sqlite3_Record_Error_Offset_Of_पाहायाग(pParse->db, p_पाहायाग);
      }
      break;
    }
  }
  assert( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  return pParse->nErr ? WRC_Abort : WRC_Continue;
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
static int resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  पाहायाग_List *pEList,  /* List of expressions to scan */
  पाहायाग *pE           /* Expression we are trying to match */
){
  int i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  if( pE->op==TK_ID ){
    const char *zCol;
    assert( !पाहायाग_Has_Property(pE, EP_Int_हालागा) );
    zCol = pE->u.zToken;
    for(i=0; i<pEList->n_पाहायाग; i++){
      if( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        return i+1;
      }
    }
  }
  return 0;
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
static int resolve_Order_By_Term_To_पाहायाग_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  पाहायाग *pE           /* The specific ORDER BY term */
){
  int i;             /* Loop counter */
  पाहायाग_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  int rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  assert( sqlite3_पाहायाग_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, sizeof(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_पाहायाग_Names(&nc, pE);
  db->suppressErr = savedSuppErr;
  if( rc ) return 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  for(i=0; i<pEList->n_पाहायाग; i++){
    if( sqlite3_पाहायाग_Compare(0, pEList->a[i].p_पाहायाग, pE, -1)<2 ){
      return i+1;
    }
  }

  /* If no match, return 0. */
  return 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
static void resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  const char *zType,     /* "ORDER" or "GROUP" */
  int i,                 /* The index (1-based) of the term out of range */
  int mx,                /* Largest permissible value of i */
  पाहायाग *pError           /* Associate the error with the expression */
){
  sqlite3_Error_मेनसाहे(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_Record_Error_Offset_Of_पाहायाग(pParse->db, pError);
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
static int resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  int i;
  पाहायाग_List *pOrderBy;
  पाहायाग_List *pEList;
  sqlite3 *db;
  int moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  if( pOrderBy==0 ) return 0;
  db = pParse->db;
  if( pOrderBy->n_पाहायाग>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_मेनसाहे(pParse, "too many terms in ORDER BY clause");
    return 1;
  }
  for(i=0; i<pOrderBy->n_पाहायाग; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  while( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  while( pSelect && moreToDo ){
    struct पाहायाग_List_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    assert( pEList!=0 );
    for(i=0, pItem=pOrderBy->a; i<pOrderBy->n_पाहायाग; i++, pItem++){
      int iCol = -1;
      पाहायाग *pE, *pDup;
      if( pItem->fg.done ) continue;
      pE = sqlite3_पाहायाग_Skip_Collate_And_Likely(pItem->p_पाहायाग);
      if( NEVER(pE==0) ) continue;
      if( sqlite3_पाहायाग_Is_Integer(pE, &iCol, 0, 1) ){
        if( iCol<=0 || iCol>pEList->n_पाहायाग ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->n_पाहायाग, pE);
          return 1;
        }
      }else{
        iCol = resolveAsName(pParse, pEList, pE);
        if( iCol==0 ){
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
          pDup = sqlite3_पाहायाग_Dup(db, pE, 0);
          if( !db->mallocFailed ){
            assert(pDup);
            iCol = resolve_Order_By_Term_To_पाहायाग_List(pParse, pSelect, pDup);
            if( IN_RENAME_OBJECT && iCol>0 ){
              resolve_Order_By_Term_To_पाहायाग_List(pParse, pSelect, pE);
            }
          }
          sqlite3_पाहायाग_Delete(db, pDup);
        }
      }
      if( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        if( !IN_RENAME_OBJECT ){
          पाहायाग *pNew = sqlite3_पाहायाग_Int32(db, iCol);
          if( pNew==0 ) return 1;
          if( pItem->p_पाहायाग==pE ){
            pItem->p_पाहायाग = pNew;
          }else{
            पाहायाग *pParent = pItem->p_पाहायाग;
            assert( pParent->op==TK_COLLATE );
            while( pParent->p_कालिवा->op==TK_COLLATE ) pParent = pParent->p_कालिवा;
            assert( pParent->p_कालिवा==pE );
            pParent->p_कालिवा = pNew;
          }
          sqlite3_पाहायाग_Delete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }else{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  for(i=0; i<pOrderBy->n_पाहायाग; i++){
    if( pOrderBy->a[i].fg.done==0 ){
      sqlite3_Error_मेनसाहे(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      return 1;
    }
  }
  return 0;
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
int sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  पाहायाग_List *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  const char *zType     /* "ORDER" or "GROUP" */
){
  int i;
  sqlite3 *db = pParse->db;
  पाहायाग_List *pEList;
  struct पाहायाग_List_item *pItem;

  if( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) return 0;
  if( pOrderBy->n_पाहायाग>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_मेनसाहे(pParse, "too many terms in %s BY clause", zType);
    return 1;
  }
  pEList = pSelect->pEList;
  assert( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  for(i=0, pItem=pOrderBy->a; i<pOrderBy->n_पाहायाग; i++, pItem++){
    if( pItem->u.x.iOrderByCol ){
      if( pItem->u.x.iOrderByCol>pEList->n_पाहायाग ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->n_पाहायाग, 0);
        return 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->p_पाहायाग,0);
    }
  }
  return 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
static int resolveRemoveWindowsCb(Walker *pWalker, पाहायाग *p_पाहायाग){
  UNUSED_PARAMETER(pWalker);
  if( पाहायाग_Has_Property(p_पाहायाग, EP_WinFunc) ){
    Window *pWin = p_पाहायाग->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  return WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
static void window_Remove_पाहायाग_From_Select(Select *pSelect, पाहायाग *p_पाहायाग){
  if( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, sizeof(Walker));
    sWalker.x_पाहायाग_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_पाहायाग(&sWalker, p_पाहायाग);
  }
}
#else
# define window_Remove_पाहायाग_From_Select(a, b)
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
static int resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  पाहायाग_List *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  const char *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  int i, j;                      /* Loop counters */
  int iCol;                      /* Column number */
  struct पाहायाग_List_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  int nResult;                   /* Number of terms in the result set */

  assert( pOrderBy!=0 );
  nResult = pSelect->pEList->n_पाहायाग;
  pParse = pNC->pParse;
  for(i=0, pItem=pOrderBy->a; i<pOrderBy->n_पाहायाग; i++, pItem++){
    पाहायाग *pE = pItem->p_पाहायाग;
    पाहायाग *pE2 = sqlite3_पाहायाग_Skip_Collate_And_Likely(pE);
    if( NEVER(pE2==0) ) continue;
    if( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      if( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        continue;
      }
    }
    if( sqlite3_पाहायाग_Is_Integer(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      if( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult, pE2);
        return 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      continue;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    if( sqlite3_Resolve_पाहायाग_Names(pNC, pE) ){
      return 1;
    }
    for(j=0; j<pSelect->pEList->n_पाहायाग; j++){
      if( sqlite3_पाहायाग_Compare(0, pE, pSelect->pEList->a[j].p_पाहायाग, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        window_Remove_पाहायाग_From_Select(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  return sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
static int resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  int isCompound;         /* True if p is a compound select */
  int nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  int i;                  /* Loop counter */
  पाहायाग_List *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  assert( p!=0 );
  if( p->selFlags & SF_Resolved ){
    return WRC_Prune;
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
  if( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    return pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  while( p ){
    assert( (p->selFlags & SF_Expanded)!=0 );
    assert( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, sizeof(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    if( sqlite3_Resolve_पाहायाग_Names(&sNC, p->pLimit) ){
      return WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    if( p->selFlags & SF_Converted ){
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
    if( pOuterNC ) pOuterNC->nNestedSelect++;
    for(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      assert( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      if( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        int nRef = pOuterNC ? pOuterNC->nRef : 0;
        const char *zSavedContext = pParse->zAuthContext;

        if( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        if( pParse->nErr ) return WRC_Abort;
        assert( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        if( pOuterNC ){
          assert( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    if( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
 
    /* Resolve names in the result set. */
    if( sqlite3_Resolve_पाहायाग_List_Names(&sNC, p->pEList) ) return WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    assert( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    if( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      assert( NC_MinMaxAgg==SF_MinMaxAgg );
      assert( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }else{
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
    if( p->pHaving ){
      if( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_मेनसाहे(pParse, "HAVING clause on a non-aggregate query");
        return WRC_Abort;
      }
      if( sqlite3_Resolve_पाहायाग_Names(&sNC, p->pHaving) ) return WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    if( sqlite3_Resolve_पाहायाग_Names(&sNC, p->pWhere) ) return WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    for(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      if( pItem->fg.isTabFunc
       && sqlite3_Resolve_पाहायाग_List_Names(&sNC, pItem->u1.pFuncArg)
      ){
        return WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    if( IN_RENAME_OBJECT ){
      Window *pWin;
      for(pWin=p->pWinDefn; pWin; pWin=pWin->pNextWin){
        if( sqlite3_Resolve_पाहायाग_List_Names(&sNC, pWin->pOrderBy)
         || sqlite3_Resolve_पाहायाग_List_Names(&sNC, pWin->pPartition)
        ){
          return WRC_Abort;
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
    if( p->selFlags & SF_Converted ){
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
    if( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      return WRC_Abort;
    }
    if( db->mallocFailed ){
      return WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    if( pGroupBy ){
      struct पाहायाग_List_item *pItem;
   
      if( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        return WRC_Abort;
      }
      for(i=0, pItem=pGroupBy->a; i<pGroupBy->n_पाहायाग; i++, pItem++){
        if( पाहायाग_Has_Property(pItem->p_पाहायाग, EP_Agg) ){
          sqlite3_Error_मेनसाहे(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          return WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    if( p->pNext && p->pEList->n_पाहायाग!=p->pNext->pEList->n_पाहायाग ){
      sqlite3_Select_Wrong_नुमेरो_Terms_Error(pParse, p->pNext);
      return WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    if( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      if( pParse->nErr ) return WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  if( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    return WRC_Abort;
  }

  return WRC_Prune;
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
int sqlite3_Resolve_पाहायाग_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  पाहायाग *p_पाहायाग             /* The expression to be analyzed. */
){
  int savedHasAgg;
  Walker w;

  if( p_पाहायाग==0 ) return SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.x_पाहायाग_Callback = resolve_पाहायाग_Step;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#if SQLITE_MAX_पाहायाग_DEPTH>0
  w.pParse->nHeight += p_पाहायाग->nHeight;
  if( sqlite3_पाहायाग_Check_Height(w.pParse, w.pParse->nHeight) ){
    return SQLITE_ERROR;
  }
#endif
  assert( p_पाहायाग!=0 );
  sqlite3_Walk_पाहायाग_NN(&w, p_पाहायाग);
#if SQLITE_MAX_पाहायाग_DEPTH>0
  w.pParse->nHeight -= p_पाहायाग->nHeight;
#endif
  assert( EP_Agg==NC_HasAgg );
  assert( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  पाहायाग_Set_Property(p_पाहायाग, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  return pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
int sqlite3_Resolve_पाहायाग_List_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  पाहायाग_List *pList         /* The expression list to be analyzed. */
){
  int i;
  int savedHasAgg = 0;
  Walker w;
  if( pList==0 ) return SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_पाहायाग_Callback = resolve_पाहायाग_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  for(i=0; i<pList->n_पाहायाग; i++){
    पाहायाग *p_पाहायाग = pList->a[i].p_पाहायाग;
    if( p_पाहायाग==0 ) continue;
#if SQLITE_MAX_पाहायाग_DEPTH>0
    w.pParse->nHeight += p_पाहायाग->nHeight;
    if( sqlite3_पाहायाग_Check_Height(w.pParse, w.pParse->nHeight) ){
      return SQLITE_ERROR;
    }
#endif
    sqlite3_Walk_पाहायाग_NN(&w, p_पाहायाग);
#if SQLITE_MAX_पाहायाग_DEPTH>0
    w.pParse->nHeight -= p_पाहायाग->nHeight;
#endif
    assert( EP_Agg==NC_HasAgg );
    assert( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    if( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      पाहायाग_Set_Property(p_पाहायाग, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    if( w.pParse->nErr>0 ) return SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  return SQLITE_OK;
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
  w.x_पाहायाग_Callback = resolve_पाहायाग_Step;
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
int sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  int type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  पाहायाग *p_पाहायाग,     /* Expression to resolve.  May be NULL. */
  पाहायाग_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  int rc;
  union {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  assert( type==0 || pTab!=0 );
  assert( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_पाहायाग
          || type==NC_GenCol || pTab==0 );
  memset(&sNC, 0, sizeof(sNC));
  memset(&uSrc, 0, sizeof(uSrc));
  pSrc = &uSrc.sSrc;
  if( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    if( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  if( (rc = sqlite3_Resolve_पाहायाग_Names(&sNC, p_पाहायाग))!=SQLITE_OK ) return rc;
  if( pList ) rc = sqlite3_Resolve_पाहायाग_List_Names(&sNC, pList);
  return rc;
}
