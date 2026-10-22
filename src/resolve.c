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
#અન્તર્ભાવ "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#define EXCLUDED_TABLE_સંખ્યા  2

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
સ્થિર પૂર્ણાંક incrAggDepth(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  જો( p_પદાવલિ->op==TK_AGG_FUNCTION ) p_પદાવલિ->op2 += pWalker->u.n;
  પાછા WRC_Continue;
}
સ્થિર શૂન્ય incrAggFunctionDepth(પદાવલિ *p_પદાવલિ, પૂર્ણાંક N){
  જો( N>0 ){
    Walker w;
    memset(&w, 0, કદ(w));
    w.x_પદાવલિ_Callback = incrAggDepth;
    w.u.n = N;
    sqlite3_Walk_પદાવલિ(&w, p_પદાવલિ);
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
સ્થિર શૂન્ય resolveAlias(
  Parse *pParse,         /* Parsing context */
  પદાવલિ_List *pEList,      /* A result set */
  પૂર્ણાંક iCol,              /* A column in the result set.  0..pEList->nExpr-1 */
  પદાવલિ *p_પદાવલિ,           /* Transform this into an alias to the result set */
  પૂર્ણાંક nSubquery          /* Number of subqueries that the label is moving */
){
  પદાવલિ *pOrig;           /* The iCol-th column of the result set */
  પદાવલિ *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  પ્રતિજ્ઞા( iCol>=0 && iCol<pEList->n_પદાવલિ );
  pOrig = pEList->a[iCol].p_પદાવલિ;
  પ્રતિજ્ઞા( pOrig!=0 );
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Reduced|EP_TokenOnly) );
  જો( p_પદાવલિ->pAggInfo ) પાછા;
  db = pParse->db;
  pDup = sqlite3_પદાવલિ_Dup(db, pOrig, 0);
  જો( db->mallocFailed ){
    sqlite3_પદાવલિ_Delete(db, pDup);
    pDup = 0;
  }નહીંતર{
    પદાવલિ temp;
    incrAggFunctionDepth(pDup, nSubquery);
    જો( p_પદાવલિ->op==TK_COLLATE ){
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      pDup = sqlite3_પદાવલિ_Add_Collate_String(pParse, pDup, p_પદાવલિ->u.zToken);
    }
    memcpy(&temp, pDup, કદ(પદાવલિ));
    memcpy(pDup, p_પદાવલિ, કદ(પદાવલિ));
    memcpy(p_પદાવલિ, &temp, કદ(પદાવલિ));
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
      જો( ALWAYS(p_પદાવલિ->y.pWin!=0) ){
        p_પદાવલિ->y.pWin->pOwner = p_પદાવલિ;
      }
    }
    sqlite3_પદાવલિ_Deferred_Delete(pParse, pDup);
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
પૂર્ણાંક sqlite3MatchEName(
  અચલ સંરચના પદાવલિ_List_item *pItem,
  અચલ અક્ષર *zCol,
  અચલ અક્ષર *zTab,
  અચલ અક્ષર *zDb,
  પૂર્ણાંક *pbRowid
){
  પૂર્ણાંક n;
  અચલ અક્ષર *zSpan;
  પૂર્ણાંક eEName = pItem->fg.eEName;
  જો( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    પાછા 0;
  }
  પ્રતિજ્ઞા( pbRowid==0 || *pbRowid==0 );
  zSpan = pItem->zEName;
  માટે(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  જો( zDb && (sqlite3StrNICmp(zSpan, zDb, n)!=0 || zDb[n]!=0) ){
    પાછા 0;
  }
  zSpan += n+1;
  માટે(n=0; ALWAYS(zSpan[n]) && zSpan[n]!='.'; n++){}
  જો( zTab && (sqlite3StrNICmp(zSpan, zTab, n)!=0 || zTab[n]!=0) ){
    પાછા 0;
  }
  zSpan += n+1;
  જો( zCol ){
    જો( eEName==ENAME_TAB && sqlite3StrICmp(zSpan, zCol)!=0 ) પાછા 0;
    જો( eEName==ENAME_ROWID && sqlite3IsRowid(zCol)==0 ) પાછા 0;
  }
  જો( eEName==ENAME_ROWID ) *pbRowid = 1;
  પાછા 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
સ્થિર પૂર્ણાંક areDoubleQuotedStringsEnabled(sqlite3 *db, NameContext *p_ટોચ_NC){
  જો( db->init.busy ) પાછા 1;  /* Always support for legacy schemas */
  જો( p_ટોચ_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    જો( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      પાછા 1;
    }
    પાછા (db->flags & SQLITE_DqsDDL)!=0;
  }નહીંતર{
    /* Currently parsing a DML statement */
    પાછા (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_પદાવલિ_Col_Used(પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક n;
  Table *pExTab;

  n = p_પદાવલિ->iColumn;
  પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
  pExTab = p_પદાવલિ->y.pTab;
  પ્રતિજ્ઞા( pExTab!=0 );
  પ્રતિજ્ઞા( n < pExTab->nCol );
  જો( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->aCol[n].colFlags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->nCol==BMS-1 );
    testcase( pExTab->nCol==BMS );
    પાછા pExTab->nCol>=BMS ? ALLBITS : MASKBIT(pExTab->nCol)-1;
  }નહીંતર{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    જો( n>=BMS ) n = BMS-1;
    પાછા ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
સ્થિર શૂન્ય extendFJMatch(
  Parse *pParse,          /* Parsing context */
  પદાવલિ_List **ppList,      /* ExprList to extend */
  SrcItem *pMatch,        /* Source table containing the column */
  i16 iColumn             /* The column number */
){
  પદાવલિ *pNew = sqlite3_પદાવલિ_Alloc(pParse->db, TK_COLUMN, 0, 0);
  જો( pNew ){
    pNew->iTable = pMatch->iCursor;
    pNew->iColumn = iColumn;
    pNew->y.pTab = pMatch->pSTab;
    પ્રતિજ્ઞા( (pMatch->fg.jointype & (JT_ડાબું|JT_LTORJ))!=0 );
    પદાવલિ_Set_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_પદાવલિ_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
સ્થિર SQLITE_NOINLINE પૂર્ણાંક isValidSchemaTableName(
  અચલ અક્ષર *zTab,         /* Name as it appears in the SQL */
  Table *pTab,              /* The schema table we are trying to match */
  અચલ અક્ષર *zDb           /* non-NULL if a database qualifier is present */
){
  અચલ અક્ષર *zLegacy;
  પ્રતિજ્ઞા( pTab!=0 );
  પ્રતિજ્ઞા( pTab->tnum==1 );
  જો( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) પાછા 0;
  zLegacy = pTab->zName;
  જો( strcmp(zLegacy+7, &LEGACY_TEMP_SCHEMA_TABLE[7])==0 ){
    જો( sqlite3StrICmp(zTab+7, &PREFERRED_TEMP_SCHEMA_TABLE[7])==0 ){
      પાછા 1;
    }
    જો( zDb==0 ) પાછા 0;
    જો( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_TABLE[7])==0 ) પાછા 1;
    જો( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) પાછા 1;
  }નહીંતર{
    જો( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_TABLE[7])==0 ) પાછા 1;
  }
  પાછા 0;
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
સ્થિર પૂર્ણાંક lookupName(
  Parse *pParse,       /* The parsing context */
  અચલ અક્ષર *zDb,     /* Name of the database containing table, or NULL */
  અચલ અક્ષર *zTab,    /* Name of table containing column, or NULL */
  અચલ પદાવલિ *p_જમણું,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  પદાવલિ *p_પદાવલિ          /* Make this EXPR node point to the selected column */
){
  પૂર્ણાંક i, j;                         /* Loop counters */
  પૂર્ણાંક cnt = 0;                      /* Number of matching column names */
  પૂર્ણાંક cntTab = 0;                   /* Number of potential "rowid" matches */
  પૂર્ણાંક nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  SrcItem *pItem;                   /* Use for looping over pSrcList items */
  SrcItem *pMatch = 0;              /* The matching pSrcList item */
  NameContext *p_ટોચ_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  પૂર્ણાંક e_New_પદાવલિ_Op = TK_COLUMN;       /* New value for pExpr->op on success */
  Table *pTab = 0;                  /* Table holding the row */
  પદાવલિ_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  અચલ અક્ષર *zCol = p_જમણું->u.zToken;

  પ્રતિજ્ઞા( pNC );     /* the name context cannot be NULL. */
  પ્રતિજ્ઞા( zCol );    /* The Z in X.Y.Z cannot be NULL */
  પ્રતિજ્ઞા( zDb==0 || zTab!=0 );
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_પદાવલિ->iTable = -1;
  પદાવલિ_Set_VVAProperty(p_પદાવલિ, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  જો( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    જો( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }નહીંતર{
      માટે(i=0; i<db->nDb; i++){
        પ્રતિજ્ઞા( db->aDb[i].zDbSName );
        જો( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          તોડો;
        }
      }
      જો( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  પ્રતિજ્ઞા( pNC && cnt==0 );
  કરો{
    પદાવલિ_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    જો( pSrcList ){
      માટે(i=0, pItem=pSrcList->a; i<pSrcList->nSrc; i++, pItem++){
        pTab = pItem->pSTab;
        પ્રતિજ્ઞા( pTab!=0 && pTab->zName!=0 );
        પ્રતિજ્ઞા( pTab->nCol>0 || pParse->nErr );
        પ્રતિજ્ઞા( (પૂર્ણાંક)pItem->fg.isNestedFrom == IsNestedFrom(pItem));
        જો( pItem->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          પૂર્ણાંક hit = 0;
          Select *pSel;
          પ્રતિજ્ઞા( pItem->fg.isSubquery );
          પ્રતિજ્ઞા( pItem->u4.pSubq!=0 );
          pSel = pItem->u4.pSubq->pSelect;
          પ્રતિજ્ઞા( pSel!=0 );
          pEList = pSel->pEList;
          પ્રતિજ્ઞા( pEList!=0 );
          પ્રતિજ્ઞા( pEList->n_પદાવલિ==pTab->nCol );
          માટે(j=0; j<pEList->n_પદાવલિ; j++){
            પૂર્ણાંક bRowid = 0;       /* True if possible rowid match */
            જો( !sqlite3MatchEName(&pEList->a[j], zCol, zTab, zDb, &bRowid) ){
              ચાલુ;
            }
            જો( bRowid==0 ){
              જો( cnt>0 ){
                જો( pItem->fg.isUsing==0
                 || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
                 || pMatch==pItem
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_પદાવલિ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }નહીંતર
                જો( (pItem->fg.jointype & JT_જમણું)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  ચાલુ;
                }નહીંતર
                જો( (pItem->fg.jointype & JT_ડાબું)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_પદાવલિ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }નહીંતર{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, pMatch, p_પદાવલિ->iColumn);
                }
              }
              cnt++;
              hit = 1;
            }નહીંતર જો( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              ચાલુ;
            }
            cntTab++;
            pMatch = pItem;
            p_પદાવલિ->iColumn = j;
            pEList->a[j].fg.bUsed = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            પ્રતિજ્ઞા( bRowid==0 || pEList->a[j].fg.bUsingTerm==0 );
            જો( pEList->a[j].fg.bUsingTerm ) તોડો;
          }
          જો( hit || zTab==0 ) ચાલુ;
        }
        પ્રતિજ્ઞા( zDb==0 || zTab!=0 );
        જો( zTab ){
          જો( zDb ){
            જો( pTab->pSchema!=pSchema ) ચાલુ;
            જો( pSchema==0 && strcmp(zDb,"*")!=0 ) ચાલુ;
          }
          જો( pItem->zAlias!=0 ){
            જો( sqlite3StrICmp(zTab, pItem->zAlias)!=0 ){
              ચાલુ;
            }
          }નહીંતર જો( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            જો( pTab->tnum!=1 ) ચાલુ;
            જો( !isValidSchemaTableName(zTab, pTab, zDb) ) ચાલુ;
          }
          પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
          જો( IN_RENAME_OBJECT && pItem->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (શૂન્ય*)&p_પદાવલિ->y.pTab);
          }
        }
        j = sqlite3ColumnIndex(pTab, zCol);
        જો( j>=0 ){
          જો( cnt>0 ){
            જો( pItem->fg.isUsing==0
             || sqlite3IdListIndex(pItem->u3.pUsing, zCol)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_પદાવલિ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }નહીંતર
            જો( (pItem->fg.jointype & JT_જમણું)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              ચાલુ;
            }નહીંતર
            જો( (pItem->fg.jointype & JT_ડાબું)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_પદાવલિ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }નહીંતર{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, pMatch, p_પદાવલિ->iColumn);
            }
          }
          cnt++;
          pMatch = pItem;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_પદાવલિ->iColumn = j==pTab->iPKey ? -1 : (i16)j;
          જો( pItem->fg.isNestedFrom ){
            sqlite3SrcItemColumnUsed(pItem, j);
          }
        }
        જો( 0==cnt && VisibleRowid(pTab) ){
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
          જો( cntTab==0
           || (cntTab==1
               && pMatch!=0
               && ALWAYS(pMatch->pSTab!=0)
               && (pMatch->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            pMatch = pItem;
          }નહીંતર{
            cntTab++;
          }
#નહીંતર
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          pMatch = pItem;
#endif
        }
      }
      જો( pMatch ){
        p_પદાવલિ->iTable = pMatch->iCursor;
        પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
        p_પદાવલિ->y.pTab = pMatch->pSTab;
        જો( (pMatch->fg.jointype & (JT_ડાબું|JT_LTORJ))!=0 ){
          પદાવલિ_Set_Property(p_પદાવલિ, EP_CanBeNull);
        }
        pSchema = p_પદાવલિ->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#જો !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    જો( cnt==0 && zDb==0 ){
      pTab = 0;
#ifndef SQLITE_OMIT_TRIGGER
      જો( pParse->pTriggerTab!=0 ){
        પૂર્ણાંક op = pParse->eTriggerOp;
        પ્રતિજ્ઞા( op==TK_DELETE || op==TK_UPDATE || op==TK_ઉમેરો );
        જો( pParse->bReturning ){
          જો( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || isValidSchemaTableName(zTab, pParse->pTriggerTab, 0))
          ){
            p_પદાવલિ->iTable = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }નહીંતર જો( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_પદાવલિ->iTable = 1;
          pTab = pParse->pTriggerTab;
        }નહીંતર જો( op!=TK_ઉમેરો && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_પદાવલિ->iTable = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#endif /* SQLITE_OMIT_TRIGGER */
#ifndef SQLITE_OMIT_UPSERT
      જો( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        જો( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_પદાવલિ->iTable = EXCLUDED_TABLE_સંખ્યા;
          જો( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (શૂન્ય*)&p_પદાવલિ->y.pTab);
          }
        }
      }
#endif /* SQLITE_OMIT_UPSERT */

      જો( pTab ){
        પૂર્ણાંક iCol;
        pSchema = pTab->pSchema;
        cntTab++;
        iCol = sqlite3ColumnIndex(pTab, zCol);
        જો( iCol>=0 ){
          જો( pTab->iPKey==iCol ) iCol = -1;
        }નહીંતર{
          જો( sqlite3IsRowid(zCol) && VisibleRowid(pTab) ){
            iCol = -1;
          }નહીંતર{
            iCol = pTab->nCol;
          }
        }
        જો( iCol<pTab->nCol ){
          cnt++;
          pMatch = 0;
#ifndef SQLITE_OMIT_UPSERT
          જો( p_પદાવલિ->iTable==EXCLUDED_TABLE_સંખ્યા ){
            testcase( iCol==(-1) );
            પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
            જો( IN_RENAME_OBJECT ){
              p_પદાવલિ->iColumn = iCol;
              p_પદાવલિ->y.pTab = pTab;
              e_New_પદાવલિ_Op = TK_COLUMN;
            }નહીંતર{
              p_પદાવલિ->iTable = pNC->uNC.pUpsert->regData +
                 sqlite3TableColumnToStorage(pTab, iCol);
              e_New_પદાવલિ_Op = TK_REGISTER;
            }
          }નહીંતર
#endif /* SQLITE_OMIT_UPSERT */
          {
            પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
            p_પદાવલિ->y.pTab = pTab;
            જો( pParse->bReturning ){
              e_New_પદાવલિ_Op = TK_REGISTER;
              p_પદાવલિ->op2 = TK_COLUMN;
              p_પદાવલિ->iColumn = iCol;
              p_પદાવલિ->iTable = pNC->uNC.iBaseReg + (pTab->nCol+1)*p_પદાવલિ->iTable +
                 sqlite3TableColumnToStorage(pTab, iCol) + 1;
            }નહીંતર{
              p_પદાવલિ->iColumn = (i16)iCol;
              e_New_પદાવલિ_Op = TK_TRIGGER;
#ifndef SQLITE_OMIT_TRIGGER
              જો( iCol<0 ){
                p_પદાવલિ->aff_પદાવલિ = SQLITE_AFF_INTEGER;
              }નહીંતર જો( p_પદાવલિ->iTable==0 ){
                testcase( iCol==31 );
                testcase( iCol==32 );
                pParse->oldmask |= (iCol>=32 ? 0xffffffff : (((u32)1)<<iCol));
              }નહીંતર{
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
    જો( cnt==0
     && cntTab>=1
     && pMatch
     && (pNC->ncFlags & (NC_Idx_પદાવલિ|NC_GenCol))==0
     && sqlite3IsRowid(zCol)
     && ALWAYS(VisibleRowid(pMatch->pSTab) || pMatch->fg.isNestedFrom)
    ){
      cnt = cntTab;
#જો SQLITE_ALLOW_ROWID_IN_VIEW+0==2
      જો( pMatch->pSTab!=0 && IsView(pMatch->pSTab) ){
        e_New_પદાવલિ_Op = TK_NULL;
      }
#endif
      જો( pMatch->fg.isNestedFrom==0 ) p_પદાવલિ->iColumn = -1;
      p_પદાવલિ->aff_પદાવલિ = SQLITE_AFF_INTEGER;
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
    જો( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      પ્રતિજ્ઞા( pEList!=0 );
      માટે(j=0; j<pEList->n_પદાવલિ; j++){
        અક્ષર *zAs = pEList->a[j].zEName;
        જો( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, zCol)==0
        ){
          પદાવલિ *pOrig;
          પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું==0 && p_પદાવલિ->p_જમણું==0 );
          પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ)==0 || p_પદાવલિ->x.pList==0 );
          પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ)==0 || p_પદાવલિ->x.pSelect==0 );
          pOrig = pEList->a[j].p_પદાવલિ;
          જો( (pNC->ncFlags&NC_AllowAgg)==0 && પદાવલિ_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_સંદેશ(pParse, "misuse of aliased aggregate %s", zAs);
            પાછા WRC_Abort;
          }
          જો( પદાવલિ_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_ટોચ_NC )
          ){
            sqlite3_Error_સંદેશ(pParse, "misuse of aliased window function %s",zAs);
            પાછા WRC_Abort;
          }
          જો( sqlite3_પદાવલિ_Vector_Size(pOrig)!=1 ){
            sqlite3_Error_સંદેશ(pParse, "row value misused");
            પાછા WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_પદાવલિ, nSubquery);
          cnt = 1;
          pMatch = 0;
          પ્રતિજ્ઞા( zTab==0 && zDb==0 );
          જો( IN_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (શૂન્ય*)p_પદાવલિ);
          }
          જાઓ lookupname_end;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    જો( cnt ) તોડો;
    pNC = pNC->pNext;
    nSubquery++;
  }જ્યારે( pNC );


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
  જો( cnt==0 && zTab==0 ){
    પ્રતિજ્ઞા( p_પદાવલિ->op==TK_ID );
    જો( પદાવલિ_Has_Property(p_પદાવલિ,EP_DblQuoted)
     && areDoubleQuotedStringsEnabled(db, p_ટોચ_NC)
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
      p_પદાવલિ->op = TK_STRING;
      memset(&p_પદાવલિ->y, 0, કદ(p_પદાવલિ->y));
      પાછા WRC_Prune;
    }
    જો( sqlite3_પદાવલિ_Id_To_True_False(p_પદાવલિ) ){
      પાછા WRC_Prune;
    }
  }

  /*
  ** cnt==0 means there was not match.
  ** cnt>1 means there were two or more matches.
  **
  ** cnt==0 is always an error.  cnt>1 is often an error, but might
  ** be multiple matches for a NATURAL OUTER JOIN or a OUTER JOIN USING.
  */
  પ્રતિજ્ઞા( pFJMatch==0 || cnt>0 );
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_xIsSelect|EP_Int_કિંમત) );
  જો( cnt!=1 ){
    અચલ અક્ષર *zErr;
    જો( pFJMatch ){
      જો( pFJMatch->n_પદાવલિ==cnt-1 ){
        જો( પદાવલિ_Has_Property(p_પદાવલિ,EP_Leaf) ){
          પદાવલિ_Clear_Property(p_પદાવલિ,EP_Leaf);
        }નહીંતર{
          sqlite3_પદાવલિ_Delete(db, p_પદાવલિ->p_ડાબું);
          p_પદાવલિ->p_ડાબું = 0;
          sqlite3_પદાવલિ_Delete(db, p_પદાવલિ->p_જમણું);
          p_પદાવલિ->p_જમણું = 0;
        }
        extendFJMatch(pParse, &pFJMatch, pMatch, p_પદાવલિ->iColumn);
        p_પદાવલિ->op = TK_FUNCTION;
        p_પદાવલિ->u.zToken = "coalesce";
        p_પદાવલિ->x.pList = pFJMatch;
        p_પદાવલિ->aff_પદાવલિ = SQLITE_AFF_DEFER;
        cnt = 1;
        જાઓ lookupname_end;
      }નહીંતર{
        sqlite3_પદાવલિ_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    જો( zDb ){
      sqlite3_Error_સંદેશ(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, zCol);
    }નહીંતર જો( zTab ){
      sqlite3_Error_સંદેશ(pParse, "%s: %s.%s", zErr, zTab, zCol);
    }નહીંતર જો( cnt==0 && પદાવલિ_Has_Property(p_જમણું,EP_DblQuoted) ){
      sqlite3_Error_સંદેશ(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, zCol);
    }નહીંતર{
      sqlite3_Error_સંદેશ(pParse, "%s: %s", zErr, zCol);
    }
    sqlite3_Record_Error_Offset_Of_પદાવલિ(pParse->db, p_પદાવલિ);
    pParse->checkSchema = 1;
    p_ટોચ_NC->nNcErr++;
    e_New_પદાવલિ_Op = TK_NULL;
  }
  પ્રતિજ્ઞા( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  જો( !પદાવલિ_Has_Property(p_પદાવલિ,(EP_TokenOnly|EP_Leaf)) ){
    sqlite3_પદાવલિ_Delete(db, p_પદાવલિ->p_ડાબું);
    p_પદાવલિ->p_ડાબું = 0;
    sqlite3_પદાવલિ_Delete(db, p_પદાવલિ->p_જમણું);
    p_પદાવલિ->p_જમણું = 0;
    પદાવલિ_Set_Property(p_પદાવલિ, EP_Leaf);
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
  જો( pMatch ){
    જો( p_પદાવલિ->iColumn>=0 ){
      pMatch->colUsed |= sqlite3_પદાવલિ_Col_Used(p_પદાવલિ);
    }નહીંતર{
      pMatch->fg.rowidUsed = 1;
    }
  }

  p_પદાવલિ->op = e_New_પદાવલિ_Op;
lookupname_end:
  જો( cnt==1 ){
    પ્રતિજ્ઞા( pNC!=0 );
#ifndef SQLITE_OMIT_AUTHORIZATION
    જો( db->xAuth ){
      જો( pFJMatch ){
        પ્રતિજ્ઞા( p_પદાવલિ->op==TK_FUNCTION );
        પ્રતિજ્ઞા( sqlite3_stricmp(p_પદાવલિ->u.zToken,"coalesce")==0 );
        પ્રતિજ્ઞા( p_પદાવલિ->x.pList==pFJMatch );
        પ્રતિજ્ઞા( pFJMatch->n_પદાવલિ>0 );
        માટે(i=0; i<pFJMatch->n_પદાવલિ; i++){
          પ્રતિજ્ઞા( pFJMatch->a[i].p_પદાવલિ->op==TK_COLUMN );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_પદાવલિ,pSchema,pNC->pSrcList);
        }
      }નહીંતર જો( p_પદાવલિ->op==TK_COLUMN || p_પદાવલિ->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_પદાવલિ, pSchema, pNC->pSrcList);
      }
    } 
#endif
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    માટે(;;){
      પ્રતિજ્ઞા( p_ટોચ_NC!=0 );
      p_ટોચ_NC->nRef++;
      જો( p_ટોચ_NC==pNC ) તોડો;
      p_ટોચ_NC = p_ટોચ_NC->pNext;
    }
    પાછા WRC_Prune;
  } નહીંતર {
    પાછા WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
પદાવલિ *sqlite3_Create_Column_પદાવલિ(sqlite3 *db, SrcList *pSrc, પૂર્ણાંક iSrc, પૂર્ણાંક iCol){
  પદાવલિ *p = sqlite3_પદાવલિ_Alloc(db, TK_COLUMN, 0, 0);
  જો( p ){
    SrcItem *pItem = &pSrc->a[iSrc];
    Table *pTab;
    પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p) );
    pTab = p->y.pTab = pItem->pSTab;
    p->iTable = pItem->iCursor;
    જો( p->y.pTab->iPKey==iCol ){
      p->iColumn = -1;
    }નહીંતર{
      p->iColumn = (ynVar)iCol;
      જો( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->aCol[iCol].colFlags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->nCol==63 );
        testcase( pTab->nCol==64 );
        pItem->colUsed = pTab->nCol>=64 ? ALLBITS : MASKBIT(pTab->nCol)-1;
      }નહીંતર{
        testcase( iCol==BMS );
        testcase( iCol==BMS-1 );
        pItem->colUsed |= ((Bitmask)1)<<(iCol>=BMS ? BMS-1 : iCol);
      }
    }
  }
  પાછા p;
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
સ્થિર શૂન્ય notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   અચલ અક્ષર *z_સંદેશ,    /* Type of error */
   પદાવલિ *p_પદાવલિ,         /* Invalidate this expression on error */
   પદાવલિ *pError         /* Associate error with this expression */
){
  અચલ અક્ષર *zIn = "partial index WHERE clauses";
  જો( pNC->ncFlags & NC_Idx_પદાવલિ )      zIn = "index expressions";
#ifndef SQLITE_OMIT_CHECK
  નહીંતર જો( pNC->ncFlags & NC_IsCheck ) zIn = "CHECK constraints";
#endif
#ifndef SQLITE_OMIT_GENERATED_COLUMNS
  નહીંતર જો( pNC->ncFlags & NC_GenCol ) zIn = "generated columns";
#endif
  sqlite3_Error_સંદેશ(pParse, "%s prohibited in %s", z_સંદેશ, zIn);
  જો( p_પદાવલિ ) p_પદાવલિ->op = TK_NULL;
  sqlite3_Record_Error_Offset_Of_પદાવલિ(pParse->db, pError);
}
#define sqlite3ResolveNotValid(P,N,M,X,E,R) \
  પ્રતિજ્ઞા( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_પદાવલિ|NC_GenCol))==0 ); \
  જો( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
સ્થિર પૂર્ણાંક પદાવલિ_Probability(પદાવલિ *p){
  દ્વિગુણ r = -1.0;
  જો( p->op!=TK_FLOAT ) પાછા -1;
  પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p, EP_Int_કિંમત) );
  sqlite3AtoF(p->u.zToken, &r);
  પ્રતિજ્ઞા( r>=0.0 );
  જો( r>1.0 ) પાછા -1;
  પાછા (પૂર્ણાંક)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
સ્થિર SQLITE_NOINLINE શૂન્ય resolve_Set_પદાવલિ_Subtype_Arg(પદાવલિ_List *pList){
  પૂર્ણાંક nn, ii;
  nn = pList ? pList->n_પદાવલિ : 0;
  માટે(ii=0; ii<nn; ii++){
    પદાવલિ *p_પદાવલિ = pList->a[ii].p_પદાવલિ;
    જ્યારે( 1 /*exit-by-break*/ ){
      પદાવલિ_Set_Property(p_પદાવલિ, EP_SubtArg);
      જો( p_પદાવલિ->op==TK_SELECT ){
        પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ) );
        પ્રતિજ્ઞા( p_પદાવલિ->x.pSelect!=0 );
        resolve_Set_પદાવલિ_Subtype_Arg(p_પદાવલિ->x.pSelect->pEList);
        તોડો;
      }
      જો( p_પદાવલિ->op==TK_UPLUS ){
        p_પદાવલિ = p_પદાવલિ->p_ડાબું;
        પ્રતિજ્ઞા( p_પદાવલિ!=0 );
      }નહીંતર{
        તોડો;
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
સ્થિર પૂર્ણાંક resolve_પદાવલિ_Step(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  પ્રતિજ્ઞા( pNC!=0 );
  pParse = pNC->pParse;
  પ્રતિજ્ઞા( pParse==pWalker->pParse );

#ifndef NDEBUG
  જો( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    પૂર્ણાંક i;
    માટે(i=0; i<pNC->pSrcList->nSrc; i++){
      પ્રતિજ્ઞા( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#endif
  પસંદગી( p_પદાવલિ->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    સ્થિતિ TK_ROW: {
      SrcList *pSrcList = pNC->pSrcList;
      SrcItem *pItem;
      પ્રતિજ્ઞા( pSrcList && pSrcList->nSrc>=1 );
      pItem = pSrcList->a;
      p_પદાવલિ->op = TK_COLUMN;
      પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
      p_પદાવલિ->y.pTab = pItem->pSTab;
      p_પદાવલિ->iTable = pItem->iCursor;
      p_પદાવલિ->iColumn--;
      p_પદાવલિ->aff_પદાવલિ = SQLITE_AFF_INTEGER;
      તોડો;
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
    સ્થિતિ TK_NOTNULL:
    સ્થિતિ TK_ISNULL: {
      પૂર્ણાંક anRef[8];
      NameContext *p;
      પૂર્ણાંક i;
      માટે(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_પદાવલિ(pWalker, p_પદાવલિ->p_ડાબું);
      જો( IN_RENAME_OBJECT ) પાછા WRC_Prune;
      જો( sqlite3_પદાવલિ_Can_Be_Null(p_પદાવલિ->p_ડાબું) ){
        /* The expression can be NULL.  So the optimization does not apply */
        પાછા WRC_Prune;
      }

      માટે(i=0, p=pNC; p; p=p->pNext, i++){
        જો( (p->ncFlags & NC_Where)==0 ){
          પાછા WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) );
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
#જો TREETRACE_ENABLED
      જો( sqlite3_વૃક્ષ_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_પદાવલિ->op==TK_NOTNULL
        );
        sqlite3_Show_પદાવલિ(p_પદાવલિ);
      }
#endif /* TREETRACE_ENABLED */
      p_પદાવલિ->u.i_કિંમત = (p_પદાવલિ->op==TK_NOTNULL);
      p_પદાવલિ->flags |= EP_Int_કિંમત;
      p_પદાવલિ->op = TK_INTEGER;
      માટે(i=0, p=pNC; p && i<ArraySize(anRef); p=p->pNext, i++){
        p->nRef = anRef[i];
      }
      sqlite3_પદાવલિ_Delete(pParse->db, p_પદાવલિ->p_ડાબું);
      p_પદાવલિ->p_ડાબું = 0;
      પાછા WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    સ્થિતિ TK_ID:
    સ્થિતિ TK_DOT: {
      અચલ અક્ષર *zTable;
      અચલ અક્ષર *zDb;
      પદાવલિ *p_જમણું;

      જો( p_પદાવલિ->op==TK_ID ){
        zDb = 0;
        zTable = 0;
        પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
        p_જમણું = p_પદાવલિ;
      }નહીંતર{
        પદાવલિ *p_ડાબું = p_પદાવલિ->p_ડાબું;
        testcase( pNC->ncFlags & NC_Idx_પદાવલિ );
        testcase( pNC->ncFlags & NC_GenCol );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_પદાવલિ|NC_GenCol, 0, p_પદાવલિ);
        p_જમણું = p_પદાવલિ->p_જમણું;
        જો( p_જમણું->op==TK_ID ){
          zDb = 0;
        }નહીંતર{
          પ્રતિજ્ઞા( p_જમણું->op==TK_DOT );
          પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_જમણું, EP_Int_કિંમત) );
          zDb = p_ડાબું->u.zToken;
          p_ડાબું = p_જમણું->p_ડાબું;
          p_જમણું = p_જમણું->p_જમણું;
        }
        પ્રતિજ્ઞા( પદાવલિ_Use_UToken(p_ડાબું) && પદાવલિ_Use_UToken(p_જમણું) );
        zTable = p_ડાબું->u.zToken;
        પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
        જો( IN_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (શૂન્ય*)p_પદાવલિ, (શૂન્ય*)p_જમણું);
          sqlite3RenameTokenRemap(pParse, (શૂન્ય*)&p_પદાવલિ->y.pTab, (શૂન્ય*)p_ડાબું);
        }
      }
      પાછા lookupName(pParse, zDb, zTable, p_જમણું, pNC, p_પદાવલિ);
    }

    /* Resolve function names
    */
    સ્થિતિ TK_FUNCTION: {
      પદાવલિ_List *pList;            /* The argument list */
      પૂર્ણાંક n;                      /* Number of arguments */
      પૂર્ણાંક no_such_func = 0;       /* True if no such function exists */
      પૂર્ણાંક wrong_અંક_args = 0;     /* True if wrong number of arguments */
      પૂર્ણાંક is_agg = 0;             /* True if is an aggregate function */
      અચલ અક્ષર *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      પૂર્ણાંક savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#ifndef SQLITE_OMIT_WINDOWFUNC
      Window *pWin = (IsWindowFunc(p_પદાવલિ) ? p_પદાવલિ->y.pWin : 0);
#endif
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_xIsSelect|EP_Int_કિંમત) );
      પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું==0 || p_પદાવલિ->p_ડાબું->op==TK_ORDER );
      pList = p_પદાવલિ->x.pList;
      n = pList ? pList->n_પદાવલિ : 0;
      zId = p_પદાવલિ->u.zToken;
      pDef = sqlite3FindFunction(pParse->db, zId, n, enc, 0);
      જો( pDef==0 ){
        pDef = sqlite3FindFunction(pParse->db, zId, -2, enc, 0);
        જો( pDef==0 ){
          no_such_func = 1;
        }નહીંતર{
          wrong_અંક_args = 1;
        }
      }નહીંતર{
        is_agg = pDef->xFinalize!=0;
        જો( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          પદાવલિ_Set_Property(p_પદાવલિ, EP_Unlikely);
          જો( n==2 ){
            p_પદાવલિ->iTable = પદાવલિ_Probability(pList->a[1].p_પદાવલિ);
            જો( p_પદાવલિ->iTable<0 ){
              sqlite3_Error_સંદેશ(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_પદાવલિ);
              pNC->nNcErr++;
            }
          }નહીંતર{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_પદાવલિ->iTable = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#ifndef SQLITE_OMIT_AUTHORIZATION
        {
          પૂર્ણાંક auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          જો( auth!=SQLITE_OK ){
            જો( auth==SQLITE_DENY ){
              sqlite3_Error_સંદેશ(pParse, "not authorized to use function: %#T",
                                      p_પદાવલિ);
              pNC->nNcErr++;
            }
            p_પદાવલિ->op = TK_NULL;
            પાછા WRC_Prune;
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
        જો( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || પદાવલિ_Has_Property(p_પદાવલિ, EP_SubtArg) 
        ){
          resolve_Set_પદાવલિ_Subtype_Arg(pList);
        }

        જો( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          પદાવલિ_Set_Property(p_પદાવલિ,EP_ConstFunc);
        }
        જો( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_પદાવલિ|NC_PartIdx|NC_GenCol, 0, p_પદાવલિ);
        }નહીંતર{
          પ્રતિજ્ઞા( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_પદાવલિ->op2 = pNC->ncFlags & NC_SelfRef;
        }
        જો( (pDef->funcFlags & SQLITE_FUNC_INTERNAL)!=0
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
        }નહીંતર
        જો( (pDef->funcFlags & (SQLITE_FUNC_DIRECT|SQLITE_FUNC_UNSAFE))!=0
         && !IN_RENAME_OBJECT
        ){
          જો( pNC->ncFlags & NC_FromDDL ) પદાવલિ_Set_Property(p_પદાવલિ, EP_FromDDL);
          sqlite3_પદાવલિ_Function_Usable(pParse, p_પદાવલિ, pDef);
        }
      }

      જો( 0==IN_RENAME_OBJECT ){
#ifndef SQLITE_OMIT_WINDOWFUNC
        પ્રતિજ્ઞા( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_કિંમત==0 && pDef->xInverse==0)
          || (pDef->x_કિંમત && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        જો( pDef && pDef->x_કિંમત==0 && pWin ){
          sqlite3_Error_સંદેશ(pParse,
              "%#T() may not be used as a window function", p_પદાવલિ
          );
          pNC->nNcErr++;
        }નહીંતર જો(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_WINDOW) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          અચલ અક્ષર *zType;
          જો( (pDef->funcFlags & SQLITE_FUNC_WINDOW) || pWin ){
            zType = "window";
          }નહીંતર{
            zType = "aggregate";
          }
          sqlite3_Error_સંદેશ(pParse, "misuse of %s function %#T()",zType,p_પદાવલિ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#નહીંતર
        જો( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_સંદેશ(pParse,"misuse of aggregate function %#T()",p_પદાવલિ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#endif
        નહીંતર જો( no_such_func
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
          sqlite3_Error_સંદેશ(pParse, "no such function: %#T", p_પદાવલિ);
          pNC->nNcErr++;
        }નહીંતર જો( wrong_અંક_args ){
          sqlite3_Error_સંદેશ(pParse,"wrong number of arguments to function %#T()",
               p_પદાવલિ);
          pNC->nNcErr++;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        નહીંતર જો( is_agg==0 && પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
          sqlite3_Error_સંદેશ(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_પદાવલિ
          );
          pNC->nNcErr++;
        }
#endif
        નહીંતર જો( is_agg==0 && p_પદાવલિ->p_ડાબું ){
          sqlite3_પદાવલિ_Order_By_Aggregate_Error(pParse, p_પદાવલિ);
          pNC->nNcErr++;
        }
        જો( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#ifndef SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#નહીંતર
          pNC->ncFlags &= ~NC_AllowAgg;
#endif
        }
      }
      નહીંતર જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) || p_પદાવલિ->p_ડાબું ){
        is_agg = 1;
      }
      sqlite3_Walk_પદાવલિ_List(pWalker, pList);
      જો( is_agg ){
        જો( p_પદાવલિ->p_ડાબું ){
          પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું->op==TK_ORDER );
          પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ->p_ડાબું) );
          sqlite3_Walk_પદાવલિ_List(pWalker, p_પદાવલિ->p_ડાબું->x.pList);
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        જો( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          પ્રતિજ્ઞા( પદાવલિ_Use_YWin(p_પદાવલિ) && pWin==p_પદાવલિ->y.pWin );
          જો( IN_RENAME_OBJECT==0 ){
            sqlite3WindowUpdate(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            જો( pParse->db->mallocFailed ) તોડો;
          }
          sqlite3_Walk_પદાવલિ_List(pWalker, pWin->pPartition);
          sqlite3_Walk_પદાવલિ_List(pWalker, pWin->pOrderBy);
          sqlite3_Walk_પદાવલિ(pWalker, pWin->pFilter);
          sqlite3WindowLink(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }નહીંતર
#endif /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_પદાવલિ->op = TK_AGG_FUNCTION;
          p_પદાવલિ->op2 = 0;
#ifndef SQLITE_OMIT_WINDOWFUNC
          જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
            sqlite3_Walk_પદાવલિ(pWalker, p_પદાવલિ->y.pWin->pFilter);
          }
#endif
          pNC2 = pNC;
          જ્યારે( pNC2
              && sqlite3ReferencesSrcList(pParse, p_પદાવલિ, pNC2->pSrcList)==0
          ){
            p_પદાવલિ->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->pNext;
          }
          પ્રતિજ્ઞા( pDef!=0 || IN_RENAME_OBJECT );
          જો( pNC2 && pDef ){
            p_પદાવલિ->op2 += pNC2->nNestedSelect;
            પ્રતિજ્ઞા( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            પ્રતિજ્ઞા( SQLITE_FUNC_ANYORDER==NC_OrderAgg );
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
      પાછા WRC_Prune;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    સ્થિતિ TK_EXISTS:
    સ્થિતિ TK_SELECT:
#endif
    સ્થિતિ TK_IN: {
      testcase( p_પદાવલિ->op==TK_IN );
      testcase( p_પદાવલિ->op==TK_EXISTS );
      testcase( p_પદાવલિ->op==TK_SELECT );
      જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
        પૂર્ણાંક nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_પદાવલિ );
        testcase( pNC->ncFlags & NC_GenCol );
        પ્રતિજ્ઞા( p_પદાવલિ->x.pSelect );
        જો( p_પદાવલિ->op==TK_EXISTS )  pParse->bHasExists = 1;
        જો( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_પદાવલિ, p_પદાવલિ);
        }નહીંતર{
          sqlite3WalkSelect(pWalker, p_પદાવલિ->x.pSelect);
        }
        પ્રતિજ્ઞા( pNC->nRef>=nRef );
        જો( nRef!=pNC->nRef ){
          પદાવલિ_Set_Property(p_પદાવલિ, EP_VarSelect);
          p_પદાવલિ->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      તોડો;
    }
    સ્થિતિ TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_પદાવલિ );
      testcase( pNC->ncFlags & NC_GenCol );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_પદાવલિ|NC_GenCol, p_પદાવલિ, p_પદાવલિ);
      તોડો;
    }
    સ્થિતિ TK_IS:
    સ્થિતિ TK_ISNOT: {
      પદાવલિ *p_જમણું = sqlite3_પદાવલિ_Skip_Collate_And_Likely(p_પદાવલિ->p_જમણું);
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      જો( ALWAYS(p_જમણું) && (p_જમણું->op==TK_ID || p_જમણું->op==TK_TRUEFALSE) ){
        પૂર્ણાંક rc = resolve_પદાવલિ_Step(pWalker, p_જમણું);
        જો( rc==WRC_Abort ) પાછા WRC_Abort;
        જો( p_જમણું->op==TK_TRUEFALSE ){
          p_પદાવલિ->op2 = p_પદાવલિ->op;
          p_પદાવલિ->op = TK_TRUTH;
          પાછા WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    સ્થિતિ TK_BETWEEN:
    સ્થિતિ TK_EQ:
    સ્થિતિ TK_NE:
    સ્થિતિ TK_LT:
    સ્થિતિ TK_LE:
    સ્થિતિ TK_GT:
    સ્થિતિ TK_GE: {
      પૂર્ણાંક n_ડાબું, n_જમણું;
      જો( pParse->db->mallocFailed ) તોડો;
      પ્રતિજ્ઞા( p_પદાવલિ->p_ડાબું!=0 );
      n_ડાબું = sqlite3_પદાવલિ_Vector_Size(p_પદાવલિ->p_ડાબું);
      જો( p_પદાવલિ->op==TK_BETWEEN ){
        પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
        n_જમણું = sqlite3_પદાવલિ_Vector_Size(p_પદાવલિ->x.pList->a[0].p_પદાવલિ);
        જો( n_જમણું==n_ડાબું ){
          n_જમણું = sqlite3_પદાવલિ_Vector_Size(p_પદાવલિ->x.pList->a[1].p_પદાવલિ);
        }
      }નહીંતર{
        પ્રતિજ્ઞા( p_પદાવલિ->p_જમણું!=0 );
        n_જમણું = sqlite3_પદાવલિ_Vector_Size(p_પદાવલિ->p_જમણું);
      }
      જો( n_ડાબું!=n_જમણું ){
        testcase( p_પદાવલિ->op==TK_EQ );
        testcase( p_પદાવલિ->op==TK_NE );
        testcase( p_પદાવલિ->op==TK_LT );
        testcase( p_પદાવલિ->op==TK_LE );
        testcase( p_પદાવલિ->op==TK_GT );
        testcase( p_પદાવલિ->op==TK_GE );
        testcase( p_પદાવલિ->op==TK_IS );
        testcase( p_પદાવલિ->op==TK_ISNOT );
        testcase( p_પદાવલિ->op==TK_BETWEEN );
        sqlite3_Error_સંદેશ(pParse, "row value misused");
        sqlite3_Record_Error_Offset_Of_પદાવલિ(pParse->db, p_પદાવલિ);
      }
      તોડો;
    }
  }
  પ્રતિજ્ઞા( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  પાછા pParse->nErr ? WRC_Abort : WRC_Continue;
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
સ્થિર પૂર્ણાંક resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  પદાવલિ_List *pEList,  /* List of expressions to scan */
  પદાવલિ *pE           /* Expression we are trying to match */
){
  પૂર્ણાંક i;             /* Loop counter */

  UNUSED_PARAMETER(pParse);

  જો( pE->op==TK_ID ){
    અચલ અક્ષર *zCol;
    પ્રતિજ્ઞા( !પદાવલિ_Has_Property(pE, EP_Int_કિંમત) );
    zCol = pE->u.zToken;
    માટે(i=0; i<pEList->n_પદાવલિ; i++){
      જો( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, zCol)==0
      ){
        પાછા i+1;
      }
    }
  }
  પાછા 0;
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
સ્થિર પૂર્ણાંક resolve_Order_By_Term_To_પદાવલિ_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  પદાવલિ *pE           /* The specific ORDER BY term */
){
  પૂર્ણાંક i;             /* Loop counter */
  પદાવલિ_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  પૂર્ણાંક rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  પ્રતિજ્ઞા( sqlite3_પદાવલિ_Is_Integer(pE, &i, 0, 1)==0 );
  pEList = pSelect->pEList;

  /* Resolve all names in the ORDER BY term expression
  */
  memset(&nc, 0, કદ(nc));
  nc.pParse = pParse;
  nc.pSrcList = pSelect->pSrc;
  nc.uNC.pEList = pEList;
  nc.ncFlags = NC_AllowAgg|NC_UEList|NC_NoSelect;
  nc.nNcErr = 0;
  db = pParse->db;
  savedSuppErr = db->suppressErr;
  db->suppressErr = 1;
  rc = sqlite3_Resolve_પદાવલિ_Names(&nc, pE);
  db->suppressErr = savedSuppErr;
  જો( rc ) પાછા 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  માટે(i=0; i<pEList->n_પદાવલિ; i++){
    જો( sqlite3_પદાવલિ_Compare(0, pEList->a[i].p_પદાવલિ, pE, -1)<2 ){
      પાછા i+1;
    }
  }

  /* If no match, return 0. */
  પાછા 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
સ્થિર શૂન્ય resolveOutOfRangeError(
  Parse *pParse,         /* The error context into which to write the error */
  અચલ અક્ષર *zType,     /* "ORDER" or "GROUP" */
  પૂર્ણાંક i,                 /* The index (1-based) of the term out of range */
  પૂર્ણાંક mx,                /* Largest permissible value of i */
  પદાવલિ *pError           /* Associate the error with the expression */
){
  sqlite3_Error_સંદેશ(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_Record_Error_Offset_Of_પદાવલિ(pParse->db, pError);
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
સ્થિર પૂર્ણાંક resolveCompoundOrderBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  પૂર્ણાંક i;
  પદાવલિ_List *pOrderBy;
  પદાવલિ_List *pEList;
  sqlite3 *db;
  પૂર્ણાંક moreToDo = 1;

  pOrderBy = pSelect->pOrderBy;
  જો( pOrderBy==0 ) પાછા 0;
  db = pParse->db;
  જો( pOrderBy->n_પદાવલિ>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_સંદેશ(pParse, "too many terms in ORDER BY clause");
    પાછા 1;
  }
  માટે(i=0; i<pOrderBy->n_પદાવલિ; i++){
    pOrderBy->a[i].fg.done = 0;
  }
  pSelect->pNext = 0;
  જ્યારે( pSelect->pPrior ){
    pSelect->pPrior->pNext = pSelect;
    pSelect = pSelect->pPrior;
  }
  જ્યારે( pSelect && moreToDo ){
    સંરચના પદાવલિ_List_item *pItem;
    moreToDo = 0;
    pEList = pSelect->pEList;
    પ્રતિજ્ઞા( pEList!=0 );
    માટે(i=0, pItem=pOrderBy->a; i<pOrderBy->n_પદાવલિ; i++, pItem++){
      પૂર્ણાંક iCol = -1;
      પદાવલિ *pE, *pDup;
      જો( pItem->fg.done ) ચાલુ;
      pE = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pItem->p_પદાવલિ);
      જો( NEVER(pE==0) ) ચાલુ;
      જો( sqlite3_પદાવલિ_Is_Integer(pE, &iCol, 0, 1) ){
        જો( iCol<=0 || iCol>pEList->n_પદાવલિ ){
          resolveOutOfRangeError(pParse, "ORDER", i+1, pEList->n_પદાવલિ, pE);
          પાછા 1;
        }
      }નહીંતર{
        iCol = resolveAsName(pParse, pEList, pE);
        જો( iCol==0 ){
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
          pDup = sqlite3_પદાવલિ_Dup(db, pE, 0);
          જો( !db->mallocFailed ){
            પ્રતિજ્ઞા(pDup);
            iCol = resolve_Order_By_Term_To_પદાવલિ_List(pParse, pSelect, pDup);
            જો( IN_RENAME_OBJECT && iCol>0 ){
              resolve_Order_By_Term_To_પદાવલિ_List(pParse, pSelect, pE);
            }
          }
          sqlite3_પદાવલિ_Delete(db, pDup);
        }
      }
      જો( iCol>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        જો( !IN_RENAME_OBJECT ){
          પદાવલિ *pNew = sqlite3_પદાવલિ_Int32(db, iCol);
          જો( pNew==0 ) પાછા 1;
          જો( pItem->p_પદાવલિ==pE ){
            pItem->p_પદાવલિ = pNew;
          }નહીંતર{
            પદાવલિ *pParent = pItem->p_પદાવલિ;
            પ્રતિજ્ઞા( pParent->op==TK_COLLATE );
            જ્યારે( pParent->p_ડાબું->op==TK_COLLATE ) pParent = pParent->p_ડાબું;
            પ્રતિજ્ઞા( pParent->p_ડાબું==pE );
            pParent->p_ડાબું = pNew;
          }
          sqlite3_પદાવલિ_Delete(db, pE);
          pItem->u.x.iOrderByCol = (u16)iCol;
        }
        pItem->fg.done = 1;
      }નહીંતર{
        moreToDo = 1;
      }
    }
    pSelect = pSelect->pNext;
  }
  માટે(i=0; i<pOrderBy->n_પદાવલિ; i++){
    જો( pOrderBy->a[i].fg.done==0 ){
      sqlite3_Error_સંદેશ(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      પાછા 1;
    }
  }
  પાછા 0;
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
પૂર્ણાંક sqlite3ResolveOrderGroupBy(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  પદાવલિ_List *pOrderBy,   /* The ORDER BY or GROUP BY clause to be processed */
  અચલ અક્ષર *zType     /* "ORDER" or "GROUP" */
){
  પૂર્ણાંક i;
  sqlite3 *db = pParse->db;
  પદાવલિ_List *pEList;
  સંરચના પદાવલિ_List_item *pItem;

  જો( pOrderBy==0 || pParse->db->mallocFailed || IN_RENAME_OBJECT ) પાછા 0;
  જો( pOrderBy->n_પદાવલિ>db->aLimit[SQLITE_LIMIT_COLUMN] ){
    sqlite3_Error_સંદેશ(pParse, "too many terms in %s BY clause", zType);
    પાછા 1;
  }
  pEList = pSelect->pEList;
  પ્રતિજ્ઞા( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  માટે(i=0, pItem=pOrderBy->a; i<pOrderBy->n_પદાવલિ; i++, pItem++){
    જો( pItem->u.x.iOrderByCol ){
      જો( pItem->u.x.iOrderByCol>pEList->n_પદાવલિ ){
        resolveOutOfRangeError(pParse, zType, i+1, pEList->n_પદાવલિ, 0);
        પાછા 1;
      }
      resolveAlias(pParse, pEList, pItem->u.x.iOrderByCol-1, pItem->p_પદાવલિ,0);
    }
  }
  પાછા 0;
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
સ્થિર પૂર્ણાંક resolveRemoveWindowsCb(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  UNUSED_PARAMETER(pWalker);
  જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
    Window *pWin = p_પદાવલિ->y.pWin;
    sqlite3WindowUnlinkFromSelect(pWin);
  }
  પાછા WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
સ્થિર શૂન્ય window_Remove_પદાવલિ_From_Select(Select *pSelect, પદાવલિ *p_પદાવલિ){
  જો( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, કદ(Walker));
    sWalker.x_પદાવલિ_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_પદાવલિ(&sWalker, p_પદાવલિ);
  }
}
#નહીંતર
# define window_Remove_પદાવલિ_From_Select(a, b)
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
સ્થિર પૂર્ણાંક resolveOrderGroupBy(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  પદાવલિ_List *pOrderBy,   /* An ORDER BY or GROUP BY clause to resolve */
  અચલ અક્ષર *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  પૂર્ણાંક i, j;                      /* Loop counters */
  પૂર્ણાંક iCol;                      /* Column number */
  સંરચના પદાવલિ_List_item *pItem;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  પૂર્ણાંક nResult;                   /* Number of terms in the result set */

  પ્રતિજ્ઞા( pOrderBy!=0 );
  nResult = pSelect->pEList->n_પદાવલિ;
  pParse = pNC->pParse;
  માટે(i=0, pItem=pOrderBy->a; i<pOrderBy->n_પદાવલિ; i++, pItem++){
    પદાવલિ *pE = pItem->p_પદાવલિ;
    પદાવલિ *pE2 = sqlite3_પદાવલિ_Skip_Collate_And_Likely(pE);
    જો( NEVER(pE2==0) ) ચાલુ;
    જો( zType[0]!='G' ){
      iCol = resolveAsName(pParse, pSelect->pEList, pE2);
      જો( iCol>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        pItem->u.x.iOrderByCol = (u16)iCol;
        ચાલુ;
      }
    }
    જો( sqlite3_પદાવલિ_Is_Integer(pE2, &iCol, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      જો( iCol<1 || iCol>0xffff ){
        resolveOutOfRangeError(pParse, zType, i+1, nResult, pE2);
        પાછા 1;
      }
      pItem->u.x.iOrderByCol = (u16)iCol;
      ચાલુ;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    pItem->u.x.iOrderByCol = 0;
    જો( sqlite3_Resolve_પદાવલિ_Names(pNC, pE) ){
      પાછા 1;
    }
    માટે(j=0; j<pSelect->pEList->n_પદાવલિ; j++){
      જો( sqlite3_પદાવલિ_Compare(0, pE, pSelect->pEList->a[j].p_પદાવલિ, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        window_Remove_પદાવલિ_From_Select(pSelect, pE);
        pItem->u.x.iOrderByCol = j+1;
      }
    }
  }
  પાછા sqlite3ResolveOrderGroupBy(pParse, pSelect, pOrderBy, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
સ્થિર પૂર્ણાંક resolveSelectStep(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  પૂર્ણાંક isCompound;         /* True if p is a compound select */
  પૂર્ણાંક nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  પૂર્ણાંક i;                  /* Loop counter */
  પદાવલિ_List *pGroupBy;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  પ્રતિજ્ઞા( p!=0 );
  જો( p->selFlags & SF_Resolved ){
    પાછા WRC_Prune;
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
  જો( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    પાછા pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->pPrior!=0;
  nCompound = 0;
  pLeftmost = p;
  જ્યારે( p ){
    પ્રતિજ્ઞા( (p->selFlags & SF_Expanded)!=0 );
    પ્રતિજ્ઞા( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, કદ(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    જો( sqlite3_Resolve_પદાવલિ_Names(&sNC, p->pLimit) ){
      પાછા WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    જો( p->selFlags & SF_Converted ){
      Select *pSub;
      પ્રતિજ્ઞા( p->pSrc->a[0].fg.isSubquery );
      પ્રતિજ્ઞા( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      પ્રતિજ્ઞા( pSub!=0 );
      પ્રતિજ્ઞા( p->pSrc->nSrc==1 && p->pOrderBy );
      પ્રતિજ્ઞા( pSub->pPrior && pSub->pOrderBy==0 );
      pSub->pOrderBy = p->pOrderBy;
      p->pOrderBy = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    જો( pOuterNC ) pOuterNC->nNestedSelect++;
    માટે(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      પ્રતિજ્ઞા( pItem->zName!=0
              || pItem->fg.isSubquery );  /* Test of tag-20240424-1*/
      જો( pItem->fg.isSubquery
       && (pItem->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        પૂર્ણાંક nRef = pOuterNC ? pOuterNC->nRef : 0;
        અચલ અક્ષર *zSavedContext = pParse->zAuthContext;

        જો( pItem->zName ) pParse->zAuthContext = pItem->zName;
        sqlite3ResolveSelectNames(pParse, pItem->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        જો( pParse->nErr ) પાછા WRC_Abort;
        પ્રતિજ્ઞા( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        જો( pOuterNC ){
          પ્રતિજ્ઞા( pItem->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          pItem->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    જો( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.pNext = pOuterNC;
 
    /* Resolve names in the result set. */
    જો( sqlite3_Resolve_પદાવલિ_List_Names(&sNC, p->pEList) ) પાછા WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    પ્રતિજ્ઞા( (p->selFlags & SF_Aggregate)==0 );
    pGroupBy = p->pGroupBy;
    જો( pGroupBy || (sNC.ncFlags & NC_HasAgg)!=0 ){
      પ્રતિજ્ઞા( NC_MinMaxAgg==SF_MinMaxAgg );
      પ્રતિજ્ઞા( NC_OrderAgg==SF_OrderByReqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_OrderAgg));
    }નહીંતર{
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
    પ્રતિજ્ઞા( (sNC.ncFlags & (NC_UAggInfo|NC_UUpsert|NC_UBaseReg))==0 );
    sNC.uNC.pEList = p->pEList;
    sNC.ncFlags |= NC_UEList;
    જો( p->pHaving ){
      જો( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_સંદેશ(pParse, "HAVING clause on a non-aggregate query");
        પાછા WRC_Abort;
      }
      જો( sqlite3_Resolve_પદાવલિ_Names(&sNC, p->pHaving) ) પાછા WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    જો( sqlite3_Resolve_પદાવલિ_Names(&sNC, p->pWhere) ) પાછા WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    માટે(i=0; i<p->pSrc->nSrc; i++){
      SrcItem *pItem = &p->pSrc->a[i];
      જો( pItem->fg.isTabFunc
       && sqlite3_Resolve_પદાવલિ_List_Names(&sNC, pItem->u1.pFuncArg)
      ){
        પાછા WRC_Abort;
      }
    }

#ifndef SQLITE_OMIT_WINDOWFUNC
    જો( IN_RENAME_OBJECT ){
      Window *pWin;
      માટે(pWin=p->pWinDefn; pWin; pWin=pWin->pNextWin){
        જો( sqlite3_Resolve_પદાવલિ_List_Names(&sNC, pWin->pOrderBy)
         || sqlite3_Resolve_પદાવલિ_List_Names(&sNC, pWin->pPartition)
        ){
          પાછા WRC_Abort;
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
    જો( p->selFlags & SF_Converted ){
      Select *pSub;
      પ્રતિજ્ઞા( p->pSrc->a[0].fg.isSubquery );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      પ્રતિજ્ઞા( pSub!=0 );
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
    જો( p->pOrderBy!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolveOrderGroupBy(&sNC, p, p->pOrderBy, "ORDER")
    ){
      પાછા WRC_Abort;
    }
    જો( db->mallocFailed ){
      પાછા WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    જો( pGroupBy ){
      સંરચના પદાવલિ_List_item *pItem;
   
      જો( resolveOrderGroupBy(&sNC, p, pGroupBy, "GROUP") || db->mallocFailed ){
        પાછા WRC_Abort;
      }
      માટે(i=0, pItem=pGroupBy->a; i<pGroupBy->n_પદાવલિ; i++, pItem++){
        જો( પદાવલિ_Has_Property(pItem->p_પદાવલિ, EP_Agg) ){
          sqlite3_Error_સંદેશ(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          પાછા WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    જો( p->pNext && p->pEList->n_પદાવલિ!=p->pNext->pEList->n_પદાવલિ ){
      sqlite3_Select_Wrong_અંક_Terms_Error(pParse, p->pNext);
      પાછા WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    જો( (p->selFlags & SF_OnToWhere) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      જો( pParse->nErr ) પાછા WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->pPrior;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  જો( isCompound && resolveCompoundOrderBy(pParse, pLeftmost) ){
    પાછા WRC_Abort;
  }

  પાછા WRC_Prune;
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
પૂર્ણાંક sqlite3_Resolve_પદાવલિ_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  પદાવલિ *p_પદાવલિ             /* The expression to be analyzed. */
){
  પૂર્ણાંક savedHasAgg;
  Walker w;

  જો( p_પદાવલિ==0 ) પાછા SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  w.pParse = pNC->pParse;
  w.x_પદાવલિ_Callback = resolve_પદાવલિ_Step;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
  w.pParse->nHeight += p_પદાવલિ->nHeight;
  જો( sqlite3_પદાવલિ_Check_Height(w.pParse, w.pParse->nHeight) ){
    પાછા SQLITE_ERROR;
  }
#endif
  પ્રતિજ્ઞા( p_પદાવલિ!=0 );
  sqlite3_Walk_પદાવલિ_NN(&w, p_પદાવલિ);
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
  w.pParse->nHeight -= p_પદાવલિ->nHeight;
#endif
  પ્રતિજ્ઞા( EP_Agg==NC_HasAgg );
  પ્રતિજ્ઞા( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  પદાવલિ_Set_Property(p_પદાવલિ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  પાછા pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
પૂર્ણાંક sqlite3_Resolve_પદાવલિ_List_Names(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  પદાવલિ_List *pList         /* The expression list to be analyzed. */
){
  પૂર્ણાંક i;
  પૂર્ણાંક savedHasAgg = 0;
  Walker w;
  જો( pList==0 ) પાછા SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_પદાવલિ_Callback = resolve_પદાવલિ_Step;
  w.xSelectCallback = resolveSelectStep;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
  માટે(i=0; i<pList->n_પદાવલિ; i++){
    પદાવલિ *p_પદાવલિ = pList->a[i].p_પદાવલિ;
    જો( p_પદાવલિ==0 ) ચાલુ;
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
    w.pParse->nHeight += p_પદાવલિ->nHeight;
    જો( sqlite3_પદાવલિ_Check_Height(w.pParse, w.pParse->nHeight) ){
      પાછા SQLITE_ERROR;
    }
#endif
    sqlite3_Walk_પદાવલિ_NN(&w, p_પદાવલિ);
#જો SQLITE_MAX_પદાવલિ_DEPTH>0
    w.pParse->nHeight -= p_પદાવલિ->nHeight;
#endif
    પ્રતિજ્ઞા( EP_Agg==NC_HasAgg );
    પ્રતિજ્ઞા( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    જો( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg) ){
      પદાવલિ_Set_Property(p_પદાવલિ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_OrderAgg);
    }
    જો( w.pParse->nErr>0 ) પાછા SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  પાછા SQLITE_OK;
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
શૂન્ય sqlite3ResolveSelectNames(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  પ્રતિજ્ઞા( p!=0 );
  w.x_પદાવલિ_Callback = resolve_પદાવલિ_Step;
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
પૂર્ણાંક sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  Table *pTab,     /* The table being referenced, or NULL */
  પૂર્ણાંક type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  પદાવલિ *p_પદાવલિ,     /* Expression to resolve.  May be NULL. */
  પદાવલિ_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  પૂર્ણાંક rc;
  સંઘ {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  પ્રતિજ્ઞા( type==0 || pTab!=0 );
  પ્રતિજ્ઞા( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_પદાવલિ
          || type==NC_GenCol || pTab==0 );
  memset(&sNC, 0, કદ(sNC));
  memset(&uSrc, 0, કદ(uSrc));
  pSrc = &uSrc.sSrc;
  જો( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    જો( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  જો( (rc = sqlite3_Resolve_પદાવલિ_Names(&sNC, p_પદાવલિ))!=SQLITE_OK ) પાછા rc;
  જો( pList ) rc = sqlite3_Resolve_પદાવલિ_List_Names(&sNC, pList);
  પાછા rc;
}
