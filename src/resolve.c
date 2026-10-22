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
#അന്തര്ഭാവ "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#define EXCLUDED_TABLE_സംഖ്യ  2

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
സ്ഥിതം പൂർണ്ണസംഖ്യ incrAggDepth(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_AGG_FUNCTION ) p_വ്യഞ്ജകം->op2 += pWalker->u.n;
  തിരികെ WRC_Continue;
}
സ്ഥിതം ശൂന്യം incrAggFunctionDepth(വ്യഞ്ജകം *p_വ്യഞ്ജകം, പൂർണ്ണസംഖ്യ N){
  എങ്കിൽ( N>0 ){
    Walker w;
    memset(&w, 0, വലുപ്പം(w));
    w.x_വ്യഞ്ജകം_Callback = incrAggDepth;
    w.u.n = N;
    sqlite3_Walk_വ്യഞ്ജകം(&w, p_വ്യഞ്ജകം);
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
സ്ഥിതം ശൂന്യം resolveAlias(
  Parse *pParse,         /* Parsing context */
  വ്യഞ്ജകം_List *pEList,      /* A result set */
  പൂർണ്ണസംഖ്യ iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,           /* Transform this into an alias to the result set */
  പൂർണ്ണസംഖ്യ nSubquery          /* Number of subqueries that the label is moving */
){
  വ്യഞ്ജകം *pOrig;           /* The iCol-th column of the result set */
  വ്യഞ്ജകം *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  പ്രതിജ്ഞാ( iCol>=0 && iCol<pEList->n_വ്യഞ്ജകം );
  pOrig = pEList->a[iCol].p_വ്യഞ്ജകം;
  പ്രതിജ്ഞാ( pOrig!=0 );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Reduced|EP_TokenOnly) );
  എങ്കിൽ( p_വ്യഞ്ജകം->pAggInfo ) തിരികെ;
  db = pParse->db;
  pDup = sqlite3_വ്യഞ്ജകം_Dup(db, pOrig, 0);
  എങ്കിൽ( db->mallocFailed ){
    sqlite3_വ്യഞ്ജകം_Delete(db, pDup);
    pDup = 0;
  }അല്ലെങ്കിൽ{
    വ്യഞ്ജകം temp;
    incrAggFunctionDepth(pDup, nSubquery);
    എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_COLLATE ){
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      pDup = sqlite3_വ്യഞ്ജകം_Add_Collate_String(pParse, pDup, p_വ്യഞ്ജകം->u.zToken);
    }
    memcpy(&temp, pDup, വലുപ്പം(വ്യഞ്ജകം));
    memcpy(pDup, p_വ്യഞ്ജകം, വലുപ്പം(വ്യഞ്ജകം));
    memcpy(p_വ്യഞ്ജകം, &temp, വലുപ്പം(വ്യഞ്ജകം));
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) ){
      എങ്കിൽ( ALWAYS(p_വ്യഞ്ജകം->y.pWin!=0) ){
        p_വ്യഞ്ജകം->y.pWin->pOwner = p_വ്യഞ്ജകം;
      }
    }
    sqlite3_വ്യഞ്ജകം_Deferred_Delete(pParse, pDup);
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
പൂർണ്ണസംഖ്യ sqlite3MatchEName(
  സ്ഥിരം ഘടന വ്യഞ്ജകം_List_item *pItem,
  സ്ഥിരം അക്ഷരം *zCol,
  സ്ഥിരം അക്ഷരം *zTab,
  സ്ഥിരം അക്ഷരം *zDb,
  പൂർണ്ണസംഖ്യ *pbRowid
){
  പൂർണ്ണസംഖ്യ n;
  സ്ഥിരം അക്ഷരം *zSpan;
  പൂർണ്ണസംഖ്യ eEName = pItem->fg.eEName;
  എങ്കിൽ( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    തിരികെ 0;
  }
  പ്രതിജ്ഞാ( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  വേണ്ടി(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  എങ്കിൽ( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    തിരികെ 0;
  }
  zSpan += n+1;
  വേണ്ടി(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  എങ്കിൽ( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    തിരികെ 0;
  }
  zSpan += n+1;
  എങ്കിൽ( zCol ){
    എങ്കിൽ( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) തിരികെ 0;
    എങ്കിൽ( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) തിരികെ 0;
  }
  എങ്കിൽ( eEName==ENAME_ROWID ) *pbRowid = 1;
  തിരികെ 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *p_മുകൾ_NC){
  എങ്കിൽ( db->init.busy ) തിരികെ 1;  /* Always support for legacy schemas */
  എങ്കിൽ( p_മുകൾ_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    എങ്കിൽ( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      തിരികെ 1;
    }
    തിരികെ (db->flags & SQLITE_DqsDDL)!=0;
  }അല്ലെങ്കിൽ{
    /* Currently parsing a DML statement */
    തിരികെ (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_വ്യഞ്ജകം_Col_Used(വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പൂർണ്ണസംഖ്യ n;
  Table *pExTab;

  n = p_വ്യഞ്ജകം->iColumn;
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
  pExTab = p_വ്യഞ്ജകം->y.pTab;
  പ്രതിജ്ഞാ( pExTab!=0 );
  പ്രതിജ്ഞാ( n < pExTab->nCol );
  എങ്കിൽ( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    തിരികെ pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }അല്ലെങ്കിൽ{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    എങ്കിൽ( n>=BMS ) n = BMS-1;
    തിരികെ ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
സ്ഥിതം ശൂന്യം extendFJMatch(
  Parse *pParse,          /* Parsing context */
  വ്യഞ്ജകം_List **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  വ്യഞ്ജകം *pNew = sqlite3_വ്യഞ്ജകം_Alloc(pParse->db, TK_COLUMN, 0, 0);
  എങ്കിൽ( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    പ്രതിജ്ഞാ( (pMatch->fg.jointype & (JT_ഇടത്|JT_LTORJ))!=0 );
    വ്യഞ്ജകം_Set_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_വ്യഞ്ജകം_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ isValidSchemaTableName(
  സ്ഥിരം അക്ഷരം *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  സ്ഥിരം അക്ഷരം *zDb           /* non-NULL if a database qualifier is present */
){
  സ്ഥിരം അക്ഷരം *zLegacy;
  പ്രതിജ്ഞാ( pTab!=0 );
  പ്രതിജ്ഞാ( pTab->tnum==1 );
  എങ്കിൽ( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) തിരികെ 0;
  zLegacy = pTab->zName;
  എങ്കിൽ( strcmp(zLegacy+7, &LEGACY_TEMP_SCHEMA_TABLE[7])==0 ){
    എങ്കിൽ( sqlite3StrICmp(zTab+7, &PREFERRED_TEMP_SCHEMA_TABLE[7])==0 ){
      തിരികെ 1;
    }
    എങ്കിൽ( zDb==0 ) തിരികെ 0;
    എങ്കിൽ( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) തിരികെ 1;
    എങ്കിൽ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) തിരികെ 1;
  }അല്ലെങ്കിൽ{
    എങ്കിൽ( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) തിരികെ 1;
  }
  തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ lookupName(
  Parse *pParse,       /* The parsing context */
  സ്ഥിരം അക്ഷരം *zDb,     /* Name of the database containing table, or NULL */
  സ്ഥിരം അക്ഷരം *zTab,    /* Name of table containing column, or NULL */
  സ്ഥിരം വ്യഞ്ജകം *p_വലത്,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം          /* Make this EXPR node point to the selected column */
){
  പൂർണ്ണസംഖ്യ i, j;                         /* Loop counters */
  പൂർണ്ണസംഖ്യ cnt = 0;                      /* Number of matching column names */
  പൂർണ്ണസംഖ്യ cntTab = 0;                   /* Number of potential "rowid" matches */
  പൂർണ്ണസംഖ്യ nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *p_മുകൾ_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  പൂർണ്ണസംഖ്യ e_New_വ്യഞ്ജകം_Op = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  വ്യഞ്ജകം_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  സ്ഥിരം അക്ഷരം *zCol = p_വലത്->u.zToken;

  പ്രതിജ്ഞാ( pNC );     /* the name context cannot be NULL. */
  പ്രതിജ്ഞാ( zCol );    /* The Z in X.Y.Z cannot be NULL */
  പ്രതിജ്ഞാ( zDb==0 || zTab!=0 );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_വ്യഞ്ജകം->iTable = -1;
  വ്യഞ്ജകം_Set_VVAProperty(p_വ്യഞ്ജകം, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  എങ്കിൽ( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    എങ്കിൽ( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }അല്ലെങ്കിൽ{
      വേണ്ടി(i=0; i<db->nDb; i++){
        പ്രതിജ്ഞാ( db->aDb[i].zDbSName );
        എങ്കിൽ( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          നിർത്തുക;
        }
      }
      എങ്കിൽ( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  പ്രതിജ്ഞാ( pNC && cnt==0 );
  ചെയ്യുക{
    വ്യഞ്ജകം_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    എങ്കിൽ( pSrcList ){
      വേണ്ടി(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        പ്രതിജ്ഞാ( pTab!=0 && pTab->zName!=0 );
        പ്രതിജ്ഞാ( pTab->nCol>0 || pParse->nErr );
        പ്രതിജ്ഞാ( (പൂർണ്ണസംഖ്യ)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        എങ്കിൽ( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          പൂർണ്ണസംഖ്യ hit = 0;
          Select *pSel;
          പ്രതിജ്ഞാ( pItem->fg.isSubquery );
          പ്രതിജ്ഞാ( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          പ്രതിജ്ഞാ( pSel!=0 );
          pEList = pSel->pEList;
          പ്രതിജ്ഞാ( pEList!=0 );
          പ്രതിജ്ഞാ( pEList->n_വ്യഞ്ജകം==pTab->nCol );
          വേണ്ടി(j=0; j<pEList->n_വ്യഞ്ജകം; j++){
            പൂർണ്ണസംഖ്യ bRowid = 0;       /* True if possible rowid match */
            എങ്കിൽ( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              തുടരുക;
            }
            എങ്കിൽ( bRowid==0 ){
              എങ്കിൽ( cnt>0 ){
                എങ്കിൽ( pItem->fg.isUsing==0
                 || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_വ്യഞ്ജകം_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }അല്ലെങ്കിൽ
                എങ്കിൽ( (pItem->fg.jointype & JT_വലത്)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  തുടരുക;
                }അല്ലെങ്കിൽ
                എങ്കിൽ( (pItem->fg.jointype & JT_ഇടത്)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_വ്യഞ്ജകം_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }അല്ലെങ്കിൽ{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, p_വ്യഞ്ജകം->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }അല്ലെങ്കിൽ എങ്കിൽ( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              തുടരുക;
            }
            cntTab++;
            pMatch = pItem;
            p_വ്യഞ്ജകം->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            പ്രതിജ്ഞാ( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            എങ്കിൽ( pEList->a[j].fg.bUsingTerm ) നിർത്തുക;
          }
          എങ്കിൽ( hit || zTab==0 ) തുടരുക;
        }
        പ്രതിജ്ഞാ( zDb==0 || zTab!=0 );
        എങ്കിൽ( zTab ){
          എങ്കിൽ( zDb ){
            എങ്കിൽ( pTab->pSchema!=pSchema ) തുടരുക;
            എങ്കിൽ( pSchema==0 && strcmp(zDb,"*")!=0 ) തുടരുക;
          }
          എങ്കിൽ( pItem->zAlias!=0 ){
            എങ്കിൽ( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              തുടരുക;
            }
          }അല്ലെങ്കിൽ എങ്കിൽ( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            എങ്കിൽ( pTab->tnum!=1 ) തുടരുക;
            എങ്കിൽ( !isValidSchemaTableName(zTab, pTab, zDb) ) തുടരുക;
          }
          പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
          എങ്കിൽ( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (ശൂന്യം*)&p_വ്യഞ്ജകം->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        എങ്കിൽ( j>=0 ){
          എങ്കിൽ( cnt>0 ){
            എങ്കിൽ( pItem->fg.isUsing==0
             || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_വ്യഞ്ജകം_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }അല്ലെങ്കിൽ
            എങ്കിൽ( (pItem->fg.jointype & JT_വലത്)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              തുടരുക;
            }അല്ലെങ്കിൽ
            എങ്കിൽ( (pItem->fg.jointype & JT_ഇടത്)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_വ്യഞ്ജകം_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }അല്ലെങ്കിൽ{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, p_വ്യഞ്ജകം->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_വ്യഞ്ജകം->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          എങ്കിൽ( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        എങ്കിൽ( 0==cnt && VisibleRowid(pTab) ){
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
          എങ്കിൽ( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }അല്ലെങ്കിൽ{
            cntTab++;
          }
#അല്ലെങ്കിൽ
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      എങ്കിൽ( pMatch ){
        p_വ്യഞ്ജകം->iTable = pMatch->iCursor;
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
        p_വ്യഞ്ജകം->y.pTab = pMatch->pSTab;
        എങ്കിൽ( (pMatch->fg.jointype & (JT_ഇടത്|JT_LTORJ))!=0 ){
          വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_CanBeNull);
        }
        pSchema = p_വ്യഞ്ജകം->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#എങ്കിൽ !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    എങ്കിൽ( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      എങ്കിൽ( pParse->pTriggerTab!=0 ){
        പൂർണ്ണസംഖ്യ op = pParse->eTriggerOp;
        പ്രതിജ്ഞാ( op==TK_DELETE || op==TK_UPDATE || op==TK_തിരുകുക );
        എങ്കിൽ( pParse->bReturning ){
          എങ്കിൽ( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            p_വ്യഞ്ജകം->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }അല്ലെങ്കിൽ എങ്കിൽ( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_വ്യഞ്ജകം->iTable = 1;
          pTab = pParse->pTriggerTab;
        }അല്ലെങ്കിൽ എങ്കിൽ( op!=TK_തിരുകുക && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_വ്യഞ്ജകം->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      എങ്കിൽ( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        എങ്കിൽ( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_വ്യഞ്ജകം->iTable = EXCLUDED_TABLE_സംഖ്യ;
          എങ്കിൽ( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ശൂന്യം*)&p_വ്യഞ്ജകം->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      എങ്കിൽ( pTab ){
        പൂർണ്ണസംഖ്യ iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        എങ്കിൽ( iCol>=0 ){
          എങ്കിൽ( pTab->iPKey==iCol ) iCol = -1;
        }അല്ലെങ്കിൽ{
          എങ്കിൽ( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }അല്ലെങ്കിൽ{
            iCol = pTab->nCol;
          }
        }
        എങ്കിൽ( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          എങ്കിൽ( p_വ്യഞ്ജകം->iTable==EXCLUDED_TABLE_സംഖ്യ ){
            testcase( iCol==(-1) );
            പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
            എങ്കിൽ( IN_RENAME_OBJECT ){
              p_വ്യഞ്ജകം->iColumn = iCol;
              p_വ്യഞ്ജകം->y.pTab = pTab;
              e_New_വ്യഞ്ജകം_Op = TK_COLUMN;
            }അല്ലെങ്കിൽ{
              p_വ്യഞ്ജകം->iTable = pNC->uNC.pUpsert->regData +
                 sqlite3TableColumnToStorage(pTab, iCol);
              e_New_വ്യഞ്ജകം_Op = TK_REGISTER;
            }
          }അല്ലെങ്കിൽ
#endif /* SQLITE_OMIT_UPSERT */
          {
            പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
            p_വ്യഞ്ജകം->y.pTab = pTab;
            എങ്കിൽ( pParse->bReturning ){
              e_New_വ്യഞ്ജകം_Op = TK_REGISTER;
              p_വ്യഞ്ജകം->op2 = TK_COLUMN;
              p_വ്യഞ്ജകം->iColumn = iCol;
              p_വ്യഞ്ജകം->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*p_വ്യഞ്ജകം->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }അല്ലെങ്കിൽ{
              p_വ്യഞ്ജകം->iColumn = (i16)iCol;
              e_New_വ്യഞ്ജകം_Op = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              എങ്കിൽ( iCol<0 ){
                p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം = SQLITE_AFF_INTEGER;
              }അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }അല്ലെങ്കിൽ{
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
    എങ്കിൽ( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_Idx_വ്യഞ്ജകം|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#എങ്കിൽ SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      എങ്കിൽ( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        e_New_വ്യഞ്ജകം_Op = TK_NULL;
      }
#endif
      എങ്കിൽ( pMatch->fg.isNestedFrom==0 ) p_വ്യഞ്ജകം->iColumn = -1;
      p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം = SQLITE_AFF_INTEGER;
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
    എങ്കിൽ( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      പ്രതിജ്ഞാ( pEList!=0 );
      വേണ്ടി(j=0; j<pEList->n_വ്യഞ്ജകം; j++){
        അക്ഷരം *zAs = pEList->a[j].zEName;
        എങ്കിൽ( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          വ്യഞ്ജകം *pOrig;
          പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത്==0 && p_വ്യഞ്ജകം->p_വലത്==0 );
          പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം)==0 || p_വ്യഞ്ജകം->x.pList==0 );
          പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം)==0 || p_വ്യഞ്ജകം->x.pSelect==0 );
          pOrig = pEList->a[j].p_വ്യഞ്ജകം;
          എങ്കിൽ( (pNC->ncFlags&NC_AllowAgg)==0 && വ്യഞ്ജകം_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_സന്ദേശം(pParse, "misuse of aliased aggregate %s", zAs);
            തിരികെ WRC_Abort;
          }
          എങ്കിൽ( വ്യഞ്ജകം_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_മുകൾ_NC )
          ){
            sqlite3_Error_സന്ദേശം(pParse, "misuse of aliased window function %s",zAs);
            തിരികെ WRC_Abort;
          }
          എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Vector_Size(pOrig)!=1 ){
            sqlite3_Error_സന്ദേശം(pParse, "row value misused");
            തിരികെ WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_വ്യഞ്ജകം, nSubquery);
          cnt = 1;
          pMatch = 0;
          പ്രതിജ്ഞാ( zTab==0 && zDb==0 );
          എങ്കിൽ( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (ശൂന്യം*)p_വ്യഞ്ജകം);
          }
          പോകുക lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    എങ്കിൽ( cnt ) നിർത്തുക;
    pNC = pNC->pNext;
    nSubquery++;
  }ആകുമ്പോൾ( pNC );


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
  എങ്കിൽ( cnt==0 && zTab==0 ){
    പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_ID );
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, p_മുകൾ_NC)
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
      p_വ്യഞ്ജകം->op = TK_STRING;
      memset(&p_വ്യഞ്ജകം->y, 0, വലുപ്പം(p_വ്യഞ്ജകം->y));
      തിരികെ WRC_Prune;
    }
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Id_To_True_False(p_വ്യഞ്ജകം) ){
      തിരികെ WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  പ്രതിജ്ഞാ( pFJMatch==0 || cnt>0 );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_xIsSelect|EP_Int_മൂല്യം) );
  എങ്കിൽ( cnt!=1 ){
    സ്ഥിരം അക്ഷരം *zErr;
    എങ്കിൽ( pFJMatch ){
      എങ്കിൽ( pFJMatch->n_വ്യഞ്ജകം==cnt-1 ){
        എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_Leaf) ){
          വ്യഞ്ജകം_Clear_Property(p_വ്യഞ്ജകം,EP_Leaf);
        }അല്ലെങ്കിൽ{
          sqlite3_വ്യഞ്ജകം_Delete(db, p_വ്യഞ്ജകം->p_ഇടത്);
          p_വ്യഞ്ജകം->p_ഇടത് = 0;
          sqlite3_വ്യഞ്ജകം_Delete(db, p_വ്യഞ്ജകം->p_വലത്);
          p_വ്യഞ്ജകം->p_വലത് = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, p_വ്യഞ്ജകം->iColumn);
        p_വ്യഞ്ജകം->op = TK_FUNCTION;
        p_വ്യഞ്ജകം->u.zToken = "coalesce";
        p_വ്യഞ്ജകം->x.pList = pFJMatch;
        p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം = SQLITE_AFF_DEFER;
        cnt = 1;
        പോകുക lookupname_end;
      }അല്ലെങ്കിൽ{
        sqlite3_വ്യഞ്ജകം_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    എങ്കിൽ( zDb ){
      sqlite3_Error_സന്ദേശം(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }അല്ലെങ്കിൽ എങ്കിൽ( zTab ){
      sqlite3_Error_സന്ദേശം(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }അല്ലെങ്കിൽ എങ്കിൽ( cnt==0 && വ്യഞ്ജകം_Has_Property(p_വലത്,EP_DblQuoted) ){
      sqlite3_Error_സന്ദേശം(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }അല്ലെങ്കിൽ{
      sqlite3_Error_സന്ദേശം(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3_Record_Error_Offset_Of_വ്യഞ്ജകം(pParse->db, p_വ്യഞ്ജകം);
    pParse->checkSchema = 1;
    p_മുകൾ_NC->nNcErr++;
    e_New_വ്യഞ്ജകം_Op = TK_NULL;
  }
  പ്രതിജ്ഞാ( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3_വ്യഞ്ജകം_Delete(db, p_വ്യഞ്ജകം->p_ഇടത്);
    p_വ്യഞ്ജകം->p_ഇടത് = 0;
    sqlite3_വ്യഞ്ജകം_Delete(db, p_വ്യഞ്ജകം->p_വലത്);
    p_വ്യഞ്ജകം->p_വലത് = 0;
    വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_Leaf);
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
  എങ്കിൽ( pMatch ){
    എങ്കിൽ( p_വ്യഞ്ജകം->iColumn>=0 ){
      pMatch->colUsed |= sqlite3_വ്യഞ്ജകം_Col_Used(p_വ്യഞ്ജകം);
    }അല്ലെങ്കിൽ{
      pMatch->fg.rowidUsed = 1;
    }
  }

  p_വ്യഞ്ജകം->op = e_New_വ്യഞ്ജകം_Op;
lookupname_end:
  എങ്കിൽ( cnt==1 ){
    പ്രതിജ്ഞാ( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    എങ്കിൽ( db->xAuth ){
      എങ്കിൽ( pFJMatch ){
        പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_FUNCTION );
        പ്രതിജ്ഞാ( sqlite3_stricmp(p_വ്യഞ്ജകം->u.zToken,"coalesce")==0 );
        പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pList==pFJMatch );
        പ്രതിജ്ഞാ( pFJMatch->n_വ്യഞ്ജകം>0 );
        വേണ്ടി(i=0; i<pFJMatch->n_വ്യഞ്ജകം; i++){
          പ്രതിജ്ഞാ( pFJMatch->a[i].p_വ്യഞ്ജകം->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_വ്യഞ്ജകം,pSchema,pNC->pSrcList);
        }
      }അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_COLUMN || p_വ്യഞ്ജകം->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_വ്യഞ്ജകം, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    വേണ്ടി(;;){
      പ്രതിജ്ഞാ( p_മുകൾ_NC!=0 );
      p_മുകൾ_NC->nRef++;
      എങ്കിൽ( p_മുകൾ_NC==pNC ) നിർത്തുക;
      p_മുകൾ_NC = p_മുകൾ_NC->pNext;
    }
    തിരികെ WRC_Prune;
  } അല്ലെങ്കിൽ {
    തിരികെ WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
വ്യഞ്ജകം *sqlite3_Create_Column_വ്യഞ്ജകം(sqlite3 *db, SrcList *pSrc, പൂർണ്ണസംഖ്യ iSrc, പൂർണ്ണസംഖ്യ iCol){
  വ്യഞ്ജകം *p = sqlite3_വ്യഞ്ജകം_Alloc(db, TK_COLUMN, 0, 0);
  എങ്കിൽ( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    എങ്കിൽ( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }അല്ലെങ്കിൽ{
      p->iColumn = (ynVar)iCol;
      എങ്കിൽ( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }അല്ലെങ്കിൽ{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  തിരികെ p;
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
സ്ഥിതം ശൂന്യം notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   സ്ഥിരം അക്ഷരം *z_സന്ദേശം,    /* Type of error */
   വ്യഞ്ജകം *p_വ്യഞ്ജകം,         /* Invalidate this expression on error */
   വ്യഞ്ജകം *pError         /* Associate error with this expression */
){
  സ്ഥിരം അക്ഷരം *zIn = "partial index WHERE clauses";
  എങ്കിൽ( pNC->ncFlags & NC_Idx_വ്യഞ്ജകം )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  അല്ലെങ്കിൽ എങ്കിൽ( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  അല്ലെങ്കിൽ എങ്കിൽ( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3_Error_സന്ദേശം(pParse, "%s prohibited in %s", z_സന്ദേശം, zIn);
  എങ്കിൽ( p_വ്യഞ്ജകം ) p_വ്യഞ്ജകം->op = TK_NULL;
  sqlite3_Record_Error_Offset_Of_വ്യഞ്ജകം(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  പ്രതിജ്ഞാ( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_വ്യഞ്ജകം|NC_GenCol))==0 ); \
  എങ്കിൽ( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Probability(വ്യഞ്ജകം *p){
  ഇരട്ട r = -1.0;
  എങ്കിൽ( p->op!=TK_FLOAT ) തിരികെ -1;
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p, EP_Int_മൂല്യം) );
  sqlite3AtoF(p->u.zToken, &r);
  പ്രതിജ്ഞാ( r>=0.0 );
  എങ്കിൽ( r>1.0 ) തിരികെ -1;
  തിരികെ (പൂർണ്ണസംഖ്യ)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
സ്ഥിതം SQLITE_NOINLINE ശൂന്യം resolve_Set_വ്യഞ്ജകം_Subtype_Arg(വ്യഞ്ജകം_List *pList){
  പൂർണ്ണസംഖ്യ nn, ii;
  nn = pList ? pList->n_വ്യഞ്ജകം : 0;
  വേണ്ടി(ii=0; ii<nn; ii++){
    വ്യഞ്ജകം *p_വ്യഞ്ജകം = pList->a[ii].p_വ്യഞ്ജകം;
    ആകുമ്പോൾ( 1 /*exit-by-break*/ ){
      വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_SubtArg);
      എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_SELECT ){
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) );
        പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pSelect!=0 );
        resolve_Set_വ്യഞ്ജകം_Subtype_Arg(p_വ്യഞ്ജകം->x.pSelect->pEList);
        നിർത്തുക;
      }
      എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_UPLUS ){
        p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_ഇടത്;
        പ്രതിജ്ഞാ( p_വ്യഞ്ജകം!=0 );
      }അല്ലെങ്കിൽ{
        നിർത്തുക;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ resolve_വ്യഞ്ജകം_Step(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  പ്രതിജ്ഞാ( pNC!=0 );
  pParse = pNC->pParse;
  പ്രതിജ്ഞാ( pParse==pWalker->pParse );

#ifndef NDEBUG
  എങ്കിൽ( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<pNC->pSrcList->nSrc; i++){
      പ്രതിജ്ഞാ( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  തിരഞ്ഞെടുപ്പ്( p_വ്യഞ്ജകം->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    അവസ്ഥ TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      പ്രതിജ്ഞാ( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      p_വ്യഞ്ജകം->op = TK_COLUMN;
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
      p_വ്യഞ്ജകം->y.pTab = pItem->pSTab;
      p_വ്യഞ്ജകം->iTable = pItem->iCursor;
      p_വ്യഞ്ജകം->iColumn--;
      p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം = SQLITE_AFF_INTEGER;
      നിർത്തുക;
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
    അവസ്ഥ TK_NOTNULL:
    അവസ്ഥ TK_ISNULL: {
      പൂർണ്ണസംഖ്യ anRef[8];
      NameContext *p;
      പൂർണ്ണസംഖ്യ i;
      വേണ്ടി(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_വ്യഞ്ജകം(pWalker, p_വ്യഞ്ജകം->p_ഇടത്);
      എങ്കിൽ( IN_RENAME_OBJECT ) തിരികെ WRC_Prune;
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Can_Be_Null(p_വ്യഞ്ജകം->p_ഇടത്) ){
        /* The expression can be NULL.  So the optimization does not apply */
        തിരികെ WRC_Prune;
      }

      വേണ്ടി(i=0, p=pNC; p; p=p->pNext, i++){
        എങ്കിൽ( (p->ncFlags & NC_Where)==0 ){
          തിരികെ WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON) );
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
#എങ്കിൽ TREETRACE_ENABLED
      എങ്കിൽ( sqlite3_വൃക്ഷം_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_വ്യഞ്ജകം->op==TK_NOTNULL
        );
        sqlite3_Show_വ്യഞ്ജകം(p_വ്യഞ്ജകം);
      }
#endif /* TREETRACE_ENABLED */
      p_വ്യഞ്ജകം->u.i_മൂല്യം = (p_വ്യഞ്ജകം->op==TK_NOTNULL);
      p_വ്യഞ്ജകം->flags |= EP_Int_മൂല്യം;
      p_വ്യഞ്ജകം->op = TK_INTEGER;
      വേണ്ടി(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        p->nRef = anRef[i];
      }
      sqlite3_വ്യഞ്ജകം_Delete(pParse->db, p_വ്യഞ്ജകം->p_ഇടത്);
      p_വ്യഞ്ജകം->p_ഇടത് = 0;
      തിരികെ WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    അവസ്ഥ TK_ID:
    അവസ്ഥ TK_DOT: {
      സ്ഥിരം അക്ഷരം *zTable;
      സ്ഥിരം അക്ഷരം *zDb;
      വ്യഞ്ജകം *p_വലത്;

      എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
        p_വലത് = p_വ്യഞ്ജകം;
      }അല്ലെങ്കിൽ{
        വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
        testcase( pNC->ncFlags & NC_Idx_വ്യഞ്ജകം );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_വ്യഞ്ജകം|NC_GenCol, 0, p_വ്യഞ്ജകം);
        p_വലത് = p_വ്യഞ്ജകം->p_വലത്;
        എങ്കിൽ( p_വലത്->op==TK_ID ){
          zDb = 0;
        }അല്ലെങ്കിൽ{
          പ്രതിജ്ഞാ( p_വലത്->op==TK_DOT );
          പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വലത്, EP_Int_മൂല്യം) );
          zDb = p_ഇടത്->u.zToken;
          p_ഇടത് = p_വലത്->p_ഇടത്;
          p_വലത് = p_വലത്->p_വലത്;
        }
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_UToken(p_ഇടത്) && വ്യഞ്ജകം_Use_UToken(p_വലത്) );
        zTable = p_ഇടത്->u.zToken;
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
        എങ്കിൽ( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (ശൂന്യം*)p_വ്യഞ്ജകം, (ശൂന്യം*)p_വലത്);
          sqlite3RenameTokenRemap(pParse, (ശൂന്യം*)&p_വ്യഞ്ജകം->y.pTab, (ശൂന്യം*)p_ഇടത്);
        }
      }
      തിരികെ lookupName(pParse, zDb, zTable, p_വലത്, pNC, p_വ്യഞ്ജകം);
    }

    /* Resolve function names
    */
    അവസ്ഥ TK_FUNCTION: {
      വ്യഞ്ജകം_List *pList;            /* The argument list */
      പൂർണ്ണസംഖ്യ n;                      /* Number of arguments */
      പൂർണ്ണസംഖ്യ no_such_func = 0;       /* True if no such function exists */
      പൂർണ്ണസംഖ്യ wrong_അക്കം_args = 0;     /* True if wrong number of arguments */
      പൂർണ്ണസംഖ്യ is_agg = 0;             /* True if is an aggregate function */
      സ്ഥിരം അക്ഷരം *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      പൂർണ്ണസംഖ്യ savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(p_വ്യഞ്ജകം) ? p_വ്യഞ്ജകം->y.pWin : 0);
#endif
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_xIsSelect|EP_Int_മൂല്യം) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത്==0 || p_വ്യഞ്ജകം->p_ഇടത്->op==TK_ORDER );
      pList = p_വ്യഞ്ജകം->x.pList;
      n = pList ? pList->n_വ്യഞ്ജകം : 0;
      zId = p_വ്യഞ്ജകം->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      എങ്കിൽ( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        എങ്കിൽ( pDef==0 ){
          no_such_func = 1;
        }അല്ലെങ്കിൽ{
          wrong_അക്കം_args = 1;
        }
      }അല്ലെങ്കിൽ{
        is_agg = pDef->xFinalize!=0;
        എങ്കിൽ( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_Unlikely);
          എങ്കിൽ( n==2 ){
            p_വ്യഞ്ജകം->iTable = വ്യഞ്ജകം_Probability(pList->a[1].p_വ്യഞ്ജകം);
            എങ്കിൽ( p_വ്യഞ്ജകം->iTable<0 ){
              sqlite3_Error_സന്ദേശം(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_വ്യഞ്ജകം);
              pNC->nNcErr++;
            }
          }അല്ലെങ്കിൽ{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_വ്യഞ്ജകം->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          പൂർണ്ണസംഖ്യ auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          എങ്കിൽ( auth!=SQLITE_OK ){
            എങ്കിൽ( auth==SQLITE_DENY ){
              sqlite3_Error_സന്ദേശം(pParse, "not authorized to use function: %#T",
                                      p_വ്യഞ്ജകം);
              pNC->nNcErr++;
            }
            p_വ്യഞ്ജകം->op = TK_NULL;
            തിരികെ WRC_Prune;
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
        എങ്കിൽ( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_SubtArg) 
        ){
          resolve_Set_വ്യഞ്ജകം_Subtype_Arg(pList);
        }

        എങ്കിൽ( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം,EP_ConstFunc);
        }
        എങ്കിൽ( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_വ്യഞ്ജകം|NC_PartIdx|NC_GenCol, 0, p_വ്യഞ്ജകം);
        }അല്ലെങ്കിൽ{
          പ്രതിജ്ഞാ( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_വ്യഞ്ജകം->op2 = pNC->ncFlags & NC_SelfRef;
        }
        എങ്കിൽ( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
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
        }അല്ലെങ്കിൽ
        എങ്കിൽ( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          എങ്കിൽ( pNC->ncFlags & NC_FromDDL ) വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_FromDDL);
          sqlite3_വ്യഞ്ജകം_Function_Usable(pParse, p_വ്യഞ്ജകം, pDef);
        }
      }

      എങ്കിൽ( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        പ്രതിജ്ഞാ( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_മൂല്യം==0 && pDef->xInverse==0)
          || (pDef->x_മൂല്യം && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        എങ്കിൽ( pDef && pDef->x_മൂല്യം==0 && pWin ){
          sqlite3_Error_സന്ദേശം(pParse,
              "%#T() may not be used as a window function", p_വ്യഞ്ജകം
          );
          pNC->nNcErr++;
        }അല്ലെങ്കിൽ എങ്കിൽ(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          സ്ഥിരം അക്ഷരം *zType;
          എങ്കിൽ( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }അല്ലെങ്കിൽ{
            zType = "aggregate";
          }
          sqlite3_Error_സന്ദേശം(pParse, "misuse of %s function %#T()",zType,p_വ്യഞ്ജകം);
          pNC->nNcErr++;
          is_agg = 0;
        }
#അല്ലെങ്കിൽ
        എങ്കിൽ( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_സന്ദേശം(pParse,"misuse of aggregate function %#T()",p_വ്യഞ്ജകം);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        അല്ലെങ്കിൽ എങ്കിൽ( no_such_func
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
          sqlite3_Error_സന്ദേശം(pParse, "no such function: %#T", p_വ്യഞ്ജകം);
          pNC->nNcErr++;
        }അല്ലെങ്കിൽ എങ്കിൽ( wrong_അക്കം_args ){
          sqlite3_Error_സന്ദേശം(pParse,"wrong number of arguments to function %#T()",
               p_വ്യഞ്ജകം);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        അല്ലെങ്കിൽ എങ്കിൽ( is_agg==0 && വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) ){
          sqlite3_Error_സന്ദേശം(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_വ്യഞ്ജകം
          );
          pNC->nNcErr++;
        }
#endif
        അല്ലെങ്കിൽ എങ്കിൽ( is_agg==0 && p_വ്യഞ്ജകം->p_ഇടത് ){
          sqlite3_വ്യഞ്ജകം_Order_By_Aggregate_Error(pParse, p_വ്യഞ്ജകം);
          pNC->nNcErr++;
        }
        എങ്കിൽ( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#അല്ലെങ്കിൽ
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      അല്ലെങ്കിൽ എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) || p_വ്യഞ്ജകം->p_ഇടത് ){
        is_agg = 1;
      }
      sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, pList);
      എങ്കിൽ( is_agg ){
        എങ്കിൽ( p_വ്യഞ്ജകം->p_ഇടത് ){
          പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത്->op==TK_ORDER );
          പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം->p_ഇടത്) );
          sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, p_വ്യഞ്ജകം->p_ഇടത്->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        എങ്കിൽ( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YWin(p_വ്യഞ്ജകം) && pWin==p_വ്യഞ്ജകം->y.pWin );
          എങ്കിൽ( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            എങ്കിൽ( pParse->db->mallocFailed ) നിർത്തുക;
          }
          sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, pWin->pPartition);
          sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, pWin->pOrderBy);
          sqlite3_Walk_വ്യഞ്ജകം(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }അല്ലെങ്കിൽ
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_വ്യഞ്ജകം->op = TK_AGG_FUNCTION;
          p_വ്യഞ്ജകം->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) ){
            sqlite3_Walk_വ്യഞ്ജകം(pWalker, p_വ്യഞ്ജകം->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          ആകുമ്പോൾ( pNC2
              && sqlite3ReferencesSrcList(pParse, p_വ്യഞ്ജകം, pNC2->pSrcList)==0
          ){
            p_വ്യഞ്ജകം->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }
          പ്രതിജ്ഞാ( pDef!=0 || IN_RENAME_OBJECT );
          എങ്കിൽ( pNC2 && pDef ){
            p_വ്യഞ്ജകം->op2 += pNC2->nNestedSelect;
            പ്രതിജ്ഞാ( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            പ്രതിജ്ഞാ( SQLITE_FUNC_ANYORDER==NC_OrderAgg );
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
      തിരികെ WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    അവസ്ഥ TK_EXISTS:
    അവസ്ഥ TK_SELECT:
#endif
    അവസ്ഥ TK_IN: {
      testcase( p_വ്യഞ്ജകം->op==TK_IN );
      testcase( p_വ്യഞ്ജകം->op==TK_EXISTS );
      testcase( p_വ്യഞ്ജകം->op==TK_SELECT );
      എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
        പൂർണ്ണസംഖ്യ nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_വ്യഞ്ജകം );
        testcase( pNC->ncFlags & NC_GenCol );
        പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pSelect );
        എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_EXISTS )  pParse->bHasExists = 1;
        എങ്കിൽ( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_വ്യഞ്ജകം, p_വ്യഞ്ജകം);
        }അല്ലെങ്കിൽ{
          sqlite3WalkSelect(pWalker, p_വ്യഞ്ജകം->x.pSelect);
        }
        പ്രതിജ്ഞാ( pNC->nRef>=nRef );
        എങ്കിൽ( nRef!=pNC->nRef ){
          വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_VarSelect);
          p_വ്യഞ്ജകം->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_വ്യഞ്ജകം );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_വ്യഞ്ജകം|NC_GenCol, p_വ്യഞ്ജകം, p_വ്യഞ്ജകം);
      നിർത്തുക;
    }
    അവസ്ഥ TK_IS:
    അവസ്ഥ TK_ISNOT: {
      വ്യഞ്ജകം *p_വലത് = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(p_വ്യഞ്ജകം->p_വലത്);
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      എങ്കിൽ( ALWAYS(p_വലത്) && (p_വലത്->op==TK_ID || p_വലത്->op==TK_TRUEFALSE) ){
        പൂർണ്ണസംഖ്യ rc = resolve_വ്യഞ്ജകം_Step(pWalker, p_വലത്);
        എങ്കിൽ( rc==WRC_Abort ) തിരികെ WRC_Abort;
        എങ്കിൽ( p_വലത്->op==TK_TRUEFALSE ){
          p_വ്യഞ്ജകം->op2 = p_വ്യഞ്ജകം->op;
          p_വ്യഞ്ജകം->op = TK_TRUTH;
          തിരികെ WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    അവസ്ഥ TK_BETWEEN:
    അവസ്ഥ TK_EQ:
    അവസ്ഥ TK_NE:
    അവസ്ഥ TK_LT:
    അവസ്ഥ TK_LE:
    അവസ്ഥ TK_GT:
    അവസ്ഥ TK_GE: {
      പൂർണ്ണസംഖ്യ n_ഇടത്, n_വലത്;
      എങ്കിൽ( pParse->db->mallocFailed ) നിർത്തുക;
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_ഇടത്!=0 );
      n_ഇടത് = sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം->p_ഇടത്);
      എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_BETWEEN ){
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
        n_വലത് = sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം->x.pList->a[0].p_വ്യഞ്ജകം);
        എങ്കിൽ( n_വലത്==n_ഇടത് ){
          n_വലത് = sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം->x.pList->a[1].p_വ്യഞ്ജകം);
        }
      }അല്ലെങ്കിൽ{
        പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_വലത്!=0 );
        n_വലത് = sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം->p_വലത്);
      }
      എങ്കിൽ( n_ഇടത്!=n_വലത് ){
        testcase( p_വ്യഞ്ജകം->op==TK_EQ );
        testcase( p_വ്യഞ്ജകം->op==TK_NE );
        testcase( p_വ്യഞ്ജകം->op==TK_LT );
        testcase( p_വ്യഞ്ജകം->op==TK_LE );
        testcase( p_വ്യഞ്ജകം->op==TK_GT );
        testcase( p_വ്യഞ്ജകം->op==TK_GE );
        testcase( p_വ്യഞ്ജകം->op==TK_IS );
        testcase( p_വ്യഞ്ജകം->op==TK_ISNOT );
        testcase( p_വ്യഞ്ജകം->op==TK_BETWEEN );
        sqlite3_Error_സന്ദേശം(pParse, "row value misused");
        sqlite3_Record_Error_Offset_Of_വ്യഞ്ജകം(pParse->db, p_വ്യഞ്ജകം);
      }
      നിർത്തുക;
    }
  }
  പ്രതിജ്ഞാ( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  തിരികെ pParse->nErr ? WRC_Abort : WRC_Continue;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  വ്യഞ്ജകം_List *pEList,  /* List of expressions to scan */
  വ്യഞ്ജകം *pE           /* Expression we are trying to match */
){
  പൂർണ്ണസംഖ്യ i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  എങ്കിൽ( pE->op==TK_ID ){
    സ്ഥിരം അക്ഷരം *zCol;
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(pE, EP_Int_മൂല്യം) );
    zCol = pE->u.zToken;
    വേണ്ടി(i=0; i<pEList->n_വ്യഞ്ജകം; i++){
      എങ്കിൽ( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        തിരികെ i+1;
      }
    }
  }
  തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ resolve_Order_By_Term_To_വ്യഞ്ജകം_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  വ്യഞ്ജകം *pE           /* The specific ORDER BY term */
){
  പൂർണ്ണസംഖ്യ i;             /* Loop counter */
  വ്യഞ്ജകം_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  പൂർണ്ണസംഖ്യ rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  പ്രതിജ്ഞാ( sqlite3_വ്യഞ്ജകം_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, വലുപ്പം(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_വ്യഞ്ജകം_Names(&nc, pE);
  db->suppressErr = savedSuppErr;
  എങ്കിൽ( rc ) തിരികെ 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  വേണ്ടി(i=0; i<pEList->n_വ്യഞ്ജകം; i++){
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(0, pEList->a[i].p_വ്യഞ്ജകം, pE, -1)<2 ){
      തിരികെ i+1;
    }
  }

  /* If no match, return 0. */
  തിരികെ 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
സ്ഥിതം ശൂന്യം resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  സ്ഥിരം അക്ഷരം *zType,     /* "ORDER" or "GROUP" */
  പൂർണ്ണസംഖ്യ i,                 /* The index (1-based) of the term out of range */
  പൂർണ്ണസംഖ്യ mx,                /* Largest permissible value of i */
  വ്യഞ്ജകം *pError           /* Associate the error with the expression */
){
  sqlite3_Error_സന്ദേശം(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_Record_Error_Offset_Of_വ്യഞ്ജകം(pParse->db, pError);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  പൂർണ്ണസംഖ്യ i;
  വ്യഞ്ജകം_List *pOrderBy;
  വ്യഞ്ജകം_List *pEList;
  sqlite3 *db;
  പൂർണ്ണസംഖ്യ moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  എങ്കിൽ( pOrderBy==0 ) തിരികെ 0;
  db = pParse->db;
  എങ്കിൽ( pOrderBy->n_വ്യഞ്ജകം>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_സന്ദേശം(pParse, "too many terms in ORDER BY clause");
    തിരികെ 1;
  }
  വേണ്ടി(i=0; i<pOrderBy->n_വ്യഞ്ജകം; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  ആകുമ്പോൾ( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  ആകുമ്പോൾ( pSelect && moreToDo ){
    ഘടന വ്യഞ്ജകം_List_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    പ്രതിജ്ഞാ( pEList!=0 );
    വേണ്ടി(i=0, pItem=pOrderBy->a; i<pOrderBy->n_വ്യഞ്ജകം; i++, pItem++){
      പൂർണ്ണസംഖ്യ iCol = -1;
      വ്യഞ്ജകം *pE, *pDup;
      എങ്കിൽ( pItem->fg.done ) തുടരുക;
      pE = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(pItem->p_വ്യഞ്ജകം);
      എങ്കിൽ( NEVER(pE==0) ) തുടരുക;
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Is_Integer(pE, &iCol, 0, 1) ){
        എങ്കിൽ( iCol<=0 || iCol>pEList->n_വ്യഞ്ജകം ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->n_വ്യഞ്ജകം, pE);
          തിരികെ 1;
        }
      }അല്ലെങ്കിൽ{
        iCol = resolveAsName(pParse, pEList, pE);
        എങ്കിൽ( iCol==0 ){
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
          pDup = sqlite3_വ്യഞ്ജകം_Dup(db, pE, 0);
          എങ്കിൽ( !db->mallocFailed ){
            പ്രതിജ്ഞാ(pDup);
            iCol = resolve_Order_By_Term_To_വ്യഞ്ജകം_List(pParse, pSelect, pDup);
            എങ്കിൽ( IN_RENAME_OBJECT && iCol>0 ){
              resolve_Order_By_Term_To_വ്യഞ്ജകം_List(pParse, pSelect, pE);
            }
          }
          sqlite3_വ്യഞ്ജകം_Delete(db, pDup);
        }
      }
      എങ്കിൽ( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        എങ്കിൽ( !IN_RENAME_OBJECT ){
          വ്യഞ്ജകം *pNew = sqlite3_വ്യഞ്ജകം_Int32(db, iCol);
          എങ്കിൽ( pNew==0 ) തിരികെ 1;
          എങ്കിൽ( pItem->p_വ്യഞ്ജകം==pE ){
            pItem->p_വ്യഞ്ജകം = pNew;
          }അല്ലെങ്കിൽ{
            വ്യഞ്ജകം *pParent = pItem->p_വ്യഞ്ജകം;
            പ്രതിജ്ഞാ( pParent->op==TK_COLLATE );
            ആകുമ്പോൾ( pParent->p_ഇടത്->op==TK_COLLATE ) pParent = pParent->p_ഇടത്;
            പ്രതിജ്ഞാ( pParent->p_ഇടത്==pE );
            pParent->p_ഇടത് = pNew;
          }
          sqlite3_വ്യഞ്ജകം_Delete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }അല്ലെങ്കിൽ{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  വേണ്ടി(i=0; i<pOrderBy->n_വ്യഞ്ജകം; i++){
    എങ്കിൽ( pOrderBy->a[i].fg.done==0 ){
      sqlite3_Error_സന്ദേശം(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      തിരികെ 1;
    }
  }
  തിരികെ 0;
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
പൂർണ്ണസംഖ്യ sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  വ്യഞ്ജകം_List *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  സ്ഥിരം അക്ഷരം *zType     /* "ORDER" or "GROUP" */
){
  പൂർണ്ണസംഖ്യ i;
  sqlite3 *db = pParse->db;
  വ്യഞ്ജകം_List *pEList;
  ഘടന വ്യഞ്ജകം_List_item *pItem;

  എങ്കിൽ( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) തിരികെ 0;
  എങ്കിൽ( pOrderBy->n_വ്യഞ്ജകം>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_സന്ദേശം(pParse, "too many terms in %s BY clause", zType);
    തിരികെ 1;
  }
  pEList = pSelect->pEList;
  പ്രതിജ്ഞാ( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  വേണ്ടി(i=0, pItem=pOrderBy->a; i<pOrderBy->n_വ്യഞ്ജകം; i++, pItem++){
    എങ്കിൽ( pItem->u.x.iOrderByCol ){
      എങ്കിൽ( pItem->u.x.iOrderByCol>pEList->n_വ്യഞ്ജകം ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->n_വ്യഞ്ജകം, 0);
        തിരികെ 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->p_വ്യഞ്ജകം,0);
    }
  }
  തിരികെ 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ resolveRemoveWindowsCb(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  UNUSED_PARAMETER(pWalker);
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) ){
    Window *pWin = p_വ്യഞ്ജകം->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  തിരികെ WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
സ്ഥിതം ശൂന്യം window_Remove_വ്യഞ്ജകം_From_Select(Select *pSelect, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  എങ്കിൽ( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, വലുപ്പം(Walker));
    sWalker.x_വ്യഞ്ജകം_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_വ്യഞ്ജകം(&sWalker, p_വ്യഞ്ജകം);
  }
}
#അല്ലെങ്കിൽ
# define window_Remove_വ്യഞ്ജകം_From_Select(a, b)
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
സ്ഥിതം പൂർണ്ണസംഖ്യ resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  വ്യഞ്ജകം_List *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  സ്ഥിരം അക്ഷരം *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  പൂർണ്ണസംഖ്യ i, j;                      /* Loop counters */
  പൂർണ്ണസംഖ്യ iCol;                      /* Column number */
  ഘടന വ്യഞ്ജകം_List_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  പൂർണ്ണസംഖ്യ nResult;                   /* Number of terms in the result set */

  പ്രതിജ്ഞാ( pOrderBy!=0 );
  nResult = pSelect->pEList->n_വ്യഞ്ജകം;
  pParse = pNC->pParse;
  വേണ്ടി(i=0, pItem=pOrderBy->a; i<pOrderBy->n_വ്യഞ്ജകം; i++, pItem++){
    വ്യഞ്ജകം *pE = pItem->p_വ്യഞ്ജകം;
    വ്യഞ്ജകം *pE2 = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(pE);
    എങ്കിൽ( NEVER(pE2==0) ) തുടരുക;
    എങ്കിൽ( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      എങ്കിൽ( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        തുടരുക;
      }
    }
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Is_Integer(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      എങ്കിൽ( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult, pE2);
        തിരികെ 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      തുടരുക;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    എങ്കിൽ( sqlite3_Resolve_വ്യഞ്ജകം_Names(pNC, pE) ){
      തിരികെ 1;
    }
    വേണ്ടി(j=0; j<pSelect->pEList->n_വ്യഞ്ജകം; j++){
      എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(0, pE, pSelect->pEList->a[j].p_വ്യഞ്ജകം, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        window_Remove_വ്യഞ്ജകം_From_Select(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  തിരികെ sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  പൂർണ്ണസംഖ്യ isCompound;         /* True if p is a compound select */
  പൂർണ്ണസംഖ്യ nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  പൂർണ്ണസംഖ്യ i;                  /* Loop counter */
  വ്യഞ്ജകം_List *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  പ്രതിജ്ഞാ( p!=0 );
  എങ്കിൽ( p->selFlags & SF_Resolved ){
    തിരികെ WRC_Prune;
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
  എങ്കിൽ( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    തിരികെ pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  ആകുമ്പോൾ( p ){
    പ്രതിജ്ഞാ( (p->selFlags & SF_Expanded)!=0 );
    പ്രതിജ്ഞാ( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, വലുപ്പം(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    എങ്കിൽ( sqlite3_Resolve_വ്യഞ്ജകം_Names(&sNC, p->pLimit) ){
      തിരികെ WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    എങ്കിൽ( p->selFlags & SF_Converted ){
      Select *pSub;
      പ്രതിജ്ഞാ( p->pSrc->a[0].fg.isSubquery );
      പ്രതിജ്ഞാ( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      പ്രതിജ്ഞാ( pSub!=0 );
      പ്രതിജ്ഞാ( p->pSrc->nSrc==1 && p->pOrderBy );
      പ്രതിജ്ഞാ( pSub->pPrior && pSub->pOrderBy==0 );
      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    എങ്കിൽ( pOuterNC ) pOuterNC->nNestedSelect++;
    വേണ്ടി(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      പ്രതിജ്ഞാ( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      എങ്കിൽ( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        പൂർണ്ണസംഖ്യ nRef = pOuterNC ? pOuterNC->nRef : 0;
        സ്ഥിരം അക്ഷരം *zSavedContext = pParse->zAuthContext;

        എങ്കിൽ( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        എങ്കിൽ( pParse->nErr ) തിരികെ WRC_Abort;
        പ്രതിജ്ഞാ( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        എങ്കിൽ( pOuterNC ){
          പ്രതിജ്ഞാ( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    എങ്കിൽ( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
 
    /* Resolve names in the result set. */
    എങ്കിൽ( sqlite3_Resolve_വ്യഞ്ജകം_List_Names(&sNC, p->pEList) ) തിരികെ WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    പ്രതിജ്ഞാ( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    എങ്കിൽ( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      പ്രതിജ്ഞാ( NC_MinMaxAgg==SF_MinMaxAgg );
      പ്രതിജ്ഞാ( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }അല്ലെങ്കിൽ{
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
    പ്രതിജ്ഞാ( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    എങ്കിൽ( p->pHaving ){
      എങ്കിൽ( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_സന്ദേശം(pParse, "HAVING clause on a non-aggregate query");
        തിരികെ WRC_Abort;
      }
      എങ്കിൽ( sqlite3_Resolve_വ്യഞ്ജകം_Names(&sNC, p->pHaving) ) തിരികെ WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    എങ്കിൽ( sqlite3_Resolve_വ്യഞ്ജകം_Names(&sNC, p->pWhere) ) തിരികെ WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    വേണ്ടി(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      എങ്കിൽ( pItem->fg.isTabFunc
       && sqlite3_Resolve_വ്യഞ്ജകം_List_Names(&sNC, pItem->u1.pFuncArg)
      ){
        തിരികെ WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    എങ്കിൽ( IN_RENAME_OBJECT ){
      Window *pWin;
      വേണ്ടി(pWin=p->pWinDefn; pWin; pWin=pWin->pNextWin){
        എങ്കിൽ( sqlite3_Resolve_വ്യഞ്ജകം_List_Names(&sNC, pWin->pOrderBy)
         || sqlite3_Resolve_വ്യഞ്ജകം_List_Names(&sNC, pWin->pPartition)
        ){
          തിരികെ WRC_Abort;
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
    എങ്കിൽ( p->selFlags & SF_Converted ){
      Select *pSub;
      പ്രതിജ്ഞാ( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      പ്രതിജ്ഞാ( pSub!=0 );
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
    എങ്കിൽ( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      തിരികെ WRC_Abort;
    }
    എങ്കിൽ( db->mallocFailed ){
      തിരികെ WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    എങ്കിൽ( pGroupBy ){
      ഘടന വ്യഞ്ജകം_List_item *pItem;
   
      എങ്കിൽ( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        തിരികെ WRC_Abort;
      }
      വേണ്ടി(i=0, pItem=pGroupBy->a; i<pGroupBy->n_വ്യഞ്ജകം; i++, pItem++){
        എങ്കിൽ( വ്യഞ്ജകം_Has_Property(pItem->p_വ്യഞ്ജകം, EP_Agg) ){
          sqlite3_Error_സന്ദേശം(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          തിരികെ WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    എങ്കിൽ( p->pNext && p->pEList->n_വ്യഞ്ജകം!=p->pNext->pEList->n_വ്യഞ്ജകം ){
      sqlite3_Select_Wrong_അക്കം_Terms_Error(pParse, p->pNext);
      തിരികെ WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    എങ്കിൽ( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      എങ്കിൽ( pParse->nErr ) തിരികെ WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  എങ്കിൽ( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    തിരികെ WRC_Abort;
  }

  തിരികെ WRC_Prune;
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
പൂർണ്ണസംഖ്യ sqlite3_Resolve_വ്യഞ്ജകം_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം             /* The expression to be analyzed. */
){
  പൂർണ്ണസംഖ്യ savedHasAgg;
  Walker w;

  എങ്കിൽ( p_വ്യഞ്ജകം==0 ) തിരികെ SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.x_വ്യഞ്ജകം_Callback = resolve_വ്യഞ്ജകം_Step;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
  w.pParse->nHeight += p_വ്യഞ്ജകം->nHeight;
  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Check_Height(w.pParse, w.pParse->nHeight) ){
    തിരികെ SQLITE_ERROR;
  }
#endif
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം!=0 );
  sqlite3_Walk_വ്യഞ്ജകം_NN(&w, p_വ്യഞ്ജകം);
#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
  w.pParse->nHeight -= p_വ്യഞ്ജകം->nHeight;
#endif
  പ്രതിജ്ഞാ( EP_Agg==NC_HasAgg );
  പ്രതിജ്ഞാ( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  തിരികെ pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
പൂർണ്ണസംഖ്യ sqlite3_Resolve_വ്യഞ്ജകം_List_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  വ്യഞ്ജകം_List *pList         /* The expression list to be analyzed. */
){
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ savedHasAgg = 0;
  Walker w;
  എങ്കിൽ( pList==0 ) തിരികെ SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_വ്യഞ്ജകം_Callback = resolve_വ്യഞ്ജകം_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  വേണ്ടി(i=0; i<pList->n_വ്യഞ്ജകം; i++){
    വ്യഞ്ജകം *p_വ്യഞ്ജകം = pList->a[i].p_വ്യഞ്ജകം;
    എങ്കിൽ( p_വ്യഞ്ജകം==0 ) തുടരുക;
#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
    w.pParse->nHeight += p_വ്യഞ്ജകം->nHeight;
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Check_Height(w.pParse, w.pParse->nHeight) ){
      തിരികെ SQLITE_ERROR;
    }
#endif
    sqlite3_Walk_വ്യഞ്ജകം_NN(&w, p_വ്യഞ്ജകം);
#എങ്കിൽ SQLITE_MAX_വ്യഞ്ജകം_DEPTH>0
    w.pParse->nHeight -= p_വ്യഞ്ജകം->nHeight;
#endif
    പ്രതിജ്ഞാ( EP_Agg==NC_HasAgg );
    പ്രതിജ്ഞാ( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    എങ്കിൽ( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    എങ്കിൽ( w.pParse->nErr>0 ) തിരികെ SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  തിരികെ SQLITE_OK;
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
ശൂന്യം sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  പ്രതിജ്ഞാ( p!=0 );
  w.x_വ്യഞ്ജകം_Callback = resolve_വ്യഞ്ജകം_Step;
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
പൂർണ്ണസംഖ്യ sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  പൂർണ്ണസംഖ്യ type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,     /* Expression to resolve.  May be NULL. */
  വ്യഞ്ജകം_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  പൂർണ്ണസംഖ്യ rc;
  സംഘം {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  പ്രതിജ്ഞാ( type==0 || pTab!=0 );
  പ്രതിജ്ഞാ( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_വ്യഞ്ജകം
          || type==NC_GenCol || pTab==0 );
  memset(&sNC, 0, വലുപ്പം(sNC));
  memset(&uSrc, 0, വലുപ്പം(uSrc));
  pSrc = &uSrc.sSrc;
  എങ്കിൽ( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    എങ്കിൽ( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  എങ്കിൽ( (rc = sqlite3_Resolve_വ്യഞ്ജകം_Names(&sNC, p_വ്യഞ്ജകം))!=SQLITE_OK ) തിരികെ rc;
  എങ്കിൽ( pList ) rc = sqlite3_Resolve_വ്യഞ്ജകം_List_Names(&sNC, pList);
  തിരികെ rc;
}
