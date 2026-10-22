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
#ضمن "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#define EXCLUDED_TABLE_نومبور  2

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
ساكن صحيح incrAggDepth(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  إذا( p_اوڠکاڤن->op==TK_AGG_FUNCTION ) p_اوڠکاڤن->op2 += pWalker->u.n;
  أرجع WRC_Continue;
}
ساكن فراغ incrAggFunctionDepth(اوڠکاڤن *p_اوڠکاڤن, صحيح N){
  إذا( N>0 ){
    Walker w;
    memset(&w, 0, حجم(w));
    w.x_اوڠکاڤن_Callback = incrAggDepth;
    w.u.n = N;
    sqlite3_Walk_اوڠکاڤن(&w, p_اوڠکاڤن);
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
ساكن فراغ resolveAlias(
  Parse *pParse,         /* Parsing context */
  اوڠکاڤن_List *pEList,      /* A result set */
  صحيح iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  اوڠکاڤن *p_اوڠکاڤن,           /* Transform this into an alias to the result set */
  صحيح nSubquery          /* Number of subqueries that the label is moving */
){
  اوڠکاڤن *pOrig;           /* The iCol-th column of the result set */
  اوڠکاڤن *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  أكد( iCol>=0 && iCol<pEList->n_اوڠکاڤن );
  pOrig = pEList->a[iCol].p_اوڠکاڤن;
  أكد( pOrig!=0 );
  أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Reduced|EP_TokenOnly) );
  إذا( p_اوڠکاڤن->pAggInfo ) أرجع;
  db = pParse->db;
  pDup = sqlite3_اوڠکاڤن_Dup(db, pOrig, 0);
  إذا( db->mallocFailed ){
    sqlite3_اوڠکاڤن_Delete(db, pDup);
    pDup = 0;
  }وإلا{
    اوڠکاڤن temp;
    incrAggFunctionDepth(pDup, nSubquery);
    إذا( p_اوڠکاڤن->op==TK_COLLATE ){
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
      pDup = sqlite3_اوڠکاڤن_Add_Collate_String(pParse, pDup, p_اوڠکاڤن->u.zToken);
    }
    memcpy(&temp, pDup, حجم(اوڠکاڤن));
    memcpy(pDup, p_اوڠکاڤن, حجم(اوڠکاڤن));
    memcpy(p_اوڠکاڤن, &temp, حجم(اوڠکاڤن));
    إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc) ){
      إذا( ALWAYS(p_اوڠکاڤن->y.pWin!=0) ){
        p_اوڠکاڤن->y.pWin->pOwner = p_اوڠکاڤن;
      }
    }
    sqlite3_اوڠکاڤن_Deferred_Delete(pParse, pDup);
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
صحيح sqlite3MatchEName(
  ثابت بنية اوڠکاڤن_List_item *pItem,
  ثابت حرف *zCol,
  ثابت حرف *zTab,
  ثابت حرف *zDb,
  صحيح *pbRowid
){
  صحيح n;
  ثابت حرف *zSpan;
  صحيح eEName = pItem->fg.eEName;
  إذا( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    أرجع 0;
  }
  أكد( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  لكل(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  إذا( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    أرجع 0;
  }
  zSpan += n+1;
  لكل(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  إذا( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    أرجع 0;
  }
  zSpan += n+1;
  إذا( zCol ){
    إذا( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) أرجع 0;
    إذا( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) أرجع 0;
  }
  إذا( eEName==ENAME_ROWID ) *pbRowid = 1;
  أرجع 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
ساكن صحيح areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *p_اتس_NC){
  إذا( db->init.busy ) أرجع 1;  /* Always support for legacy schemas */
  إذا( p_اتس_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    إذا( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      أرجع 1;
    }
    أرجع (db->flags & SQLITE_DqsDDL)!=0;
  }وإلا{
    /* Currently parsing a DML statement */
    أرجع (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_اوڠکاڤن_Col_Used(اوڠکاڤن *p_اوڠکاڤن){
  صحيح n;
  Table *pExTab;

  n = p_اوڠکاڤن->iColumn;
  أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
  pExTab = p_اوڠکاڤن->y.pTab;
  أكد( pExTab!=0 );
  أكد( n < pExTab->nCol );
  إذا( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    أرجع pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }وإلا{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    إذا( n>=BMS ) n = BMS-1;
    أرجع ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
ساكن فراغ extendFJMatch(
  Parse *pParse,          /* Parsing context */
  اوڠکاڤن_List **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  اوڠکاڤن *pNew = sqlite3_اوڠکاڤن_Alloc(pParse->db, TK_COLUMN, 0, 0);
  إذا( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    أكد( (pMatch->fg.jointype & (JT_کيري|JT_LTORJ))!=0 );
    اوڠکاڤن_Set_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_اوڠکاڤن_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
ساكن SQLITE_NOINLINE صحيح isValidSchemaTableName(
  ثابت حرف *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  ثابت حرف *zDb           /* non-NULL if a database qualifier is present */
){
  ثابت حرف *zLegacy;
  أكد( pTab!=0 );
  أكد( pTab->tnum==1 );
  إذا( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) أرجع 0;
  zLegacy = pTab->zName;
  إذا( strcmp(zLegacy+7, &LEGACY_TEMP_SCHEMA_TABLE[7])==0 ){
    إذا( sqlite3StrICmp(zTab+7, &PREFERRED_TEMP_SCHEMA_TABLE[7])==0 ){
      أرجع 1;
    }
    إذا( zDb==0 ) أرجع 0;
    إذا( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) أرجع 1;
    إذا( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) أرجع 1;
  }وإلا{
    إذا( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) أرجع 1;
  }
  أرجع 0;
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
ساكن صحيح lookupName(
  Parse *pParse,       /* The parsing context */
  ثابت حرف *zDb,     /* Name of the database containing table, or NULL */
  ثابت حرف *zTab,    /* Name of table containing column, or NULL */
  ثابت اوڠکاڤن *p_کانن,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  اوڠکاڤن *p_اوڠکاڤن          /* Make this EXPR node point to the selected column */
){
  صحيح i, j;                         /* Loop counters */
  صحيح cnt = 0;                      /* Number of matching column names */
  صحيح cntTab = 0;                   /* Number of potential "rowid" matches */
  صحيح nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *p_اتس_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  صحيح e_New_اوڠکاڤن_Op = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  اوڠکاڤن_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  ثابت حرف *zCol = p_کانن->u.zToken;

  أكد( pNC );     /* the name context cannot be NULL. */
  أكد( zCol );    /* The Z in X.Y.Z cannot be NULL */
  أكد( zDb==0 || zTab!=0 );
  أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_اوڠکاڤن->iTable = -1;
  اوڠکاڤن_Set_VVAProperty(p_اوڠکاڤن, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  إذا( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    إذا( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }وإلا{
      لكل(i=0; i<db->nDb; i++){
        أكد( db->aDb[i].zDbSName );
        إذا( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          اكسر;
        }
      }
      إذا( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  أكد( pNC && cnt==0 );
  افعل{
    اوڠکاڤن_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    إذا( pSrcList ){
      لكل(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        أكد( pTab!=0 && pTab->zName!=0 );
        أكد( pTab->nCol>0 || pParse->nErr );
        أكد( (صحيح)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        إذا( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          صحيح hit = 0;
          Select *pSel;
          أكد( pItem->fg.isSubquery );
          أكد( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          أكد( pSel!=0 );
          pEList = pSel->pEList;
          أكد( pEList!=0 );
          أكد( pEList->n_اوڠکاڤن==pTab->nCol );
          لكل(j=0; j<pEList->n_اوڠکاڤن; j++){
            صحيح bRowid = 0;       /* True if possible rowid match */
            إذا( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              استمر;
            }
            إذا( bRowid==0 ){
              إذا( cnt>0 ){
                إذا( pItem->fg.isUsing==0
                 || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_اوڠکاڤن_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }وإلا
                إذا( (pItem->fg.jointype & JT_کانن)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  استمر;
                }وإلا
                إذا( (pItem->fg.jointype & JT_کيري)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_اوڠکاڤن_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }وإلا{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, p_اوڠکاڤن->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }وإلا إذا( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              استمر;
            }
            cntTab++;
            pMatch = pItem;
            p_اوڠکاڤن->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            أكد( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            إذا( pEList->a[j].fg.bUsingTerm ) اكسر;
          }
          إذا( hit || zTab==0 ) استمر;
        }
        أكد( zDb==0 || zTab!=0 );
        إذا( zTab ){
          إذا( zDb ){
            إذا( pTab->pSchema!=pSchema ) استمر;
            إذا( pSchema==0 && strcmp(zDb,"*")!=0 ) استمر;
          }
          إذا( pItem->zAlias!=0 ){
            إذا( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              استمر;
            }
          }وإلا إذا( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            إذا( pTab->tnum!=1 ) استمر;
            إذا( !isValidSchemaTableName(zTab, pTab, zDb) ) استمر;
          }
          أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
          إذا( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (فراغ*)&p_اوڠکاڤن->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        إذا( j>=0 ){
          إذا( cnt>0 ){
            إذا( pItem->fg.isUsing==0
             || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_اوڠکاڤن_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }وإلا
            إذا( (pItem->fg.jointype & JT_کانن)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              استمر;
            }وإلا
            إذا( (pItem->fg.jointype & JT_کيري)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_اوڠکاڤن_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }وإلا{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, p_اوڠکاڤن->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_اوڠکاڤن->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          إذا( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        إذا( 0==cnt && VisibleRowid(pTab) ){
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
          إذا( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }وإلا{
            cntTab++;
          }
#وإلا
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      إذا( pMatch ){
        p_اوڠکاڤن->iTable = pMatch->iCursor;
        أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
        p_اوڠکاڤن->y.pTab = pMatch->pSTab;
        إذا( (pMatch->fg.jointype & (JT_کيري|JT_LTORJ))!=0 ){
          اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_CanBeNull);
        }
        pSchema = p_اوڠکاڤن->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#إذا !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    إذا( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      إذا( pParse->pTriggerTab!=0 ){
        صحيح op = pParse->eTriggerOp;
        أكد( op==TK_DELETE || op==TK_UPDATE || op==TK_سيسيڤ );
        إذا( pParse->bReturning ){
          إذا( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            p_اوڠکاڤن->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }وإلا إذا( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_اوڠکاڤن->iTable = 1;
          pTab = pParse->pTriggerTab;
        }وإلا إذا( op!=TK_سيسيڤ && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_اوڠکاڤن->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      إذا( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        إذا( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_اوڠکاڤن->iTable = EXCLUDED_TABLE_نومبور;
          إذا( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (فراغ*)&p_اوڠکاڤن->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      إذا( pTab ){
        صحيح iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        إذا( iCol>=0 ){
          إذا( pTab->iPKey==iCol ) iCol = -1;
        }وإلا{
          إذا( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }وإلا{
            iCol = pTab->nCol;
          }
        }
        إذا( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          إذا( p_اوڠکاڤن->iTable==EXCLUDED_TABLE_نومبور ){
            testcase( iCol==(-1) );
            أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
            إذا( IN_RENAME_OBJECT ){
              p_اوڠکاڤن->iColumn = iCol;
              p_اوڠکاڤن->y.pTab = pTab;
              e_New_اوڠکاڤن_Op = TK_COLUMN;
            }وإلا{
              p_اوڠکاڤن->iTable = pNC->uNC.pUpsert->regData +
                 sqlite3TableColumnToStorage(pTab, iCol);
              e_New_اوڠکاڤن_Op = TK_REGISTER;
            }
          }وإلا
#endif /* SQLITE_OMIT_UPSERT */
          {
            أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
            p_اوڠکاڤن->y.pTab = pTab;
            إذا( pParse->bReturning ){
              e_New_اوڠکاڤن_Op = TK_REGISTER;
              p_اوڠکاڤن->op2 = TK_COLUMN;
              p_اوڠکاڤن->iColumn = iCol;
              p_اوڠکاڤن->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*p_اوڠکاڤن->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }وإلا{
              p_اوڠکاڤن->iColumn = (i16)iCol;
              e_New_اوڠکاڤن_Op = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              إذا( iCol<0 ){
                p_اوڠکاڤن->aff_اوڠکاڤن = SQLITE_AFF_INTEGER;
              }وإلا إذا( p_اوڠکاڤن->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }وإلا{
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
    إذا( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_Idx_اوڠکاڤن|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#إذا SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      إذا( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        e_New_اوڠکاڤن_Op = TK_NULL;
      }
#endif
      إذا( pMatch->fg.isNestedFrom==0 ) p_اوڠکاڤن->iColumn = -1;
      p_اوڠکاڤن->aff_اوڠکاڤن = SQLITE_AFF_INTEGER;
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
    إذا( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      أكد( pEList!=0 );
      لكل(j=0; j<pEList->n_اوڠکاڤن; j++){
        حرف *zAs = pEList->a[j].zEName;
        إذا( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          اوڠکاڤن *pOrig;
          أكد( p_اوڠکاڤن->p_کيري==0 && p_اوڠکاڤن->p_کانن==0 );
          أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن)==0 || p_اوڠکاڤن->x.pList==0 );
          أكد( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن)==0 || p_اوڠکاڤن->x.pSelect==0 );
          pOrig = pEList->a[j].p_اوڠکاڤن;
          إذا( (pNC->ncFlags&NC_AllowAgg)==0 && اوڠکاڤن_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_ڤسنن(pParse, "misuse of aliased aggregate %s", zAs);
            أرجع WRC_Abort;
          }
          إذا( اوڠکاڤن_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_اتس_NC )
          ){
            sqlite3_Error_ڤسنن(pParse, "misuse of aliased window function %s",zAs);
            أرجع WRC_Abort;
          }
          إذا( sqlite3_اوڠکاڤن_Vector_Size(pOrig)!=1 ){
            sqlite3_Error_ڤسنن(pParse, "row value misused");
            أرجع WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_اوڠکاڤن, nSubquery);
          cnt = 1;
          pMatch = 0;
          أكد( zTab==0 && zDb==0 );
          إذا( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (فراغ*)p_اوڠکاڤن);
          }
          اذهب lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    إذا( cnt ) اكسر;
    pNC = pNC->pNext;
    nSubquery++;
  }طالما( pNC );


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
  إذا( cnt==0 && zTab==0 ){
    أكد( p_اوڠکاڤن->op==TK_ID );
    إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, p_اتس_NC)
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
      p_اوڠکاڤن->op = TK_STRING;
      memset(&p_اوڠکاڤن->y, 0, حجم(p_اوڠکاڤن->y));
      أرجع WRC_Prune;
    }
    إذا( sqlite3_اوڠکاڤن_Id_To_True_False(p_اوڠکاڤن) ){
      أرجع WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  أكد( pFJMatch==0 || cnt>0 );
  أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_xIsSelect|EP_Int_نيلاي) );
  إذا( cnt!=1 ){
    ثابت حرف *zErr;
    إذا( pFJMatch ){
      إذا( pFJMatch->n_اوڠکاڤن==cnt-1 ){
        إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن,EP_Leaf) ){
          اوڠکاڤن_Clear_Property(p_اوڠکاڤن,EP_Leaf);
        }وإلا{
          sqlite3_اوڠکاڤن_Delete(db, p_اوڠکاڤن->p_کيري);
          p_اوڠکاڤن->p_کيري = 0;
          sqlite3_اوڠکاڤن_Delete(db, p_اوڠکاڤن->p_کانن);
          p_اوڠکاڤن->p_کانن = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, p_اوڠکاڤن->iColumn);
        p_اوڠکاڤن->op = TK_FUNCTION;
        p_اوڠکاڤن->u.zToken = "coalesce";
        p_اوڠکاڤن->x.pList = pFJMatch;
        p_اوڠکاڤن->aff_اوڠکاڤن = SQLITE_AFF_DEFER;
        cnt = 1;
        اذهب lookupname_end;
      }وإلا{
        sqlite3_اوڠکاڤن_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    إذا( zDb ){
      sqlite3_Error_ڤسنن(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }وإلا إذا( zTab ){
      sqlite3_Error_ڤسنن(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }وإلا إذا( cnt==0 && اوڠکاڤن_Has_Property(p_کانن,EP_DblQuoted) ){
      sqlite3_Error_ڤسنن(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }وإلا{
      sqlite3_Error_ڤسنن(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3_Record_Error_Offset_Of_اوڠکاڤن(pParse->db, p_اوڠکاڤن);
    pParse->checkSchema = 1;
    p_اتس_NC->nNcErr++;
    e_New_اوڠکاڤن_Op = TK_NULL;
  }
  أكد( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  إذا( !اوڠکاڤن_Has_Property(p_اوڠکاڤن,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3_اوڠکاڤن_Delete(db, p_اوڠکاڤن->p_کيري);
    p_اوڠکاڤن->p_کيري = 0;
    sqlite3_اوڠکاڤن_Delete(db, p_اوڠکاڤن->p_کانن);
    p_اوڠکاڤن->p_کانن = 0;
    اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_Leaf);
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
  إذا( pMatch ){
    إذا( p_اوڠکاڤن->iColumn>=0 ){
      pMatch->colUsed |= sqlite3_اوڠکاڤن_Col_Used(p_اوڠکاڤن);
    }وإلا{
      pMatch->fg.rowidUsed = 1;
    }
  }

  p_اوڠکاڤن->op = e_New_اوڠکاڤن_Op;
lookupname_end:
  إذا( cnt==1 ){
    أكد( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    إذا( db->xAuth ){
      إذا( pFJMatch ){
        أكد( p_اوڠکاڤن->op==TK_FUNCTION );
        أكد( sqlite3_stricmp(p_اوڠکاڤن->u.zToken,"coalesce")==0 );
        أكد( p_اوڠکاڤن->x.pList==pFJMatch );
        أكد( pFJMatch->n_اوڠکاڤن>0 );
        لكل(i=0; i<pFJMatch->n_اوڠکاڤن; i++){
          أكد( pFJMatch->a[i].p_اوڠکاڤن->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_اوڠکاڤن,pSchema,pNC->pSrcList);
        }
      }وإلا إذا( p_اوڠکاڤن->op==TK_COLUMN || p_اوڠکاڤن->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_اوڠکاڤن, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    لكل(;;){
      أكد( p_اتس_NC!=0 );
      p_اتس_NC->nRef++;
      إذا( p_اتس_NC==pNC ) اكسر;
      p_اتس_NC = p_اتس_NC->pNext;
    }
    أرجع WRC_Prune;
  } وإلا {
    أرجع WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
اوڠکاڤن *sqlite3_Create_Column_اوڠکاڤن(sqlite3 *db, SrcList *pSrc, صحيح iSrc, صحيح iCol){
  اوڠکاڤن *p = sqlite3_اوڠکاڤن_Alloc(db, TK_COLUMN, 0, 0);
  إذا( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    أكد( اوڠکاڤن_Use_YTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    إذا( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }وإلا{
      p->iColumn = (ynVar)iCol;
      إذا( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }وإلا{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  أرجع p;
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
ساكن فراغ notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   ثابت حرف *z_ڤسنن,    /* Type of error */
   اوڠکاڤن *p_اوڠکاڤن,         /* Invalidate this expression on error */
   اوڠکاڤن *pError         /* Associate error with this expression */
){
  ثابت حرف *zIn = "partial index WHERE clauses";
  إذا( pNC->ncFlags & NC_Idx_اوڠکاڤن )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  وإلا إذا( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  وإلا إذا( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3_Error_ڤسنن(pParse, "%s prohibited in %s", z_ڤسنن, zIn);
  إذا( p_اوڠکاڤن ) p_اوڠکاڤن->op = TK_NULL;
  sqlite3_Record_Error_Offset_Of_اوڠکاڤن(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  أكد( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_اوڠکاڤن|NC_GenCol))==0 ); \
  إذا( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
ساكن صحيح اوڠکاڤن_Probability(اوڠکاڤن *p){
  مزدوج r = -1.0;
  إذا( p->op!=TK_FLOAT ) أرجع -1;
  أكد( !اوڠکاڤن_Has_Property(p, EP_Int_نيلاي) );
  sqlite3AtoF(p->u.zToken, &r);
  أكد( r>=0.0 );
  إذا( r>1.0 ) أرجع -1;
  أرجع (صحيح)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
ساكن SQLITE_NOINLINE فراغ resolve_Set_اوڠکاڤن_Subtype_Arg(اوڠکاڤن_List *pList){
  صحيح nn, ii;
  nn = pList ? pList->n_اوڠکاڤن : 0;
  لكل(ii=0; ii<nn; ii++){
    اوڠکاڤن *p_اوڠکاڤن = pList->a[ii].p_اوڠکاڤن;
    طالما( 1 /*exit-by-break*/ ){
      اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_SubtArg);
      إذا( p_اوڠکاڤن->op==TK_SELECT ){
        أكد( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) );
        أكد( p_اوڠکاڤن->x.pSelect!=0 );
        resolve_Set_اوڠکاڤن_Subtype_Arg(p_اوڠکاڤن->x.pSelect->pEList);
        اكسر;
      }
      إذا( p_اوڠکاڤن->op==TK_UPLUS ){
        p_اوڠکاڤن = p_اوڠکاڤن->p_کيري;
        أكد( p_اوڠکاڤن!=0 );
      }وإلا{
        اكسر;
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
ساكن صحيح resolve_اوڠکاڤن_Step(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  أكد( pNC!=0 );
  pParse = pNC->pParse;
  أكد( pParse==pWalker->pParse );

#ifndef NDEBUG
  إذا( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    صحيح i;
    لكل(i=0; i<pNC->pSrcList->nSrc; i++){
      أكد( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  بدل( p_اوڠکاڤن->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    حالة TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      أكد( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      p_اوڠکاڤن->op = TK_COLUMN;
      أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
      p_اوڠکاڤن->y.pTab = pItem->pSTab;
      p_اوڠکاڤن->iTable = pItem->iCursor;
      p_اوڠکاڤن->iColumn--;
      p_اوڠکاڤن->aff_اوڠکاڤن = SQLITE_AFF_INTEGER;
      اكسر;
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
    حالة TK_NOTNULL:
    حالة TK_ISNULL: {
      صحيح anRef[8];
      NameContext *p;
      صحيح i;
      لكل(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_اوڠکاڤن(pWalker, p_اوڠکاڤن->p_کيري);
      إذا( IN_RENAME_OBJECT ) أرجع WRC_Prune;
      إذا( sqlite3_اوڠکاڤن_Can_Be_Null(p_اوڠکاڤن->p_کيري) ){
        /* The expression can be NULL.  So the optimization does not apply */
        أرجع WRC_Prune;
      }

      لكل(i=0, p=pNC; p; p=p->pNext, i++){
        إذا( (p->ncFlags & NC_Where)==0 ){
          أرجع WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_OuterON) );
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
#إذا TREETRACE_ENABLED
      إذا( sqlite3_ڤوکوق_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_اوڠکاڤن->op==TK_NOTNULL
        );
        sqlite3_Show_اوڠکاڤن(p_اوڠکاڤن);
      }
#endif /* TREETRACE_ENABLED */
      p_اوڠکاڤن->u.i_نيلاي = (p_اوڠکاڤن->op==TK_NOTNULL);
      p_اوڠکاڤن->flags |= EP_Int_نيلاي;
      p_اوڠکاڤن->op = TK_INTEGER;
      لكل(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        p->nRef = anRef[i];
      }
      sqlite3_اوڠکاڤن_Delete(pParse->db, p_اوڠکاڤن->p_کيري);
      p_اوڠکاڤن->p_کيري = 0;
      أرجع WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    حالة TK_ID:
    حالة TK_DOT: {
      ثابت حرف *zTable;
      ثابت حرف *zDb;
      اوڠکاڤن *p_کانن;

      إذا( p_اوڠکاڤن->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Int_نيلاي) );
        p_کانن = p_اوڠکاڤن;
      }وإلا{
        اوڠکاڤن *p_کيري = p_اوڠکاڤن->p_کيري;
        testcase( pNC->ncFlags & NC_Idx_اوڠکاڤن );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_اوڠکاڤن|NC_GenCol, 0, p_اوڠکاڤن);
        p_کانن = p_اوڠکاڤن->p_کانن;
        إذا( p_کانن->op==TK_ID ){
          zDb = 0;
        }وإلا{
          أكد( p_کانن->op==TK_DOT );
          أكد( !اوڠکاڤن_Has_Property(p_کانن, EP_Int_نيلاي) );
          zDb = p_کيري->u.zToken;
          p_کيري = p_کانن->p_کيري;
          p_کانن = p_کانن->p_کانن;
        }
        أكد( اوڠکاڤن_Use_UToken(p_کيري) && اوڠکاڤن_Use_UToken(p_کانن) );
        zTable = p_کيري->u.zToken;
        أكد( اوڠکاڤن_Use_YTab(p_اوڠکاڤن) );
        إذا( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (فراغ*)p_اوڠکاڤن, (فراغ*)p_کانن);
          sqlite3RenameTokenRemap(pParse, (فراغ*)&p_اوڠکاڤن->y.pTab, (فراغ*)p_کيري);
        }
      }
      أرجع lookupName(pParse, zDb, zTable, p_کانن, pNC, p_اوڠکاڤن);
    }

    /* Resolve function names
    */
    حالة TK_FUNCTION: {
      اوڠکاڤن_List *pList;            /* The argument list */
      صحيح n;                      /* Number of arguments */
      صحيح no_such_func = 0;       /* True if no such function exists */
      صحيح wrong_اڠک_args = 0;     /* True if wrong number of arguments */
      صحيح is_agg = 0;             /* True if is an aggregate function */
      ثابت حرف *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      صحيح savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(p_اوڠکاڤن) ? p_اوڠکاڤن->y.pWin : 0);
#endif
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_xIsSelect|EP_Int_نيلاي) );
      أكد( p_اوڠکاڤن->p_کيري==0 || p_اوڠکاڤن->p_کيري->op==TK_ORDER );
      pList = p_اوڠکاڤن->x.pList;
      n = pList ? pList->n_اوڠکاڤن : 0;
      zId = p_اوڠکاڤن->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      إذا( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        إذا( pDef==0 ){
          no_such_func = 1;
        }وإلا{
          wrong_اڠک_args = 1;
        }
      }وإلا{
        is_agg = pDef->xFinalize!=0;
        إذا( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_Unlikely);
          إذا( n==2 ){
            p_اوڠکاڤن->iTable = اوڠکاڤن_Probability(pList->a[1].p_اوڠکاڤن);
            إذا( p_اوڠکاڤن->iTable<0 ){
              sqlite3_Error_ڤسنن(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_اوڠکاڤن);
              pNC->nNcErr++;
            }
          }وإلا{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_اوڠکاڤن->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          صحيح auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          إذا( auth!=SQLITE_OK ){
            إذا( auth==SQLITE_DENY ){
              sqlite3_Error_ڤسنن(pParse, "not authorized to use function: %#T",
                                      p_اوڠکاڤن);
              pNC->nNcErr++;
            }
            p_اوڠکاڤن->op = TK_NULL;
            أرجع WRC_Prune;
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
        إذا( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_SubtArg) 
        ){
          resolve_Set_اوڠکاڤن_Subtype_Arg(pList);
        }

        إذا( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          اوڠکاڤن_Set_Property(p_اوڠکاڤن,EP_ConstFunc);
        }
        إذا( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_اوڠکاڤن|NC_PartIdx|NC_GenCol, 0, p_اوڠکاڤن);
        }وإلا{
          أكد( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_اوڠکاڤن->op2 = pNC->ncFlags & NC_SelfRef;
        }
        إذا( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
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
        }وإلا
        إذا( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          إذا( pNC->ncFlags & NC_FromDDL ) اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_FromDDL);
          sqlite3_اوڠکاڤن_Function_Usable(pParse, p_اوڠکاڤن, pDef);
        }
      }

      إذا( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        أكد( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_نيلاي==0 && pDef->xInverse==0)
          || (pDef->x_نيلاي && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        إذا( pDef && pDef->x_نيلاي==0 && pWin ){
          sqlite3_Error_ڤسنن(pParse,
              "%#T() may not be used as a window function", p_اوڠکاڤن
          );
          pNC->nNcErr++;
        }وإلا إذا(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          ثابت حرف *zType;
          إذا( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }وإلا{
            zType = "aggregate";
          }
          sqlite3_Error_ڤسنن(pParse, "misuse of %s function %#T()",zType,p_اوڠکاڤن);
          pNC->nNcErr++;
          is_agg = 0;
        }
#وإلا
        إذا( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_ڤسنن(pParse,"misuse of aggregate function %#T()",p_اوڠکاڤن);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        وإلا إذا( no_such_func
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
          sqlite3_Error_ڤسنن(pParse, "no such function: %#T", p_اوڠکاڤن);
          pNC->nNcErr++;
        }وإلا إذا( wrong_اڠک_args ){
          sqlite3_Error_ڤسنن(pParse,"wrong number of arguments to function %#T()",
               p_اوڠکاڤن);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        وإلا إذا( is_agg==0 && اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc) ){
          sqlite3_Error_ڤسنن(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_اوڠکاڤن
          );
          pNC->nNcErr++;
        }
#endif
        وإلا إذا( is_agg==0 && p_اوڠکاڤن->p_کيري ){
          sqlite3_اوڠکاڤن_Order_By_Aggregate_Error(pParse, p_اوڠکاڤن);
          pNC->nNcErr++;
        }
        إذا( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#وإلا
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      وإلا إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc) || p_اوڠکاڤن->p_کيري ){
        is_agg = 1;
      }
      sqlite3_Walk_اوڠکاڤن_List(pWalker, pList);
      إذا( is_agg ){
        إذا( p_اوڠکاڤن->p_کيري ){
          أكد( p_اوڠکاڤن->p_کيري->op==TK_ORDER );
          أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن->p_کيري) );
          sqlite3_Walk_اوڠکاڤن_List(pWalker, p_اوڠکاڤن->p_کيري->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        إذا( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          أكد( اوڠکاڤن_Use_YWin(p_اوڠکاڤن) && pWin==p_اوڠکاڤن->y.pWin );
          إذا( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            إذا( pParse->db->mallocFailed ) اكسر;
          }
          sqlite3_Walk_اوڠکاڤن_List(pWalker, pWin->pPartition);
          sqlite3_Walk_اوڠکاڤن_List(pWalker, pWin->pOrderBy);
          sqlite3_Walk_اوڠکاڤن(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }وإلا
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_اوڠکاڤن->op = TK_AGG_FUNCTION;
          p_اوڠکاڤن->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc) ){
            sqlite3_Walk_اوڠکاڤن(pWalker, p_اوڠکاڤن->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          طالما( pNC2
              && sqlite3ReferencesSrcList(pParse, p_اوڠکاڤن, pNC2->pSrcList)==0
          ){
            p_اوڠکاڤن->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }
          أكد( pDef!=0 || IN_RENAME_OBJECT );
          إذا( pNC2 && pDef ){
            p_اوڠکاڤن->op2 += pNC2->nNestedSelect;
            أكد( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            أكد( SQLITE_FUNC_ANYORDER==NC_OrderAgg );
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
      أرجع WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS:
    حالة TK_SELECT:
#endif
    حالة TK_IN: {
      testcase( p_اوڠکاڤن->op==TK_IN );
      testcase( p_اوڠکاڤن->op==TK_EXISTS );
      testcase( p_اوڠکاڤن->op==TK_SELECT );
      إذا( اوڠکاڤن_Use_XSelect(p_اوڠکاڤن) ){
        صحيح nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_اوڠکاڤن );
        testcase( pNC->ncFlags & NC_GenCol );
        أكد( p_اوڠکاڤن->x.pSelect );
        إذا( p_اوڠکاڤن->op==TK_EXISTS )  pParse->bHasExists = 1;
        إذا( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_اوڠکاڤن, p_اوڠکاڤن);
        }وإلا{
          sqlite3WalkSelect(pWalker, p_اوڠکاڤن->x.pSelect);
        }
        أكد( pNC->nRef>=nRef );
        إذا( nRef!=pNC->nRef ){
          اوڠکاڤن_Set_Property(p_اوڠکاڤن, EP_VarSelect);
          p_اوڠکاڤن->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      اكسر;
    }
    حالة TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_اوڠکاڤن );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_اوڠکاڤن|NC_GenCol, p_اوڠکاڤن, p_اوڠکاڤن);
      اكسر;
    }
    حالة TK_IS:
    حالة TK_ISNOT: {
      اوڠکاڤن *p_کانن = sqlite3_اوڠکاڤن_Skip_Collate_And_Likely(p_اوڠکاڤن->p_کانن);
      أكد( !اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      إذا( ALWAYS(p_کانن) && (p_کانن->op==TK_ID || p_کانن->op==TK_TRUEFALSE) ){
        صحيح rc = resolve_اوڠکاڤن_Step(pWalker, p_کانن);
        إذا( rc==WRC_Abort ) أرجع WRC_Abort;
        إذا( p_کانن->op==TK_TRUEFALSE ){
          p_اوڠکاڤن->op2 = p_اوڠکاڤن->op;
          p_اوڠکاڤن->op = TK_TRUTH;
          أرجع WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    حالة TK_BETWEEN:
    حالة TK_EQ:
    حالة TK_NE:
    حالة TK_LT:
    حالة TK_LE:
    حالة TK_GT:
    حالة TK_GE: {
      صحيح n_کيري, n_کانن;
      إذا( pParse->db->mallocFailed ) اكسر;
      أكد( p_اوڠکاڤن->p_کيري!=0 );
      n_کيري = sqlite3_اوڠکاڤن_Vector_Size(p_اوڠکاڤن->p_کيري);
      إذا( p_اوڠکاڤن->op==TK_BETWEEN ){
        أكد( اوڠکاڤن_Use_XList(p_اوڠکاڤن) );
        n_کانن = sqlite3_اوڠکاڤن_Vector_Size(p_اوڠکاڤن->x.pList->a[0].p_اوڠکاڤن);
        إذا( n_کانن==n_کيري ){
          n_کانن = sqlite3_اوڠکاڤن_Vector_Size(p_اوڠکاڤن->x.pList->a[1].p_اوڠکاڤن);
        }
      }وإلا{
        أكد( p_اوڠکاڤن->p_کانن!=0 );
        n_کانن = sqlite3_اوڠکاڤن_Vector_Size(p_اوڠکاڤن->p_کانن);
      }
      إذا( n_کيري!=n_کانن ){
        testcase( p_اوڠکاڤن->op==TK_EQ );
        testcase( p_اوڠکاڤن->op==TK_NE );
        testcase( p_اوڠکاڤن->op==TK_LT );
        testcase( p_اوڠکاڤن->op==TK_LE );
        testcase( p_اوڠکاڤن->op==TK_GT );
        testcase( p_اوڠکاڤن->op==TK_GE );
        testcase( p_اوڠکاڤن->op==TK_IS );
        testcase( p_اوڠکاڤن->op==TK_ISNOT );
        testcase( p_اوڠکاڤن->op==TK_BETWEEN );
        sqlite3_Error_ڤسنن(pParse, "row value misused");
        sqlite3_Record_Error_Offset_Of_اوڠکاڤن(pParse->db, p_اوڠکاڤن);
      }
      اكسر;
    }
  }
  أكد( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  أرجع pParse->nErr ? WRC_Abort : WRC_Continue;
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
ساكن صحيح resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  اوڠکاڤن_List *pEList,  /* List of expressions to scan */
  اوڠکاڤن *pE           /* Expression we are trying to match */
){
  صحيح i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  إذا( pE->op==TK_ID ){
    ثابت حرف *zCol;
    أكد( !اوڠکاڤن_Has_Property(pE, EP_Int_نيلاي) );
    zCol = pE->u.zToken;
    لكل(i=0; i<pEList->n_اوڠکاڤن; i++){
      إذا( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        أرجع i+1;
      }
    }
  }
  أرجع 0;
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
ساكن صحيح resolve_Order_By_Term_To_اوڠکاڤن_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  اوڠکاڤن *pE           /* The specific ORDER BY term */
){
  صحيح i;             /* Loop counter */
  اوڠکاڤن_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  صحيح rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  أكد( sqlite3_اوڠکاڤن_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, حجم(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_اوڠکاڤن_Names(&nc, pE);
  db->suppressErr = savedSuppErr;
  إذا( rc ) أرجع 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  لكل(i=0; i<pEList->n_اوڠکاڤن; i++){
    إذا( sqlite3_اوڠکاڤن_Compare(0, pEList->a[i].p_اوڠکاڤن, pE, -1)<2 ){
      أرجع i+1;
    }
  }

  /* If no match, return 0. */
  أرجع 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
ساكن فراغ resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  ثابت حرف *zType,     /* "ORDER" or "GROUP" */
  صحيح i,                 /* The index (1-based) of the term out of range */
  صحيح mx,                /* Largest permissible value of i */
  اوڠکاڤن *pError           /* Associate the error with the expression */
){
  sqlite3_Error_ڤسنن(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_Record_Error_Offset_Of_اوڠکاڤن(pParse->db, pError);
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
ساكن صحيح resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  صحيح i;
  اوڠکاڤن_List *pOrderBy;
  اوڠکاڤن_List *pEList;
  sqlite3 *db;
  صحيح moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  إذا( pOrderBy==0 ) أرجع 0;
  db = pParse->db;
  إذا( pOrderBy->n_اوڠکاڤن>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ڤسنن(pParse, "too many terms in ORDER BY clause");
    أرجع 1;
  }
  لكل(i=0; i<pOrderBy->n_اوڠکاڤن; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  طالما( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  طالما( pSelect && moreToDo ){
    بنية اوڠکاڤن_List_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    أكد( pEList!=0 );
    لكل(i=0, pItem=pOrderBy->a; i<pOrderBy->n_اوڠکاڤن; i++, pItem++){
      صحيح iCol = -1;
      اوڠکاڤن *pE, *pDup;
      إذا( pItem->fg.done ) استمر;
      pE = sqlite3_اوڠکاڤن_Skip_Collate_And_Likely(pItem->p_اوڠکاڤن);
      إذا( NEVER(pE==0) ) استمر;
      إذا( sqlite3_اوڠکاڤن_Is_Integer(pE, &iCol, 0, 1) ){
        إذا( iCol<=0 || iCol>pEList->n_اوڠکاڤن ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->n_اوڠکاڤن, pE);
          أرجع 1;
        }
      }وإلا{
        iCol = resolveAsName(pParse, pEList, pE);
        إذا( iCol==0 ){
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
          pDup = sqlite3_اوڠکاڤن_Dup(db, pE, 0);
          إذا( !db->mallocFailed ){
            أكد(pDup);
            iCol = resolve_Order_By_Term_To_اوڠکاڤن_List(pParse, pSelect, pDup);
            إذا( IN_RENAME_OBJECT && iCol>0 ){
              resolve_Order_By_Term_To_اوڠکاڤن_List(pParse, pSelect, pE);
            }
          }
          sqlite3_اوڠکاڤن_Delete(db, pDup);
        }
      }
      إذا( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        إذا( !IN_RENAME_OBJECT ){
          اوڠکاڤن *pNew = sqlite3_اوڠکاڤن_Int32(db, iCol);
          إذا( pNew==0 ) أرجع 1;
          إذا( pItem->p_اوڠکاڤن==pE ){
            pItem->p_اوڠکاڤن = pNew;
          }وإلا{
            اوڠکاڤن *pParent = pItem->p_اوڠکاڤن;
            أكد( pParent->op==TK_COLLATE );
            طالما( pParent->p_کيري->op==TK_COLLATE ) pParent = pParent->p_کيري;
            أكد( pParent->p_کيري==pE );
            pParent->p_کيري = pNew;
          }
          sqlite3_اوڠکاڤن_Delete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }وإلا{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  لكل(i=0; i<pOrderBy->n_اوڠکاڤن; i++){
    إذا( pOrderBy->a[i].fg.done==0 ){
      sqlite3_Error_ڤسنن(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      أرجع 1;
    }
  }
  أرجع 0;
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
صحيح sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  اوڠکاڤن_List *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  ثابت حرف *zType     /* "ORDER" or "GROUP" */
){
  صحيح i;
  sqlite3 *db = pParse->db;
  اوڠکاڤن_List *pEList;
  بنية اوڠکاڤن_List_item *pItem;

  إذا( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) أرجع 0;
  إذا( pOrderBy->n_اوڠکاڤن>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_ڤسنن(pParse, "too many terms in %s BY clause", zType);
    أرجع 1;
  }
  pEList = pSelect->pEList;
  أكد( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  لكل(i=0, pItem=pOrderBy->a; i<pOrderBy->n_اوڠکاڤن; i++, pItem++){
    إذا( pItem->u.x.iOrderByCol ){
      إذا( pItem->u.x.iOrderByCol>pEList->n_اوڠکاڤن ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->n_اوڠکاڤن, 0);
        أرجع 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->p_اوڠکاڤن,0);
    }
  }
  أرجع 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
ساكن صحيح resolveRemoveWindowsCb(Walker *pWalker, اوڠکاڤن *p_اوڠکاڤن){
  UNUSED_PARAMETER(pWalker);
  إذا( اوڠکاڤن_Has_Property(p_اوڠکاڤن, EP_WinFunc) ){
    Window *pWin = p_اوڠکاڤن->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  أرجع WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
ساكن فراغ window_Remove_اوڠکاڤن_From_Select(Select *pSelect, اوڠکاڤن *p_اوڠکاڤن){
  إذا( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, حجم(Walker));
    sWalker.x_اوڠکاڤن_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_اوڠکاڤن(&sWalker, p_اوڠکاڤن);
  }
}
#وإلا
# define window_Remove_اوڠکاڤن_From_Select(a, b)
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
ساكن صحيح resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  اوڠکاڤن_List *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  ثابت حرف *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  صحيح i, j;                      /* Loop counters */
  صحيح iCol;                      /* Column number */
  بنية اوڠکاڤن_List_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  صحيح nResult;                   /* Number of terms in the result set */

  أكد( pOrderBy!=0 );
  nResult = pSelect->pEList->n_اوڠکاڤن;
  pParse = pNC->pParse;
  لكل(i=0, pItem=pOrderBy->a; i<pOrderBy->n_اوڠکاڤن; i++, pItem++){
    اوڠکاڤن *pE = pItem->p_اوڠکاڤن;
    اوڠکاڤن *pE2 = sqlite3_اوڠکاڤن_Skip_Collate_And_Likely(pE);
    إذا( NEVER(pE2==0) ) استمر;
    إذا( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      إذا( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        استمر;
      }
    }
    إذا( sqlite3_اوڠکاڤن_Is_Integer(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      إذا( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult, pE2);
        أرجع 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      استمر;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    إذا( sqlite3_Resolve_اوڠکاڤن_Names(pNC, pE) ){
      أرجع 1;
    }
    لكل(j=0; j<pSelect->pEList->n_اوڠکاڤن; j++){
      إذا( sqlite3_اوڠکاڤن_Compare(0, pE, pSelect->pEList->a[j].p_اوڠکاڤن, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        window_Remove_اوڠکاڤن_From_Select(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  أرجع sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
ساكن صحيح resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  صحيح isCompound;         /* True if p is a compound select */
  صحيح nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  صحيح i;                  /* Loop counter */
  اوڠکاڤن_List *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  أكد( p!=0 );
  إذا( p->selFlags & SF_Resolved ){
    أرجع WRC_Prune;
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
  إذا( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    أرجع pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  طالما( p ){
    أكد( (p->selFlags & SF_Expanded)!=0 );
    أكد( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, حجم(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    إذا( sqlite3_Resolve_اوڠکاڤن_Names(&sNC, p->pLimit) ){
      أرجع WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    إذا( p->selFlags & SF_Converted ){
      Select *pSub;
      أكد( p->pSrc->a[0].fg.isSubquery );
      أكد( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      أكد( pSub!=0 );
      أكد( p->pSrc->nSrc==1 && p->pOrderBy );
      أكد( pSub->pPrior && pSub->pOrderBy==0 );
      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    إذا( pOuterNC ) pOuterNC->nNestedSelect++;
    لكل(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      أكد( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      إذا( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        صحيح nRef = pOuterNC ? pOuterNC->nRef : 0;
        ثابت حرف *zSavedContext = pParse->zAuthContext;

        إذا( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        إذا( pParse->nErr ) أرجع WRC_Abort;
        أكد( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        إذا( pOuterNC ){
          أكد( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    إذا( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
 
    /* Resolve names in the result set. */
    إذا( sqlite3_Resolve_اوڠکاڤن_List_Names(&sNC, p->pEList) ) أرجع WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    أكد( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    إذا( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      أكد( NC_MinMaxAgg==SF_MinMaxAgg );
      أكد( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }وإلا{
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
    أكد( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    إذا( p->pHaving ){
      إذا( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_ڤسنن(pParse, "HAVING clause on a non-aggregate query");
        أرجع WRC_Abort;
      }
      إذا( sqlite3_Resolve_اوڠکاڤن_Names(&sNC, p->pHaving) ) أرجع WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    إذا( sqlite3_Resolve_اوڠکاڤن_Names(&sNC, p->pWhere) ) أرجع WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    لكل(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      إذا( pItem->fg.isTabFunc
       && sqlite3_Resolve_اوڠکاڤن_List_Names(&sNC, pItem->u1.pFuncArg)
      ){
        أرجع WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    إذا( IN_RENAME_OBJECT ){
      Window *pWin;
      لكل(pWin=p->pWinDefn; pWin; pWin=pWin->pNextWin){
        إذا( sqlite3_Resolve_اوڠکاڤن_List_Names(&sNC, pWin->pOrderBy)
         || sqlite3_Resolve_اوڠکاڤن_List_Names(&sNC, pWin->pPartition)
        ){
          أرجع WRC_Abort;
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
    إذا( p->selFlags & SF_Converted ){
      Select *pSub;
      أكد( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      أكد( pSub!=0 );
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
    إذا( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      أرجع WRC_Abort;
    }
    إذا( db->mallocFailed ){
      أرجع WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    إذا( pGroupBy ){
      بنية اوڠکاڤن_List_item *pItem;
   
      إذا( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        أرجع WRC_Abort;
      }
      لكل(i=0, pItem=pGroupBy->a; i<pGroupBy->n_اوڠکاڤن; i++, pItem++){
        إذا( اوڠکاڤن_Has_Property(pItem->p_اوڠکاڤن, EP_Agg) ){
          sqlite3_Error_ڤسنن(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          أرجع WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    إذا( p->pNext && p->pEList->n_اوڠکاڤن!=p->pNext->pEList->n_اوڠکاڤن ){
      sqlite3_Select_Wrong_اڠک_Terms_Error(pParse, p->pNext);
      أرجع WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    إذا( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      إذا( pParse->nErr ) أرجع WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  إذا( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    أرجع WRC_Abort;
  }

  أرجع WRC_Prune;
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
صحيح sqlite3_Resolve_اوڠکاڤن_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  اوڠکاڤن *p_اوڠکاڤن             /* The expression to be analyzed. */
){
  صحيح savedHasAgg;
  Walker w;

  إذا( p_اوڠکاڤن==0 ) أرجع SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.x_اوڠکاڤن_Callback = resolve_اوڠکاڤن_Step;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
  w.pParse->nHeight += p_اوڠکاڤن->nHeight;
  إذا( sqlite3_اوڠکاڤن_Check_Height(w.pParse, w.pParse->nHeight) ){
    أرجع SQLITE_ERROR;
  }
#endif
  أكد( p_اوڠکاڤن!=0 );
  sqlite3_Walk_اوڠکاڤن_NN(&w, p_اوڠکاڤن);
#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
  w.pParse->nHeight -= p_اوڠکاڤن->nHeight;
#endif
  أكد( EP_Agg==NC_HasAgg );
  أكد( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  اوڠکاڤن_Set_Property(p_اوڠکاڤن, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  أرجع pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
صحيح sqlite3_Resolve_اوڠکاڤن_List_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  اوڠکاڤن_List *pList         /* The expression list to be analyzed. */
){
  صحيح i;
  صحيح savedHasAgg = 0;
  Walker w;
  إذا( pList==0 ) أرجع SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_اوڠکاڤن_Callback = resolve_اوڠکاڤن_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  لكل(i=0; i<pList->n_اوڠکاڤن; i++){
    اوڠکاڤن *p_اوڠکاڤن = pList->a[i].p_اوڠکاڤن;
    إذا( p_اوڠکاڤن==0 ) استمر;
#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
    w.pParse->nHeight += p_اوڠکاڤن->nHeight;
    إذا( sqlite3_اوڠکاڤن_Check_Height(w.pParse, w.pParse->nHeight) ){
      أرجع SQLITE_ERROR;
    }
#endif
    sqlite3_Walk_اوڠکاڤن_NN(&w, p_اوڠکاڤن);
#إذا SQLITE_MAX_اوڠکاڤن_DEPTH>0
    w.pParse->nHeight -= p_اوڠکاڤن->nHeight;
#endif
    أكد( EP_Agg==NC_HasAgg );
    أكد( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    إذا( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      اوڠکاڤن_Set_Property(p_اوڠکاڤن, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    إذا( w.pParse->nErr>0 ) أرجع SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  أرجع SQLITE_OK;
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
فراغ sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  أكد( p!=0 );
  w.x_اوڠکاڤن_Callback = resolve_اوڠکاڤن_Step;
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
صحيح sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  صحيح type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  اوڠکاڤن *p_اوڠکاڤن,     /* Expression to resolve.  May be NULL. */
  اوڠکاڤن_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  صحيح rc;
  اتحاد {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  أكد( type==0 || pTab!=0 );
  أكد( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_اوڠکاڤن
          || type==NC_GenCol || pTab==0 );
  memset(&sNC, 0, حجم(sNC));
  memset(&uSrc, 0, حجم(uSrc));
  pSrc = &uSrc.sSrc;
  إذا( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    إذا( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  إذا( (rc = sqlite3_Resolve_اوڠکاڤن_Names(&sNC, p_اوڠکاڤن))!=SQLITE_OK ) أرجع rc;
  إذا( pList ) rc = sqlite3_Resolve_اوڠکاڤن_List_Names(&sNC, pList);
  أرجع rc;
}
