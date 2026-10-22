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
#تعریف_کړه EXCLUDED_جدول_شمېره  2

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
ساكن صحيح incr_Agg_ژوروالی(Walker *pWalker, بیان *p_بیان){
  إذا( p_بیان->op==TK_AGG_FUNCTION ) p_بیان->op2 += pWalker->u.n;
  أرجع WRC_Continue;
}
ساكن فراغ incr_Agg_Function_ژوروالی(بیان *p_بیان, صحيح N){
  إذا( N>0 ){
    Walker w;
    memset(&w, 0, حجم(w));
    w.x_بیان_Callback = incr_Agg_ژوروالی;
    w.u.n = N;
    sqlite3_Walk_بیان(&w, p_بیان);
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
  بیان_List *pEList,      /* A result set */
  صحيح i_ستون,              /* A column in the result set.  0..pEList->nExpr-1 */
  بیان *p_بیان,           /* Transform this into an alias to the result set */
  صحيح nSubquery          /* Number of subqueries that the label is moving */
){
  بیان *pOrig;           /* The iCol-th column of the result set */
  بیان *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  أكد( i_ستون>=0 && i_ستون<pEList->n_بیان );
  pOrig = pEList->a[i_ستون].p_بیان;
  أكد( pOrig!=0 );
  أكد( !بیان_Has_Property(p_بیان, EP_Reduced|EP_TokenOnly) );
  إذا( p_بیان->pAggInfo ) أرجع;
  db = pParse->db;
  pDup = sqlite3_بیان_Dup(db, pOrig, 0);
  إذا( db->mallocFailed ){
    sqlite3_بیان_Delete(db, pDup);
    pDup = 0;
  }وإلا{
    بیان لنډمهاله;
    incr_Agg_Function_ژوروالی(pDup, nSubquery);
    إذا( p_بیان->op==TK_COLLATE ){
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      pDup = sqlite3_بیان_جمع_کړه_Collate_String(pParse, pDup, p_بیان->u.zToken);
    }
    حافظه_نقل(&لنډمهاله, pDup, حجم(بیان));
    حافظه_نقل(pDup, p_بیان, حجم(بیان));
    حافظه_نقل(p_بیان, &لنډمهاله, حجم(بیان));
    إذا( بیان_Has_Property(p_بیان, EP_WinFunc) ){
      إذا( ALWAYS(p_بیان->y.pWin!=0) ){
        p_بیان->y.pWin->pOwner = p_بیان;
      }
    }
    sqlite3_بیان_Deferred_Delete(pParse, pDup);
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
صحيح sqlite3_سمون_EName(
  ثابت بنية بیان_List_توکی *p_توکی,
  ثابت حرف *z_ستون,
  ثابت حرف *zTab,
  ثابت حرف *zDb,
  صحيح *pbRowid
){
  صحيح n;
  ثابت حرف *z_پراختیا;
  صحيح eEName = p_توکی->fg.eEName;
  إذا( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    أرجع 0;
  }
  أكد( pbRowid==0 || *pbRowid==0 );
  z_پراختیا = p_توکی->zEName;
  لكل(n=0; ALWAYS(z_پراختیا[n]) && z_پراختیا[n]!='.'; n++){}
  إذا( zDb && (sqlite3StrNICmp(z_پراختیا, zDb, n)!=0 || zDb[n]!=0) ){
    أرجع 0;
  }
  z_پراختیا += n+1;
  لكل(n=0; ALWAYS(z_پراختیا[n]) && z_پراختیا[n]!='.'; n++){}
  إذا( zTab && (sqlite3StrNICmp(z_پراختیا, zTab, n)!=0 || zTab[n]!=0) ){
    أرجع 0;
  }
  z_پراختیا += n+1;
  إذا( z_ستون ){
    إذا( eEName==ENAME_TAB && sqlite3StrICmp(z_پراختیا, z_ستون)!=0 ) أرجع 0;
    إذا( eEName==ENAME_ROWID && sqlite3IsRowid(z_ستون)==0 ) أرجع 0;
  }
  إذا( eEName==ENAME_ROWID ) *pbRowid = 1;
  أرجع 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
ساكن صحيح are_Double_Quoted_مزي_ګانې_Enabled(sqlite3 *db, NameContext *p_سر_NC){
  إذا( db->init.busy ) أرجع 1;  /* Always support for legacy schemas */
  إذا( p_سر_NC->ncFlags & NC_IsDDL ){
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
Bitmask sqlite3_بیان_ستون_کارول_شوی(بیان *p_بیان){
  صحيح n;
  جدول *pExTab;

  n = p_بیان->i_ستنه;
  أكد( بیان_Use_YTab(p_بیان) );
  pExTab = p_بیان->y.pTab;
  أكد( pExTab!=0 );
  أكد( n < pExTab->n_ستون );
  إذا( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->a_ستون[n].ستون_Flags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->n_ستون==BMS-1 );
    testcase( pExTab->n_ستون==BMS );
    أرجع pExTab->n_ستون>=BMS ? ALLBITS : MASKBIT(pExTab->n_ستون)-1;
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
  بیان_List **ppList,      /* ExprList to extend */
  Src_توکی *p_سمون,        /* Source table containing the column */
  i16 i_ستنه             /* The column number */
){
  بیان *pNew = sqlite3_بیان_Alloc(pParse->db, TK_ستنه, 0, 0);
  إذا( pNew ){
    pNew->i_جدول = p_سمون->iCursor;
    pNew->i_ستنه = i_ستنه;
    pNew->y.pTab = p_سمون->pSTab;
    أكد( (p_سمون->fg.jointype & (JT_کیڼ|JT_LTORJ))!=0 );
    بیان_سېټ_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_بیان_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
ساكن SQLITE_NOINLINE صحيح is_Valid_Schema_جدول_Name(
  ثابت حرف *zTab,         /* Name as it appears in the SQL */
  جدول *pTab,              /* The schema table we are trying to match */
  ثابت حرف *zDb           /* non-NULL if a database qualifier is present */
){
  ثابت حرف *zLegacy;
  أكد( pTab!=0 );
  أكد( pTab->tnum==1 );
  إذا( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) أرجع 0;
  zLegacy = pTab->zName;
  إذا( strcmp(zLegacy+7, &LEGACY_لنډمهاله_SCHEMA_جدول[7])==0 ){
    إذا( sqlite3StrICmp(zTab+7, &PREFERRED_لنډمهاله_SCHEMA_جدول[7])==0 ){
      أرجع 1;
    }
    إذا( zDb==0 ) أرجع 0;
    إذا( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_جدول[7])==0 ) أرجع 1;
    إذا( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_جدول[7])==0 ) أرجع 1;
  }وإلا{
    إذا( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_جدول[7])==0 ) أرجع 1;
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
  ثابت بیان *p_ښی,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  بیان *p_بیان          /* Make this EXPR node point to the selected column */
){
  صحيح i, j;                         /* Loop counters */
  صحيح cnt = 0;                      /* Number of matching column names */
  صحيح cntTab = 0;                   /* Number of potential "rowid" matches */
  صحيح nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  Src_توکی *p_توکی;                   /* Use for looping over pSrcList items */
  Src_توکی *p_سمون = 0;              /* The matching pSrcList item */
  NameContext *p_سر_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  صحيح e_New_بیان_Op = TK_ستنه;       /* New value for pExpr->op on success */
  جدول *pTab = 0;                  /* Table holding the row */
  بیان_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  ثابت حرف *z_ستون = p_ښی->u.zToken;

  أكد( pNC );     /* the name context cannot be NULL. */
  أكد( z_ستون );    /* The Z in X.Y.Z cannot be NULL */
  أكد( zDb==0 || zTab!=0 );
  أكد( !بیان_Has_Property(p_بیان, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_بیان->i_جدول = -1;
  بیان_سېټ_VVAProperty(p_بیان, EP_NoReduce);

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
    بیان_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    إذا( pSrcList ){
      لكل(i=0, p_توکی=pSrcList->a; i<pSrcList->nSrc; i++, p_توکی++){
        pTab = p_توکی->pSTab;
        أكد( pTab!=0 && pTab->zName!=0 );
        أكد( pTab->n_ستون>0 || pParse->nErr );
        أكد( (صحيح)p_توکی->fg.isNestedFrom == IsNestedFrom(p_توکی));
        إذا( p_توکی->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          صحيح موندنه = 0;
          Select *pSel;
          أكد( p_توکی->fg.isSubquery );
          أكد( p_توکی->u4.pSubq!=0 );
          pSel = p_توکی->u4.pSubq->pSelect;
          أكد( pSel!=0 );
          pEList = pSel->pEList;
          أكد( pEList!=0 );
          أكد( pEList->n_بیان==pTab->n_ستون );
          لكل(j=0; j<pEList->n_بیان; j++){
            صحيح bRowid = 0;       /* True if possible rowid match */
            إذا( !sqlite3_سمون_EName(&pEList->a[j], z_ستون, zTab, zDb, &bRowid) ){
              استمر;
            }
            إذا( bRowid==0 ){
              إذا( cnt>0 ){
                إذا( p_توکی->fg.isUsing==0
                 || sqlite3_Id_List_شاخص(p_توکی->u3.pUsing, z_ستون)<0
                 || p_سمون==p_توکی
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_بیان_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }وإلا
                إذا( (p_توکی->fg.jointype & JT_ښی)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  استمر;
                }وإلا
                إذا( (p_توکی->fg.jointype & JT_کیڼ)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_بیان_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }وإلا{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, p_سمون, p_بیان->i_ستنه);
                }
              }
              cnt++;
              موندنه = 1;
            }وإلا إذا( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              استمر;
            }
            cntTab++;
            p_سمون = p_توکی;
            p_بیان->i_ستنه = j;
            pEList->a[j].fg.b_کارول_شوی = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            أكد( bRowid==0 || pEList->a[j].fg.b_Using_غړی==0 );
            إذا( pEList->a[j].fg.b_Using_غړی ) اكسر;
          }
          إذا( موندنه || zTab==0 ) استمر;
        }
        أكد( zDb==0 || zTab!=0 );
        إذا( zTab ){
          إذا( zDb ){
            إذا( pTab->pSchema!=pSchema ) استمر;
            إذا( pSchema==0 && strcmp(zDb,"*")!=0 ) استمر;
          }
          إذا( p_توکی->zAlias!=0 ){
            إذا( sqlite3StrICmp(zTab, p_توکی->zAlias)!=0 ){
              استمر;
            }
          }وإلا إذا( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            إذا( pTab->tnum!=1 ) استمر;
            إذا( !is_Valid_Schema_جدول_Name(zTab, pTab, zDb) ) استمر;
          }
          أكد( بیان_Use_YTab(p_بیان) );
          إذا( دننه_RENAME_OBJECT && p_توکی->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (فراغ*)&p_بیان->y.pTab);
          }
        }
        j = sqlite3_ستنه_شاخص(pTab, z_ستون);
        إذا( j>=0 ){
          إذا( cnt>0 ){
            إذا( p_توکی->fg.isUsing==0
             || sqlite3_Id_List_شاخص(p_توکی->u3.pUsing, z_ستون)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_بیان_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }وإلا
            إذا( (p_توکی->fg.jointype & JT_ښی)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              استمر;
            }وإلا
            إذا( (p_توکی->fg.jointype & JT_کیڼ)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_بیان_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }وإلا{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, p_سمون, p_بیان->i_ستنه);
            }
          }
          cnt++;
          p_سمون = p_توکی;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_بیان->i_ستنه = j==pTab->iPKey ? -1 : (i16)j;
          إذا( p_توکی->fg.isNestedFrom ){
            sqlite3_Src_توکی_ستنه_کارول_شوی(p_توکی, j);
          }
        }
        إذا( 0==cnt && VisibleRowid(pTab) ){
          /* pTab is a potential ROWID match.  Keep track of it and match
          ** the ROWID later if that seems appropriate.  (Search for "cntTab"
          ** to find related code.)  Only allow a ROWID match if there is
          ** a single ROWID match candidate.
          */
#که_تعریف_شوی SQLITE_ALLOW_ROWID_دننه_VIEW
          /* In SQLITE_ALLOW_ROWID_IN_VIEW mode, allow a ROWID match
          ** if there is a single VIEW candidate or if there is a single
          ** non-VIEW candidate plus multiple VIEW candidates.  In other
          ** words non-VIEW candidate terms take precedence over VIEWs.
          */
          إذا( cntTab==0
           || (cntTab==1
               && p_سمون!=0
               && ALWAYS(p_سمون->pSTab!=0)
               && (p_سمون->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            p_سمون = p_توکی;
          }وإلا{
            cntTab++;
          }
#وإلا
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          p_سمون = p_توکی;
#د_که_پای
        }
      }
      إذا( p_سمون ){
        p_بیان->i_جدول = p_سمون->iCursor;
        أكد( بیان_Use_YTab(p_بیان) );
        p_بیان->y.pTab = p_سمون->pSTab;
        إذا( (p_سمون->fg.jointype & (JT_کیڼ|JT_LTORJ))!=0 ){
          بیان_سېټ_Property(p_بیان, EP_CanBeNull);
        }
        pSchema = p_بیان->y.pTab->pSchema;
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
#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
      إذا( pParse->pTriggerTab!=0 ){
        صحيح op = pParse->eTriggerOp;
        أكد( op==TK_DELETE || op==TK_UPDATE || op==TK_ورننباسه );
        إذا( pParse->bReturning ){
          إذا( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || is_Valid_Schema_جدول_Name(zTab, pParse->pTriggerTab, 0))
          ){
            p_بیان->i_جدول = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }وإلا إذا( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_بیان->i_جدول = 1;
          pTab = pParse->pTriggerTab;
        }وإلا إذا( op!=TK_ورننباسه && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_بیان->i_جدول = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#د_که_پای /* SQLITE_OMIT_TRIGGER */
#که_نه_تعریف_شوی SQLITE_OMIT_UPSERT
      إذا( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        إذا( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_بیان->i_جدول = EXCLUDED_جدول_شمېره;
          إذا( دننه_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (فراغ*)&p_بیان->y.pTab);
          }
        }
      }
#د_که_پای /* SQLITE_OMIT_UPSERT */

      إذا( pTab ){
        صحيح i_ستون;
        pSchema = pTab->pSchema;
        cntTab++;
        i_ستون = sqlite3_ستنه_شاخص(pTab, z_ستون);
        إذا( i_ستون>=0 ){
          إذا( pTab->iPKey==i_ستون ) i_ستون = -1;
        }وإلا{
          إذا( sqlite3IsRowid(z_ستون) && VisibleRowid(pTab) ){
            i_ستون = -1;
          }وإلا{
            i_ستون = pTab->n_ستون;
          }
        }
        إذا( i_ستون<pTab->n_ستون ){
          cnt++;
          p_سمون = 0;
#که_نه_تعریف_شوی SQLITE_OMIT_UPSERT
          إذا( p_بیان->i_جدول==EXCLUDED_جدول_شمېره ){
            testcase( i_ستون==(-1) );
            أكد( بیان_Use_YTab(p_بیان) );
            إذا( دننه_RENAME_OBJECT ){
              p_بیان->i_ستنه = i_ستون;
              p_بیان->y.pTab = pTab;
              e_New_بیان_Op = TK_ستنه;
            }وإلا{
              p_بیان->i_جدول = pNC->uNC.pUpsert->reg_معلومات +
                 sqlite3_جدول_ستنه_منزل_Storage(pTab, i_ستون);
              e_New_بیان_Op = TK_ثبتوونکی;
            }
          }وإلا
#د_که_پای /* SQLITE_OMIT_UPSERT */
          {
            أكد( بیان_Use_YTab(p_بیان) );
            p_بیان->y.pTab = pTab;
            إذا( pParse->bReturning ){
              e_New_بیان_Op = TK_ثبتوونکی;
              p_بیان->op2 = TK_ستنه;
              p_بیان->i_ستنه = i_ستون;
              p_بیان->i_جدول = pNC->uNC.i_بنسټ_Reg + (pTab->n_ستون+1)*p_بیان->i_جدول +
                 sqlite3_جدول_ستنه_منزل_Storage(pTab, i_ستون) + 1;
            }وإلا{
              p_بیان->i_ستنه = (i16)i_ستون;
              e_New_بیان_Op = TK_TRIGGER;
#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
              إذا( i_ستون<0 ){
                p_بیان->aff_بیان = SQLITE_AFF_INTEGER;
              }وإلا إذا( p_بیان->i_جدول==0 ){
                testcase( i_ستون==31 );
                testcase( i_ستون==32 );
                pParse->oldmask |= (i_ستون>=32 ? 0xffffffff : (((u32)1)<<i_ستون));
              }وإلا{
                testcase( i_ستون==31 );
                testcase( i_ستون==32 );
                pParse->newmask |= (i_ستون>=32 ? 0xffffffff : (((u32)1)<<i_ستون));
              }
#د_که_پای /* SQLITE_OMIT_TRIGGER */
            }
          }
        }
      }
    }
#د_که_پای /* !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT) */

    /*
    ** Perhaps the name is a reference to the ROWID
    */
    إذا( cnt==0
     && cntTab>=1
     && p_سمون
     && (pNC->ncFlags & (NC_Idx_بیان|NC_Gen_ستون))==0
     && sqlite3IsRowid(z_ستون)
     && ALWAYS(VisibleRowid(p_سمون->pSTab) || p_سمون->fg.isNestedFrom)
    ){
      cnt = cntTab;
#إذا SQLITE_ALLOW_ROWID_دننه_VIEW+0==2
      إذا( p_سمون->pSTab!=0 && IsView(p_سمون->pSTab) ){
        e_New_بیان_Op = TK_NULL;
      }
#د_که_پای
      إذا( p_سمون->fg.isNestedFrom==0 ) p_بیان->i_ستنه = -1;
      p_بیان->aff_بیان = SQLITE_AFF_INTEGER;
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
      لكل(j=0; j<pEList->n_بیان; j++){
        حرف *zAs = pEList->a[j].zEName;
        إذا( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, z_ستون)==0
        ){
          بیان *pOrig;
          أكد( p_بیان->p_کیڼ==0 && p_بیان->p_ښی==0 );
          أكد( بیان_Use_XList(p_بیان)==0 || p_بیان->x.pList==0 );
          أكد( بیان_Use_XSelect(p_بیان)==0 || p_بیان->x.pSelect==0 );
          pOrig = pEList->a[j].p_بیان;
          إذا( (pNC->ncFlags&NC_AllowAgg)==0 && بیان_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_پیغام(pParse, "misuse of aliased aggregate %s", zAs);
            أرجع WRC_Abort;
          }
          إذا( بیان_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_سر_NC )
          ){
            sqlite3_Error_پیغام(pParse, "misuse of aliased window function %s",zAs);
            أرجع WRC_Abort;
          }
          إذا( sqlite3_بیان_ویکټور_اندازه(pOrig)!=1 ){
            sqlite3_Error_پیغام(pParse, "row value misused");
            أرجع WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_بیان, nSubquery);
          cnt = 1;
          p_سمون = 0;
          أكد( zTab==0 && zDb==0 );
          إذا( دننه_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (فراغ*)p_بیان);
          }
          اذهب lookupname_پای_ټکی;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    إذا( cnt ) اكسر;
    pNC = pNC->p_بل;
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
    أكد( p_بیان->op==TK_ID );
    إذا( بیان_Has_Property(p_بیان,EP_DblQuoted)
     && are_Double_Quoted_مزي_ګانې_Enabled(db, p_سر_NC)
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
        "double-quoted string literal: \"%w\"", z_ستون);
#که_تعریف_شوی SQLITE_ENABLE_NORMALIZE
      sqlite3_Vdbe_جمع_کړه_Dblquote_Str(db, pParse->pVdbe, z_ستون);
#د_که_پای
      p_بیان->op = TK_STRING;
      memset(&p_بیان->y, 0, حجم(p_بیان->y));
      أرجع WRC_Prune;
    }
    إذا( sqlite3_بیان_Id_منزل_True_False(p_بیان) ){
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
  أكد( !بیان_Has_Property(p_بیان, EP_xIsSelect|EP_Int_ارزښت) );
  إذا( cnt!=1 ){
    ثابت حرف *zErr;
    إذا( pFJMatch ){
      إذا( pFJMatch->n_بیان==cnt-1 ){
        إذا( بیان_Has_Property(p_بیان,EP_ونې_پاڼه) ){
          بیان_Clear_Property(p_بیان,EP_ونې_پاڼه);
        }وإلا{
          sqlite3_بیان_Delete(db, p_بیان->p_کیڼ);
          p_بیان->p_کیڼ = 0;
          sqlite3_بیان_Delete(db, p_بیان->p_ښی);
          p_بیان->p_ښی = 0;
        }
        extendFJMatch(pParse, &pFJMatch, p_سمون, p_بیان->i_ستنه);
        p_بیان->op = TK_FUNCTION;
        p_بیان->u.zToken = "coalesce";
        p_بیان->x.pList = pFJMatch;
        p_بیان->aff_بیان = SQLITE_AFF_DEFER;
        cnt = 1;
        اذهب lookupname_پای_ټکی;
      }وإلا{
        sqlite3_بیان_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    إذا( zDb ){
      sqlite3_Error_پیغام(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, z_ستون);
    }وإلا إذا( zTab ){
      sqlite3_Error_پیغام(pParse, "%s: %s.%s", zErr, zTab, z_ستون);
    }وإلا إذا( cnt==0 && بیان_Has_Property(p_ښی,EP_DblQuoted) ){
      sqlite3_Error_پیغام(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, z_ستون);
    }وإلا{
      sqlite3_Error_پیغام(pParse, "%s: %s", zErr, z_ستون);
    }
    sqlite3_ریکارډ_Error_دننه_واټن_د_بیان(pParse->db, p_بیان);
    pParse->checkSchema = 1;
    p_سر_NC->nNcErr++;
    e_New_بیان_Op = TK_NULL;
  }
  أكد( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  إذا( !بیان_Has_Property(p_بیان,(EP_TokenOnly|EP_ونې_پاڼه)) ){
    sqlite3_بیان_Delete(db, p_بیان->p_کیڼ);
    p_بیان->p_کیڼ = 0;
    sqlite3_بیان_Delete(db, p_بیان->p_ښی);
    p_بیان->p_ښی = 0;
    بیان_سېټ_Property(p_بیان, EP_ونې_پاڼه);
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
  إذا( p_سمون ){
    إذا( p_بیان->i_ستنه>=0 ){
      p_سمون->ستون_کارول_شوی |= sqlite3_بیان_ستون_کارول_شوی(p_بیان);
    }وإلا{
      p_سمون->fg.rowid_کارول_شوی = 1;
    }
  }

  p_بیان->op = e_New_بیان_Op;
lookupname_پای_ټکی:
  إذا( cnt==1 ){
    أكد( pNC!=0 );
#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
    إذا( db->xAuth ){
      إذا( pFJMatch ){
        أكد( p_بیان->op==TK_FUNCTION );
        أكد( sqlite3_stricmp(p_بیان->u.zToken,"coalesce")==0 );
        أكد( p_بیان->x.pList==pFJMatch );
        أكد( pFJMatch->n_بیان>0 );
        لكل(i=0; i<pFJMatch->n_بیان; i++){
          أكد( pFJMatch->a[i].p_بیان->op==TK_ستنه );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_بیان,pSchema,pNC->pSrcList);
        }
      }وإلا إذا( p_بیان->op==TK_ستنه || p_بیان->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_بیان, pSchema, pNC->pSrcList);
      }
    } 
#د_که_پای
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    لكل(;;){
      أكد( p_سر_NC!=0 );
      p_سر_NC->nRef++;
      إذا( p_سر_NC==pNC ) اكسر;
      p_سر_NC = p_سر_NC->p_بل;
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
بیان *sqlite3_Create_ستنه_بیان(sqlite3 *db, SrcList *pSrc, صحيح iSrc, صحيح i_ستون){
  بیان *p = sqlite3_بیان_Alloc(db, TK_ستنه, 0, 0);
  إذا( p ){
    Src_توکی *p_توکی = &pSrc->a[iSrc];
    جدول *pTab;
    أكد( بیان_Use_YTab(p) );
    pTab = p->y.pTab = p_توکی->pSTab;
    p->i_جدول = p_توکی->iCursor;
    إذا( p->y.pTab->iPKey==i_ستون ){
      p->i_ستنه = -1;
    }وإلا{
      p->i_ستنه = (ynVar)i_ستون;
      إذا( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->a_ستون[i_ستون].ستون_Flags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->n_ستون==63 );
        testcase( pTab->n_ستون==64 );
        p_توکی->ستون_کارول_شوی = pTab->n_ستون>=64 ? ALLBITS : MASKBIT(pTab->n_ستون)-1;
      }وإلا{
        testcase( i_ستون==BMS );
        testcase( i_ستون==BMS-1 );
        p_توکی->ستون_کارول_شوی |= ((Bitmask)1)<<(i_ستون>=BMS ? BMS-1 : i_ستون);
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
   ثابت حرف *z_پیغام,    /* Type of error */
   بیان *p_بیان,         /* Invalidate this expression on error */
   بیان *pError         /* Associate error with this expression */
){
  ثابت حرف *z_دننه = "partial index WHERE clauses";
  إذا( pNC->ncFlags & NC_Idx_بیان )      z_دننه = "index expressions";
#که_نه_تعریف_شوی SQLITE_OMIT_CHECK
  وإلا إذا( pNC->ncFlags & NC_IsCheck ) z_دننه = "CHECK constraints";
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_GENERATED_COLUMNS
  وإلا إذا( pNC->ncFlags & NC_Gen_ستون ) z_دننه = "generated columns";
#د_که_پای
  sqlite3_Error_پیغام(pParse, "%s prohibited in %s", z_پیغام, z_دننه);
  إذا( p_بیان ) p_بیان->op = TK_NULL;
  sqlite3_ریکارډ_Error_دننه_واټن_د_بیان(pParse->db, pError);
}
#تعریف_کړه sqlite3ResolveNotValid(P,N,M,X,E,R) \
  أكد( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_بیان|NC_Gen_ستون))==0 ); \
  إذا( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
ساكن صحيح بیان_Probability(بیان *p){
  مزدوج r = -1.0;
  إذا( p->op!=TK_FLOAT ) أرجع -1;
  أكد( !بیان_Has_Property(p, EP_Int_ارزښت) );
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
ساكن SQLITE_NOINLINE فراغ resolve_سېټ_بیان_Subtype_Arg(بیان_List *pList){
  صحيح nn, ii;
  nn = pList ? pList->n_بیان : 0;
  لكل(ii=0; ii<nn; ii++){
    بیان *p_بیان = pList->a[ii].p_بیان;
    طالما( 1 /*exit-by-break*/ ){
      بیان_سېټ_Property(p_بیان, EP_SubtArg);
      إذا( p_بیان->op==TK_SELECT ){
        أكد( بیان_Use_XSelect(p_بیان) );
        أكد( p_بیان->x.pSelect!=0 );
        resolve_سېټ_بیان_Subtype_Arg(p_بیان->x.pSelect->pEList);
        اكسر;
      }
      إذا( p_بیان->op==TK_UPLUS ){
        p_بیان = p_بیان->p_کیڼ;
        أكد( p_بیان!=0 );
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
ساكن صحيح resolve_بیان_ګام(Walker *pWalker, بیان *p_بیان){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  أكد( pNC!=0 );
  pParse = pNC->pParse;
  أكد( pParse==pWalker->pParse );

#که_نه_تعریف_شوی NDEBUG
  إذا( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    صحيح i;
    لكل(i=0; i<pNC->pSrcList->nSrc; i++){
      أكد( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#د_که_پای
  بدل( p_بیان->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    حالة TK_لیکه: {
      SrcList *pSrcList = pNC->pSrcList;
      Src_توکی *p_توکی;
      أكد( pSrcList && pSrcList->nSrc>=1 );
      p_توکی = pSrcList->a;
      p_بیان->op = TK_ستنه;
      أكد( بیان_Use_YTab(p_بیان) );
      p_بیان->y.pTab = p_توکی->pSTab;
      p_بیان->i_جدول = p_توکی->iCursor;
      p_بیان->i_ستنه--;
      p_بیان->aff_بیان = SQLITE_AFF_INTEGER;
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
      لكل(i=0, p=pNC; p && i<Array_اندازه(anRef); p=p->p_بل, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_بیان(pWalker, p_بیان->p_کیڼ);
      إذا( دننه_RENAME_OBJECT ) أرجع WRC_Prune;
      إذا( sqlite3_بیان_Can_Be_Null(p_بیان->p_کیڼ) ){
        /* The expression can be NULL.  So the optimization does not apply */
        أرجع WRC_Prune;
      }

      لكل(i=0, p=pNC; p; p=p->p_بل, i++){
        إذا( (p->ncFlags & NC_Where)==0 ){
          أرجع WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( بیان_Has_Property(p_بیان, EP_OuterON) );
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
#إذا TREETRACE_ENABLED
      إذا( sqlite3_ونه_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_بیان->op==TK_NOTNULL
        );
        sqlite3_Show_بیان(p_بیان);
      }
#د_که_پای /* TREETRACE_ENABLED */
      p_بیان->u.i_ارزښت = (p_بیان->op==TK_NOTNULL);
      p_بیان->flags |= EP_Int_ارزښت;
      p_بیان->op = TK_INTEGER;
      لكل(i=0, p=pNC; p && i<Array_اندازه(anRef); p=p->p_بل, i++){
        p->nRef = anRef[i];
      }
      sqlite3_بیان_Delete(pParse->db, p_بیان->p_کیڼ);
      p_بیان->p_کیڼ = 0;
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
      ثابت حرف *z_جدول;
      ثابت حرف *zDb;
      بیان *p_ښی;

      إذا( p_بیان->op==TK_ID ){
        zDb = 0;
        z_جدول = 0;
        أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
        p_ښی = p_بیان;
      }وإلا{
        بیان *p_کیڼ = p_بیان->p_کیڼ;
        testcase( pNC->ncFlags & NC_Idx_بیان );
        testcase( pNC->ncFlags & NC_Gen_ستون );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_بیان|NC_Gen_ستون, 0, p_بیان);
        p_ښی = p_بیان->p_ښی;
        إذا( p_ښی->op==TK_ID ){
          zDb = 0;
        }وإلا{
          أكد( p_ښی->op==TK_DOT );
          أكد( !بیان_Has_Property(p_ښی, EP_Int_ارزښت) );
          zDb = p_کیڼ->u.zToken;
          p_کیڼ = p_ښی->p_کیڼ;
          p_ښی = p_ښی->p_ښی;
        }
        أكد( بیان_Use_UToken(p_کیڼ) && بیان_Use_UToken(p_ښی) );
        z_جدول = p_کیڼ->u.zToken;
        أكد( بیان_Use_YTab(p_بیان) );
        إذا( دننه_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (فراغ*)p_بیان, (فراغ*)p_ښی);
          sqlite3RenameTokenRemap(pParse, (فراغ*)&p_بیان->y.pTab, (فراغ*)p_کیڼ);
        }
      }
      أرجع lookupName(pParse, zDb, z_جدول, p_ښی, pNC, p_بیان);
    }

    /* Resolve function names
    */
    حالة TK_FUNCTION: {
      بیان_List *pList;            /* The argument list */
      صحيح n;                      /* Number of arguments */
      صحيح no_such_func = 0;       /* True if no such function exists */
      صحيح wrong_عدد_دلیلونه = 0;     /* True if wrong number of arguments */
      صحيح is_agg = 0;             /* True if is an aggregate function */
      ثابت حرف *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      صحيح savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
      کړکۍ *pWin = (Is_کړکۍ_Func(p_بیان) ? p_بیان->y.pWin : 0);
#د_که_پای
      أكد( !بیان_Has_Property(p_بیان, EP_xIsSelect|EP_Int_ارزښت) );
      أكد( p_بیان->p_کیڼ==0 || p_بیان->p_کیڼ->op==TK_ترتیب );
      pList = p_بیان->x.pList;
      n = pList ? pList->n_بیان : 0;
      zId = p_بیان->u.zToken;
      pDef = sqlite3_پیدا_کړه_Function(pParse->db, zId, n, enc, 0);
      إذا( pDef==0 ){
        pDef = sqlite3_پیدا_کړه_Function(pParse->db, zId, -2, enc, 0);
        إذا( pDef==0 ){
          no_such_func = 1;
        }وإلا{
          wrong_عدد_دلیلونه = 1;
        }
      }وإلا{
        is_agg = pDef->xFinalize!=0;
        إذا( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          بیان_سېټ_Property(p_بیان, EP_Unlikely);
          إذا( n==2 ){
            p_بیان->i_جدول = بیان_Probability(pList->a[1].p_بیان);
            إذا( p_بیان->i_جدول<0 ){
              sqlite3_Error_پیغام(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_بیان);
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
            p_بیان->i_جدول = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#که_نه_تعریف_شوی SQLITE_OMIT_AUTHORIZATION
        {
          صحيح auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          إذا( auth!=SQLITE_OK ){
            إذا( auth==SQLITE_DENY ){
              sqlite3_Error_پیغام(pParse, "not authorized to use function: %#T",
                                      p_بیان);
              pNC->nNcErr++;
            }
            p_بیان->op = TK_NULL;
            أرجع WRC_Prune;
          }
        }
#د_که_پای

        /* If the function may call sqlite3_value_subtype(), then set the 
        ** EP_SubtArg flag on all of its argument expressions. This prevents
        ** where.c from replacing the expression with a value read from an
        ** index on the same expression, which will not have the correct 
        ** subtype. Also set the flag if the function expression itself is
        ** an EP_SubtArg expression. In this case subtypes are required as 
        ** the function may return a value with a subtype back to its 
        ** caller using sqlite3_result_value().  */
        إذا( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || بیان_Has_Property(p_بیان, EP_SubtArg) 
        ){
          resolve_سېټ_بیان_Subtype_Arg(pList);
        }

        إذا( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          بیان_سېټ_Property(p_بیان,EP_ConstFunc);
        }
        إذا( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_بیان|NC_PartIdx|NC_Gen_ستون, 0, p_بیان);
        }وإلا{
          أكد( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_بیان->op2 = pNC->ncFlags & NC_SelfRef;
        }
        إذا( (pDef->funcFlags & SQLITE_FUNC_دننی)!=0
         && pParse->nested==0
         && (pParse->db->mDbFlags & DBFLAG_دننی_Func)==0
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
        إذا( (pDef->funcFlags & (SQLITE_FUNC_مستقیم|SQLITE_FUNC_UNSAFE))!=0
         && !دننه_RENAME_OBJECT
        ){
          إذا( pNC->ncFlags & NC_FromDDL ) بیان_سېټ_Property(p_بیان, EP_FromDDL);
          sqlite3_بیان_Function_Usable(pParse, p_بیان, pDef);
        }
      }

      إذا( 0==دننه_RENAME_OBJECT ){
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
        أكد( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_ارزښت==0 && pDef->xInverse==0)
          || (pDef->x_ارزښت && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        إذا( pDef && pDef->x_ارزښت==0 && pWin ){
          sqlite3_Error_پیغام(pParse,
              "%#T() may not be used as a window function", p_بیان
          );
          pNC->nNcErr++;
        }وإلا إذا(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_کړکۍ) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          ثابت حرف *zType;
          إذا( (pDef->funcFlags & SQLITE_FUNC_کړکۍ) || pWin ){
            zType = "window";
          }وإلا{
            zType = "aggregate";
          }
          sqlite3_Error_پیغام(pParse, "misuse of %s function %#T()",zType,p_بیان);
          pNC->nNcErr++;
          is_agg = 0;
        }
#وإلا
        إذا( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_پیغام(pParse,"misuse of aggregate function %#T()",p_بیان);
          pNC->nNcErr++;
          is_agg = 0;
        }
#د_که_پای
        وإلا إذا( no_such_func
              && (pParse->db->init.busy==0 ||
                  (no_such_func==2 && pParse->db->init.busy==2))
              /* Suppress "no such function" errors when reading
              ** the sqlite_schema table.  Except, do raise the error
              ** if init.busy is 2, meaning the schema parse is due
              ** to an ALTER TABLE ADD COLUMN statement, and the function
              ** is an internal-use-only function (no_such_func==2). */
#که_تعریف_شوی SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
              && pParse->explain==0
#د_که_پای
        ){
          sqlite3_Error_پیغام(pParse, "no such function: %#T", p_بیان);
          pNC->nNcErr++;
        }وإلا إذا( wrong_عدد_دلیلونه ){
          sqlite3_Error_پیغام(pParse,"wrong number of arguments to function %#T()",
               p_بیان);
          pNC->nNcErr++;
        }
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
        وإلا إذا( is_agg==0 && بیان_Has_Property(p_بیان, EP_WinFunc) ){
          sqlite3_Error_پیغام(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_بیان
          );
          pNC->nNcErr++;
        }
#د_که_پای
        وإلا إذا( is_agg==0 && p_بیان->p_کیڼ ){
          sqlite3_بیان_ترتیب_By_Aggregate_Error(pParse, p_بیان);
          pNC->nNcErr++;
        }
        إذا( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#وإلا
          pNC->ncFlags &= ~NC_AllowAgg;
#د_که_پای
        }
      }
      وإلا إذا( بیان_Has_Property(p_بیان, EP_WinFunc) || p_بیان->p_کیڼ ){
        is_agg = 1;
      }
      sqlite3_Walk_بیان_List(pWalker, pList);
      إذا( is_agg ){
        إذا( p_بیان->p_کیڼ ){
          أكد( p_بیان->p_کیڼ->op==TK_ترتیب );
          أكد( بیان_Use_XList(p_بیان->p_کیڼ) );
          sqlite3_Walk_بیان_List(pWalker, p_بیان->p_کیڼ->x.pList);
        }
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
        إذا( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          أكد( بیان_Use_YWin(p_بیان) && pWin==p_بیان->y.pWin );
          إذا( دننه_RENAME_OBJECT==0 ){
            sqlite3_کړکۍ_Update(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            إذا( pParse->db->mallocFailed ) اكسر;
          }
          sqlite3_Walk_بیان_List(pWalker, pWin->p_برخه_کول);
          sqlite3_Walk_بیان_List(pWalker, pWin->p_ترتیب_By);
          sqlite3_Walk_بیان(pWalker, pWin->pFilter);
          sqlite3_کړکۍ_Link(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }وإلا
#د_که_پای /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_بیان->op = TK_AGG_FUNCTION;
          p_بیان->op2 = 0;
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
          إذا( بیان_Has_Property(p_بیان, EP_WinFunc) ){
            sqlite3_Walk_بیان(pWalker, p_بیان->y.pWin->pFilter);
          }
#د_که_پای
          pNC2 = pNC;
          طالما( pNC2
              && sqlite3ReferencesSrcList(pParse, p_بیان, pNC2->pSrcList)==0
          ){
            p_بیان->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->p_بل;
          }
          أكد( pDef!=0 || دننه_RENAME_OBJECT );
          إذا( pNC2 && pDef ){
            p_بیان->op2 += pNC2->nNestedSelect;
            أكد( SQLITE_FUNC_MINMAX==NC_MinMaxAgg );
            أكد( SQLITE_FUNC_ANYORDER==NC_ترتیب_Agg );
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
#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS:
    حالة TK_SELECT:
#د_که_پای
    حالة TK_دننه: {
      testcase( p_بیان->op==TK_دننه );
      testcase( p_بیان->op==TK_EXISTS );
      testcase( p_بیان->op==TK_SELECT );
      إذا( بیان_Use_XSelect(p_بیان) ){
        صحيح nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_بیان );
        testcase( pNC->ncFlags & NC_Gen_ستون );
        أكد( p_بیان->x.pSelect );
        إذا( p_بیان->op==TK_EXISTS )  pParse->bHasExists = 1;
        إذا( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_بیان, p_بیان);
        }وإلا{
          sqlite3WalkSelect(pWalker, p_بیان->x.pSelect);
        }
        أكد( pNC->nRef>=nRef );
        إذا( nRef!=pNC->nRef ){
          بیان_سېټ_Property(p_بیان, EP_VarSelect);
          p_بیان->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      اكسر;
    }
    حالة TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_بیان );
      testcase( pNC->ncFlags & NC_Gen_ستون );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_بیان|NC_Gen_ستون, p_بیان, p_بیان);
      اكسر;
    }
    حالة TK_IS:
    حالة TK_ISNOT: {
      بیان *p_ښی = sqlite3_بیان_Skip_Collate_And_Likely(p_بیان->p_ښی);
      أكد( !بیان_Has_Property(p_بیان, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      إذا( ALWAYS(p_ښی) && (p_ښی->op==TK_ID || p_ښی->op==TK_TRUEFALSE) ){
        صحيح rc = resolve_بیان_ګام(pWalker, p_ښی);
        إذا( rc==WRC_Abort ) أرجع WRC_Abort;
        إذا( p_ښی->op==TK_TRUEFALSE ){
          p_بیان->op2 = p_بیان->op;
          p_بیان->op = TK_TRUTH;
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
      صحيح n_کیڼ, n_ښی;
      إذا( pParse->db->mallocFailed ) اكسر;
      أكد( p_بیان->p_کیڼ!=0 );
      n_کیڼ = sqlite3_بیان_ویکټور_اندازه(p_بیان->p_کیڼ);
      إذا( p_بیان->op==TK_BETWEEN ){
        أكد( بیان_Use_XList(p_بیان) );
        n_ښی = sqlite3_بیان_ویکټور_اندازه(p_بیان->x.pList->a[0].p_بیان);
        إذا( n_ښی==n_کیڼ ){
          n_ښی = sqlite3_بیان_ویکټور_اندازه(p_بیان->x.pList->a[1].p_بیان);
        }
      }وإلا{
        أكد( p_بیان->p_ښی!=0 );
        n_ښی = sqlite3_بیان_ویکټور_اندازه(p_بیان->p_ښی);
      }
      إذا( n_کیڼ!=n_ښی ){
        testcase( p_بیان->op==TK_EQ );
        testcase( p_بیان->op==TK_NE );
        testcase( p_بیان->op==TK_LT );
        testcase( p_بیان->op==TK_LE );
        testcase( p_بیان->op==TK_GT );
        testcase( p_بیان->op==TK_GE );
        testcase( p_بیان->op==TK_IS );
        testcase( p_بیان->op==TK_ISNOT );
        testcase( p_بیان->op==TK_BETWEEN );
        sqlite3_Error_پیغام(pParse, "row value misused");
        sqlite3_ریکارډ_Error_دننه_واټن_د_بیان(pParse->db, p_بیان);
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
  بیان_List *pEList,  /* List of expressions to scan */
  بیان *pE           /* Expression we are trying to match */
){
  صحيح i;             /* Loop counter */

  ناکارول_شوی_PARAMETER(pParse);

  إذا( pE->op==TK_ID ){
    ثابت حرف *z_ستون;
    أكد( !بیان_Has_Property(pE, EP_Int_ارزښت) );
    z_ستون = pE->u.zToken;
    لكل(i=0; i<pEList->n_بیان; i++){
      إذا( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, z_ستون)==0
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
ساكن صحيح resolve_ترتیب_By_غړی_منزل_بیان_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  بیان *pE           /* The specific ORDER BY term */
){
  صحيح i;             /* Loop counter */
  بیان_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  صحيح rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  أكد( sqlite3_بیان_Is_Integer(pE, &i, 0, 1)==0 );
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
  rc = sqlite3_Resolve_بیان_نومونه(&nc, pE);
  db->suppressErr = savedSuppErr;
  إذا( rc ) أرجع 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  لكل(i=0; i<pEList->n_بیان; i++){
    إذا( sqlite3_بیان_Compare(0, pEList->a[i].p_بیان, pE, -1)<2 ){
      أرجع i+1;
    }
  }

  /* If no match, return 0. */
  أرجع 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
ساكن فراغ resolve_بهر_د_Range_Error(
  Parse *pParse,         /* The error context into which to write the error */
  ثابت حرف *zType,     /* "ORDER" or "GROUP" */
  صحيح i,                 /* The index (1-based) of the term out of range */
  صحيح mx,                /* Largest permissible value of i */
  بیان *pError           /* Associate the error with the expression */
){
  sqlite3_Error_پیغام(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_ریکارډ_Error_دننه_واټن_د_بیان(pParse->db, pError);
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
ساكن صحيح resolve_Compound_ترتیب_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  صحيح i;
  بیان_List *p_ترتیب_By;
  بیان_List *pEList;
  sqlite3 *db;
  صحيح نور_منزل_Do = 1;

  p_ترتیب_By = pSelect->p_ترتیب_By;
  إذا( p_ترتیب_By==0 ) أرجع 0;
  db = pParse->db;
  إذا( p_ترتیب_By->n_بیان>db->a_حد[SQLITE_حد_ستنه] ){
    sqlite3_Error_پیغام(pParse, "too many terms in ORDER BY clause");
    أرجع 1;
  }
  لكل(i=0; i<p_ترتیب_By->n_بیان; i++){
    p_ترتیب_By->a[i].fg.بشپړ = 0;
  }
  pSelect->p_بل = 0;
  طالما( pSelect->p_مخکینی ){
    pSelect->p_مخکینی->p_بل = pSelect;
    pSelect = pSelect->p_مخکینی;
  }
  طالما( pSelect && نور_منزل_Do ){
    بنية بیان_List_توکی *p_توکی;
    نور_منزل_Do = 0;
    pEList = pSelect->pEList;
    أكد( pEList!=0 );
    لكل(i=0, p_توکی=p_ترتیب_By->a; i<p_ترتیب_By->n_بیان; i++, p_توکی++){
      صحيح i_ستون = -1;
      بیان *pE, *pDup;
      إذا( p_توکی->fg.بشپړ ) استمر;
      pE = sqlite3_بیان_Skip_Collate_And_Likely(p_توکی->p_بیان);
      إذا( NEVER(pE==0) ) استمر;
      إذا( sqlite3_بیان_Is_Integer(pE, &i_ستون, 0, 1) ){
        إذا( i_ستون<=0 || i_ستون>pEList->n_بیان ){
          resolve_بهر_د_Range_Error(pParse, "ORDER", i+1, pEList->n_بیان, pE);
          أرجع 1;
        }
      }وإلا{
        i_ستون = resolveAsName(pParse, pEList, pE);
        إذا( i_ستون==0 ){
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
          pDup = sqlite3_بیان_Dup(db, pE, 0);
          إذا( !db->mallocFailed ){
            أكد(pDup);
            i_ستون = resolve_ترتیب_By_غړی_منزل_بیان_List(pParse, pSelect, pDup);
            إذا( دننه_RENAME_OBJECT && i_ستون>0 ){
              resolve_ترتیب_By_غړی_منزل_بیان_List(pParse, pSelect, pE);
            }
          }
          sqlite3_بیان_Delete(db, pDup);
        }
      }
      إذا( i_ستون>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        إذا( !دننه_RENAME_OBJECT ){
          بیان *pNew = sqlite3_بیان_Int32(db, i_ستون);
          إذا( pNew==0 ) أرجع 1;
          إذا( p_توکی->p_بیان==pE ){
            p_توکی->p_بیان = pNew;
          }وإلا{
            بیان *p_پلار = p_توکی->p_بیان;
            أكد( p_پلار->op==TK_COLLATE );
            طالما( p_پلار->p_کیڼ->op==TK_COLLATE ) p_پلار = p_پلار->p_کیڼ;
            أكد( p_پلار->p_کیڼ==pE );
            p_پلار->p_کیڼ = pNew;
          }
          sqlite3_بیان_Delete(db, pE);
          p_توکی->u.x.i_ترتیب_By_ستون = (u16)i_ستون;
        }
        p_توکی->fg.بشپړ = 1;
      }وإلا{
        نور_منزل_Do = 1;
      }
    }
    pSelect = pSelect->p_بل;
  }
  لكل(i=0; i<p_ترتیب_By->n_بیان; i++){
    إذا( p_ترتیب_By->a[i].fg.بشپړ==0 ){
      sqlite3_Error_پیغام(pParse, "%r ORDER BY term does not match any "
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
صحيح sqlite3_Resolve_ترتیب_ډله_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  بیان_List *p_ترتیب_By,   /* The ORDER BY or GROUP BY clause to be processed */
  ثابت حرف *zType     /* "ORDER" or "GROUP" */
){
  صحيح i;
  sqlite3 *db = pParse->db;
  بیان_List *pEList;
  بنية بیان_List_توکی *p_توکی;

  إذا( p_ترتیب_By==0 || pParse->db->mallocFailed || دننه_RENAME_OBJECT ) أرجع 0;
  إذا( p_ترتیب_By->n_بیان>db->a_حد[SQLITE_حد_ستنه] ){
    sqlite3_Error_پیغام(pParse, "too many terms in %s BY clause", zType);
    أرجع 1;
  }
  pEList = pSelect->pEList;
  أكد( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  لكل(i=0, p_توکی=p_ترتیب_By->a; i<p_ترتیب_By->n_بیان; i++, p_توکی++){
    إذا( p_توکی->u.x.i_ترتیب_By_ستون ){
      إذا( p_توکی->u.x.i_ترتیب_By_ستون>pEList->n_بیان ){
        resolve_بهر_د_Range_Error(pParse, zType, i+1, pEList->n_بیان, 0);
        أرجع 1;
      }
      resolveAlias(pParse, pEList, p_توکی->u.x.i_ترتیب_By_ستون-1, p_توکی->p_بیان,0);
    }
  }
  أرجع 0;
}

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
ساكن صحيح resolveRemoveWindowsCb(Walker *pWalker, بیان *p_بیان){
  ناکارول_شوی_PARAMETER(pWalker);
  إذا( بیان_Has_Property(p_بیان, EP_WinFunc) ){
    کړکۍ *pWin = p_بیان->y.pWin;
    sqlite3_کړکۍ_Unlink_From_Select(pWin);
  }
  أرجع WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
ساكن فراغ کړکۍ_Remove_بیان_From_Select(Select *pSelect, بیان *p_بیان){
  إذا( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, حجم(Walker));
    sWalker.x_بیان_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_بیان(&sWalker, p_بیان);
  }
}
#وإلا
# define کړکۍ_Remove_بیان_From_Select(a, b)
#د_که_پای /* SQLITE_OMIT_WINDOWFUNC */

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
ساكن صحيح resolve_ترتیب_ډله_By(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  بیان_List *p_ترتیب_By,   /* An ORDER BY or GROUP BY clause to resolve */
  ثابت حرف *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  صحيح i, j;                      /* Loop counters */
  صحيح i_ستون;                      /* Column number */
  بنية بیان_List_توکی *p_توکی;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  صحيح n_پایله;                   /* Number of terms in the result set */

  أكد( p_ترتیب_By!=0 );
  n_پایله = pSelect->pEList->n_بیان;
  pParse = pNC->pParse;
  لكل(i=0, p_توکی=p_ترتیب_By->a; i<p_ترتیب_By->n_بیان; i++, p_توکی++){
    بیان *pE = p_توکی->p_بیان;
    بیان *pE2 = sqlite3_بیان_Skip_Collate_And_Likely(pE);
    إذا( NEVER(pE2==0) ) استمر;
    إذا( zType[0]!='G' ){
      i_ستون = resolveAsName(pParse, pSelect->pEList, pE2);
      إذا( i_ستون>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        p_توکی->u.x.i_ترتیب_By_ستون = (u16)i_ستون;
        استمر;
      }
    }
    إذا( sqlite3_بیان_Is_Integer(pE2, &i_ستون, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      إذا( i_ستون<1 || i_ستون>0xffff ){
        resolve_بهر_د_Range_Error(pParse, zType, i+1, n_پایله, pE2);
        أرجع 1;
      }
      p_توکی->u.x.i_ترتیب_By_ستون = (u16)i_ستون;
      استمر;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    p_توکی->u.x.i_ترتیب_By_ستون = 0;
    إذا( sqlite3_Resolve_بیان_نومونه(pNC, pE) ){
      أرجع 1;
    }
    لكل(j=0; j<pSelect->pEList->n_بیان; j++){
      إذا( sqlite3_بیان_Compare(0, pE, pSelect->pEList->a[j].p_بیان, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        کړکۍ_Remove_بیان_From_Select(pSelect, pE);
        p_توکی->u.x.i_ترتیب_By_ستون = j+1;
      }
    }
  }
  أرجع sqlite3_Resolve_ترتیب_ډله_By(pParse, pSelect, p_ترتیب_By, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
ساكن صحيح resolve_Select_ګام(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  صحيح isCompound;         /* True if p is a compound select */
  صحيح nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  صحيح i;                  /* Loop counter */
  بیان_List *p_ډله_By;     /* The GROUP BY clause */
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

  isCompound = p->p_مخکینی!=0;
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
    إذا( sqlite3_Resolve_بیان_نومونه(&sNC, p->p_حد) ){
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
      أكد( p->pSrc->nSrc==1 && p->p_ترتیب_By );
      أكد( pSub->p_مخکینی && pSub->p_ترتیب_By==0 );
      pSub->p_ترتیب_By = p->p_ترتیب_By;
      p->p_ترتیب_By = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    إذا( pOuterNC ) pOuterNC->nNestedSelect++;
    لكل(i=0; i<p->pSrc->nSrc; i++){
      Src_توکی *p_توکی = &p->pSrc->a[i];
      أكد( p_توکی->zName!=0
              || p_توکی->fg.isSubquery );  /* Test of tag-20240424-1*/
      إذا( p_توکی->fg.isSubquery
       && (p_توکی->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        صحيح nRef = pOuterNC ? pOuterNC->nRef : 0;
        ثابت حرف *zSavedContext = pParse->zAuthContext;

        إذا( p_توکی->zName ) pParse->zAuthContext = p_توکی->zName;
        sqlite3_Resolve_Select_نومونه(pParse, p_توکی->u4.pSubq->pSelect, pOuterNC);
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
          أكد( p_توکی->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          p_توکی->fg.isCorrelated = (pOuterNC->nRef>nRef);
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
    sNC.p_بل = pOuterNC;
 
    /* Resolve names in the result set. */
    إذا( sqlite3_Resolve_بیان_List_نومونه(&sNC, p->pEList) ) أرجع WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    أكد( (p->selFlags & SF_Aggregate)==0 );
    p_ډله_By = p->p_ډله_By;
    إذا( p_ډله_By || (sNC.ncFlags & NC_HasAgg)!=0 ){
      أكد( NC_MinMaxAgg==SF_MinMaxAgg );
      أكد( NC_ترتیب_Agg==SF_ترتیب_By_Reqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_ترتیب_Agg));
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
        sqlite3_Error_پیغام(pParse, "HAVING clause on a non-aggregate query");
        أرجع WRC_Abort;
      }
      إذا( sqlite3_Resolve_بیان_نومونه(&sNC, p->pHaving) ) أرجع WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    إذا( sqlite3_Resolve_بیان_نومونه(&sNC, p->pWhere) ) أرجع WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    لكل(i=0; i<p->pSrc->nSrc; i++){
      Src_توکی *p_توکی = &p->pSrc->a[i];
      إذا( p_توکی->fg.isTabFunc
       && sqlite3_Resolve_بیان_List_نومونه(&sNC, p_توکی->u1.pFuncArg)
      ){
        أرجع WRC_Abort;
      }
    }

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    إذا( دننه_RENAME_OBJECT ){
      کړکۍ *pWin;
      لكل(pWin=p->pWinDefn; pWin; pWin=pWin->p_بل_Win){
        إذا( sqlite3_Resolve_بیان_List_نومونه(&sNC, pWin->p_ترتیب_By)
         || sqlite3_Resolve_بیان_List_نومونه(&sNC, pWin->p_برخه_کول)
        ){
          أرجع WRC_Abort;
        }
      }
    }
#د_که_پای

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
      p->p_ترتیب_By = pSub->p_ترتیب_By;
      pSub->p_ترتیب_By = 0;
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
    إذا( p->p_ترتیب_By!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolve_ترتیب_ډله_By(&sNC, p, p->p_ترتیب_By, "ORDER")
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
    إذا( p_ډله_By ){
      بنية بیان_List_توکی *p_توکی;
   
      إذا( resolve_ترتیب_ډله_By(&sNC, p, p_ډله_By, "GROUP") || db->mallocFailed ){
        أرجع WRC_Abort;
      }
      لكل(i=0, p_توکی=p_ډله_By->a; i<p_ډله_By->n_بیان; i++, p_توکی++){
        إذا( بیان_Has_Property(p_توکی->p_بیان, EP_Agg) ){
          sqlite3_Error_پیغام(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          أرجع WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    إذا( p->p_بل && p->pEList->n_بیان!=p->p_بل->pEList->n_بیان ){
      sqlite3_Select_Wrong_عدد_Terms_Error(pParse, p->p_بل);
      أرجع WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    إذا( (p->selFlags & SF_On_منزل_Where) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      إذا( pParse->nErr ) أرجع WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->p_مخکینی;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  إذا( isCompound && resolve_Compound_ترتیب_By(pParse, pLeftmost) ){
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
صحيح sqlite3_Resolve_بیان_نومونه(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  بیان *p_بیان             /* The expression to be analyzed. */
){
  صحيح savedHasAgg;
  Walker w;

  إذا( p_بیان==0 ) أرجع SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
  w.pParse = pNC->pParse;
  w.x_بیان_Callback = resolve_بیان_ګام;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolve_Select_ګام;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#إذا SQLITE_MAX_بیان_ژوروالی>0
  w.pParse->n_لوړوالی += p_بیان->n_لوړوالی;
  إذا( sqlite3_بیان_Check_لوړوالی(w.pParse, w.pParse->n_لوړوالی) ){
    أرجع SQLITE_ERROR;
  }
#د_که_پای
  أكد( p_بیان!=0 );
  sqlite3_Walk_بیان_NN(&w, p_بیان);
#إذا SQLITE_MAX_بیان_ژوروالی>0
  w.pParse->n_لوړوالی -= p_بیان->n_لوړوالی;
#د_که_پای
  أكد( EP_Agg==NC_HasAgg );
  أكد( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  بیان_سېټ_Property(p_بیان, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
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
صحيح sqlite3_Resolve_بیان_List_نومونه(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  بیان_List *pList         /* The expression list to be analyzed. */
){
  صحيح i;
  صحيح savedHasAgg = 0;
  Walker w;
  إذا( pList==0 ) أرجع SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_بیان_Callback = resolve_بیان_ګام;
  w.xSelectCallback = resolve_Select_ګام;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
  لكل(i=0; i<pList->n_بیان; i++){
    بیان *p_بیان = pList->a[i].p_بیان;
    إذا( p_بیان==0 ) استمر;
#إذا SQLITE_MAX_بیان_ژوروالی>0
    w.pParse->n_لوړوالی += p_بیان->n_لوړوالی;
    إذا( sqlite3_بیان_Check_لوړوالی(w.pParse, w.pParse->n_لوړوالی) ){
      أرجع SQLITE_ERROR;
    }
#د_که_پای
    sqlite3_Walk_بیان_NN(&w, p_بیان);
#إذا SQLITE_MAX_بیان_ژوروالی>0
    w.pParse->n_لوړوالی -= p_بیان->n_لوړوالی;
#د_که_پای
    أكد( EP_Agg==NC_HasAgg );
    أكد( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    إذا( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg) ){
      بیان_سېټ_Property(p_بیان, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
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
فراغ sqlite3_Resolve_Select_نومونه(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  أكد( p!=0 );
  w.x_بیان_Callback = resolve_بیان_ګام;
  w.xSelectCallback = resolve_Select_ګام;
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
  جدول *pTab,     /* The table being referenced, or NULL */
  صحيح type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  بیان *p_بیان,     /* Expression to resolve.  May be NULL. */
  بیان_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  صحيح rc;
  اتحاد {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  أكد( type==0 || pTab!=0 );
  أكد( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_بیان
          || type==NC_Gen_ستون || pTab==0 );
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
  إذا( (rc = sqlite3_Resolve_بیان_نومونه(&sNC, p_بیان))!=SQLITE_OK ) أرجع rc;
  إذا( pList ) rc = sqlite3_Resolve_بیان_List_نومونه(&sNC, pList);
  أرجع rc;
}
