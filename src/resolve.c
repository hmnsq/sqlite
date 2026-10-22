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
#अन्तर्भाव "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#define EXCLUDED_TABLE_संख्या  2

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
स्थिर पूर्णाङ्क incrAggDepth(Walker *pWalker, व्यंजक *p_व्यंजक){
  यदि( p_व्यंजक->op==TK_AGG_FUNCTION ) p_व्यंजक->op2 += pWalker->u.n;
  प्रत्यागम WRC_Continue;
}
स्थिर शून्य incrAggFunctionDepth(व्यंजक *p_व्यंजक, पूर्णाङ्क N){
  यदि( N>0 ){
    Walker w;
    स_रखो(&w, 0, परिमाण(w));
    w.x_व्यंजक_Callback = incrAggDepth;
    w.u.n = N;
    sqlite3_Walk_व्यंजक(&w, p_व्यंजक);
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
स्थिर शून्य resolveAlias(
  Parse *pParse,         /* Parsing context */
  व्यंजक_List *pEList,      /* A result set */
  पूर्णाङ्क iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  व्यंजक *p_व्यंजक,           /* Transform this into an alias to the result set */
  पूर्णाङ्क nSubquery          /* Number of subqueries that the label is moving */
){
  व्यंजक *pOrig;           /* The iCol-th column of the result set */
  व्यंजक *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  प्रतिज्ञा( iCol>=0 && iCol<pEList->n_व्यंजक );
  pOrig = pEList->a[iCol].p_व्यंजक;
  प्रतिज्ञा( pOrig!=0 );
  प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Reduced|EP_TokenOnly) );
  यदि( p_व्यंजक->pAggInfo ) प्रत्यागम;
  db = pParse->db;
  pDup = sqlite3_व्यंजक_Dup(db, pOrig, 0);
  यदि( db->mallocFailed ){
    sqlite3_व्यंजक_Delete(db, pDup);
    pDup = 0;
  }अन्यथा{
    व्यंजक temp;
    incrAggFunctionDepth(pDup, nSubquery);
    यदि( p_व्यंजक->op==TK_COLLATE ){
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      pDup = sqlite3_व्यंजक_Add_Collate_String(pParse, pDup, p_व्यंजक->u.zToken);
    }
    स_नकल(&temp, pDup, परिमाण(व्यंजक));
    स_नकल(pDup, p_व्यंजक, परिमाण(व्यंजक));
    स_नकल(p_व्यंजक, &temp, परिमाण(व्यंजक));
    यदि( व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) ){
      यदि( ALWAYS(p_व्यंजक->y.pWin!=0) ){
        p_व्यंजक->y.pWin->pOwner = p_व्यंजक;
      }
    }
    sqlite3_व्यंजक_Deferred_Delete(pParse, pDup);
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
पूर्णाङ्क sqlite3MatchEName(
  ध्रुव संरचना व्यंजक_List_item *pItem,
  ध्रुव वर्ण *zCol,
  ध्रुव वर्ण *zTab,
  ध्रुव वर्ण *zDb,
  पूर्णाङ्क *pbRowid
){
  पूर्णाङ्क n;
  ध्रुव वर्ण *zSpan;
  पूर्णाङ्क eEName = pItem->fg.eEName;
  यदि( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    प्रत्यागम 0;
  }
  प्रतिज्ञा( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  प्रति(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  यदि( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    प्रत्यागम 0;
  }
  zSpan += n+1;
  प्रति(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  यदि( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    प्रत्यागम 0;
  }
  zSpan += n+1;
  यदि( zCol ){
    यदि( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) प्रत्यागम 0;
    यदि( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) प्रत्यागम 0;
  }
  यदि( eEName==ENAME_ROWID ) *pbRowid = 1;
  प्रत्यागम 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
स्थिर पूर्णाङ्क areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *p_शीर्ष_NC){
  यदि( db->init.busy ) प्रत्यागम 1;  /* Always support for legacy schemas */
  यदि( p_शीर्ष_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    यदि( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      प्रत्यागम 1;
    }
    प्रत्यागम (db->flags & SQLITE_DqsDDL)!=0;
  }अन्यथा{
    /* Currently parsing a DML statement */
    प्रत्यागम (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_व्यंजक_Col_Used(व्यंजक *p_व्यंजक){
  पूर्णाङ्क n;
  Table *pExTab;

  n = p_व्यंजक->iColumn;
  प्रतिज्ञा( व्यंजक_Use_YTab(p_व्यंजक) );
  pExTab = p_व्यंजक->y.pTab;
  प्रतिज्ञा( pExTab!=0 );
  प्रतिज्ञा( n < pExTab->nCol );
  यदि( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    प्रत्यागम pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }अन्यथा{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    यदि( n>=BMS ) n = BMS-1;
    प्रत्यागम ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
स्थिर शून्य extendFJMatch(
  Parse *pParse,          /* Parsing context */
  व्यंजक_List **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  व्यंजक *pNew = sqlite3_व्यंजक_Alloc(pParse->db, TK_COLUMN, 0, 0);
  यदि( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    प्रतिज्ञा( (pMatch->fg.jointype & (JT_बायाँ|JT_LTORJ))!=0 );
    व्यंजक_Set_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_व्यंजक_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
स्थिर SQLITE_NOINLINE पूर्णाङ्क isValidSchemaTableName(
  ध्रुव वर्ण *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  ध्रुव वर्ण *zDb           /* non-NULL if a database qualifier is present */
){
  ध्रुव वर्ण *zLegacy;
  प्रतिज्ञा( pTab!=0 );
  प्रतिज्ञा( pTab->tnum==1 );
  यदि( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) प्रत्यागम 0;
  zLegacy = pTab->zName;
  यदि( म_भेद(zLegacy+7, &LEGACY_TEMP_SCHEMA_TABLE[7])==0 ){
    यदि( sqlite3StrICmp(zTab+7, &PREFERRED_TEMP_SCHEMA_TABLE[7])==0 ){
      प्रत्यागम 1;
    }
    यदि( zDb==0 ) प्रत्यागम 0;
    यदि( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) प्रत्यागम 1;
    यदि( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) प्रत्यागम 1;
  }अन्यथा{
    यदि( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) प्रत्यागम 1;
  }
  प्रत्यागम 0;
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
स्थिर पूर्णाङ्क lookupName(
  Parse *pParse,       /* The parsing context */
  ध्रुव वर्ण *zDb,     /* Name of the database containing table, or NULL */
  ध्रुव वर्ण *zTab,    /* Name of table containing column, or NULL */
  ध्रुव व्यंजक *p_दायाँ,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  व्यंजक *p_व्यंजक          /* Make this EXPR node point to the selected column */
){
  पूर्णाङ्क i, j;                         /* Loop counters */
  पूर्णाङ्क cnt = 0;                      /* Number of matching column names */
  पूर्णाङ्क cntTab = 0;                   /* Number of potential "rowid" matches */
  पूर्णाङ्क nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *p_शीर्ष_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  पूर्णाङ्क e_New_व्यंजक_Op = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  व्यंजक_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  ध्रुव वर्ण *zCol = p_दायाँ->u.zToken;

  प्रतिज्ञा( pNC );     /* the name context cannot be NULL. */
  प्रतिज्ञा( zCol );    /* The Z in X.Y.Z cannot be NULL */
  प्रतिज्ञा( zDb==0 || zTab!=0 );
  प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_व्यंजक->iTable = -1;
  व्यंजक_Set_VVAProperty(p_व्यंजक, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  यदि( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    यदि( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }अन्यथा{
      प्रति(i=0; i<db->nDb; i++){
        प्रतिज्ञा( db->aDb[i].zDbSName );
        यदि( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          भङ्ग;
        }
      }
      यदि( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  प्रतिज्ञा( pNC && cnt==0 );
  कुरु{
    व्यंजक_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    यदि( pSrcList ){
      प्रति(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        प्रतिज्ञा( pTab!=0 && pTab->zName!=0 );
        प्रतिज्ञा( pTab->nCol>0 || pParse->nErr );
        प्रतिज्ञा( (पूर्णाङ्क)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        यदि( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          पूर्णाङ्क hit = 0;
          Select *pSel;
          प्रतिज्ञा( pItem->fg.isSubquery );
          प्रतिज्ञा( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          प्रतिज्ञा( pSel!=0 );
          pEList = pSel->pEList;
          प्रतिज्ञा( pEList!=0 );
          प्रतिज्ञा( pEList->n_व्यंजक==pTab->nCol );
          प्रति(j=0; j<pEList->n_व्यंजक; j++){
            पूर्णाङ्क bRowid = 0;       /* True if possible rowid match */
            यदि( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              अनुवर्त;
            }
            यदि( bRowid==0 ){
              यदि( cnt>0 ){
                यदि( pItem->fg.isUsing==0
                 || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_व्यंजक_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }अन्यथा
                यदि( (pItem->fg.jointype & JT_दायाँ)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  अनुवर्त;
                }अन्यथा
                यदि( (pItem->fg.jointype & JT_बायाँ)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_व्यंजक_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }अन्यथा{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, p_व्यंजक->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }अन्यथा यदि( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              अनुवर्त;
            }
            cntTab++;
            pMatch = pItem;
            p_व्यंजक->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            प्रतिज्ञा( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            यदि( pEList->a[j].fg.bUsingTerm ) भङ्ग;
          }
          यदि( hit || zTab==0 ) अनुवर्त;
        }
        प्रतिज्ञा( zDb==0 || zTab!=0 );
        यदि( zTab ){
          यदि( zDb ){
            यदि( pTab->pSchema!=pSchema ) अनुवर्त;
            यदि( pSchema==0 && म_भेद(zDb,"*")!=0 ) अनुवर्त;
          }
          यदि( pItem->zAlias!=0 ){
            यदि( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              अनुवर्त;
            }
          }अन्यथा यदि( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            यदि( pTab->tnum!=1 ) अनुवर्त;
            यदि( !isValidSchemaTableName(zTab, pTab, zDb) ) अनुवर्त;
          }
          प्रतिज्ञा( व्यंजक_Use_YTab(p_व्यंजक) );
          यदि( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (शून्य*)&p_व्यंजक->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        यदि( j>=0 ){
          यदि( cnt>0 ){
            यदि( pItem->fg.isUsing==0
             || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_व्यंजक_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }अन्यथा
            यदि( (pItem->fg.jointype & JT_दायाँ)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              अनुवर्त;
            }अन्यथा
            यदि( (pItem->fg.jointype & JT_बायाँ)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_व्यंजक_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }अन्यथा{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, p_व्यंजक->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_व्यंजक->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          यदि( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        यदि( 0==cnt && VisibleRowid(pTab) ){
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
          यदि( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }अन्यथा{
            cntTab++;
          }
#अन्यथा
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      यदि( pMatch ){
        p_व्यंजक->iTable = pMatch->iCursor;
        प्रतिज्ञा( व्यंजक_Use_YTab(p_व्यंजक) );
        p_व्यंजक->y.pTab = pMatch->pSTab;
        यदि( (pMatch->fg.jointype & (JT_बायाँ|JT_LTORJ))!=0 ){
          व्यंजक_Set_Property(p_व्यंजक, EP_CanBeNull);
        }
        pSchema = p_व्यंजक->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#यदि !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    यदि( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      यदि( pParse->pTriggerTab!=0 ){
        पूर्णाङ्क op = pParse->eTriggerOp;
        प्रतिज्ञा( op==TK_DELETE || op==TK_UPDATE || op==TK_जोड़ो );
        यदि( pParse->bReturning ){
          यदि( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            p_व्यंजक->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }अन्यथा यदि( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_व्यंजक->iTable = 1;
          pTab = pParse->pTriggerTab;
        }अन्यथा यदि( op!=TK_जोड़ो && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_व्यंजक->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      यदि( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        यदि( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_व्यंजक->iTable = EXCLUDED_TABLE_संख्या;
          यदि( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (शून्य*)&p_व्यंजक->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      यदि( pTab ){
        पूर्णाङ्क iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        यदि( iCol>=0 ){
          यदि( pTab->iPKey==iCol ) iCol = -1;
        }अन्यथा{
          यदि( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }अन्यथा{
            iCol = pTab->nCol;
          }
        }
        यदि( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          यदि( p_व्यंजक->iTable==EXCLUDED_TABLE_संख्या ){
            testcase( iCol==(-1) );
            प्रतिज्ञा( व्यंजक_Use_YTab(p_व्यंजक) );
            यदि( IN_RENAME_OBJECT ){
              p_व्यंजक->iColumn = iCol;
              p_व्यंजक->y.pTab = pTab;
              e_New_व्यंजक_Op = TK_COLUMN;
            }अन्यथा{
              p_व्यंजक->iTable = pNC->uNC.pUpsert->regData +
                 sqlite3TableColumnToStorage(pTab, iCol);
              e_New_व्यंजक_Op = TK_REGISTER;
            }
          }अन्यथा
#endif /* SQLITE_OMIT_UPSERT */
          {
            प्रतिज्ञा( व्यंजक_Use_YTab(p_व्यंजक) );
            p_व्यंजक->y.pTab = pTab;
            यदि( pParse->bReturning ){
              e_New_व्यंजक_Op = TK_REGISTER;
              p_व्यंजक->op2 = TK_COLUMN;
              p_व्यंजक->iColumn = iCol;
              p_व्यंजक->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*p_व्यंजक->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }अन्यथा{
              p_व्यंजक->iColumn = (i16)iCol;
              e_New_व्यंजक_Op = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              यदि( iCol<0 ){
                p_व्यंजक->aff_व्यंजक = SQLITE_AFF_INTEGER;
              }अन्यथा यदि( p_व्यंजक->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }अन्यथा{
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
    यदि( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_Idx_व्यंजक|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#यदि SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      यदि( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        e_New_व्यंजक_Op = TK_NULL;
      }
#endif
      यदि( pMatch->fg.isNestedFrom==0 ) p_व्यंजक->iColumn = -1;
      p_व्यंजक->aff_व्यंजक = SQLITE_AFF_INTEGER;
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
    यदि( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      प्रतिज्ञा( pEList!=0 );
      प्रति(j=0; j<pEList->n_व्यंजक; j++){
        वर्ण *zAs = pEList->a[j].zEName;
        यदि( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          व्यंजक *pOrig;
          प्रतिज्ञा( p_व्यंजक->p_बायाँ==0 && p_व्यंजक->p_दायाँ==0 );
          प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक)==0 || p_व्यंजक->x.pList==0 );
          प्रतिज्ञा( व्यंजक_Use_XSelect(p_व्यंजक)==0 || p_व्यंजक->x.pSelect==0 );
          pOrig = pEList->a[j].p_व्यंजक;
          यदि( (pNC->ncFlags&NC_AllowAgg)==0 && व्यंजक_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_संदेश(pParse, "misuse of aliased aggregate %s", zAs);
            प्रत्यागम WRC_Abort;
          }
          यदि( व्यंजक_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_शीर्ष_NC )
          ){
            sqlite3_Error_संदेश(pParse, "misuse of aliased window function %s",zAs);
            प्रत्यागम WRC_Abort;
          }
          यदि( sqlite3_व्यंजक_Vector_Size(pOrig)!=1 ){
            sqlite3_Error_संदेश(pParse, "row value misused");
            प्रत्यागम WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_व्यंजक, nSubquery);
          cnt = 1;
          pMatch = 0;
          प्रतिज्ञा( zTab==0 && zDb==0 );
          यदि( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (शून्य*)p_व्यंजक);
          }
          गच्छ lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    यदि( cnt ) भङ्ग;
    pNC = pNC->pNext;
    nSubquery++;
  }यावत्( pNC );


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
  यदि( cnt==0 && zTab==0 ){
    प्रतिज्ञा( p_व्यंजक->op==TK_ID );
    यदि( व्यंजक_Has_Property(p_व्यंजक,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, p_शीर्ष_NC)
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
      p_व्यंजक->op = TK_STRING;
      स_रखो(&p_व्यंजक->y, 0, परिमाण(p_व्यंजक->y));
      प्रत्यागम WRC_Prune;
    }
    यदि( sqlite3_व्यंजक_Id_To_True_False(p_व्यंजक) ){
      प्रत्यागम WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  प्रतिज्ञा( pFJMatch==0 || cnt>0 );
  प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_xIsSelect|EP_Int_मूल्य) );
  यदि( cnt!=1 ){
    ध्रुव वर्ण *zErr;
    यदि( pFJMatch ){
      यदि( pFJMatch->n_व्यंजक==cnt-1 ){
        यदि( व्यंजक_Has_Property(p_व्यंजक,EP_Leaf) ){
          व्यंजक_Clear_Property(p_व्यंजक,EP_Leaf);
        }अन्यथा{
          sqlite3_व्यंजक_Delete(db, p_व्यंजक->p_बायाँ);
          p_व्यंजक->p_बायाँ = 0;
          sqlite3_व्यंजक_Delete(db, p_व्यंजक->p_दायाँ);
          p_व्यंजक->p_दायाँ = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, p_व्यंजक->iColumn);
        p_व्यंजक->op = TK_FUNCTION;
        p_व्यंजक->u.zToken = "coalesce";
        p_व्यंजक->x.pList = pFJMatch;
        p_व्यंजक->aff_व्यंजक = SQLITE_AFF_DEFER;
        cnt = 1;
        गच्छ lookupname_end;
      }अन्यथा{
        sqlite3_व्यंजक_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    यदि( zDb ){
      sqlite3_Error_संदेश(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }अन्यथा यदि( zTab ){
      sqlite3_Error_संदेश(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }अन्यथा यदि( cnt==0 && व्यंजक_Has_Property(p_दायाँ,EP_DblQuoted) ){
      sqlite3_Error_संदेश(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }अन्यथा{
      sqlite3_Error_संदेश(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3_Record_Error_Offset_Of_व्यंजक(pParse->db, p_व्यंजक);
    pParse->checkSchema = 1;
    p_शीर्ष_NC->nNcErr++;
    e_New_व्यंजक_Op = TK_NULL;
  }
  प्रतिज्ञा( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  यदि( !व्यंजक_Has_Property(p_व्यंजक,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3_व्यंजक_Delete(db, p_व्यंजक->p_बायाँ);
    p_व्यंजक->p_बायाँ = 0;
    sqlite3_व्यंजक_Delete(db, p_व्यंजक->p_दायाँ);
    p_व्यंजक->p_दायाँ = 0;
    व्यंजक_Set_Property(p_व्यंजक, EP_Leaf);
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
  यदि( pMatch ){
    यदि( p_व्यंजक->iColumn>=0 ){
      pMatch->colUsed |= sqlite3_व्यंजक_Col_Used(p_व्यंजक);
    }अन्यथा{
      pMatch->fg.rowidUsed = 1;
    }
  }

  p_व्यंजक->op = e_New_व्यंजक_Op;
lookupname_end:
  यदि( cnt==1 ){
    प्रतिज्ञा( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    यदि( db->xAuth ){
      यदि( pFJMatch ){
        प्रतिज्ञा( p_व्यंजक->op==TK_FUNCTION );
        प्रतिज्ञा( sqlite3_stricmp(p_व्यंजक->u.zToken,"coalesce")==0 );
        प्रतिज्ञा( p_व्यंजक->x.pList==pFJMatch );
        प्रतिज्ञा( pFJMatch->n_व्यंजक>0 );
        प्रति(i=0; i<pFJMatch->n_व्यंजक; i++){
          प्रतिज्ञा( pFJMatch->a[i].p_व्यंजक->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_व्यंजक,pSchema,pNC->pSrcList);
        }
      }अन्यथा यदि( p_व्यंजक->op==TK_COLUMN || p_व्यंजक->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_व्यंजक, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    प्रति(;;){
      प्रतिज्ञा( p_शीर्ष_NC!=0 );
      p_शीर्ष_NC->nRef++;
      यदि( p_शीर्ष_NC==pNC ) भङ्ग;
      p_शीर्ष_NC = p_शीर्ष_NC->pNext;
    }
    प्रत्यागम WRC_Prune;
  } अन्यथा {
    प्रत्यागम WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
व्यंजक *sqlite3_Create_Column_व्यंजक(sqlite3 *db, SrcList *pSrc, पूर्णाङ्क iSrc, पूर्णाङ्क iCol){
  व्यंजक *p = sqlite3_व्यंजक_Alloc(db, TK_COLUMN, 0, 0);
  यदि( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    प्रतिज्ञा( व्यंजक_Use_YTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    यदि( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }अन्यथा{
      p->iColumn = (ynVar)iCol;
      यदि( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }अन्यथा{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  प्रत्यागम p;
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
स्थिर शून्य notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   ध्रुव वर्ण *z_संदेश,    /* Type of error */
   व्यंजक *p_व्यंजक,         /* Invalidate this expression on error */
   व्यंजक *pError         /* Associate error with this expression */
){
  ध्रुव वर्ण *zIn = "partial index WHERE clauses";
  यदि( pNC->ncFlags & NC_Idx_व्यंजक )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  अन्यथा यदि( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  अन्यथा यदि( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3_Error_संदेश(pParse, "%s prohibited in %s", z_संदेश, zIn);
  यदि( p_व्यंजक ) p_व्यंजक->op = TK_NULL;
  sqlite3_Record_Error_Offset_Of_व्यंजक(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  प्रतिज्ञा( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_व्यंजक|NC_GenCol))==0 ); \
  यदि( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
स्थिर पूर्णाङ्क व्यंजक_Probability(व्यंजक *p){
  द्विगुण r = -1.0;
  यदि( p->op!=TK_FLOAT ) प्रत्यागम -1;
  प्रतिज्ञा( !व्यंजक_Has_Property(p, EP_Int_मूल्य) );
  sqlite3AtoF(p->u.zToken, &r);
  प्रतिज्ञा( r>=0.0 );
  यदि( r>1.0 ) प्रत्यागम -1;
  प्रत्यागम (पूर्णाङ्क)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
स्थिर SQLITE_NOINLINE शून्य resolve_Set_व्यंजक_Subtype_Arg(व्यंजक_List *pList){
  पूर्णाङ्क nn, ii;
  nn = pList ? pList->n_व्यंजक : 0;
  प्रति(ii=0; ii<nn; ii++){
    व्यंजक *p_व्यंजक = pList->a[ii].p_व्यंजक;
    यावत्( 1 /*exit-by-break*/ ){
      व्यंजक_Set_Property(p_व्यंजक, EP_SubtArg);
      यदि( p_व्यंजक->op==TK_SELECT ){
        प्रतिज्ञा( व्यंजक_Use_XSelect(p_व्यंजक) );
        प्रतिज्ञा( p_व्यंजक->x.pSelect!=0 );
        resolve_Set_व्यंजक_Subtype_Arg(p_व्यंजक->x.pSelect->pEList);
        भङ्ग;
      }
      यदि( p_व्यंजक->op==TK_UPLUS ){
        p_व्यंजक = p_व्यंजक->p_बायाँ;
        प्रतिज्ञा( p_व्यंजक!=0 );
      }अन्यथा{
        भङ्ग;
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
स्थिर पूर्णाङ्क resolve_व्यंजक_Step(Walker *pWalker, व्यंजक *p_व्यंजक){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  प्रतिज्ञा( pNC!=0 );
  pParse = pNC->pParse;
  प्रतिज्ञा( pParse==pWalker->pParse );

#ifndef न_संशोधन
  यदि( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    पूर्णाङ्क i;
    प्रति(i=0; i<pNC->pSrcList->nSrc; i++){
      प्रतिज्ञा( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  विकल्प( p_व्यंजक->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    पक्ष TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      प्रतिज्ञा( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      p_व्यंजक->op = TK_COLUMN;
      प्रतिज्ञा( व्यंजक_Use_YTab(p_व्यंजक) );
      p_व्यंजक->y.pTab = pItem->pSTab;
      p_व्यंजक->iTable = pItem->iCursor;
      p_व्यंजक->iColumn--;
      p_व्यंजक->aff_व्यंजक = SQLITE_AFF_INTEGER;
      भङ्ग;
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
    पक्ष TK_NOTNULL:
    पक्ष TK_ISNULL: {
      पूर्णाङ्क anRef[8];
      NameContext *p;
      पूर्णाङ्क i;
      प्रति(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_व्यंजक(pWalker, p_व्यंजक->p_बायाँ);
      यदि( IN_RENAME_OBJECT ) प्रत्यागम WRC_Prune;
      यदि( sqlite3_व्यंजक_Can_Be_Null(p_व्यंजक->p_बायाँ) ){
        /* The expression can be NULL.  So the optimization does not apply */
        प्रत्यागम WRC_Prune;
      }

      प्रति(i=0, p=pNC; p; p=p->pNext, i++){
        यदि( (p->ncFlags & NC_Where)==0 ){
          प्रत्यागम WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) );
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
#यदि TREETRACE_ENABLED
      यदि( sqlite3_वृक्ष_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_व्यंजक->op==TK_NOTNULL
        );
        sqlite3_Show_व्यंजक(p_व्यंजक);
      }
#endif /* TREETRACE_ENABLED */
      p_व्यंजक->u.i_मूल्य = (p_व्यंजक->op==TK_NOTNULL);
      p_व्यंजक->flags |= EP_Int_मूल्य;
      p_व्यंजक->op = TK_INTEGER;
      प्रति(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        p->nRef = anRef[i];
      }
      sqlite3_व्यंजक_Delete(pParse->db, p_व्यंजक->p_बायाँ);
      p_व्यंजक->p_बायाँ = 0;
      प्रत्यागम WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    पक्ष TK_ID:
    पक्ष TK_DOT: {
      ध्रुव वर्ण *zTable;
      ध्रुव वर्ण *zDb;
      व्यंजक *p_दायाँ;

      यदि( p_व्यंजक->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
        p_दायाँ = p_व्यंजक;
      }अन्यथा{
        व्यंजक *p_बायाँ = p_व्यंजक->p_बायाँ;
        testcase( pNC->ncFlags & NC_Idx_व्यंजक );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_व्यंजक|NC_GenCol, 0, p_व्यंजक);
        p_दायाँ = p_व्यंजक->p_दायाँ;
        यदि( p_दायाँ->op==TK_ID ){
          zDb = 0;
        }अन्यथा{
          प्रतिज्ञा( p_दायाँ->op==TK_DOT );
          प्रतिज्ञा( !व्यंजक_Has_Property(p_दायाँ, EP_Int_मूल्य) );
          zDb = p_बायाँ->u.zToken;
          p_बायाँ = p_दायाँ->p_बायाँ;
          p_दायाँ = p_दायाँ->p_दायाँ;
        }
        प्रतिज्ञा( व्यंजक_Use_UToken(p_बायाँ) && व्यंजक_Use_UToken(p_दायाँ) );
        zTable = p_बायाँ->u.zToken;
        प्रतिज्ञा( व्यंजक_Use_YTab(p_व्यंजक) );
        यदि( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (शून्य*)p_व्यंजक, (शून्य*)p_दायाँ);
          sqlite3RenameTokenRemap(pParse, (शून्य*)&p_व्यंजक->y.pTab, (शून्य*)p_बायाँ);
        }
      }
      प्रत्यागम lookupName(pParse, zDb, zTable, p_दायाँ, pNC, p_व्यंजक);
    }

    /* Resolve function names
    */
    पक्ष TK_FUNCTION: {
      व्यंजक_List *pList;            /* The argument list */
      पूर्णाङ्क n;                      /* Number of arguments */
      पूर्णाङ्क no_such_func = 0;       /* True if no such function exists */
      पूर्णाङ्क wrong_अंक_args = 0;     /* True if wrong number of arguments */
      पूर्णाङ्क is_agg = 0;             /* True if is an aggregate function */
      ध्रुव वर्ण *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      पूर्णाङ्क savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(p_व्यंजक) ? p_व्यंजक->y.pWin : 0);
#endif
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_xIsSelect|EP_Int_मूल्य) );
      प्रतिज्ञा( p_व्यंजक->p_बायाँ==0 || p_व्यंजक->p_बायाँ->op==TK_ORDER );
      pList = p_व्यंजक->x.pList;
      n = pList ? pList->n_व्यंजक : 0;
      zId = p_व्यंजक->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      यदि( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        यदि( pDef==0 ){
          no_such_func = 1;
        }अन्यथा{
          wrong_अंक_args = 1;
        }
      }अन्यथा{
        is_agg = pDef->xFinalize!=0;
        यदि( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          व्यंजक_Set_Property(p_व्यंजक, EP_Unlikely);
          यदि( n==2 ){
            p_व्यंजक->iTable = व्यंजक_Probability(pList->a[1].p_व्यंजक);
            यदि( p_व्यंजक->iTable<0 ){
              sqlite3_Error_संदेश(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_व्यंजक);
              pNC->nNcErr++;
            }
          }अन्यथा{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_व्यंजक->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          पूर्णाङ्क auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          यदि( auth!=SQLITE_OK ){
            यदि( auth==SQLITE_DENY ){
              sqlite3_Error_संदेश(pParse, "not authorized to use function: %#T",
                                      p_व्यंजक);
              pNC->nNcErr++;
            }
            p_व्यंजक->op = TK_NULL;
            प्रत्यागम WRC_Prune;
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
        यदि( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || व्यंजक_Has_Property(p_व्यंजक, EP_SubtArg) 
        ){
          resolve_Set_व्यंजक_Subtype_Arg(pList);
        }

        यदि( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          व्यंजक_Set_Property(p_व्यंजक,EP_ConstFunc);
        }
        यदि( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_व्यंजक|NC_PartIdx|NC_GenCol, 0, p_व्यंजक);
        }अन्यथा{
          प्रतिज्ञा( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_व्यंजक->op2 = pNC->ncFlags & NC_SelfRef;
        }
        यदि( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
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
        }अन्यथा
        यदि( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          यदि( pNC->ncFlags & NC_FromDDL ) व्यंजक_Set_Property(p_व्यंजक, EP_FromDDL);
          sqlite3_व्यंजक_Function_Usable(pParse, p_व्यंजक, pDef);
        }
      }

      यदि( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        प्रतिज्ञा( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_मूल्य==0 && pDef->xInverse==0)
          || (pDef->x_मूल्य && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        यदि( pDef && pDef->x_मूल्य==0 && pWin ){
          sqlite3_Error_संदेश(pParse,
              "%#T() may not be used as a window function", p_व्यंजक
          );
          pNC->nNcErr++;
        }अन्यथा यदि(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          ध्रुव वर्ण *zType;
          यदि( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }अन्यथा{
            zType = "aggregate";
          }
          sqlite3_Error_संदेश(pParse, "misuse of %s function %#T()",zType,p_व्यंजक);
          pNC->nNcErr++;
          is_agg = 0;
        }
#अन्यथा
        यदि( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_संदेश(pParse,"misuse of aggregate function %#T()",p_व्यंजक);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        अन्यथा यदि( no_such_func
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
          sqlite3_Error_संदेश(pParse, "no such function: %#T", p_व्यंजक);
          pNC->nNcErr++;
        }अन्यथा यदि( wrong_अंक_args ){
          sqlite3_Error_संदेश(pParse,"wrong number of arguments to function %#T()",
               p_व्यंजक);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        अन्यथा यदि( is_agg==0 && व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) ){
          sqlite3_Error_संदेश(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_व्यंजक
          );
          pNC->nNcErr++;
        }
#endif
        अन्यथा यदि( is_agg==0 && p_व्यंजक->p_बायाँ ){
          sqlite3_व्यंजक_Order_By_Aggregate_Error(pParse, p_व्यंजक);
          pNC->nNcErr++;
        }
        यदि( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#अन्यथा
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      अन्यथा यदि( व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) || p_व्यंजक->p_बायाँ ){
        is_agg = 1;
      }
      sqlite3_Walk_व्यंजक_List(pWalker, pList);
      यदि( is_agg ){
        यदि( p_व्यंजक->p_बायाँ ){
          प्रतिज्ञा( p_व्यंजक->p_बायाँ->op==TK_ORDER );
          प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक->p_बायाँ) );
          sqlite3_Walk_व्यंजक_List(pWalker, p_व्यंजक->p_बायाँ->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        यदि( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          प्रतिज्ञा( व्यंजक_Use_YWin(p_व्यंजक) && pWin==p_व्यंजक->y.pWin );
          यदि( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            यदि( pParse->db->mallocFailed ) भङ्ग;
          }
          sqlite3_Walk_व्यंजक_List(pWalker, pWin->pPartition);
          sqlite3_Walk_व्यंजक_List(pWalker, pWin->pOrderBy);
          sqlite3_Walk_व्यंजक(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }अन्यथा
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_व्यंजक->op = TK_AGG_FUNCTION;
          p_व्यंजक->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          यदि( व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) ){
            sqlite3_Walk_व्यंजक(pWalker, p_व्यंजक->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          यावत्( pNC2
              && sqlite3ReferencesSrcList(pParse, p_व्यंजक, pNC2->pSrcList)==0
          ){
            p_व्यंजक->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }
          प्रतिज्ञा( pDef!=0 || IN_RENAME_OBJECT );
          यदि( pNC2 && pDef ){
            p_व्यंजक->op2 += pNC2->nNestedSelect;
            प्रतिज्ञा( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            प्रतिज्ञा( SQLITE_FUNC_ANYORDER==NC_OrderAgg );
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
      प्रत्यागम WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    पक्ष TK_EXISTS:
    पक्ष TK_SELECT:
#endif
    पक्ष TK_IN: {
      testcase( p_व्यंजक->op==TK_IN );
      testcase( p_व्यंजक->op==TK_EXISTS );
      testcase( p_व्यंजक->op==TK_SELECT );
      यदि( व्यंजक_Use_XSelect(p_व्यंजक) ){
        पूर्णाङ्क nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_व्यंजक );
        testcase( pNC->ncFlags & NC_GenCol );
        प्रतिज्ञा( p_व्यंजक->x.pSelect );
        यदि( p_व्यंजक->op==TK_EXISTS )  pParse->bHasExists = 1;
        यदि( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_व्यंजक, p_व्यंजक);
        }अन्यथा{
          sqlite3WalkSelect(pWalker, p_व्यंजक->x.pSelect);
        }
        प्रतिज्ञा( pNC->nRef>=nRef );
        यदि( nRef!=pNC->nRef ){
          व्यंजक_Set_Property(p_व्यंजक, EP_VarSelect);
          p_व्यंजक->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      भङ्ग;
    }
    पक्ष TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_व्यंजक );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_व्यंजक|NC_GenCol, p_व्यंजक, p_व्यंजक);
      भङ्ग;
    }
    पक्ष TK_IS:
    पक्ष TK_ISNOT: {
      व्यंजक *p_दायाँ = sqlite3_व्यंजक_Skip_Collate_And_Likely(p_व्यंजक->p_दायाँ);
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      यदि( ALWAYS(p_दायाँ) && (p_दायाँ->op==TK_ID || p_दायाँ->op==TK_TRUEFALSE) ){
        पूर्णाङ्क rc = resolve_व्यंजक_Step(pWalker, p_दायाँ);
        यदि( rc==WRC_Abort ) प्रत्यागम WRC_Abort;
        यदि( p_दायाँ->op==TK_TRUEFALSE ){
          p_व्यंजक->op2 = p_व्यंजक->op;
          p_व्यंजक->op = TK_TRUTH;
          प्रत्यागम WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    पक्ष TK_BETWEEN:
    पक्ष TK_EQ:
    पक्ष TK_NE:
    पक्ष TK_LT:
    पक्ष TK_LE:
    पक्ष TK_GT:
    पक्ष TK_GE: {
      पूर्णाङ्क n_बायाँ, n_दायाँ;
      यदि( pParse->db->mallocFailed ) भङ्ग;
      प्रतिज्ञा( p_व्यंजक->p_बायाँ!=0 );
      n_बायाँ = sqlite3_व्यंजक_Vector_Size(p_व्यंजक->p_बायाँ);
      यदि( p_व्यंजक->op==TK_BETWEEN ){
        प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
        n_दायाँ = sqlite3_व्यंजक_Vector_Size(p_व्यंजक->x.pList->a[0].p_व्यंजक);
        यदि( n_दायाँ==n_बायाँ ){
          n_दायाँ = sqlite3_व्यंजक_Vector_Size(p_व्यंजक->x.pList->a[1].p_व्यंजक);
        }
      }अन्यथा{
        प्रतिज्ञा( p_व्यंजक->p_दायाँ!=0 );
        n_दायाँ = sqlite3_व्यंजक_Vector_Size(p_व्यंजक->p_दायाँ);
      }
      यदि( n_बायाँ!=n_दायाँ ){
        testcase( p_व्यंजक->op==TK_EQ );
        testcase( p_व्यंजक->op==TK_NE );
        testcase( p_व्यंजक->op==TK_LT );
        testcase( p_व्यंजक->op==TK_LE );
        testcase( p_व्यंजक->op==TK_GT );
        testcase( p_व्यंजक->op==TK_GE );
        testcase( p_व्यंजक->op==TK_IS );
        testcase( p_व्यंजक->op==TK_ISNOT );
        testcase( p_व्यंजक->op==TK_BETWEEN );
        sqlite3_Error_संदेश(pParse, "row value misused");
        sqlite3_Record_Error_Offset_Of_व्यंजक(pParse->db, p_व्यंजक);
      }
      भङ्ग;
    }
  }
  प्रतिज्ञा( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  प्रत्यागम pParse->nErr ? WRC_Abort : WRC_Continue;
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
स्थिर पूर्णाङ्क resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  व्यंजक_List *pEList,  /* List of expressions to scan */
  व्यंजक *pE           /* Expression we are trying to match */
){
  पूर्णाङ्क i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  यदि( pE->op==TK_ID ){
    ध्रुव वर्ण *zCol;
    प्रतिज्ञा( !व्यंजक_Has_Property(pE, EP_Int_मूल्य) );
    zCol = pE->u.zToken;
    प्रति(i=0; i<pEList->n_व्यंजक; i++){
      यदि( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        प्रत्यागम i+1;
      }
    }
  }
  प्रत्यागम 0;
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
स्थिर पूर्णाङ्क resolve_Order_By_Term_To_व्यंजक_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  व्यंजक *pE           /* The specific ORDER BY term */
){
  पूर्णाङ्क i;             /* Loop counter */
  व्यंजक_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  पूर्णाङ्क rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  प्रतिज्ञा( sqlite3_व्यंजक_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  स_रखो(&nc, 0, परिमाण(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_व्यंजक_Names(&nc, pE);
  db->suppressErr = savedSuppErr;
  यदि( rc ) प्रत्यागम 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  प्रति(i=0; i<pEList->n_व्यंजक; i++){
    यदि( sqlite3_व्यंजक_Compare(0, pEList->a[i].p_व्यंजक, pE, -1)<2 ){
      प्रत्यागम i+1;
    }
  }

  /* If no match, return 0. */
  प्रत्यागम 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
स्थिर शून्य resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  ध्रुव वर्ण *zType,     /* "ORDER" or "GROUP" */
  पूर्णाङ्क i,                 /* The index (1-based) of the term out of range */
  पूर्णाङ्क mx,                /* Largest permissible value of i */
  व्यंजक *pError           /* Associate the error with the expression */
){
  sqlite3_Error_संदेश(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_Record_Error_Offset_Of_व्यंजक(pParse->db, pError);
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
स्थिर पूर्णाङ्क resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  पूर्णाङ्क i;
  व्यंजक_List *pOrderBy;
  व्यंजक_List *pEList;
  sqlite3 *db;
  पूर्णाङ्क moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  यदि( pOrderBy==0 ) प्रत्यागम 0;
  db = pParse->db;
  यदि( pOrderBy->n_व्यंजक>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_संदेश(pParse, "too many terms in ORDER BY clause");
    प्रत्यागम 1;
  }
  प्रति(i=0; i<pOrderBy->n_व्यंजक; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  यावत्( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  यावत्( pSelect && moreToDo ){
    संरचना व्यंजक_List_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    प्रतिज्ञा( pEList!=0 );
    प्रति(i=0, pItem=pOrderBy->a; i<pOrderBy->n_व्यंजक; i++, pItem++){
      पूर्णाङ्क iCol = -1;
      व्यंजक *pE, *pDup;
      यदि( pItem->fg.done ) अनुवर्त;
      pE = sqlite3_व्यंजक_Skip_Collate_And_Likely(pItem->p_व्यंजक);
      यदि( NEVER(pE==0) ) अनुवर्त;
      यदि( sqlite3_व्यंजक_Is_Integer(pE, &iCol, 0, 1) ){
        यदि( iCol<=0 || iCol>pEList->n_व्यंजक ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->n_व्यंजक, pE);
          प्रत्यागम 1;
        }
      }अन्यथा{
        iCol = resolveAsName(pParse, pEList, pE);
        यदि( iCol==0 ){
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
          pDup = sqlite3_व्यंजक_Dup(db, pE, 0);
          यदि( !db->mallocFailed ){
            प्रतिज्ञा(pDup);
            iCol = resolve_Order_By_Term_To_व्यंजक_List(pParse, pSelect, pDup);
            यदि( IN_RENAME_OBJECT && iCol>0 ){
              resolve_Order_By_Term_To_व्यंजक_List(pParse, pSelect, pE);
            }
          }
          sqlite3_व्यंजक_Delete(db, pDup);
        }
      }
      यदि( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        यदि( !IN_RENAME_OBJECT ){
          व्यंजक *pNew = sqlite3_व्यंजक_Int32(db, iCol);
          यदि( pNew==0 ) प्रत्यागम 1;
          यदि( pItem->p_व्यंजक==pE ){
            pItem->p_व्यंजक = pNew;
          }अन्यथा{
            व्यंजक *pParent = pItem->p_व्यंजक;
            प्रतिज्ञा( pParent->op==TK_COLLATE );
            यावत्( pParent->p_बायाँ->op==TK_COLLATE ) pParent = pParent->p_बायाँ;
            प्रतिज्ञा( pParent->p_बायाँ==pE );
            pParent->p_बायाँ = pNew;
          }
          sqlite3_व्यंजक_Delete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }अन्यथा{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  प्रति(i=0; i<pOrderBy->n_व्यंजक; i++){
    यदि( pOrderBy->a[i].fg.done==0 ){
      sqlite3_Error_संदेश(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      प्रत्यागम 1;
    }
  }
  प्रत्यागम 0;
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
पूर्णाङ्क sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  व्यंजक_List *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  ध्रुव वर्ण *zType     /* "ORDER" or "GROUP" */
){
  पूर्णाङ्क i;
  sqlite3 *db = pParse->db;
  व्यंजक_List *pEList;
  संरचना व्यंजक_List_item *pItem;

  यदि( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) प्रत्यागम 0;
  यदि( pOrderBy->n_व्यंजक>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_संदेश(pParse, "too many terms in %s BY clause", zType);
    प्रत्यागम 1;
  }
  pEList = pSelect->pEList;
  प्रतिज्ञा( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  प्रति(i=0, pItem=pOrderBy->a; i<pOrderBy->n_व्यंजक; i++, pItem++){
    यदि( pItem->u.x.iOrderByCol ){
      यदि( pItem->u.x.iOrderByCol>pEList->n_व्यंजक ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->n_व्यंजक, 0);
        प्रत्यागम 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->p_व्यंजक,0);
    }
  }
  प्रत्यागम 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
स्थिर पूर्णाङ्क resolveRemoveWindowsCb(Walker *pWalker, व्यंजक *p_व्यंजक){
  UNUSED_PARAMETER(pWalker);
  यदि( व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) ){
    Window *pWin = p_व्यंजक->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  प्रत्यागम WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
स्थिर शून्य window_Remove_व्यंजक_From_Select(Select *pSelect, व्यंजक *p_व्यंजक){
  यदि( pSelect->pWin ){
    Walker sWalker;
    स_रखो(&sWalker, 0, परिमाण(Walker));
    sWalker.x_व्यंजक_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_व्यंजक(&sWalker, p_व्यंजक);
  }
}
#अन्यथा
# define window_Remove_व्यंजक_From_Select(a, b)
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
स्थिर पूर्णाङ्क resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  व्यंजक_List *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  ध्रुव वर्ण *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  पूर्णाङ्क i, j;                      /* Loop counters */
  पूर्णाङ्क iCol;                      /* Column number */
  संरचना व्यंजक_List_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  पूर्णाङ्क nResult;                   /* Number of terms in the result set */

  प्रतिज्ञा( pOrderBy!=0 );
  nResult = pSelect->pEList->n_व्यंजक;
  pParse = pNC->pParse;
  प्रति(i=0, pItem=pOrderBy->a; i<pOrderBy->n_व्यंजक; i++, pItem++){
    व्यंजक *pE = pItem->p_व्यंजक;
    व्यंजक *pE2 = sqlite3_व्यंजक_Skip_Collate_And_Likely(pE);
    यदि( NEVER(pE2==0) ) अनुवर्त;
    यदि( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      यदि( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        अनुवर्त;
      }
    }
    यदि( sqlite3_व्यंजक_Is_Integer(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      यदि( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult, pE2);
        प्रत्यागम 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      अनुवर्त;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    यदि( sqlite3_Resolve_व्यंजक_Names(pNC, pE) ){
      प्रत्यागम 1;
    }
    प्रति(j=0; j<pSelect->pEList->n_व्यंजक; j++){
      यदि( sqlite3_व्यंजक_Compare(0, pE, pSelect->pEList->a[j].p_व्यंजक, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        window_Remove_व्यंजक_From_Select(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  प्रत्यागम sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
स्थिर पूर्णाङ्क resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  पूर्णाङ्क isCompound;         /* True if p is a compound select */
  पूर्णाङ्क nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  पूर्णाङ्क i;                  /* Loop counter */
  व्यंजक_List *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  प्रतिज्ञा( p!=0 );
  यदि( p->selFlags & SF_Resolved ){
    प्रत्यागम WRC_Prune;
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
  यदि( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    प्रत्यागम pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  यावत्( p ){
    प्रतिज्ञा( (p->selFlags & SF_Expanded)!=0 );
    प्रतिज्ञा( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    स_रखो(&sNC, 0, परिमाण(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    यदि( sqlite3_Resolve_व्यंजक_Names(&sNC, p->pLimit) ){
      प्रत्यागम WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    यदि( p->selFlags & SF_Converted ){
      Select *pSub;
      प्रतिज्ञा( p->pSrc->a[0].fg.isSubquery );
      प्रतिज्ञा( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      प्रतिज्ञा( pSub!=0 );
      प्रतिज्ञा( p->pSrc->nSrc==1 && p->pOrderBy );
      प्रतिज्ञा( pSub->pPrior && pSub->pOrderBy==0 );
      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    यदि( pOuterNC ) pOuterNC->nNestedSelect++;
    प्रति(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      प्रतिज्ञा( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      यदि( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        पूर्णाङ्क nRef = pOuterNC ? pOuterNC->nRef : 0;
        ध्रुव वर्ण *zSavedContext = pParse->zAuthContext;

        यदि( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        यदि( pParse->nErr ) प्रत्यागम WRC_Abort;
        प्रतिज्ञा( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        यदि( pOuterNC ){
          प्रतिज्ञा( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    यदि( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
 
    /* Resolve names in the result set. */
    यदि( sqlite3_Resolve_व्यंजक_List_Names(&sNC, p->pEList) ) प्रत्यागम WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    प्रतिज्ञा( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    यदि( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      प्रतिज्ञा( NC_MinMaxAgg==SF_MinMaxAgg );
      प्रतिज्ञा( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }अन्यथा{
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
    प्रतिज्ञा( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    यदि( p->pHaving ){
      यदि( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_संदेश(pParse, "HAVING clause on a non-aggregate query");
        प्रत्यागम WRC_Abort;
      }
      यदि( sqlite3_Resolve_व्यंजक_Names(&sNC, p->pHaving) ) प्रत्यागम WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    यदि( sqlite3_Resolve_व्यंजक_Names(&sNC, p->pWhere) ) प्रत्यागम WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    प्रति(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      यदि( pItem->fg.isTabFunc
       && sqlite3_Resolve_व्यंजक_List_Names(&sNC, pItem->u1.pFuncArg)
      ){
        प्रत्यागम WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    यदि( IN_RENAME_OBJECT ){
      Window *pWin;
      प्रति(pWin=p->pWinDefn; pWin; pWin=pWin->pNextWin){
        यदि( sqlite3_Resolve_व्यंजक_List_Names(&sNC, pWin->pOrderBy)
         || sqlite3_Resolve_व्यंजक_List_Names(&sNC, pWin->pPartition)
        ){
          प्रत्यागम WRC_Abort;
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
    यदि( p->selFlags & SF_Converted ){
      Select *pSub;
      प्रतिज्ञा( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      प्रतिज्ञा( pSub!=0 );
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
    यदि( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      प्रत्यागम WRC_Abort;
    }
    यदि( db->mallocFailed ){
      प्रत्यागम WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    यदि( pGroupBy ){
      संरचना व्यंजक_List_item *pItem;
   
      यदि( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        प्रत्यागम WRC_Abort;
      }
      प्रति(i=0, pItem=pGroupBy->a; i<pGroupBy->n_व्यंजक; i++, pItem++){
        यदि( व्यंजक_Has_Property(pItem->p_व्यंजक, EP_Agg) ){
          sqlite3_Error_संदेश(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          प्रत्यागम WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    यदि( p->pNext && p->pEList->n_व्यंजक!=p->pNext->pEList->n_व्यंजक ){
      sqlite3_Select_Wrong_अंक_Terms_Error(pParse, p->pNext);
      प्रत्यागम WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    यदि( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      यदि( pParse->nErr ) प्रत्यागम WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  यदि( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    प्रत्यागम WRC_Abort;
  }

  प्रत्यागम WRC_Prune;
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
पूर्णाङ्क sqlite3_Resolve_व्यंजक_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  व्यंजक *p_व्यंजक             /* The expression to be analyzed. */
){
  पूर्णाङ्क savedHasAgg;
  Walker w;

  यदि( p_व्यंजक==0 ) प्रत्यागम SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.x_व्यंजक_Callback = resolve_व्यंजक_Step;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#यदि SQLITE_MAX_व्यंजक_DEPTH>0
  w.pParse->nHeight += p_व्यंजक->nHeight;
  यदि( sqlite3_व्यंजक_Check_Height(w.pParse, w.pParse->nHeight) ){
    प्रत्यागम SQLITE_ERROR;
  }
#endif
  प्रतिज्ञा( p_व्यंजक!=0 );
  sqlite3_Walk_व्यंजक_NN(&w, p_व्यंजक);
#यदि SQLITE_MAX_व्यंजक_DEPTH>0
  w.pParse->nHeight -= p_व्यंजक->nHeight;
#endif
  प्रतिज्ञा( EP_Agg==NC_HasAgg );
  प्रतिज्ञा( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  व्यंजक_Set_Property(p_व्यंजक, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  प्रत्यागम pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
पूर्णाङ्क sqlite3_Resolve_व्यंजक_List_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  व्यंजक_List *pList         /* The expression list to be analyzed. */
){
  पूर्णाङ्क i;
  पूर्णाङ्क savedHasAgg = 0;
  Walker w;
  यदि( pList==0 ) प्रत्यागम SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_व्यंजक_Callback = resolve_व्यंजक_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  प्रति(i=0; i<pList->n_व्यंजक; i++){
    व्यंजक *p_व्यंजक = pList->a[i].p_व्यंजक;
    यदि( p_व्यंजक==0 ) अनुवर्त;
#यदि SQLITE_MAX_व्यंजक_DEPTH>0
    w.pParse->nHeight += p_व्यंजक->nHeight;
    यदि( sqlite3_व्यंजक_Check_Height(w.pParse, w.pParse->nHeight) ){
      प्रत्यागम SQLITE_ERROR;
    }
#endif
    sqlite3_Walk_व्यंजक_NN(&w, p_व्यंजक);
#यदि SQLITE_MAX_व्यंजक_DEPTH>0
    w.pParse->nHeight -= p_व्यंजक->nHeight;
#endif
    प्रतिज्ञा( EP_Agg==NC_HasAgg );
    प्रतिज्ञा( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    यदि( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      व्यंजक_Set_Property(p_व्यंजक, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    यदि( w.pParse->nErr>0 ) प्रत्यागम SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  प्रत्यागम SQLITE_OK;
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
शून्य sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  प्रतिज्ञा( p!=0 );
  w.x_व्यंजक_Callback = resolve_व्यंजक_Step;
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
पूर्णाङ्क sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  पूर्णाङ्क type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  व्यंजक *p_व्यंजक,     /* Expression to resolve.  May be NULL. */
  व्यंजक_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  पूर्णाङ्क rc;
  सङ्घ {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  प्रतिज्ञा( type==0 || pTab!=0 );
  प्रतिज्ञा( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_व्यंजक
          || type==NC_GenCol || pTab==0 );
  स_रखो(&sNC, 0, परिमाण(sNC));
  स_रखो(&uSrc, 0, परिमाण(uSrc));
  pSrc = &uSrc.sSrc;
  यदि( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    यदि( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  यदि( (rc = sqlite3_Resolve_व्यंजक_Names(&sNC, p_व्यंजक))!=SQLITE_OK ) प्रत्यागम rc;
  यदि( pList ) rc = sqlite3_Resolve_व्यंजक_List_Names(&sNC, pList);
  प्रत्यागम rc;
}
