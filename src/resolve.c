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
#அந்தர்பாவ "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#define EXCLUDED_TABLE_எண்  2

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
நிலையான முழு incrAggDepth(Walker *pWalker, கோவை *p_கோவை){
  எனில்( p_கோவை->op==TK_AGG_FUNCTION ) p_கோவை->op2 += pWalker->u.n;
  திரும்பு WRC_Continue;
}
நிலையான வெறுமை incrAggFunctionDepth(கோவை *p_கோவை, முழு N){
  எனில்( N>0 ){
    Walker w;
    memset(&w, 0, அளவு(w));
    w.x_கோவை_Callback = incrAggDepth;
    w.u.n = N;
    sqlite3_Walk_கோவை(&w, p_கோவை);
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
நிலையான வெறுமை resolveAlias(
  Parse *pParse,         /* Parsing context */
  கோவை_List *pEList,      /* A result set */
  முழு iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  கோவை *p_கோவை,           /* Transform this into an alias to the result set */
  முழு nSubquery          /* Number of subqueries that the label is moving */
){
  கோவை *pOrig;           /* The iCol-th column of the result set */
  கோவை *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  ப்ரதிஜ்ஞா( iCol>=0 && iCol<pEList->n_கோவை );
  pOrig = pEList->a[iCol].p_கோவை;
  ப்ரதிஜ்ஞா( pOrig!=0 );
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Reduced|EP_TokenOnly) );
  எனில்( p_கோவை->pAggInfo ) திரும்பு;
  db = pParse->db;
  pDup = sqlite3_கோவை_Dup(db, pOrig, 0);
  எனில்( db->mallocFailed ){
    sqlite3_கோவை_Delete(db, pDup);
    pDup = 0;
  }இல்லையெனில்{
    கோவை temp;
    incrAggFunctionDepth(pDup, nSubquery);
    எனில்( p_கோவை->op==TK_COLLATE ){
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      pDup = sqlite3_கோவை_Add_Collate_String(pParse, pDup, p_கோவை->u.zToken);
    }
    memcpy(&temp, pDup, அளவு(கோவை));
    memcpy(pDup, p_கோவை, அளவு(கோவை));
    memcpy(p_கோவை, &temp, அளவு(கோவை));
    எனில்( கோவை_Has_Property(p_கோவை, EP_WinFunc) ){
      எனில்( ALWAYS(p_கோவை->y.pWin!=0) ){
        p_கோவை->y.pWin->pOwner = p_கோவை;
      }
    }
    sqlite3_கோவை_Deferred_Delete(pParse, pDup);
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
முழு sqlite3MatchEName(
  மாறிலி கட்டமைப்பு கோவை_List_item *pItem,
  மாறிலி எழுத்து *zCol,
  மாறிலி எழுத்து *zTab,
  மாறிலி எழுத்து *zDb,
  முழு *pbRowid
){
  முழு n;
  மாறிலி எழுத்து *zSpan;
  முழு eEName = pItem->fg.eEName;
  எனில்( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    திரும்பு 0;
  }
  ப்ரதிஜ்ஞா( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  க்கு(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  எனில்( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    திரும்பு 0;
  }
  zSpan += n+1;
  க்கு(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  எனில்( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    திரும்பு 0;
  }
  zSpan += n+1;
  எனில்( zCol ){
    எனில்( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) திரும்பு 0;
    எனில்( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) திரும்பு 0;
  }
  எனில்( eEName==ENAME_ROWID ) *pbRowid = 1;
  திரும்பு 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
நிலையான முழு areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *p_உச்சி_NC){
  எனில்( db->init.busy ) திரும்பு 1;  /* Always support for legacy schemas */
  எனில்( p_உச்சி_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    எனில்( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      திரும்பு 1;
    }
    திரும்பு (db->flags & SQLITE_DqsDDL)!=0;
  }இல்லையெனில்{
    /* Currently parsing a DML statement */
    திரும்பு (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_கோவை_Col_Used(கோவை *p_கோவை){
  முழு n;
  Table *pExTab;

  n = p_கோவை->iColumn;
  ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
  pExTab = p_கோவை->y.pTab;
  ப்ரதிஜ்ஞா( pExTab!=0 );
  ப்ரதிஜ்ஞா( n < pExTab->nCol );
  எனில்( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    திரும்பு pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }இல்லையெனில்{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    எனில்( n>=BMS ) n = BMS-1;
    திரும்பு ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
நிலையான வெறுமை extendFJMatch(
  Parse *pParse,          /* Parsing context */
  கோவை_List **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  கோவை *pNew = sqlite3_கோவை_Alloc(pParse->db, TK_COLUMN, 0, 0);
  எனில்( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    ப்ரதிஜ்ஞா( (pMatch->fg.jointype & (JT_இடது|JT_LTORJ))!=0 );
    கோவை_Set_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_கோவை_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
நிலையான SQLITE_NOINLINE முழு isValidSchemaTableName(
  மாறிலி எழுத்து *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  மாறிலி எழுத்து *zDb           /* non-NULL if a database qualifier is present */
){
  மாறிலி எழுத்து *zLegacy;
  ப்ரதிஜ்ஞா( pTab!=0 );
  ப்ரதிஜ்ஞா( pTab->tnum==1 );
  எனில்( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) திரும்பு 0;
  zLegacy = pTab->zName;
  எனில்( strcmp(zLegacy+7, &LEGACY_TEMP_SCHEMA_TABLE[7])==0 ){
    எனில்( sqlite3StrICmp(zTab+7, &PREFERRED_TEMP_SCHEMA_TABLE[7])==0 ){
      திரும்பு 1;
    }
    எனில்( zDb==0 ) திரும்பு 0;
    எனில்( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) திரும்பு 1;
    எனில்( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) திரும்பு 1;
  }இல்லையெனில்{
    எனில்( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) திரும்பு 1;
  }
  திரும்பு 0;
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
நிலையான முழு lookupName(
  Parse *pParse,       /* The parsing context */
  மாறிலி எழுத்து *zDb,     /* Name of the database containing table, or NULL */
  மாறிலி எழுத்து *zTab,    /* Name of table containing column, or NULL */
  மாறிலி கோவை *p_வலது,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  கோவை *p_கோவை          /* Make this EXPR node point to the selected column */
){
  முழு i, j;                         /* Loop counters */
  முழு cnt = 0;                      /* Number of matching column names */
  முழு cntTab = 0;                   /* Number of potential "rowid" matches */
  முழு nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *p_உச்சி_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  முழு e_New_கோவை_Op = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  கோவை_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  மாறிலி எழுத்து *zCol = p_வலது->u.zToken;

  ப்ரதிஜ்ஞா( pNC );     /* the name context cannot be NULL. */
  ப்ரதிஜ்ஞா( zCol );    /* The Z in X.Y.Z cannot be NULL */
  ப்ரதிஜ்ஞா( zDb==0 || zTab!=0 );
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_கோவை->iTable = -1;
  கோவை_Set_VVAProperty(p_கோவை, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  எனில்( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    எனில்( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }இல்லையெனில்{
      க்கு(i=0; i<db->nDb; i++){
        ப்ரதிஜ்ஞா( db->aDb[i].zDbSName );
        எனில்( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          நிறுத்து;
        }
      }
      எனில்( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  ப்ரதிஜ்ஞா( pNC && cnt==0 );
  செய்{
    கோவை_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    எனில்( pSrcList ){
      க்கு(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        ப்ரதிஜ்ஞா( pTab!=0 && pTab->zName!=0 );
        ப்ரதிஜ்ஞா( pTab->nCol>0 || pParse->nErr );
        ப்ரதிஜ்ஞா( (முழு)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        எனில்( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          முழு hit = 0;
          Select *pSel;
          ப்ரதிஜ்ஞா( pItem->fg.isSubquery );
          ப்ரதிஜ்ஞா( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          ப்ரதிஜ்ஞா( pSel!=0 );
          pEList = pSel->pEList;
          ப்ரதிஜ்ஞா( pEList!=0 );
          ப்ரதிஜ்ஞா( pEList->n_கோவை==pTab->nCol );
          க்கு(j=0; j<pEList->n_கோவை; j++){
            முழு bRowid = 0;       /* True if possible rowid match */
            எனில்( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              தொடர்;
            }
            எனில்( bRowid==0 ){
              எனில்( cnt>0 ){
                எனில்( pItem->fg.isUsing==0
                 || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_கோவை_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }இல்லையெனில்
                எனில்( (pItem->fg.jointype & JT_வலது)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  தொடர்;
                }இல்லையெனில்
                எனில்( (pItem->fg.jointype & JT_இடது)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_கோவை_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }இல்லையெனில்{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, p_கோவை->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }இல்லையெனில் எனில்( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              தொடர்;
            }
            cntTab++;
            pMatch = pItem;
            p_கோவை->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            ப்ரதிஜ்ஞா( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            எனில்( pEList->a[j].fg.bUsingTerm ) நிறுத்து;
          }
          எனில்( hit || zTab==0 ) தொடர்;
        }
        ப்ரதிஜ்ஞா( zDb==0 || zTab!=0 );
        எனில்( zTab ){
          எனில்( zDb ){
            எனில்( pTab->pSchema!=pSchema ) தொடர்;
            எனில்( pSchema==0 && strcmp(zDb,"*")!=0 ) தொடர்;
          }
          எனில்( pItem->zAlias!=0 ){
            எனில்( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              தொடர்;
            }
          }இல்லையெனில் எனில்( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            எனில்( pTab->tnum!=1 ) தொடர்;
            எனில்( !isValidSchemaTableName(zTab, pTab, zDb) ) தொடர்;
          }
          ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
          எனில்( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (வெறுமை*)&p_கோவை->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        எனில்( j>=0 ){
          எனில்( cnt>0 ){
            எனில்( pItem->fg.isUsing==0
             || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_கோவை_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }இல்லையெனில்
            எனில்( (pItem->fg.jointype & JT_வலது)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              தொடர்;
            }இல்லையெனில்
            எனில்( (pItem->fg.jointype & JT_இடது)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_கோவை_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }இல்லையெனில்{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, p_கோவை->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_கோவை->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          எனில்( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        எனில்( 0==cnt && VisibleRowid(pTab) ){
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
          எனில்( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }இல்லையெனில்{
            cntTab++;
          }
#இல்லையெனில்
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      எனில்( pMatch ){
        p_கோவை->iTable = pMatch->iCursor;
        ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
        p_கோவை->y.pTab = pMatch->pSTab;
        எனில்( (pMatch->fg.jointype & (JT_இடது|JT_LTORJ))!=0 ){
          கோவை_Set_Property(p_கோவை, EP_CanBeNull);
        }
        pSchema = p_கோவை->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#எனில் !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    எனில்( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      எனில்( pParse->pTriggerTab!=0 ){
        முழு op = pParse->eTriggerOp;
        ப்ரதிஜ்ஞா( op==TK_DELETE || op==TK_UPDATE || op==TK_செருகு );
        எனில்( pParse->bReturning ){
          எனில்( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            p_கோவை->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }இல்லையெனில் எனில்( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_கோவை->iTable = 1;
          pTab = pParse->pTriggerTab;
        }இல்லையெனில் எனில்( op!=TK_செருகு && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_கோவை->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      எனில்( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        எனில்( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_கோவை->iTable = EXCLUDED_TABLE_எண்;
          எனில்( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (வெறுமை*)&p_கோவை->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      எனில்( pTab ){
        முழு iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        எனில்( iCol>=0 ){
          எனில்( pTab->iPKey==iCol ) iCol = -1;
        }இல்லையெனில்{
          எனில்( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }இல்லையெனில்{
            iCol = pTab->nCol;
          }
        }
        எனில்( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          எனில்( p_கோவை->iTable==EXCLUDED_TABLE_எண் ){
            testcase( iCol==(-1) );
            ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
            எனில்( IN_RENAME_OBJECT ){
              p_கோவை->iColumn = iCol;
              p_கோவை->y.pTab = pTab;
              e_New_கோவை_Op = TK_COLUMN;
            }இல்லையெனில்{
              p_கோவை->iTable = pNC->uNC.pUpsert->regData +
                 sqlite3TableColumnToStorage(pTab, iCol);
              e_New_கோவை_Op = TK_REGISTER;
            }
          }இல்லையெனில்
#endif /* SQLITE_OMIT_UPSERT */
          {
            ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
            p_கோவை->y.pTab = pTab;
            எனில்( pParse->bReturning ){
              e_New_கோவை_Op = TK_REGISTER;
              p_கோவை->op2 = TK_COLUMN;
              p_கோவை->iColumn = iCol;
              p_கோவை->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*p_கோவை->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }இல்லையெனில்{
              p_கோவை->iColumn = (i16)iCol;
              e_New_கோவை_Op = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              எனில்( iCol<0 ){
                p_கோவை->aff_கோவை = SQLITE_AFF_INTEGER;
              }இல்லையெனில் எனில்( p_கோவை->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }இல்லையெனில்{
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
    எனில்( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_Idx_கோவை|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#எனில் SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      எனில்( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        e_New_கோவை_Op = TK_NULL;
      }
#endif
      எனில்( pMatch->fg.isNestedFrom==0 ) p_கோவை->iColumn = -1;
      p_கோவை->aff_கோவை = SQLITE_AFF_INTEGER;
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
    எனில்( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      ப்ரதிஜ்ஞா( pEList!=0 );
      க்கு(j=0; j<pEList->n_கோவை; j++){
        எழுத்து *zAs = pEList->a[j].zEName;
        எனில்( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          கோவை *pOrig;
          ப்ரதிஜ்ஞா( p_கோவை->p_இடது==0 && p_கோவை->p_வலது==0 );
          ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை)==0 || p_கோவை->x.pList==0 );
          ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p_கோவை)==0 || p_கோவை->x.pSelect==0 );
          pOrig = pEList->a[j].p_கோவை;
          எனில்( (pNC->ncFlags&NC_AllowAgg)==0 && கோவை_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_செய்தி(pParse, "misuse of aliased aggregate %s", zAs);
            திரும்பு WRC_Abort;
          }
          எனில்( கோவை_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_உச்சி_NC )
          ){
            sqlite3_Error_செய்தி(pParse, "misuse of aliased window function %s",zAs);
            திரும்பு WRC_Abort;
          }
          எனில்( sqlite3_கோவை_Vector_Size(pOrig)!=1 ){
            sqlite3_Error_செய்தி(pParse, "row value misused");
            திரும்பு WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_கோவை, nSubquery);
          cnt = 1;
          pMatch = 0;
          ப்ரதிஜ்ஞா( zTab==0 && zDb==0 );
          எனில்( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (வெறுமை*)p_கோவை);
          }
          செல் lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    எனில்( cnt ) நிறுத்து;
    pNC = pNC->pNext;
    nSubquery++;
  }போது( pNC );


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
  எனில்( cnt==0 && zTab==0 ){
    ப்ரதிஜ்ஞா( p_கோவை->op==TK_ID );
    எனில்( கோவை_Has_Property(p_கோவை,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, p_உச்சி_NC)
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
      p_கோவை->op = TK_STRING;
      memset(&p_கோவை->y, 0, அளவு(p_கோவை->y));
      திரும்பு WRC_Prune;
    }
    எனில்( sqlite3_கோவை_Id_To_True_False(p_கோவை) ){
      திரும்பு WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  ப்ரதிஜ்ஞா( pFJMatch==0 || cnt>0 );
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_xIsSelect|EP_Int_மதிப்பு) );
  எனில்( cnt!=1 ){
    மாறிலி எழுத்து *zErr;
    எனில்( pFJMatch ){
      எனில்( pFJMatch->n_கோவை==cnt-1 ){
        எனில்( கோவை_Has_Property(p_கோவை,EP_Leaf) ){
          கோவை_Clear_Property(p_கோவை,EP_Leaf);
        }இல்லையெனில்{
          sqlite3_கோவை_Delete(db, p_கோவை->p_இடது);
          p_கோவை->p_இடது = 0;
          sqlite3_கோவை_Delete(db, p_கோவை->p_வலது);
          p_கோவை->p_வலது = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, p_கோவை->iColumn);
        p_கோவை->op = TK_FUNCTION;
        p_கோவை->u.zToken = "coalesce";
        p_கோவை->x.pList = pFJMatch;
        p_கோவை->aff_கோவை = SQLITE_AFF_DEFER;
        cnt = 1;
        செல் lookupname_end;
      }இல்லையெனில்{
        sqlite3_கோவை_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    எனில்( zDb ){
      sqlite3_Error_செய்தி(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }இல்லையெனில் எனில்( zTab ){
      sqlite3_Error_செய்தி(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }இல்லையெனில் எனில்( cnt==0 && கோவை_Has_Property(p_வலது,EP_DblQuoted) ){
      sqlite3_Error_செய்தி(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }இல்லையெனில்{
      sqlite3_Error_செய்தி(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3_Record_Error_Offset_Of_கோவை(pParse->db, p_கோவை);
    pParse->checkSchema = 1;
    p_உச்சி_NC->nNcErr++;
    e_New_கோவை_Op = TK_NULL;
  }
  ப்ரதிஜ்ஞா( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  எனில்( !கோவை_Has_Property(p_கோவை,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3_கோவை_Delete(db, p_கோவை->p_இடது);
    p_கோவை->p_இடது = 0;
    sqlite3_கோவை_Delete(db, p_கோவை->p_வலது);
    p_கோவை->p_வலது = 0;
    கோவை_Set_Property(p_கோவை, EP_Leaf);
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
  எனில்( pMatch ){
    எனில்( p_கோவை->iColumn>=0 ){
      pMatch->colUsed |= sqlite3_கோவை_Col_Used(p_கோவை);
    }இல்லையெனில்{
      pMatch->fg.rowidUsed = 1;
    }
  }

  p_கோவை->op = e_New_கோவை_Op;
lookupname_end:
  எனில்( cnt==1 ){
    ப்ரதிஜ்ஞா( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    எனில்( db->xAuth ){
      எனில்( pFJMatch ){
        ப்ரதிஜ்ஞா( p_கோவை->op==TK_FUNCTION );
        ப்ரதிஜ்ஞா( sqlite3_stricmp(p_கோவை->u.zToken,"coalesce")==0 );
        ப்ரதிஜ்ஞா( p_கோவை->x.pList==pFJMatch );
        ப்ரதிஜ்ஞா( pFJMatch->n_கோவை>0 );
        க்கு(i=0; i<pFJMatch->n_கோவை; i++){
          ப்ரதிஜ்ஞா( pFJMatch->a[i].p_கோவை->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_கோவை,pSchema,pNC->pSrcList);
        }
      }இல்லையெனில் எனில்( p_கோவை->op==TK_COLUMN || p_கோவை->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_கோவை, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    க்கு(;;){
      ப்ரதிஜ்ஞா( p_உச்சி_NC!=0 );
      p_உச்சி_NC->nRef++;
      எனில்( p_உச்சி_NC==pNC ) நிறுத்து;
      p_உச்சி_NC = p_உச்சி_NC->pNext;
    }
    திரும்பு WRC_Prune;
  } இல்லையெனில் {
    திரும்பு WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
கோவை *sqlite3_Create_Column_கோவை(sqlite3 *db, SrcList *pSrc, முழு iSrc, முழு iCol){
  கோவை *p = sqlite3_கோவை_Alloc(db, TK_COLUMN, 0, 0);
  எனில்( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    ப்ரதிஜ்ஞா( கோவை_Use_YTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    எனில்( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }இல்லையெனில்{
      p->iColumn = (ynVar)iCol;
      எனில்( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }இல்லையெனில்{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  திரும்பு p;
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
நிலையான வெறுமை notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   மாறிலி எழுத்து *z_செய்தி,    /* Type of error */
   கோவை *p_கோவை,         /* Invalidate this expression on error */
   கோவை *pError         /* Associate error with this expression */
){
  மாறிலி எழுத்து *zIn = "partial index WHERE clauses";
  எனில்( pNC->ncFlags & NC_Idx_கோவை )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  இல்லையெனில் எனில்( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  இல்லையெனில் எனில்( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3_Error_செய்தி(pParse, "%s prohibited in %s", z_செய்தி, zIn);
  எனில்( p_கோவை ) p_கோவை->op = TK_NULL;
  sqlite3_Record_Error_Offset_Of_கோவை(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  ப்ரதிஜ்ஞா( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_கோவை|NC_GenCol))==0 ); \
  எனில்( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
நிலையான முழு கோவை_Probability(கோவை *p){
  இரட்டை r = -1.0;
  எனில்( p->op!=TK_FLOAT ) திரும்பு -1;
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p, EP_Int_மதிப்பு) );
  sqlite3AtoF(p->u.zToken, &r);
  ப்ரதிஜ்ஞா( r>=0.0 );
  எனில்( r>1.0 ) திரும்பு -1;
  திரும்பு (முழு)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
நிலையான SQLITE_NOINLINE வெறுமை resolve_Set_கோவை_Subtype_Arg(கோவை_List *pList){
  முழு nn, ii;
  nn = pList ? pList->n_கோவை : 0;
  க்கு(ii=0; ii<nn; ii++){
    கோவை *p_கோவை = pList->a[ii].p_கோவை;
    போது( 1 /*exit-by-break*/ ){
      கோவை_Set_Property(p_கோவை, EP_SubtArg);
      எனில்( p_கோவை->op==TK_SELECT ){
        ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p_கோவை) );
        ப்ரதிஜ்ஞா( p_கோவை->x.pSelect!=0 );
        resolve_Set_கோவை_Subtype_Arg(p_கோவை->x.pSelect->pEList);
        நிறுத்து;
      }
      எனில்( p_கோவை->op==TK_UPLUS ){
        p_கோவை = p_கோவை->p_இடது;
        ப்ரதிஜ்ஞா( p_கோவை!=0 );
      }இல்லையெனில்{
        நிறுத்து;
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
நிலையான முழு resolve_கோவை_Step(Walker *pWalker, கோவை *p_கோவை){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  ப்ரதிஜ்ஞா( pNC!=0 );
  pParse = pNC->pParse;
  ப்ரதிஜ்ஞா( pParse==pWalker->pParse );

#ifndef NDEBUG
  எனில்( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    முழு i;
    க்கு(i=0; i<pNC->pSrcList->nSrc; i++){
      ப்ரதிஜ்ஞா( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  தேர்வு( p_கோவை->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    நிலை TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      ப்ரதிஜ்ஞா( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      p_கோவை->op = TK_COLUMN;
      ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
      p_கோவை->y.pTab = pItem->pSTab;
      p_கோவை->iTable = pItem->iCursor;
      p_கோவை->iColumn--;
      p_கோவை->aff_கோவை = SQLITE_AFF_INTEGER;
      நிறுத்து;
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
    நிலை TK_NOTNULL:
    நிலை TK_ISNULL: {
      முழு anRef[8];
      NameContext *p;
      முழு i;
      க்கு(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_கோவை(pWalker, p_கோவை->p_இடது);
      எனில்( IN_RENAME_OBJECT ) திரும்பு WRC_Prune;
      எனில்( sqlite3_கோவை_Can_Be_Null(p_கோவை->p_இடது) ){
        /* The expression can be NULL.  So the optimization does not apply */
        திரும்பு WRC_Prune;
      }

      க்கு(i=0, p=pNC; p; p=p->pNext, i++){
        எனில்( (p->ncFlags & NC_Where)==0 ){
          திரும்பு WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( கோவை_Has_Property(p_கோவை, EP_OuterON) );
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
#எனில் TREETRACE_ENABLED
      எனில்( sqlite3_மரம்_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_கோவை->op==TK_NOTNULL
        );
        sqlite3_Show_கோவை(p_கோவை);
      }
#endif /* TREETRACE_ENABLED */
      p_கோவை->u.i_மதிப்பு = (p_கோவை->op==TK_NOTNULL);
      p_கோவை->flags |= EP_Int_மதிப்பு;
      p_கோவை->op = TK_INTEGER;
      க்கு(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        p->nRef = anRef[i];
      }
      sqlite3_கோவை_Delete(pParse->db, p_கோவை->p_இடது);
      p_கோவை->p_இடது = 0;
      திரும்பு WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    நிலை TK_ID:
    நிலை TK_DOT: {
      மாறிலி எழுத்து *zTable;
      மாறிலி எழுத்து *zDb;
      கோவை *p_வலது;

      எனில்( p_கோவை->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
        p_வலது = p_கோவை;
      }இல்லையெனில்{
        கோவை *p_இடது = p_கோவை->p_இடது;
        testcase( pNC->ncFlags & NC_Idx_கோவை );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_கோவை|NC_GenCol, 0, p_கோவை);
        p_வலது = p_கோவை->p_வலது;
        எனில்( p_வலது->op==TK_ID ){
          zDb = 0;
        }இல்லையெனில்{
          ப்ரதிஜ்ஞா( p_வலது->op==TK_DOT );
          ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_வலது, EP_Int_மதிப்பு) );
          zDb = p_இடது->u.zToken;
          p_இடது = p_வலது->p_இடது;
          p_வலது = p_வலது->p_வலது;
        }
        ப்ரதிஜ்ஞா( கோவை_Use_UToken(p_இடது) && கோவை_Use_UToken(p_வலது) );
        zTable = p_இடது->u.zToken;
        ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
        எனில்( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (வெறுமை*)p_கோவை, (வெறுமை*)p_வலது);
          sqlite3RenameTokenRemap(pParse, (வெறுமை*)&p_கோவை->y.pTab, (வெறுமை*)p_இடது);
        }
      }
      திரும்பு lookupName(pParse, zDb, zTable, p_வலது, pNC, p_கோவை);
    }

    /* Resolve function names
    */
    நிலை TK_FUNCTION: {
      கோவை_List *pList;            /* The argument list */
      முழு n;                      /* Number of arguments */
      முழு no_such_func = 0;       /* True if no such function exists */
      முழு wrong_இலக்கம்_args = 0;     /* True if wrong number of arguments */
      முழு is_agg = 0;             /* True if is an aggregate function */
      மாறிலி எழுத்து *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      முழு savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(p_கோவை) ? p_கோவை->y.pWin : 0);
#endif
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_xIsSelect|EP_Int_மதிப்பு) );
      ப்ரதிஜ்ஞா( p_கோவை->p_இடது==0 || p_கோவை->p_இடது->op==TK_ORDER );
      pList = p_கோவை->x.pList;
      n = pList ? pList->n_கோவை : 0;
      zId = p_கோவை->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      எனில்( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        எனில்( pDef==0 ){
          no_such_func = 1;
        }இல்லையெனில்{
          wrong_இலக்கம்_args = 1;
        }
      }இல்லையெனில்{
        is_agg = pDef->xFinalize!=0;
        எனில்( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          கோவை_Set_Property(p_கோவை, EP_Unlikely);
          எனில்( n==2 ){
            p_கோவை->iTable = கோவை_Probability(pList->a[1].p_கோவை);
            எனில்( p_கோவை->iTable<0 ){
              sqlite3_Error_செய்தி(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_கோவை);
              pNC->nNcErr++;
            }
          }இல்லையெனில்{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_கோவை->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          முழு auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          எனில்( auth!=SQLITE_OK ){
            எனில்( auth==SQLITE_DENY ){
              sqlite3_Error_செய்தி(pParse, "not authorized to use function: %#T",
                                      p_கோவை);
              pNC->nNcErr++;
            }
            p_கோவை->op = TK_NULL;
            திரும்பு WRC_Prune;
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
        எனில்( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || கோவை_Has_Property(p_கோவை, EP_SubtArg) 
        ){
          resolve_Set_கோவை_Subtype_Arg(pList);
        }

        எனில்( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          கோவை_Set_Property(p_கோவை,EP_ConstFunc);
        }
        எனில்( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_கோவை|NC_PartIdx|NC_GenCol, 0, p_கோவை);
        }இல்லையெனில்{
          ப்ரதிஜ்ஞா( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_கோவை->op2 = pNC->ncFlags & NC_SelfRef;
        }
        எனில்( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
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
        }இல்லையெனில்
        எனில்( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          எனில்( pNC->ncFlags & NC_FromDDL ) கோவை_Set_Property(p_கோவை, EP_FromDDL);
          sqlite3_கோவை_Function_Usable(pParse, p_கோவை, pDef);
        }
      }

      எனில்( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        ப்ரதிஜ்ஞா( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_மதிப்பு==0 && pDef->xInverse==0)
          || (pDef->x_மதிப்பு && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        எனில்( pDef && pDef->x_மதிப்பு==0 && pWin ){
          sqlite3_Error_செய்தி(pParse,
              "%#T() may not be used as a window function", p_கோவை
          );
          pNC->nNcErr++;
        }இல்லையெனில் எனில்(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          மாறிலி எழுத்து *zType;
          எனில்( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }இல்லையெனில்{
            zType = "aggregate";
          }
          sqlite3_Error_செய்தி(pParse, "misuse of %s function %#T()",zType,p_கோவை);
          pNC->nNcErr++;
          is_agg = 0;
        }
#இல்லையெனில்
        எனில்( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_செய்தி(pParse,"misuse of aggregate function %#T()",p_கோவை);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        இல்லையெனில் எனில்( no_such_func
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
          sqlite3_Error_செய்தி(pParse, "no such function: %#T", p_கோவை);
          pNC->nNcErr++;
        }இல்லையெனில் எனில்( wrong_இலக்கம்_args ){
          sqlite3_Error_செய்தி(pParse,"wrong number of arguments to function %#T()",
               p_கோவை);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        இல்லையெனில் எனில்( is_agg==0 && கோவை_Has_Property(p_கோவை, EP_WinFunc) ){
          sqlite3_Error_செய்தி(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_கோவை
          );
          pNC->nNcErr++;
        }
#endif
        இல்லையெனில் எனில்( is_agg==0 && p_கோவை->p_இடது ){
          sqlite3_கோவை_Order_By_Aggregate_Error(pParse, p_கோவை);
          pNC->nNcErr++;
        }
        எனில்( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#இல்லையெனில்
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      இல்லையெனில் எனில்( கோவை_Has_Property(p_கோவை, EP_WinFunc) || p_கோவை->p_இடது ){
        is_agg = 1;
      }
      sqlite3_Walk_கோவை_List(pWalker, pList);
      எனில்( is_agg ){
        எனில்( p_கோவை->p_இடது ){
          ப்ரதிஜ்ஞா( p_கோவை->p_இடது->op==TK_ORDER );
          ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை->p_இடது) );
          sqlite3_Walk_கோவை_List(pWalker, p_கோவை->p_இடது->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        எனில்( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          ப்ரதிஜ்ஞா( கோவை_Use_YWin(p_கோவை) && pWin==p_கோவை->y.pWin );
          எனில்( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            எனில்( pParse->db->mallocFailed ) நிறுத்து;
          }
          sqlite3_Walk_கோவை_List(pWalker, pWin->pPartition);
          sqlite3_Walk_கோவை_List(pWalker, pWin->pOrderBy);
          sqlite3_Walk_கோவை(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }இல்லையெனில்
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_கோவை->op = TK_AGG_FUNCTION;
          p_கோவை->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          எனில்( கோவை_Has_Property(p_கோவை, EP_WinFunc) ){
            sqlite3_Walk_கோவை(pWalker, p_கோவை->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          போது( pNC2
              && sqlite3ReferencesSrcList(pParse, p_கோவை, pNC2->pSrcList)==0
          ){
            p_கோவை->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }
          ப்ரதிஜ்ஞா( pDef!=0 || IN_RENAME_OBJECT );
          எனில்( pNC2 && pDef ){
            p_கோவை->op2 += pNC2->nNestedSelect;
            ப்ரதிஜ்ஞா( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            ப்ரதிஜ்ஞா( SQLITE_FUNC_ANYORDER==NC_OrderAgg );
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
      திரும்பு WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    நிலை TK_EXISTS:
    நிலை TK_SELECT:
#endif
    நிலை TK_IN: {
      testcase( p_கோவை->op==TK_IN );
      testcase( p_கோவை->op==TK_EXISTS );
      testcase( p_கோவை->op==TK_SELECT );
      எனில்( கோவை_Use_XSelect(p_கோவை) ){
        முழு nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_கோவை );
        testcase( pNC->ncFlags & NC_GenCol );
        ப்ரதிஜ்ஞா( p_கோவை->x.pSelect );
        எனில்( p_கோவை->op==TK_EXISTS )  pParse->bHasExists = 1;
        எனில்( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_கோவை, p_கோவை);
        }இல்லையெனில்{
          sqlite3WalkSelect(pWalker, p_கோவை->x.pSelect);
        }
        ப்ரதிஜ்ஞா( pNC->nRef>=nRef );
        எனில்( nRef!=pNC->nRef ){
          கோவை_Set_Property(p_கோவை, EP_VarSelect);
          p_கோவை->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      நிறுத்து;
    }
    நிலை TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_கோவை );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_கோவை|NC_GenCol, p_கோவை, p_கோவை);
      நிறுத்து;
    }
    நிலை TK_IS:
    நிலை TK_ISNOT: {
      கோவை *p_வலது = sqlite3_கோவை_Skip_Collate_And_Likely(p_கோவை->p_வலது);
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      எனில்( ALWAYS(p_வலது) && (p_வலது->op==TK_ID || p_வலது->op==TK_TRUEFALSE) ){
        முழு rc = resolve_கோவை_Step(pWalker, p_வலது);
        எனில்( rc==WRC_Abort ) திரும்பு WRC_Abort;
        எனில்( p_வலது->op==TK_TRUEFALSE ){
          p_கோவை->op2 = p_கோவை->op;
          p_கோவை->op = TK_TRUTH;
          திரும்பு WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    நிலை TK_BETWEEN:
    நிலை TK_EQ:
    நிலை TK_NE:
    நிலை TK_LT:
    நிலை TK_LE:
    நிலை TK_GT:
    நிலை TK_GE: {
      முழு n_இடது, n_வலது;
      எனில்( pParse->db->mallocFailed ) நிறுத்து;
      ப்ரதிஜ்ஞா( p_கோவை->p_இடது!=0 );
      n_இடது = sqlite3_கோவை_Vector_Size(p_கோவை->p_இடது);
      எனில்( p_கோவை->op==TK_BETWEEN ){
        ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
        n_வலது = sqlite3_கோவை_Vector_Size(p_கோவை->x.pList->a[0].p_கோவை);
        எனில்( n_வலது==n_இடது ){
          n_வலது = sqlite3_கோவை_Vector_Size(p_கோவை->x.pList->a[1].p_கோவை);
        }
      }இல்லையெனில்{
        ப்ரதிஜ்ஞா( p_கோவை->p_வலது!=0 );
        n_வலது = sqlite3_கோவை_Vector_Size(p_கோவை->p_வலது);
      }
      எனில்( n_இடது!=n_வலது ){
        testcase( p_கோவை->op==TK_EQ );
        testcase( p_கோவை->op==TK_NE );
        testcase( p_கோவை->op==TK_LT );
        testcase( p_கோவை->op==TK_LE );
        testcase( p_கோவை->op==TK_GT );
        testcase( p_கோவை->op==TK_GE );
        testcase( p_கோவை->op==TK_IS );
        testcase( p_கோவை->op==TK_ISNOT );
        testcase( p_கோவை->op==TK_BETWEEN );
        sqlite3_Error_செய்தி(pParse, "row value misused");
        sqlite3_Record_Error_Offset_Of_கோவை(pParse->db, p_கோவை);
      }
      நிறுத்து;
    }
  }
  ப்ரதிஜ்ஞா( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  திரும்பு pParse->nErr ? WRC_Abort : WRC_Continue;
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
நிலையான முழு resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  கோவை_List *pEList,  /* List of expressions to scan */
  கோவை *pE           /* Expression we are trying to match */
){
  முழு i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  எனில்( pE->op==TK_ID ){
    மாறிலி எழுத்து *zCol;
    ப்ரதிஜ்ஞா( !கோவை_Has_Property(pE, EP_Int_மதிப்பு) );
    zCol = pE->u.zToken;
    க்கு(i=0; i<pEList->n_கோவை; i++){
      எனில்( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        திரும்பு i+1;
      }
    }
  }
  திரும்பு 0;
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
நிலையான முழு resolve_Order_By_Term_To_கோவை_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  கோவை *pE           /* The specific ORDER BY term */
){
  முழு i;             /* Loop counter */
  கோவை_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  முழு rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  ப்ரதிஜ்ஞா( sqlite3_கோவை_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, அளவு(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_கோவை_Names(&nc, pE);
  db->suppressErr = savedSuppErr;
  எனில்( rc ) திரும்பு 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  க்கு(i=0; i<pEList->n_கோவை; i++){
    எனில்( sqlite3_கோவை_Compare(0, pEList->a[i].p_கோவை, pE, -1)<2 ){
      திரும்பு i+1;
    }
  }

  /* If no match, return 0. */
  திரும்பு 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
நிலையான வெறுமை resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  மாறிலி எழுத்து *zType,     /* "ORDER" or "GROUP" */
  முழு i,                 /* The index (1-based) of the term out of range */
  முழு mx,                /* Largest permissible value of i */
  கோவை *pError           /* Associate the error with the expression */
){
  sqlite3_Error_செய்தி(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_Record_Error_Offset_Of_கோவை(pParse->db, pError);
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
நிலையான முழு resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  முழு i;
  கோவை_List *pOrderBy;
  கோவை_List *pEList;
  sqlite3 *db;
  முழு moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  எனில்( pOrderBy==0 ) திரும்பு 0;
  db = pParse->db;
  எனில்( pOrderBy->n_கோவை>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_செய்தி(pParse, "too many terms in ORDER BY clause");
    திரும்பு 1;
  }
  க்கு(i=0; i<pOrderBy->n_கோவை; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  போது( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  போது( pSelect && moreToDo ){
    கட்டமைப்பு கோவை_List_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    ப்ரதிஜ்ஞா( pEList!=0 );
    க்கு(i=0, pItem=pOrderBy->a; i<pOrderBy->n_கோவை; i++, pItem++){
      முழு iCol = -1;
      கோவை *pE, *pDup;
      எனில்( pItem->fg.done ) தொடர்;
      pE = sqlite3_கோவை_Skip_Collate_And_Likely(pItem->p_கோவை);
      எனில்( NEVER(pE==0) ) தொடர்;
      எனில்( sqlite3_கோவை_Is_Integer(pE, &iCol, 0, 1) ){
        எனில்( iCol<=0 || iCol>pEList->n_கோவை ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->n_கோவை, pE);
          திரும்பு 1;
        }
      }இல்லையெனில்{
        iCol = resolveAsName(pParse, pEList, pE);
        எனில்( iCol==0 ){
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
          pDup = sqlite3_கோவை_Dup(db, pE, 0);
          எனில்( !db->mallocFailed ){
            ப்ரதிஜ்ஞா(pDup);
            iCol = resolve_Order_By_Term_To_கோவை_List(pParse, pSelect, pDup);
            எனில்( IN_RENAME_OBJECT && iCol>0 ){
              resolve_Order_By_Term_To_கோவை_List(pParse, pSelect, pE);
            }
          }
          sqlite3_கோவை_Delete(db, pDup);
        }
      }
      எனில்( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        எனில்( !IN_RENAME_OBJECT ){
          கோவை *pNew = sqlite3_கோவை_Int32(db, iCol);
          எனில்( pNew==0 ) திரும்பு 1;
          எனில்( pItem->p_கோவை==pE ){
            pItem->p_கோவை = pNew;
          }இல்லையெனில்{
            கோவை *pParent = pItem->p_கோவை;
            ப்ரதிஜ்ஞா( pParent->op==TK_COLLATE );
            போது( pParent->p_இடது->op==TK_COLLATE ) pParent = pParent->p_இடது;
            ப்ரதிஜ்ஞா( pParent->p_இடது==pE );
            pParent->p_இடது = pNew;
          }
          sqlite3_கோவை_Delete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }இல்லையெனில்{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  க்கு(i=0; i<pOrderBy->n_கோவை; i++){
    எனில்( pOrderBy->a[i].fg.done==0 ){
      sqlite3_Error_செய்தி(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      திரும்பு 1;
    }
  }
  திரும்பு 0;
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
முழு sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  கோவை_List *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  மாறிலி எழுத்து *zType     /* "ORDER" or "GROUP" */
){
  முழு i;
  sqlite3 *db = pParse->db;
  கோவை_List *pEList;
  கட்டமைப்பு கோவை_List_item *pItem;

  எனில்( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) திரும்பு 0;
  எனில்( pOrderBy->n_கோவை>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_செய்தி(pParse, "too many terms in %s BY clause", zType);
    திரும்பு 1;
  }
  pEList = pSelect->pEList;
  ப்ரதிஜ்ஞா( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  க்கு(i=0, pItem=pOrderBy->a; i<pOrderBy->n_கோவை; i++, pItem++){
    எனில்( pItem->u.x.iOrderByCol ){
      எனில்( pItem->u.x.iOrderByCol>pEList->n_கோவை ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->n_கோவை, 0);
        திரும்பு 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->p_கோவை,0);
    }
  }
  திரும்பு 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
நிலையான முழு resolveRemoveWindowsCb(Walker *pWalker, கோவை *p_கோவை){
  UNUSED_PARAMETER(pWalker);
  எனில்( கோவை_Has_Property(p_கோவை, EP_WinFunc) ){
    Window *pWin = p_கோவை->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  திரும்பு WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
நிலையான வெறுமை window_Remove_கோவை_From_Select(Select *pSelect, கோவை *p_கோவை){
  எனில்( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, அளவு(Walker));
    sWalker.x_கோவை_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_கோவை(&sWalker, p_கோவை);
  }
}
#இல்லையெனில்
# define window_Remove_கோவை_From_Select(a, b)
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
நிலையான முழு resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  கோவை_List *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  மாறிலி எழுத்து *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  முழு i, j;                      /* Loop counters */
  முழு iCol;                      /* Column number */
  கட்டமைப்பு கோவை_List_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  முழு nResult;                   /* Number of terms in the result set */

  ப்ரதிஜ்ஞா( pOrderBy!=0 );
  nResult = pSelect->pEList->n_கோவை;
  pParse = pNC->pParse;
  க்கு(i=0, pItem=pOrderBy->a; i<pOrderBy->n_கோவை; i++, pItem++){
    கோவை *pE = pItem->p_கோவை;
    கோவை *pE2 = sqlite3_கோவை_Skip_Collate_And_Likely(pE);
    எனில்( NEVER(pE2==0) ) தொடர்;
    எனில்( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      எனில்( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        தொடர்;
      }
    }
    எனில்( sqlite3_கோவை_Is_Integer(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      எனில்( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult, pE2);
        திரும்பு 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      தொடர்;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    எனில்( sqlite3_Resolve_கோவை_Names(pNC, pE) ){
      திரும்பு 1;
    }
    க்கு(j=0; j<pSelect->pEList->n_கோவை; j++){
      எனில்( sqlite3_கோவை_Compare(0, pE, pSelect->pEList->a[j].p_கோவை, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        window_Remove_கோவை_From_Select(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  திரும்பு sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
நிலையான முழு resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  முழு isCompound;         /* True if p is a compound select */
  முழு nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  முழு i;                  /* Loop counter */
  கோவை_List *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  ப்ரதிஜ்ஞா( p!=0 );
  எனில்( p->selFlags & SF_Resolved ){
    திரும்பு WRC_Prune;
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
  எனில்( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    திரும்பு pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  போது( p ){
    ப்ரதிஜ்ஞா( (p->selFlags & SF_Expanded)!=0 );
    ப்ரதிஜ்ஞா( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, அளவு(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    எனில்( sqlite3_Resolve_கோவை_Names(&sNC, p->pLimit) ){
      திரும்பு WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    எனில்( p->selFlags & SF_Converted ){
      Select *pSub;
      ப்ரதிஜ்ஞா( p->pSrc->a[0].fg.isSubquery );
      ப்ரதிஜ்ஞா( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ப்ரதிஜ்ஞா( pSub!=0 );
      ப்ரதிஜ்ஞா( p->pSrc->nSrc==1 && p->pOrderBy );
      ப்ரதிஜ்ஞா( pSub->pPrior && pSub->pOrderBy==0 );
      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    எனில்( pOuterNC ) pOuterNC->nNestedSelect++;
    க்கு(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      ப்ரதிஜ்ஞா( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      எனில்( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        முழு nRef = pOuterNC ? pOuterNC->nRef : 0;
        மாறிலி எழுத்து *zSavedContext = pParse->zAuthContext;

        எனில்( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        எனில்( pParse->nErr ) திரும்பு WRC_Abort;
        ப்ரதிஜ்ஞா( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        எனில்( pOuterNC ){
          ப்ரதிஜ்ஞா( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    எனில்( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
 
    /* Resolve names in the result set. */
    எனில்( sqlite3_Resolve_கோவை_List_Names(&sNC, p->pEList) ) திரும்பு WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    ப்ரதிஜ்ஞா( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    எனில்( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      ப்ரதிஜ்ஞா( NC_MinMaxAgg==SF_MinMaxAgg );
      ப்ரதிஜ்ஞா( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }இல்லையெனில்{
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
    ப்ரதிஜ்ஞா( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    எனில்( p->pHaving ){
      எனில்( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_செய்தி(pParse, "HAVING clause on a non-aggregate query");
        திரும்பு WRC_Abort;
      }
      எனில்( sqlite3_Resolve_கோவை_Names(&sNC, p->pHaving) ) திரும்பு WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    எனில்( sqlite3_Resolve_கோவை_Names(&sNC, p->pWhere) ) திரும்பு WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    க்கு(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      எனில்( pItem->fg.isTabFunc
       && sqlite3_Resolve_கோவை_List_Names(&sNC, pItem->u1.pFuncArg)
      ){
        திரும்பு WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    எனில்( IN_RENAME_OBJECT ){
      Window *pWin;
      க்கு(pWin=p->pWinDefn; pWin; pWin=pWin->pNextWin){
        எனில்( sqlite3_Resolve_கோவை_List_Names(&sNC, pWin->pOrderBy)
         || sqlite3_Resolve_கோவை_List_Names(&sNC, pWin->pPartition)
        ){
          திரும்பு WRC_Abort;
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
    எனில்( p->selFlags & SF_Converted ){
      Select *pSub;
      ப்ரதிஜ்ஞா( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      ப்ரதிஜ்ஞா( pSub!=0 );
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
    எனில்( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      திரும்பு WRC_Abort;
    }
    எனில்( db->mallocFailed ){
      திரும்பு WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    எனில்( pGroupBy ){
      கட்டமைப்பு கோவை_List_item *pItem;
   
      எனில்( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        திரும்பு WRC_Abort;
      }
      க்கு(i=0, pItem=pGroupBy->a; i<pGroupBy->n_கோவை; i++, pItem++){
        எனில்( கோவை_Has_Property(pItem->p_கோவை, EP_Agg) ){
          sqlite3_Error_செய்தி(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          திரும்பு WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    எனில்( p->pNext && p->pEList->n_கோவை!=p->pNext->pEList->n_கோவை ){
      sqlite3_Select_Wrong_இலக்கம்_Terms_Error(pParse, p->pNext);
      திரும்பு WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    எனில்( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      எனில்( pParse->nErr ) திரும்பு WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  எனில்( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    திரும்பு WRC_Abort;
  }

  திரும்பு WRC_Prune;
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
முழு sqlite3_Resolve_கோவை_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  கோவை *p_கோவை             /* The expression to be analyzed. */
){
  முழு savedHasAgg;
  Walker w;

  எனில்( p_கோவை==0 ) திரும்பு SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.x_கோவை_Callback = resolve_கோவை_Step;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#எனில் SQLITE_MAX_கோவை_DEPTH>0
  w.pParse->nHeight += p_கோவை->nHeight;
  எனில்( sqlite3_கோவை_Check_Height(w.pParse, w.pParse->nHeight) ){
    திரும்பு SQLITE_ERROR;
  }
#endif
  ப்ரதிஜ்ஞா( p_கோவை!=0 );
  sqlite3_Walk_கோவை_NN(&w, p_கோவை);
#எனில் SQLITE_MAX_கோவை_DEPTH>0
  w.pParse->nHeight -= p_கோவை->nHeight;
#endif
  ப்ரதிஜ்ஞா( EP_Agg==NC_HasAgg );
  ப்ரதிஜ்ஞா( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  கோவை_Set_Property(p_கோவை, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  திரும்பு pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
முழு sqlite3_Resolve_கோவை_List_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  கோவை_List *pList         /* The expression list to be analyzed. */
){
  முழு i;
  முழு savedHasAgg = 0;
  Walker w;
  எனில்( pList==0 ) திரும்பு SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_கோவை_Callback = resolve_கோவை_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  க்கு(i=0; i<pList->n_கோவை; i++){
    கோவை *p_கோவை = pList->a[i].p_கோவை;
    எனில்( p_கோவை==0 ) தொடர்;
#எனில் SQLITE_MAX_கோவை_DEPTH>0
    w.pParse->nHeight += p_கோவை->nHeight;
    எனில்( sqlite3_கோவை_Check_Height(w.pParse, w.pParse->nHeight) ){
      திரும்பு SQLITE_ERROR;
    }
#endif
    sqlite3_Walk_கோவை_NN(&w, p_கோவை);
#எனில் SQLITE_MAX_கோவை_DEPTH>0
    w.pParse->nHeight -= p_கோவை->nHeight;
#endif
    ப்ரதிஜ்ஞா( EP_Agg==NC_HasAgg );
    ப்ரதிஜ்ஞா( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    எனில்( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      கோவை_Set_Property(p_கோவை, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    எனில்( w.pParse->nErr>0 ) திரும்பு SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  திரும்பு SQLITE_OK;
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
வெறுமை sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  ப்ரதிஜ்ஞா( p!=0 );
  w.x_கோவை_Callback = resolve_கோவை_Step;
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
முழு sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  முழு type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  கோவை *p_கோவை,     /* Expression to resolve.  May be NULL. */
  கோவை_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  முழு rc;
  ஒன்றிப்பு {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  ப்ரதிஜ்ஞா( type==0 || pTab!=0 );
  ப்ரதிஜ்ஞா( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_கோவை
          || type==NC_GenCol || pTab==0 );
  memset(&sNC, 0, அளவு(sNC));
  memset(&uSrc, 0, அளவு(uSrc));
  pSrc = &uSrc.sSrc;
  எனில்( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    எனில்( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  எனில்( (rc = sqlite3_Resolve_கோவை_Names(&sNC, p_கோவை))!=SQLITE_OK ) திரும்பு rc;
  எனில்( pList ) rc = sqlite3_Resolve_கோவை_List_Names(&sNC, pList);
  திரும்பு rc;
}
