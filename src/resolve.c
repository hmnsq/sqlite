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
#অন্তর্ভাব "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#define EXCLUDED_TABLE_সংখ্যা  2

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
স্থির পূর্ণসংখ্যা incrAggDepth(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  যদি( p_রাশিমালা->op==TK_AGG_FUNCTION ) p_রাশিমালা->op2 += pWalker->u.n;
  ফেরত WRC_Continue;
}
স্থির শূন্য incrAggFunctionDepth(রাশিমালা *p_রাশিমালা, পূর্ণসংখ্যা N){
  যদি( N>0 ){
    Walker w;
    memset(&w, 0, আকার(w));
    w.x_রাশিমালা_Callback = incrAggDepth;
    w.u.n = N;
    sqlite3_Walk_রাশিমালা(&w, p_রাশিমালা);
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
স্থির শূন্য resolveAlias(
  Parse *pParse,         /* Parsing context */
  রাশিমালা_List *pEList,      /* A result set */
  পূর্ণসংখ্যা iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  রাশিমালা *p_রাশিমালা,           /* Transform this into an alias to the result set */
  পূর্ণসংখ্যা nSubquery          /* Number of subqueries that the label is moving */
){
  রাশিমালা *pOrig;           /* The iCol-th column of the result set */
  রাশিমালা *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  প্রতিজ্ঞা( iCol>=0 && iCol<pEList->n_রাশিমালা );
  pOrig = pEList->a[iCol].p_রাশিমালা;
  প্রতিজ্ঞা( pOrig!=0 );
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Reduced|EP_TokenOnly) );
  যদি( p_রাশিমালা->pAggInfo ) ফেরত;
  db = pParse->db;
  pDup = sqlite3_রাশিমালা_Dup(db, pOrig, 0);
  যদি( db->mallocFailed ){
    sqlite3_রাশিমালা_Delete(db, pDup);
    pDup = 0;
  }অন্যথা{
    রাশিমালা অস্থায়ী;
    incrAggFunctionDepth(pDup, nSubquery);
    যদি( p_রাশিমালা->op==TK_COLLATE ){
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      pDup = sqlite3_রাশিমালা_Add_Collate_String(pParse, pDup, p_রাশিমালা->u.zToken);
    }
    memcpy(&অস্থায়ী, pDup, আকার(রাশিমালা));
    memcpy(pDup, p_রাশিমালা, আকার(রাশিমালা));
    memcpy(p_রাশিমালা, &অস্থায়ী, আকার(রাশিমালা));
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc) ){
      যদি( ALWAYS(p_রাশিমালা->y.pWin!=0) ){
        p_রাশিমালা->y.pWin->pOwner = p_রাশিমালা;
      }
    }
    sqlite3_রাশিমালা_Deferred_Delete(pParse, pDup);
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
পূর্ণসংখ্যা sqlite3MatchEName(
  ধ্রুবক কাঠামো রাশিমালা_List_item *pItem,
  ধ্রুবক অক্ষর *zCol,
  ধ্রুবক অক্ষর *zTab,
  ধ্রুবক অক্ষর *zDb,
  পূর্ণসংখ্যা *pbRowid
){
  পূর্ণসংখ্যা n;
  ধ্রুবক অক্ষর *zSpan;
  পূর্ণসংখ্যা eEName = pItem->fg.eEName;
  যদি( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    ফেরত 0;
  }
  প্রতিজ্ঞা( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  জন্য(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  যদি( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    ফেরত 0;
  }
  zSpan += n+1;
  জন্য(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  যদি( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    ফেরত 0;
  }
  zSpan += n+1;
  যদি( zCol ){
    যদি( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) ফেরত 0;
    যদি( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) ফেরত 0;
  }
  যদি( eEName==ENAME_ROWID ) *pbRowid = 1;
  ফেরত 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
স্থির পূর্ণসংখ্যা areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *p_শীর্ষ_NC){
  যদি( db->init.busy ) ফেরত 1;  /* Always support for legacy schemas */
  যদি( p_শীর্ষ_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    যদি( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      ফেরত 1;
    }
    ফেরত (db->flags & SQLITE_DqsDDL)!=0;
  }অন্যথা{
    /* Currently parsing a DML statement */
    ফেরত (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_রাশিমালা_Col_Used(রাশিমালা *p_রাশিমালা){
  পূর্ণসংখ্যা n;
  Table *pExTab;

  n = p_রাশিমালা->iColumn;
  প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
  pExTab = p_রাশিমালা->y.pTab;
  প্রতিজ্ঞা( pExTab!=0 );
  প্রতিজ্ঞা( n < pExTab->nCol );
  যদি( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    ফেরত pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }অন্যথা{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    যদি( n>=BMS ) n = BMS-1;
    ফেরত ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
স্থির শূন্য extendFJMatch(
  Parse *pParse,          /* Parsing context */
  রাশিমালা_List **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  রাশিমালা *pNew = sqlite3_রাশিমালা_Alloc(pParse->db, TK_COLUMN, 0, 0);
  যদি( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    প্রতিজ্ঞা( (pMatch->fg.jointype & (JT_বাম|JT_LTORJ))!=0 );
    রাশিমালা_Set_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_রাশিমালা_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
স্থির SQLITE_NOINLINE পূর্ণসংখ্যা isValidSchemaTableName(
  ধ্রুবক অক্ষর *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  ধ্রুবক অক্ষর *zDb           /* non-NULL if a database qualifier is present */
){
  ধ্রুবক অক্ষর *zLegacy;
  প্রতিজ্ঞা( pTab!=0 );
  প্রতিজ্ঞা( pTab->tnum==1 );
  যদি( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) ফেরত 0;
  zLegacy = pTab->zName;
  যদি( strcmp(zLegacy+7, &LEGACY_অস্থায়ী_SCHEMA_TABLE[7])==0 ){
    যদি( sqlite3StrICmp(zTab+7, &PREFERRED_অস্থায়ী_SCHEMA_TABLE[7])==0 ){
      ফেরত 1;
    }
    যদি( zDb==0 ) ফেরত 0;
    যদি( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) ফেরত 1;
    যদি( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) ফেরত 1;
  }অন্যথা{
    যদি( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) ফেরত 1;
  }
  ফেরত 0;
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
স্থির পূর্ণসংখ্যা lookupName(
  Parse *pParse,       /* The parsing context */
  ধ্রুবক অক্ষর *zDb,     /* Name of the database containing table, or NULL */
  ধ্রুবক অক্ষর *zTab,    /* Name of table containing column, or NULL */
  ধ্রুবক রাশিমালা *p_ডান,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  রাশিমালা *p_রাশিমালা          /* Make this EXPR node point to the selected column */
){
  পূর্ণসংখ্যা i, j;                         /* Loop counters */
  পূর্ণসংখ্যা cnt = 0;                      /* Number of matching column names */
  পূর্ণসংখ্যা cntTab = 0;                   /* Number of potential "rowid" matches */
  পূর্ণসংখ্যা nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *p_শীর্ষ_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  পূর্ণসংখ্যা e_New_রাশিমালা_Op = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  রাশিমালা_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  ধ্রুবক অক্ষর *zCol = p_ডান->u.zToken;

  প্রতিজ্ঞা( pNC );     /* the name context cannot be NULL. */
  প্রতিজ্ঞা( zCol );    /* The Z in X.Y.Z cannot be NULL */
  প্রতিজ্ঞা( zDb==0 || zTab!=0 );
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_রাশিমালা->iTable = -1;
  রাশিমালা_Set_VVAProperty(p_রাশিমালা, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  যদি( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    যদি( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }অন্যথা{
      জন্য(i=0; i<db->nDb; i++){
        প্রতিজ্ঞা( db->aDb[i].zDbSName );
        যদি( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          বিরতি;
        }
      }
      যদি( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  প্রতিজ্ঞা( pNC && cnt==0 );
  করো{
    রাশিমালা_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    যদি( pSrcList ){
      জন্য(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        প্রতিজ্ঞা( pTab!=0 && pTab->zName!=0 );
        প্রতিজ্ঞা( pTab->nCol>0 || pParse->nErr );
        প্রতিজ্ঞা( (পূর্ণসংখ্যা)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        যদি( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          পূর্ণসংখ্যা hit = 0;
          Select *pSel;
          প্রতিজ্ঞা( pItem->fg.isSubquery );
          প্রতিজ্ঞা( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          প্রতিজ্ঞা( pSel!=0 );
          pEList = pSel->pEList;
          প্রতিজ্ঞা( pEList!=0 );
          প্রতিজ্ঞা( pEList->n_রাশিমালা==pTab->nCol );
          জন্য(j=0; j<pEList->n_রাশিমালা; j++){
            পূর্ণসংখ্যা bRowid = 0;       /* True if possible rowid match */
            যদি( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              চালু;
            }
            যদি( bRowid==0 ){
              যদি( cnt>0 ){
                যদি( pItem->fg.isUsing==0
                 || sqlite3_Id_List_সূচক(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_রাশিমালা_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }অন্যথা
                যদি( (pItem->fg.jointype & JT_ডান)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  চালু;
                }অন্যথা
                যদি( (pItem->fg.jointype & JT_বাম)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_রাশিমালা_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }অন্যথা{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, p_রাশিমালা->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }অন্যথা যদি( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              চালু;
            }
            cntTab++;
            pMatch = pItem;
            p_রাশিমালা->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            প্রতিজ্ঞা( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            যদি( pEList->a[j].fg.bUsingTerm ) বিরতি;
          }
          যদি( hit || zTab==0 ) চালু;
        }
        প্রতিজ্ঞা( zDb==0 || zTab!=0 );
        যদি( zTab ){
          যদি( zDb ){
            যদি( pTab->pSchema!=pSchema ) চালু;
            যদি( pSchema==0 && strcmp(zDb,"*")!=0 ) চালু;
          }
          যদি( pItem->zAlias!=0 ){
            যদি( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              চালু;
            }
          }অন্যথা যদি( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            যদি( pTab->tnum!=1 ) চালু;
            যদি( !isValidSchemaTableName(zTab, pTab, zDb) ) চালু;
          }
          প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
          যদি( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (শূন্য*)&p_রাশিমালা->y.pTab);
          }
        }
        j = sqlite3_Column_সূচক(pTab, zCol);
        যদি( j>=0 ){
          যদি( cnt>0 ){
            যদি( pItem->fg.isUsing==0
             || sqlite3_Id_List_সূচক(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_রাশিমালা_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }অন্যথা
            যদি( (pItem->fg.jointype & JT_ডান)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              চালু;
            }অন্যথা
            যদি( (pItem->fg.jointype & JT_বাম)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_রাশিমালা_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }অন্যথা{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, p_রাশিমালা->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_রাশিমালা->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          যদি( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        যদি( 0==cnt && VisibleRowid(pTab) ){
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
          যদি( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }অন্যথা{
            cntTab++;
          }
#অন্যথা
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      যদি( pMatch ){
        p_রাশিমালা->iTable = pMatch->iCursor;
        প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
        p_রাশিমালা->y.pTab = pMatch->pSTab;
        যদি( (pMatch->fg.jointype & (JT_বাম|JT_LTORJ))!=0 ){
          রাশিমালা_Set_Property(p_রাশিমালা, EP_CanBeNull);
        }
        pSchema = p_রাশিমালা->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#যদি !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    যদি( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      যদি( pParse->pTriggerTab!=0 ){
        পূর্ণসংখ্যা op = pParse->eTriggerOp;
        প্রতিজ্ঞা( op==TK_DELETE || op==TK_UPDATE || op==TK_ঢোকাও );
        যদি( pParse->bReturning ){
          যদি( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            p_রাশিমালা->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }অন্যথা যদি( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_রাশিমালা->iTable = 1;
          pTab = pParse->pTriggerTab;
        }অন্যথা যদি( op!=TK_ঢোকাও && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_রাশিমালা->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      যদি( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        যদি( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_রাশিমালা->iTable = EXCLUDED_TABLE_সংখ্যা;
          যদি( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (শূন্য*)&p_রাশিমালা->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      যদি( pTab ){
        পূর্ণসংখ্যা iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3_Column_সূচক(pTab, zCol);
        যদি( iCol>=0 ){
          যদি( pTab->iPKey==iCol ) iCol = -1;
        }অন্যথা{
          যদি( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }অন্যথা{
            iCol = pTab->nCol;
          }
        }
        যদি( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          যদি( p_রাশিমালা->iTable==EXCLUDED_TABLE_সংখ্যা ){
            testcase( iCol==(-1) );
            প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
            যদি( IN_RENAME_OBJECT ){
              p_রাশিমালা->iColumn = iCol;
              p_রাশিমালা->y.pTab = pTab;
              e_New_রাশিমালা_Op = TK_COLUMN;
            }অন্যথা{
              p_রাশিমালা->iTable = pNC->uNC.pUpsert->reg_তথ্য +
                 sqlite3TableColumnToStorage(pTab, iCol);
              e_New_রাশিমালা_Op = TK_REGISTER;
            }
          }অন্যথা
#endif /* SQLITE_OMIT_UPSERT */
          {
            প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
            p_রাশিমালা->y.pTab = pTab;
            যদি( pParse->bReturning ){
              e_New_রাশিমালা_Op = TK_REGISTER;
              p_রাশিমালা->op2 = TK_COLUMN;
              p_রাশিমালা->iColumn = iCol;
              p_রাশিমালা->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*p_রাশিমালা->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }অন্যথা{
              p_রাশিমালা->iColumn = (i16)iCol;
              e_New_রাশিমালা_Op = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              যদি( iCol<0 ){
                p_রাশিমালা->aff_রাশিমালা = SQLITE_AFF_INTEGER;
              }অন্যথা যদি( p_রাশিমালা->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }অন্যথা{
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
    যদি( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_Idx_রাশিমালা|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#যদি SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      যদি( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        e_New_রাশিমালা_Op = TK_NULL;
      }
#endif
      যদি( pMatch->fg.isNestedFrom==0 ) p_রাশিমালা->iColumn = -1;
      p_রাশিমালা->aff_রাশিমালা = SQLITE_AFF_INTEGER;
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
    যদি( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      প্রতিজ্ঞা( pEList!=0 );
      জন্য(j=0; j<pEList->n_রাশিমালা; j++){
        অক্ষর *zAs = pEList->a[j].zEName;
        যদি( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          রাশিমালা *pOrig;
          প্রতিজ্ঞা( p_রাশিমালা->p_বাম==0 && p_রাশিমালা->p_ডান==0 );
          প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা)==0 || p_রাশিমালা->x.pList==0 );
          প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p_রাশিমালা)==0 || p_রাশিমালা->x.pSelect==0 );
          pOrig = pEList->a[j].p_রাশিমালা;
          যদি( (pNC->ncFlags&NC_AllowAgg)==0 && রাশিমালা_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_বার্তা(pParse, "misuse of aliased aggregate %s", zAs);
            ফেরত WRC_Abort;
          }
          যদি( রাশিমালা_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_শীর্ষ_NC )
          ){
            sqlite3_Error_বার্তা(pParse, "misuse of aliased window function %s",zAs);
            ফেরত WRC_Abort;
          }
          যদি( sqlite3_রাশিমালা_Vector_আকার(pOrig)!=1 ){
            sqlite3_Error_বার্তা(pParse, "row value misused");
            ফেরত WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_রাশিমালা, nSubquery);
          cnt = 1;
          pMatch = 0;
          প্রতিজ্ঞা( zTab==0 && zDb==0 );
          যদি( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (শূন্য*)p_রাশিমালা);
          }
          যাও lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    যদি( cnt ) বিরতি;
    pNC = pNC->p_পরবর্তী;
    nSubquery++;
  }যখন( pNC );


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
  যদি( cnt==0 && zTab==0 ){
    প্রতিজ্ঞা( p_রাশিমালা->op==TK_ID );
    যদি( রাশিমালা_Has_Property(p_রাশিমালা,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, p_শীর্ষ_NC)
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
      p_রাশিমালা->op = TK_STRING;
      memset(&p_রাশিমালা->y, 0, আকার(p_রাশিমালা->y));
      ফেরত WRC_Prune;
    }
    যদি( sqlite3_রাশিমালা_Id_To_True_False(p_রাশিমালা) ){
      ফেরত WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  প্রতিজ্ঞা( pFJMatch==0 || cnt>0 );
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_xIsSelect|EP_Int_মান) );
  যদি( cnt!=1 ){
    ধ্রুবক অক্ষর *zErr;
    যদি( pFJMatch ){
      যদি( pFJMatch->n_রাশিমালা==cnt-1 ){
        যদি( রাশিমালা_Has_Property(p_রাশিমালা,EP_Leaf) ){
          রাশিমালা_Clear_Property(p_রাশিমালা,EP_Leaf);
        }অন্যথা{
          sqlite3_রাশিমালা_Delete(db, p_রাশিমালা->p_বাম);
          p_রাশিমালা->p_বাম = 0;
          sqlite3_রাশিমালা_Delete(db, p_রাশিমালা->p_ডান);
          p_রাশিমালা->p_ডান = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, p_রাশিমালা->iColumn);
        p_রাশিমালা->op = TK_FUNCTION;
        p_রাশিমালা->u.zToken = "coalesce";
        p_রাশিমালা->x.pList = pFJMatch;
        p_রাশিমালা->aff_রাশিমালা = SQLITE_AFF_DEFER;
        cnt = 1;
        যাও lookupname_end;
      }অন্যথা{
        sqlite3_রাশিমালা_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    যদি( zDb ){
      sqlite3_Error_বার্তা(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }অন্যথা যদি( zTab ){
      sqlite3_Error_বার্তা(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }অন্যথা যদি( cnt==0 && রাশিমালা_Has_Property(p_ডান,EP_DblQuoted) ){
      sqlite3_Error_বার্তা(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }অন্যথা{
      sqlite3_Error_বার্তা(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3_Record_Error_Offset_Of_রাশিমালা(pParse->db, p_রাশিমালা);
    pParse->checkSchema = 1;
    p_শীর্ষ_NC->nNcErr++;
    e_New_রাশিমালা_Op = TK_NULL;
  }
  প্রতিজ্ঞা( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  যদি( !রাশিমালা_Has_Property(p_রাশিমালা,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3_রাশিমালা_Delete(db, p_রাশিমালা->p_বাম);
    p_রাশিমালা->p_বাম = 0;
    sqlite3_রাশিমালা_Delete(db, p_রাশিমালা->p_ডান);
    p_রাশিমালা->p_ডান = 0;
    রাশিমালা_Set_Property(p_রাশিমালা, EP_Leaf);
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
  যদি( pMatch ){
    যদি( p_রাশিমালা->iColumn>=0 ){
      pMatch->colUsed |= sqlite3_রাশিমালা_Col_Used(p_রাশিমালা);
    }অন্যথা{
      pMatch->fg.rowidUsed = 1;
    }
  }

  p_রাশিমালা->op = e_New_রাশিমালা_Op;
lookupname_end:
  যদি( cnt==1 ){
    প্রতিজ্ঞা( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    যদি( db->xAuth ){
      যদি( pFJMatch ){
        প্রতিজ্ঞা( p_রাশিমালা->op==TK_FUNCTION );
        প্রতিজ্ঞা( sqlite3_stricmp(p_রাশিমালা->u.zToken,"coalesce")==0 );
        প্রতিজ্ঞা( p_রাশিমালা->x.pList==pFJMatch );
        প্রতিজ্ঞা( pFJMatch->n_রাশিমালা>0 );
        জন্য(i=0; i<pFJMatch->n_রাশিমালা; i++){
          প্রতিজ্ঞা( pFJMatch->a[i].p_রাশিমালা->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_রাশিমালা,pSchema,pNC->pSrcList);
        }
      }অন্যথা যদি( p_রাশিমালা->op==TK_COLUMN || p_রাশিমালা->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_রাশিমালা, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    জন্য(;;){
      প্রতিজ্ঞা( p_শীর্ষ_NC!=0 );
      p_শীর্ষ_NC->nRef++;
      যদি( p_শীর্ষ_NC==pNC ) বিরতি;
      p_শীর্ষ_NC = p_শীর্ষ_NC->p_পরবর্তী;
    }
    ফেরত WRC_Prune;
  } অন্যথা {
    ফেরত WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
রাশিমালা *sqlite3_Create_Column_রাশিমালা(sqlite3 *db, SrcList *pSrc, পূর্ণসংখ্যা iSrc, পূর্ণসংখ্যা iCol){
  রাশিমালা *p = sqlite3_রাশিমালা_Alloc(db, TK_COLUMN, 0, 0);
  যদি( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    প্রতিজ্ঞা( রাশিমালা_Use_YTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    যদি( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }অন্যথা{
      p->iColumn = (ynVar)iCol;
      যদি( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }অন্যথা{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  ফেরত p;
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
স্থির শূন্য notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   ধ্রুবক অক্ষর *z_বার্তা,    /* Type of error */
   রাশিমালা *p_রাশিমালা,         /* Invalidate this expression on error */
   রাশিমালা *pError         /* Associate error with this expression */
){
  ধ্রুবক অক্ষর *zIn = "partial index WHERE clauses";
  যদি( pNC->ncFlags & NC_Idx_রাশিমালা )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  অন্যথা যদি( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  অন্যথা যদি( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3_Error_বার্তা(pParse, "%s prohibited in %s", z_বার্তা, zIn);
  যদি( p_রাশিমালা ) p_রাশিমালা->op = TK_NULL;
  sqlite3_Record_Error_Offset_Of_রাশিমালা(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  প্রতিজ্ঞা( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_রাশিমালা|NC_GenCol))==0 ); \
  যদি( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
স্থির পূর্ণসংখ্যা রাশিমালা_Probability(রাশিমালা *p){
  দ্বিগুণ r = -1.0;
  যদি( p->op!=TK_FLOAT ) ফেরত -1;
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p, EP_Int_মান) );
  sqlite3AtoF(p->u.zToken, &r);
  প্রতিজ্ঞা( r>=0.0 );
  যদি( r>1.0 ) ফেরত -1;
  ফেরত (পূর্ণসংখ্যা)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
স্থির SQLITE_NOINLINE শূন্য resolve_Set_রাশিমালা_Subtype_Arg(রাশিমালা_List *pList){
  পূর্ণসংখ্যা nn, ii;
  nn = pList ? pList->n_রাশিমালা : 0;
  জন্য(ii=0; ii<nn; ii++){
    রাশিমালা *p_রাশিমালা = pList->a[ii].p_রাশিমালা;
    যখন( 1 /*exit-by-break*/ ){
      রাশিমালা_Set_Property(p_রাশিমালা, EP_SubtArg);
      যদি( p_রাশিমালা->op==TK_SELECT ){
        প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p_রাশিমালা) );
        প্রতিজ্ঞা( p_রাশিমালা->x.pSelect!=0 );
        resolve_Set_রাশিমালা_Subtype_Arg(p_রাশিমালা->x.pSelect->pEList);
        বিরতি;
      }
      যদি( p_রাশিমালা->op==TK_UPLUS ){
        p_রাশিমালা = p_রাশিমালা->p_বাম;
        প্রতিজ্ঞা( p_রাশিমালা!=0 );
      }অন্যথা{
        বিরতি;
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
স্থির পূর্ণসংখ্যা resolve_রাশিমালা_Step(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  প্রতিজ্ঞা( pNC!=0 );
  pParse = pNC->pParse;
  প্রতিজ্ঞা( pParse==pWalker->pParse );

#ifndef NDEBUG
  যদি( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    পূর্ণসংখ্যা i;
    জন্য(i=0; i<pNC->pSrcList->nSrc; i++){
      প্রতিজ্ঞা( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  নির্বাচন( p_রাশিমালা->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    ক্ষেত্র TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      প্রতিজ্ঞা( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      p_রাশিমালা->op = TK_COLUMN;
      প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
      p_রাশিমালা->y.pTab = pItem->pSTab;
      p_রাশিমালা->iTable = pItem->iCursor;
      p_রাশিমালা->iColumn--;
      p_রাশিমালা->aff_রাশিমালা = SQLITE_AFF_INTEGER;
      বিরতি;
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
    ক্ষেত্র TK_NOTNULL:
    ক্ষেত্র TK_ISNULL: {
      পূর্ণসংখ্যা anRef[8];
      NameContext *p;
      পূর্ণসংখ্যা i;
      জন্য(i=0, p=pNC; p && i<Array_আকার(anRef); p=p->p_পরবর্তী, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_রাশিমালা(pWalker, p_রাশিমালা->p_বাম);
      যদি( IN_RENAME_OBJECT ) ফেরত WRC_Prune;
      যদি( sqlite3_রাশিমালা_Can_Be_Null(p_রাশিমালা->p_বাম) ){
        /* The expression can be NULL.  So the optimization does not apply */
        ফেরত WRC_Prune;
      }

      জন্য(i=0, p=pNC; p; p=p->p_পরবর্তী, i++){
        যদি( (p->ncFlags & NC_Where)==0 ){
          ফেরত WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON) );
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
#যদি TREETRACE_ENABLED
      যদি( sqlite3_বৃক্ষ_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_রাশিমালা->op==TK_NOTNULL
        );
        sqlite3_Show_রাশিমালা(p_রাশিমালা);
      }
#endif /* TREETRACE_ENABLED */
      p_রাশিমালা->u.i_মান = (p_রাশিমালা->op==TK_NOTNULL);
      p_রাশিমালা->flags |= EP_Int_মান;
      p_রাশিমালা->op = TK_INTEGER;
      জন্য(i=0, p=pNC; p && i<Array_আকার(anRef); p=p->p_পরবর্তী, i++){
        p->nRef = anRef[i];
      }
      sqlite3_রাশিমালা_Delete(pParse->db, p_রাশিমালা->p_বাম);
      p_রাশিমালা->p_বাম = 0;
      ফেরত WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    ক্ষেত্র TK_ID:
    ক্ষেত্র TK_DOT: {
      ধ্রুবক অক্ষর *zTable;
      ধ্রুবক অক্ষর *zDb;
      রাশিমালা *p_ডান;

      যদি( p_রাশিমালা->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
        p_ডান = p_রাশিমালা;
      }অন্যথা{
        রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
        testcase( pNC->ncFlags & NC_Idx_রাশিমালা );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_রাশিমালা|NC_GenCol, 0, p_রাশিমালা);
        p_ডান = p_রাশিমালা->p_ডান;
        যদি( p_ডান->op==TK_ID ){
          zDb = 0;
        }অন্যথা{
          প্রতিজ্ঞা( p_ডান->op==TK_DOT );
          প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_ডান, EP_Int_মান) );
          zDb = p_বাম->u.zToken;
          p_বাম = p_ডান->p_বাম;
          p_ডান = p_ডান->p_ডান;
        }
        প্রতিজ্ঞা( রাশিমালা_Use_UToken(p_বাম) && রাশিমালা_Use_UToken(p_ডান) );
        zTable = p_বাম->u.zToken;
        প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
        যদি( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (শূন্য*)p_রাশিমালা, (শূন্য*)p_ডান);
          sqlite3RenameTokenRemap(pParse, (শূন্য*)&p_রাশিমালা->y.pTab, (শূন্য*)p_বাম);
        }
      }
      ফেরত lookupName(pParse, zDb, zTable, p_ডান, pNC, p_রাশিমালা);
    }

    /* Resolve function names
    */
    ক্ষেত্র TK_FUNCTION: {
      রাশিমালা_List *pList;            /* The argument list */
      পূর্ণসংখ্যা n;                      /* Number of arguments */
      পূর্ণসংখ্যা no_such_func = 0;       /* True if no such function exists */
      পূর্ণসংখ্যা wrong_অঙ্ক_args = 0;     /* True if wrong number of arguments */
      পূর্ণসংখ্যা is_agg = 0;             /* True if is an aggregate function */
      ধ্রুবক অক্ষর *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      পূর্ণসংখ্যা savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(p_রাশিমালা) ? p_রাশিমালা->y.pWin : 0);
#endif
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_xIsSelect|EP_Int_মান) );
      প্রতিজ্ঞা( p_রাশিমালা->p_বাম==0 || p_রাশিমালা->p_বাম->op==TK_ORDER );
      pList = p_রাশিমালা->x.pList;
      n = pList ? pList->n_রাশিমালা : 0;
      zId = p_রাশিমালা->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      যদি( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        যদি( pDef==0 ){
          no_such_func = 1;
        }অন্যথা{
          wrong_অঙ্ক_args = 1;
        }
      }অন্যথা{
        is_agg = pDef->xFinalize!=0;
        যদি( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          রাশিমালা_Set_Property(p_রাশিমালা, EP_Unlikely);
          যদি( n==2 ){
            p_রাশিমালা->iTable = রাশিমালা_Probability(pList->a[1].p_রাশিমালা);
            যদি( p_রাশিমালা->iTable<0 ){
              sqlite3_Error_বার্তা(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_রাশিমালা);
              pNC->nNcErr++;
            }
          }অন্যথা{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_রাশিমালা->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          পূর্ণসংখ্যা auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          যদি( auth!=SQLITE_OK ){
            যদি( auth==SQLITE_DENY ){
              sqlite3_Error_বার্তা(pParse, "not authorized to use function: %#T",
                                      p_রাশিমালা);
              pNC->nNcErr++;
            }
            p_রাশিমালা->op = TK_NULL;
            ফেরত WRC_Prune;
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
        যদি( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || রাশিমালা_Has_Property(p_রাশিমালা, EP_SubtArg) 
        ){
          resolve_Set_রাশিমালা_Subtype_Arg(pList);
        }

        যদি( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          রাশিমালা_Set_Property(p_রাশিমালা,EP_ConstFunc);
        }
        যদি( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_রাশিমালা|NC_PartIdx|NC_GenCol, 0, p_রাশিমালা);
        }অন্যথা{
          প্রতিজ্ঞা( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_রাশিমালা->op2 = pNC->ncFlags & NC_SelfRef;
        }
        যদি( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
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
        }অন্যথা
        যদি( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          যদি( pNC->ncFlags & NC_FromDDL ) রাশিমালা_Set_Property(p_রাশিমালা, EP_FromDDL);
          sqlite3_রাশিমালা_Function_Usable(pParse, p_রাশিমালা, pDef);
        }
      }

      যদি( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        প্রতিজ্ঞা( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_মান==0 && pDef->xInverse==0)
          || (pDef->x_মান && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        যদি( pDef && pDef->x_মান==0 && pWin ){
          sqlite3_Error_বার্তা(pParse,
              "%#T() may not be used as a window function", p_রাশিমালা
          );
          pNC->nNcErr++;
        }অন্যথা যদি(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          ধ্রুবক অক্ষর *zType;
          যদি( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }অন্যথা{
            zType = "aggregate";
          }
          sqlite3_Error_বার্তা(pParse, "misuse of %s function %#T()",zType,p_রাশিমালা);
          pNC->nNcErr++;
          is_agg = 0;
        }
#অন্যথা
        যদি( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_বার্তা(pParse,"misuse of aggregate function %#T()",p_রাশিমালা);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        অন্যথা যদি( no_such_func
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
          sqlite3_Error_বার্তা(pParse, "no such function: %#T", p_রাশিমালা);
          pNC->nNcErr++;
        }অন্যথা যদি( wrong_অঙ্ক_args ){
          sqlite3_Error_বার্তা(pParse,"wrong number of arguments to function %#T()",
               p_রাশিমালা);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        অন্যথা যদি( is_agg==0 && রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc) ){
          sqlite3_Error_বার্তা(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_রাশিমালা
          );
          pNC->nNcErr++;
        }
#endif
        অন্যথা যদি( is_agg==0 && p_রাশিমালা->p_বাম ){
          sqlite3_রাশিমালা_Order_By_Aggregate_Error(pParse, p_রাশিমালা);
          pNC->nNcErr++;
        }
        যদি( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#অন্যথা
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      অন্যথা যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc) || p_রাশিমালা->p_বাম ){
        is_agg = 1;
      }
      sqlite3_Walk_রাশিমালা_List(pWalker, pList);
      যদি( is_agg ){
        যদি( p_রাশিমালা->p_বাম ){
          প্রতিজ্ঞা( p_রাশিমালা->p_বাম->op==TK_ORDER );
          প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা->p_বাম) );
          sqlite3_Walk_রাশিমালা_List(pWalker, p_রাশিমালা->p_বাম->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        যদি( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          প্রতিজ্ঞা( রাশিমালা_Use_YWin(p_রাশিমালা) && pWin==p_রাশিমালা->y.pWin );
          যদি( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            যদি( pParse->db->mallocFailed ) বিরতি;
          }
          sqlite3_Walk_রাশিমালা_List(pWalker, pWin->pPartition);
          sqlite3_Walk_রাশিমালা_List(pWalker, pWin->pOrderBy);
          sqlite3_Walk_রাশিমালা(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }অন্যথা
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_রাশিমালা->op = TK_AGG_FUNCTION;
          p_রাশিমালা->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc) ){
            sqlite3_Walk_রাশিমালা(pWalker, p_রাশিমালা->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          যখন( pNC2
              && sqlite3ReferencesSrcList(pParse, p_রাশিমালা, pNC2->pSrcList)==0
          ){
            p_রাশিমালা->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->p_পরবর্তী;
          }
          প্রতিজ্ঞা( pDef!=0 || IN_RENAME_OBJECT );
          যদি( pNC2 && pDef ){
            p_রাশিমালা->op2 += pNC2->nNestedSelect;
            প্রতিজ্ঞা( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            প্রতিজ্ঞা( SQLITE_FUNC_ANYORDER==NC_OrderAgg );
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
      ফেরত WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ক্ষেত্র TK_EXISTS:
    ক্ষেত্র TK_SELECT:
#endif
    ক্ষেত্র TK_IN: {
      testcase( p_রাশিমালা->op==TK_IN );
      testcase( p_রাশিমালা->op==TK_EXISTS );
      testcase( p_রাশিমালা->op==TK_SELECT );
      যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
        পূর্ণসংখ্যা nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_রাশিমালা );
        testcase( pNC->ncFlags & NC_GenCol );
        প্রতিজ্ঞা( p_রাশিমালা->x.pSelect );
        যদি( p_রাশিমালা->op==TK_EXISTS )  pParse->bHasExists = 1;
        যদি( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_রাশিমালা, p_রাশিমালা);
        }অন্যথা{
          sqlite3WalkSelect(pWalker, p_রাশিমালা->x.pSelect);
        }
        প্রতিজ্ঞা( pNC->nRef>=nRef );
        যদি( nRef!=pNC->nRef ){
          রাশিমালা_Set_Property(p_রাশিমালা, EP_VarSelect);
          p_রাশিমালা->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      বিরতি;
    }
    ক্ষেত্র TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_রাশিমালা );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_রাশিমালা|NC_GenCol, p_রাশিমালা, p_রাশিমালা);
      বিরতি;
    }
    ক্ষেত্র TK_IS:
    ক্ষেত্র TK_ISNOT: {
      রাশিমালা *p_ডান = sqlite3_রাশিমালা_Skip_Collate_And_Likely(p_রাশিমালা->p_ডান);
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      যদি( ALWAYS(p_ডান) && (p_ডান->op==TK_ID || p_ডান->op==TK_TRUEFALSE) ){
        পূর্ণসংখ্যা rc = resolve_রাশিমালা_Step(pWalker, p_ডান);
        যদি( rc==WRC_Abort ) ফেরত WRC_Abort;
        যদি( p_ডান->op==TK_TRUEFALSE ){
          p_রাশিমালা->op2 = p_রাশিমালা->op;
          p_রাশিমালা->op = TK_TRUTH;
          ফেরত WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    ক্ষেত্র TK_BETWEEN:
    ক্ষেত্র TK_EQ:
    ক্ষেত্র TK_NE:
    ক্ষেত্র TK_LT:
    ক্ষেত্র TK_LE:
    ক্ষেত্র TK_GT:
    ক্ষেত্র TK_GE: {
      পূর্ণসংখ্যা n_বাম, n_ডান;
      যদি( pParse->db->mallocFailed ) বিরতি;
      প্রতিজ্ঞা( p_রাশিমালা->p_বাম!=0 );
      n_বাম = sqlite3_রাশিমালা_Vector_আকার(p_রাশিমালা->p_বাম);
      যদি( p_রাশিমালা->op==TK_BETWEEN ){
        প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
        n_ডান = sqlite3_রাশিমালা_Vector_আকার(p_রাশিমালা->x.pList->a[0].p_রাশিমালা);
        যদি( n_ডান==n_বাম ){
          n_ডান = sqlite3_রাশিমালা_Vector_আকার(p_রাশিমালা->x.pList->a[1].p_রাশিমালা);
        }
      }অন্যথা{
        প্রতিজ্ঞা( p_রাশিমালা->p_ডান!=0 );
        n_ডান = sqlite3_রাশিমালা_Vector_আকার(p_রাশিমালা->p_ডান);
      }
      যদি( n_বাম!=n_ডান ){
        testcase( p_রাশিমালা->op==TK_EQ );
        testcase( p_রাশিমালা->op==TK_NE );
        testcase( p_রাশিমালা->op==TK_LT );
        testcase( p_রাশিমালা->op==TK_LE );
        testcase( p_রাশিমালা->op==TK_GT );
        testcase( p_রাশিমালা->op==TK_GE );
        testcase( p_রাশিমালা->op==TK_IS );
        testcase( p_রাশিমালা->op==TK_ISNOT );
        testcase( p_রাশিমালা->op==TK_BETWEEN );
        sqlite3_Error_বার্তা(pParse, "row value misused");
        sqlite3_Record_Error_Offset_Of_রাশিমালা(pParse->db, p_রাশিমালা);
      }
      বিরতি;
    }
  }
  প্রতিজ্ঞা( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  ফেরত pParse->nErr ? WRC_Abort : WRC_Continue;
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
স্থির পূর্ণসংখ্যা resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  রাশিমালা_List *pEList,  /* List of expressions to scan */
  রাশিমালা *pE           /* Expression we are trying to match */
){
  পূর্ণসংখ্যা i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  যদি( pE->op==TK_ID ){
    ধ্রুবক অক্ষর *zCol;
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(pE, EP_Int_মান) );
    zCol = pE->u.zToken;
    জন্য(i=0; i<pEList->n_রাশিমালা; i++){
      যদি( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        ফেরত i+1;
      }
    }
  }
  ফেরত 0;
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
স্থির পূর্ণসংখ্যা resolve_Order_By_Term_To_রাশিমালা_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  রাশিমালা *pE           /* The specific ORDER BY term */
){
  পূর্ণসংখ্যা i;             /* Loop counter */
  রাশিমালা_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  পূর্ণসংখ্যা rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  প্রতিজ্ঞা( sqlite3_রাশিমালা_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, আকার(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_রাশিমালা_Names(&nc, pE);
  db->suppressErr = savedSuppErr;
  যদি( rc ) ফেরত 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  জন্য(i=0; i<pEList->n_রাশিমালা; i++){
    যদি( sqlite3_রাশিমালা_Compare(0, pEList->a[i].p_রাশিমালা, pE, -1)<2 ){
      ফেরত i+1;
    }
  }

  /* If no match, return 0. */
  ফেরত 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
স্থির শূন্য resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  ধ্রুবক অক্ষর *zType,     /* "ORDER" or "GROUP" */
  পূর্ণসংখ্যা i,                 /* The index (1-based) of the term out of range */
  পূর্ণসংখ্যা mx,                /* Largest permissible value of i */
  রাশিমালা *pError           /* Associate the error with the expression */
){
  sqlite3_Error_বার্তা(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_Record_Error_Offset_Of_রাশিমালা(pParse->db, pError);
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
স্থির পূর্ণসংখ্যা resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  পূর্ণসংখ্যা i;
  রাশিমালা_List *pOrderBy;
  রাশিমালা_List *pEList;
  sqlite3 *db;
  পূর্ণসংখ্যা moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  যদি( pOrderBy==0 ) ফেরত 0;
  db = pParse->db;
  যদি( pOrderBy->n_রাশিমালা>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_বার্তা(pParse, "too many terms in ORDER BY clause");
    ফেরত 1;
  }
  জন্য(i=0; i<pOrderBy->n_রাশিমালা; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->p_পরবর্তী = 0;
  যখন( pSelect->pPrior ){
    pSelect->pPrior->p_পরবর্তী = pSelect;
    pSelect = pSelect->pPrior;
  }
  যখন( pSelect && moreToDo ){
    কাঠামো রাশিমালা_List_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    প্রতিজ্ঞা( pEList!=0 );
    জন্য(i=0, pItem=pOrderBy->a; i<pOrderBy->n_রাশিমালা; i++, pItem++){
      পূর্ণসংখ্যা iCol = -1;
      রাশিমালা *pE, *pDup;
      যদি( pItem->fg.done ) চালু;
      pE = sqlite3_রাশিমালা_Skip_Collate_And_Likely(pItem->p_রাশিমালা);
      যদি( NEVER(pE==0) ) চালু;
      যদি( sqlite3_রাশিমালা_Is_Integer(pE, &iCol, 0, 1) ){
        যদি( iCol<=0 || iCol>pEList->n_রাশিমালা ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->n_রাশিমালা, pE);
          ফেরত 1;
        }
      }অন্যথা{
        iCol = resolveAsName(pParse, pEList, pE);
        যদি( iCol==0 ){
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
          pDup = sqlite3_রাশিমালা_Dup(db, pE, 0);
          যদি( !db->mallocFailed ){
            প্রতিজ্ঞা(pDup);
            iCol = resolve_Order_By_Term_To_রাশিমালা_List(pParse, pSelect, pDup);
            যদি( IN_RENAME_OBJECT && iCol>0 ){
              resolve_Order_By_Term_To_রাশিমালা_List(pParse, pSelect, pE);
            }
          }
          sqlite3_রাশিমালা_Delete(db, pDup);
        }
      }
      যদি( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        যদি( !IN_RENAME_OBJECT ){
          রাশিমালা *pNew = sqlite3_রাশিমালা_Int32(db, iCol);
          যদি( pNew==0 ) ফেরত 1;
          যদি( pItem->p_রাশিমালা==pE ){
            pItem->p_রাশিমালা = pNew;
          }অন্যথা{
            রাশিমালা *pParent = pItem->p_রাশিমালা;
            প্রতিজ্ঞা( pParent->op==TK_COLLATE );
            যখন( pParent->p_বাম->op==TK_COLLATE ) pParent = pParent->p_বাম;
            প্রতিজ্ঞা( pParent->p_বাম==pE );
            pParent->p_বাম = pNew;
          }
          sqlite3_রাশিমালা_Delete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }অন্যথা{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->p_পরবর্তী;
  }
  জন্য(i=0; i<pOrderBy->n_রাশিমালা; i++){
    যদি( pOrderBy->a[i].fg.done==0 ){
      sqlite3_Error_বার্তা(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      ফেরত 1;
    }
  }
  ফেরত 0;
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
পূর্ণসংখ্যা sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  রাশিমালা_List *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  ধ্রুবক অক্ষর *zType     /* "ORDER" or "GROUP" */
){
  পূর্ণসংখ্যা i;
  sqlite3 *db = pParse->db;
  রাশিমালা_List *pEList;
  কাঠামো রাশিমালা_List_item *pItem;

  যদি( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) ফেরত 0;
  যদি( pOrderBy->n_রাশিমালা>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_বার্তা(pParse, "too many terms in %s BY clause", zType);
    ফেরত 1;
  }
  pEList = pSelect->pEList;
  প্রতিজ্ঞা( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  জন্য(i=0, pItem=pOrderBy->a; i<pOrderBy->n_রাশিমালা; i++, pItem++){
    যদি( pItem->u.x.iOrderByCol ){
      যদি( pItem->u.x.iOrderByCol>pEList->n_রাশিমালা ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->n_রাশিমালা, 0);
        ফেরত 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->p_রাশিমালা,0);
    }
  }
  ফেরত 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
স্থির পূর্ণসংখ্যা resolveRemoveWindowsCb(Walker *pWalker, রাশিমালা *p_রাশিমালা){
  UNUSED_PARAMETER(pWalker);
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_WinFunc) ){
    Window *pWin = p_রাশিমালা->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  ফেরত WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
স্থির শূন্য window_Remove_রাশিমালা_From_Select(Select *pSelect, রাশিমালা *p_রাশিমালা){
  যদি( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, আকার(Walker));
    sWalker.x_রাশিমালা_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_রাশিমালা(&sWalker, p_রাশিমালা);
  }
}
#অন্যথা
# define window_Remove_রাশিমালা_From_Select(a, b)
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
স্থির পূর্ণসংখ্যা resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  রাশিমালা_List *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  ধ্রুবক অক্ষর *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  পূর্ণসংখ্যা i, j;                      /* Loop counters */
  পূর্ণসংখ্যা iCol;                      /* Column number */
  কাঠামো রাশিমালা_List_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  পূর্ণসংখ্যা n_ফলাফল;                   /* Number of terms in the result set */

  প্রতিজ্ঞা( pOrderBy!=0 );
  n_ফলাফল = pSelect->pEList->n_রাশিমালা;
  pParse = pNC->pParse;
  জন্য(i=0, pItem=pOrderBy->a; i<pOrderBy->n_রাশিমালা; i++, pItem++){
    রাশিমালা *pE = pItem->p_রাশিমালা;
    রাশিমালা *pE2 = sqlite3_রাশিমালা_Skip_Collate_And_Likely(pE);
    যদি( NEVER(pE2==0) ) চালু;
    যদি( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      যদি( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        চালু;
      }
    }
    যদি( sqlite3_রাশিমালা_Is_Integer(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      যদি( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, n_ফলাফল, pE2);
        ফেরত 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      চালু;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    যদি( sqlite3_Resolve_রাশিমালা_Names(pNC, pE) ){
      ফেরত 1;
    }
    জন্য(j=0; j<pSelect->pEList->n_রাশিমালা; j++){
      যদি( sqlite3_রাশিমালা_Compare(0, pE, pSelect->pEList->a[j].p_রাশিমালা, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        window_Remove_রাশিমালা_From_Select(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  ফেরত sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
স্থির পূর্ণসংখ্যা resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  পূর্ণসংখ্যা isCompound;         /* True if p is a compound select */
  পূর্ণসংখ্যা nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  পূর্ণসংখ্যা i;                  /* Loop counter */
  রাশিমালা_List *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  প্রতিজ্ঞা( p!=0 );
  যদি( p->selFlags & SF_Resolved ){
    ফেরত WRC_Prune;
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
  যদি( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    ফেরত pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  যখন( p ){
    প্রতিজ্ঞা( (p->selFlags & SF_Expanded)!=0 );
    প্রতিজ্ঞা( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, আকার(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    যদি( sqlite3_Resolve_রাশিমালা_Names(&sNC, p->pLimit) ){
      ফেরত WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    যদি( p->selFlags & SF_Converted ){
      Select *pSub;
      প্রতিজ্ঞা( p->pSrc->a[0].fg.isSubquery );
      প্রতিজ্ঞা( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      প্রতিজ্ঞা( pSub!=0 );
      প্রতিজ্ঞা( p->pSrc->nSrc==1 && p->pOrderBy );
      প্রতিজ্ঞা( pSub->pPrior && pSub->pOrderBy==0 );
      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    যদি( pOuterNC ) pOuterNC->nNestedSelect++;
    জন্য(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      প্রতিজ্ঞা( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      যদি( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        পূর্ণসংখ্যা nRef = pOuterNC ? pOuterNC->nRef : 0;
        ধ্রুবক অক্ষর *zSavedContext = pParse->zAuthContext;

        যদি( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        যদি( pParse->nErr ) ফেরত WRC_Abort;
        প্রতিজ্ঞা( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        যদি( pOuterNC ){
          প্রতিজ্ঞা( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    যদি( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.p_পরবর্তী = pOuterNC;
 
    /* Resolve names in the result set. */
    যদি( sqlite3_Resolve_রাশিমালা_List_Names(&sNC, p->pEList) ) ফেরত WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    প্রতিজ্ঞা( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    যদি( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      প্রতিজ্ঞা( NC_MinMaxAgg==SF_MinMaxAgg );
      প্রতিজ্ঞা( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }অন্যথা{
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
    প্রতিজ্ঞা( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    যদি( p->pHaving ){
      যদি( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_বার্তা(pParse, "HAVING clause on a non-aggregate query");
        ফেরত WRC_Abort;
      }
      যদি( sqlite3_Resolve_রাশিমালা_Names(&sNC, p->pHaving) ) ফেরত WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    যদি( sqlite3_Resolve_রাশিমালা_Names(&sNC, p->pWhere) ) ফেরত WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    জন্য(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      যদি( pItem->fg.isTabFunc
       && sqlite3_Resolve_রাশিমালা_List_Names(&sNC, pItem->u1.pFuncArg)
      ){
        ফেরত WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    যদি( IN_RENAME_OBJECT ){
      Window *pWin;
      জন্য(pWin=p->pWinDefn; pWin; pWin=pWin->p_পরবর্তী_Win){
        যদি( sqlite3_Resolve_রাশিমালা_List_Names(&sNC, pWin->pOrderBy)
         || sqlite3_Resolve_রাশিমালা_List_Names(&sNC, pWin->pPartition)
        ){
          ফেরত WRC_Abort;
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
    যদি( p->selFlags & SF_Converted ){
      Select *pSub;
      প্রতিজ্ঞা( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      প্রতিজ্ঞা( pSub!=0 );
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
    যদি( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      ফেরত WRC_Abort;
    }
    যদি( db->mallocFailed ){
      ফেরত WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    যদি( pGroupBy ){
      কাঠামো রাশিমালা_List_item *pItem;
   
      যদি( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        ফেরত WRC_Abort;
      }
      জন্য(i=0, pItem=pGroupBy->a; i<pGroupBy->n_রাশিমালা; i++, pItem++){
        যদি( রাশিমালা_Has_Property(pItem->p_রাশিমালা, EP_Agg) ){
          sqlite3_Error_বার্তা(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          ফেরত WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    যদি( p->p_পরবর্তী && p->pEList->n_রাশিমালা!=p->p_পরবর্তী->pEList->n_রাশিমালা ){
      sqlite3_Select_Wrong_অঙ্ক_Terms_Error(pParse, p->p_পরবর্তী);
      ফেরত WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    যদি( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      যদি( pParse->nErr ) ফেরত WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  যদি( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    ফেরত WRC_Abort;
  }

  ফেরত WRC_Prune;
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
পূর্ণসংখ্যা sqlite3_Resolve_রাশিমালা_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  রাশিমালা *p_রাশিমালা             /* The expression to be analyzed. */
){
  পূর্ণসংখ্যা savedHasAgg;
  Walker w;

  যদি( p_রাশিমালা==0 ) ফেরত SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.x_রাশিমালা_Callback = resolve_রাশিমালা_Step;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
  w.pParse->nHeight += p_রাশিমালা->nHeight;
  যদি( sqlite3_রাশিমালা_Check_Height(w.pParse, w.pParse->nHeight) ){
    ফেরত SQLITE_ERROR;
  }
#endif
  প্রতিজ্ঞা( p_রাশিমালা!=0 );
  sqlite3_Walk_রাশিমালা_NN(&w, p_রাশিমালা);
#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
  w.pParse->nHeight -= p_রাশিমালা->nHeight;
#endif
  প্রতিজ্ঞা( EP_Agg==NC_HasAgg );
  প্রতিজ্ঞা( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  রাশিমালা_Set_Property(p_রাশিমালা, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  ফেরত pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
পূর্ণসংখ্যা sqlite3_Resolve_রাশিমালা_List_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  রাশিমালা_List *pList         /* The expression list to be analyzed. */
){
  পূর্ণসংখ্যা i;
  পূর্ণসংখ্যা savedHasAgg = 0;
  Walker w;
  যদি( pList==0 ) ফেরত SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_রাশিমালা_Callback = resolve_রাশিমালা_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  জন্য(i=0; i<pList->n_রাশিমালা; i++){
    রাশিমালা *p_রাশিমালা = pList->a[i].p_রাশিমালা;
    যদি( p_রাশিমালা==0 ) চালু;
#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
    w.pParse->nHeight += p_রাশিমালা->nHeight;
    যদি( sqlite3_রাশিমালা_Check_Height(w.pParse, w.pParse->nHeight) ){
      ফেরত SQLITE_ERROR;
    }
#endif
    sqlite3_Walk_রাশিমালা_NN(&w, p_রাশিমালা);
#যদি SQLITE_MAX_রাশিমালা_DEPTH>0
    w.pParse->nHeight -= p_রাশিমালা->nHeight;
#endif
    প্রতিজ্ঞা( EP_Agg==NC_HasAgg );
    প্রতিজ্ঞা( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    যদি( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      রাশিমালা_Set_Property(p_রাশিমালা, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    যদি( w.pParse->nErr>0 ) ফেরত SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  ফেরত SQLITE_OK;
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
শূন্য sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  প্রতিজ্ঞা( p!=0 );
  w.x_রাশিমালা_Callback = resolve_রাশিমালা_Step;
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
পূর্ণসংখ্যা sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  পূর্ণসংখ্যা type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  রাশিমালা *p_রাশিমালা,     /* Expression to resolve.  May be NULL. */
  রাশিমালা_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  পূর্ণসংখ্যা rc;
  সংঘ {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  প্রতিজ্ঞা( type==0 || pTab!=0 );
  প্রতিজ্ঞা( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_রাশিমালা
          || type==NC_GenCol || pTab==0 );
  memset(&sNC, 0, আকার(sNC));
  memset(&uSrc, 0, আকার(uSrc));
  pSrc = &uSrc.sSrc;
  যদি( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    যদি( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  যদি( (rc = sqlite3_Resolve_রাশিমালা_Names(&sNC, p_রাশিমালা))!=SQLITE_OK ) ফেরত rc;
  যদি( pList ) rc = sqlite3_Resolve_রাশিমালা_List_Names(&sNC, pList);
  ফেরত rc;
}
