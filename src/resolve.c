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
#شامل "sqliteInt.h"

/*
** Magic table number to mean the EXCLUDED table in an UPSERT statement.
*/
#تعریف_کرو EXCLUDED_جدول_نمبر  2

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
ساکن صحیح incr_Agg_گہرائی(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  اگر( p_اظہاریہ->op==TK_AGG_FUNCTION ) p_اظہاریہ->op2 += pWalker->u.n;
  واپس WRC_Continue;
}
ساکن خالی incr_Agg_Function_گہرائی(اظہاریہ *p_اظہاریہ, صحیح N){
  اگر( N>0 ){
    Walker w;
    memset(&w, 0, حجم(w));
    w.x_اظہاریہ_Callback = incr_Agg_گہرائی;
    w.u.n = N;
    sqlite3_Walk_اظہاریہ(&w, p_اظہاریہ);
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
ساکن خالی resolveAlias(
  Parse *pParse,         /* Parsing context */
  اظہاریہ_List *pEList,      /* A result set */
  صحیح i_ستون,              /* A column in the result set.  0..pEList->nExpr-1 */
  اظہاریہ *p_اظہاریہ,           /* Transform this into an alias to the result set */
  صحیح nSubquery          /* Number of subqueries that the label is moving */
){
  اظہاریہ *pOrig;           /* The iCol-th column of the result set */
  اظہاریہ *pDup;            /* Copy of pOrig */
  sqlite3 *db;           /* The database connection */

  أكد( i_ستون>=0 && i_ستون<pEList->n_اظہاریہ );
  pOrig = pEList->a[i_ستون].p_اظہاریہ;
  أكد( pOrig!=0 );
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Reduced|EP_TokenOnly) );
  اگر( p_اظہاریہ->pAggInfo ) واپس;
  db = pParse->db;
  pDup = sqlite3_اظہاریہ_Dup(db, pOrig, 0);
  اگر( db->mallocFailed ){
    sqlite3_اظہاریہ_Delete(db, pDup);
    pDup = 0;
  }ورنہ{
    اظہاریہ عارضی;
    incr_Agg_Function_گہرائی(pDup, nSubquery);
    اگر( p_اظہاریہ->op==TK_COLLATE ){
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      pDup = sqlite3_اظہاریہ_جوڑو_Collate_String(pParse, pDup, p_اظہاریہ->u.zToken);
    }
    حافظہ_نقل(&عارضی, pDup, حجم(اظہاریہ));
    حافظہ_نقل(pDup, p_اظہاریہ, حجم(اظہاریہ));
    حافظہ_نقل(p_اظہاریہ, &عارضی, حجم(اظہاریہ));
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
      اگر( ALWAYS(p_اظہاریہ->y.pWin!=0) ){
        p_اظہاریہ->y.pWin->pOwner = p_اظہاریہ;
      }
    }
    sqlite3_اظہاریہ_Deferred_Delete(pParse, pDup);
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
صحیح sqlite3_مطابقت_EName(
  ثابت ساخت اظہاریہ_List_شے *p_شے,
  ثابت حرف *z_ستون,
  ثابت حرف *zTab,
  ثابت حرف *zDb,
  صحیح *pbRowid
){
  صحیح n;
  ثابت حرف *z_وسعت;
  صحیح eEName = p_شے->fg.eEName;
  اگر( eEName!=ENAME_TAB && (eEName!=ENAME_ROWID || NEVER(pbRowid==0)) ){
    واپس 0;
  }
  أكد( pbRowid==0 || *pbRowid==0 );
  z_وسعت = p_شے->zEName;
  برائے(n=0; ALWAYS(z_وسعت[n]) && z_وسعت[n]!='.'; n++){}
  اگر( zDb && (sqlite3StrNICmp(z_وسعت, zDb, n)!=0 || zDb[n]!=0) ){
    واپس 0;
  }
  z_وسعت += n+1;
  برائے(n=0; ALWAYS(z_وسعت[n]) && z_وسعت[n]!='.'; n++){}
  اگر( zTab && (sqlite3StrNICmp(z_وسعت, zTab, n)!=0 || zTab[n]!=0) ){
    واپس 0;
  }
  z_وسعت += n+1;
  اگر( z_ستون ){
    اگر( eEName==ENAME_TAB && sqlite3StrICmp(z_وسعت, z_ستون)!=0 ) واپس 0;
    اگر( eEName==ENAME_ROWID && sqlite3IsRowid(z_ستون)==0 ) واپس 0;
  }
  اگر( eEName==ENAME_ROWID ) *pbRowid = 1;
  واپس 1;
}

/*
** Return TRUE if the double-quoted string  mis-feature should be supported.
*/
ساکن صحیح are_Double_Quoted_لڑیاں_Enabled(sqlite3 *db, NameContext *p_چوٹی_NC){
  اگر( db->init.busy ) واپس 1;  /* Always support for legacy schemas */
  اگر( p_چوٹی_NC->ncFlags & NC_IsDDL ){
    /* Currently parsing a DDL statement */
    اگر( sqlite3WritableSchema(db) && (db->flags & SQLITE_DqsDML)!=0 ){
      واپس 1;
    }
    واپس (db->flags & SQLITE_DqsDDL)!=0;
  }ورنہ{
    /* Currently parsing a DML statement */
    واپس (db->flags & SQLITE_DqsDML)!=0;
  }
}

/*
** The argument is guaranteed to be a non-NULL Expr node of type TK_COLUMN.
** return the appropriate colUsed mask.
*/
Bitmask sqlite3_اظہاریہ_ستون_مستعمل(اظہاریہ *p_اظہاریہ){
  صحیح n;
  جدول *pExTab;

  n = p_اظہاریہ->i_عمود;
  أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
  pExTab = p_اظہاریہ->y.pTab;
  أكد( pExTab!=0 );
  أكد( n < pExTab->n_ستون );
  اگر( (pExTab->tabFlags & TF_HasGenerated)!=0
   && (pExTab->a_ستون[n].ستون_Flags & COLFLAG_GENERATED)!=0
  ){
    testcase( pExTab->n_ستون==BMS-1 );
    testcase( pExTab->n_ستون==BMS );
    واپس pExTab->n_ستون>=BMS ? ALLBITS : MASKBIT(pExTab->n_ستون)-1;
  }ورنہ{
    testcase( n==BMS-1 );
    testcase( n==BMS );
    اگر( n>=BMS ) n = BMS-1;
    واپس ((Bitmask)1)<<n;
  }
}

/*
** Create a new expression term for the column specified by pMatch and
** iColumn.  Append this new expression term to the FULL JOIN Match set
** in *ppList.  Create a new *ppList if this is the first term in the
** set.
*/
ساکن خالی extendFJMatch(
  Parse *pParse,          /* Parsing context */
  اظہاریہ_List **ppList,      /* ExprList to extend */
  Src_شے *p_مطابقت,        /* Source table containing the column */
  i16 i_عمود             /* The column number */
){
  اظہاریہ *pNew = sqlite3_اظہاریہ_Alloc(pParse->db, TK_عمود, 0, 0);
  اگر( pNew ){
    pNew->i_جدول = p_مطابقت->iCursor;
    pNew->i_عمود = i_عمود;
    pNew->y.pTab = p_مطابقت->pSTab;
    أكد( (p_مطابقت->fg.jointype & (JT_بایاں|JT_LTORJ))!=0 );
    اظہاریہ_سیٹ_Property(pNew, EP_CanBeNull);
    *ppList = sqlite3_اظہاریہ_List_Append(pParse, *ppList, pNew);
  }
}

/*
** Return TRUE (non-zero) if zTab is a valid name for the schema table pTab.
*/
ساکن SQLITE_NOINLINE صحیح is_Valid_Schema_جدول_Name(
  ثابت حرف *zTab,         /* Name as it appears in the SQL */
  جدول *pTab,              /* The schema table we are trying to match */
  ثابت حرف *zDb           /* non-NULL if a database qualifier is present */
){
  ثابت حرف *zLegacy;
  أكد( pTab!=0 );
  أكد( pTab->tnum==1 );
  اگر( sqlite3StrNICmp(zTab, "sqlite_", 7)!=0 ) واپس 0;
  zLegacy = pTab->zName;
  اگر( strcmp(zLegacy+7, &LEGACY_عارضی_SCHEMA_جدول[7])==0 ){
    اگر( sqlite3StrICmp(zTab+7, &PREFERRED_عارضی_SCHEMA_جدول[7])==0 ){
      واپس 1;
    }
    اگر( zDb==0 ) واپس 0;
    اگر( sqlite3StrICmp(zTab+7, &LEGACY_SCHEMA_جدول[7])==0 ) واپس 1;
    اگر( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_جدول[7])==0 ) واپس 1;
  }ورنہ{
    اگر( sqlite3StrICmp(zTab+7, &PREFERRED_SCHEMA_جدول[7])==0 ) واپس 1;
  }
  واپس 0;
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
ساکن صحیح lookupName(
  Parse *pParse,       /* The parsing context */
  ثابت حرف *zDb,     /* Name of the database containing table, or NULL */
  ثابت حرف *zTab,    /* Name of table containing column, or NULL */
  ثابت اظہاریہ *p_دایاں,  /* Name of the column. */
  NameContext *pNC,    /* The name context used to resolve the name */
  اظہاریہ *p_اظہاریہ          /* Make this EXPR node point to the selected column */
){
  صحیح i, j;                         /* Loop counters */
  صحیح cnt = 0;                      /* Number of matching column names */
  صحیح cntTab = 0;                   /* Number of potential "rowid" matches */
  صحیح nSubquery = 0;                /* How many levels of subquery */
  sqlite3 *db = pParse->db;         /* The database connection */
  Src_شے *p_شے;                   /* Use for looping over pSrcList items */
  Src_شے *p_مطابقت = 0;              /* The matching pSrcList item */
  NameContext *p_چوٹی_NC = pNC;        /* First namecontext in the list */
  Schema *pSchema = 0;              /* Schema of the expression */
  صحیح e_New_اظہاریہ_Op = TK_عمود;       /* New value for pExpr->op on success */
  جدول *pTab = 0;                  /* Table holding the row */
  اظہاریہ_List *pFJMatch = 0;           /* Matches for FULL JOIN .. USING */
  ثابت حرف *z_ستون = p_دایاں->u.zToken;

  أكد( pNC );     /* the name context cannot be NULL. */
  أكد( z_ستون );    /* The Z in X.Y.Z cannot be NULL */
  أكد( zDb==0 || zTab!=0 );
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly|EP_Reduced) );

  /* Initialize the node to no-match */
  p_اظہاریہ->i_جدول = -1;
  اظہاریہ_سیٹ_VVAProperty(p_اظہاریہ, EP_NoReduce);

  /* Translate the schema name in zDb into a pointer to the corresponding
  ** schema.  If not found, pSchema will remain NULL and nothing will match
  ** resulting in an appropriate error message toward the end of this routine
  */
  اگر( zDb ){
    testcase( pNC->ncFlags & NC_PartIdx );
    testcase( pNC->ncFlags & NC_IsCheck );
    اگر( (pNC->ncFlags & (NC_PartIdx|NC_IsCheck))!=0 ){
      /* Silently ignore database qualifiers inside CHECK constraints and
      ** partial indices.  Do not raise errors because that might break
      ** legacy and because it does not hurt anything to just ignore the
      ** database name. */
      zDb = 0;
    }ورنہ{
      برائے(i=0; i<db->nDb; i++){
        أكد( db->aDb[i].zDbSName );
        اگر( sqlite3StrICmp(db->aDb[i].zDbSName,zDb)==0 ){
          pSchema = db->aDb[i].pSchema;
          توڑو;
        }
      }
      اگر( i==db->nDb && sqlite3StrICmp("main", zDb)==0 ){
        /* This branch is taken when the main database has been renamed
        ** using SQLITE_DBCONFIG_MAINDBNAME. */
        pSchema = db->aDb[0].pSchema;
        zDb = db->aDb[0].zDbSName;
      }
    }
  }

  /* Start at the inner-most context and move outward until a match is found */
  أكد( pNC && cnt==0 );
  کرو{
    اظہاریہ_List *pEList;
    SrcList *pSrcList = pNC->pSrcList;

    اگر( pSrcList ){
      برائے(i=0, p_شے=pSrcList->a; i<pSrcList->nSrc; i++, p_شے++){
        pTab = p_شے->pSTab;
        أكد( pTab!=0 && pTab->zName!=0 );
        أكد( pTab->n_ستون>0 || pParse->nErr );
        أكد( (صحیح)p_شے->fg.isNestedFrom == IsNestedFrom(p_شے));
        اگر( p_شے->fg.isNestedFrom ){
          /* In this case, pItem is a subquery that has been formed from a
          ** parenthesized subset of the FROM clause terms.  Example:
          **   .... FROM t1 LEFT JOIN (t2 RIGHT JOIN t3 USING(x)) USING(y) ...
          **                          \_________________________/
          **             This pItem -------------^
          */
          صحیح کامیابی = 0;
          Select *pSel;
          أكد( p_شے->fg.isSubquery );
          أكد( p_شے->u4.pSubq!=0 );
          pSel = p_شے->u4.pSubq->pSelect;
          أكد( pSel!=0 );
          pEList = pSel->pEList;
          أكد( pEList!=0 );
          أكد( pEList->n_اظہاریہ==pTab->n_ستون );
          برائے(j=0; j<pEList->n_اظہاریہ; j++){
            صحیح bRowid = 0;       /* True if possible rowid match */
            اگر( !sqlite3_مطابقت_EName(&pEList->a[j], z_ستون, zTab, zDb, &bRowid) ){
              جاری;
            }
            اگر( bRowid==0 ){
              اگر( cnt>0 ){
                اگر( p_شے->fg.isUsing==0
                 || sqlite3_Id_List_اشاریہ(p_شے->u3.pUsing, z_ستون)<0
                 || p_مطابقت==p_شے
                ){
                  /* Two or more tables have the same column name which is
                  ** not joined by USING. Or, a single table has two columns
                  ** that match a USING term (if pMatch==pItem). These are both
                  ** "ambiguous column name" errors. Signal as much by clearing
                  ** pFJMatch and letting cnt go above 1. */
                  sqlite3_اظہاریہ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ورنہ
                اگر( (p_شے->fg.jointype & JT_دایاں)==0 ){
                  /* An INNER or LEFT JOIN.  Use the left-most table */
                  جاری;
                }ورنہ
                اگر( (p_شے->fg.jointype & JT_بایاں)==0 ){
                  /* A RIGHT JOIN.  Use the right-most table */
                  cnt = 0;
                  sqlite3_اظہاریہ_List_Delete(db, pFJMatch);
                  pFJMatch = 0;
                }ورنہ{
                  /* For a FULL JOIN, we must construct a coalesce() func */
                  extendFJMatch(pParse, &pFJMatch, p_مطابقت, p_اظہاریہ->i_عمود);
                }
              }
              cnt++;
              کامیابی = 1;
            }ورنہ اگر( cnt>0 ){
              /* This is a potential rowid match, but there has already been
              ** a real match found. So this can be ignored.  */
              جاری;
            }
            cntTab++;
            p_مطابقت = p_شے;
            p_اظہاریہ->i_عمود = j;
            pEList->a[j].fg.b_مستعمل = 1;

            /* rowid cannot be part of a USING clause - assert() this. */
            أكد( bRowid==0 || pEList->a[j].fg.b_Using_رکن==0 );
            اگر( pEList->a[j].fg.b_Using_رکن ) توڑو;
          }
          اگر( کامیابی || zTab==0 ) جاری;
        }
        أكد( zDb==0 || zTab!=0 );
        اگر( zTab ){
          اگر( zDb ){
            اگر( pTab->pSchema!=pSchema ) جاری;
            اگر( pSchema==0 && strcmp(zDb,"*")!=0 ) جاری;
          }
          اگر( p_شے->zAlias!=0 ){
            اگر( sqlite3StrICmp(zTab, p_شے->zAlias)!=0 ){
              جاری;
            }
          }ورنہ اگر( sqlite3StrICmp(zTab, pTab->zName)!=0 ){
            اگر( pTab->tnum!=1 ) جاری;
            اگر( !is_Valid_Schema_جدول_Name(zTab, pTab, zDb) ) جاری;
          }
          أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
          اگر( اندر_RENAME_OBJECT && p_شے->zAlias ){
            sqlite3RenameTokenRemap(pParse, 0, (خالی*)&p_اظہاریہ->y.pTab);
          }
        }
        j = sqlite3_عمود_اشاریہ(pTab, z_ستون);
        اگر( j>=0 ){
          اگر( cnt>0 ){
            اگر( p_شے->fg.isUsing==0
             || sqlite3_Id_List_اشاریہ(p_شے->u3.pUsing, z_ستون)<0
            ){
              /* Two or more tables have the same column name which is
              ** not joined by USING.  This is an error.  Signal as much
              ** by clearing pFJMatch and letting cnt go above 1. */
              sqlite3_اظہاریہ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ورنہ
            اگر( (p_شے->fg.jointype & JT_دایاں)==0 ){
              /* An INNER or LEFT JOIN.  Use the left-most table */
              جاری;
            }ورنہ
            اگر( (p_شے->fg.jointype & JT_بایاں)==0 ){
              /* A RIGHT JOIN.  Use the right-most table */
              cnt = 0;
              sqlite3_اظہاریہ_List_Delete(db, pFJMatch);
              pFJMatch = 0;
            }ورنہ{
              /* For a FULL JOIN, we must construct a coalesce() func */
              extendFJMatch(pParse, &pFJMatch, p_مطابقت, p_اظہاریہ->i_عمود);
            }
          }
          cnt++;
          p_مطابقت = p_شے;
          /* Substitute the rowid (column -1) for the INTEGER PRIMARY KEY */
          p_اظہاریہ->i_عمود = j==pTab->iPKey ? -1 : (i16)j;
          اگر( p_شے->fg.isNestedFrom ){
            sqlite3_Src_شے_عمود_مستعمل(p_شے, j);
          }
        }
        اگر( 0==cnt && VisibleRowid(pTab) ){
          /* pTab is a potential ROWID match.  Keep track of it and match
          ** the ROWID later if that seems appropriate.  (Search for "cntTab"
          ** to find related code.)  Only allow a ROWID match if there is
          ** a single ROWID match candidate.
          */
#اگر_تعریف_شدہ SQLITE_ALLOW_ROWID_اندر_VIEW
          /* In SQLITE_ALLOW_ROWID_IN_VIEW mode, allow a ROWID match
          ** if there is a single VIEW candidate or if there is a single
          ** non-VIEW candidate plus multiple VIEW candidates.  In other
          ** words non-VIEW candidate terms take precedence over VIEWs.
          */
          اگر( cntTab==0
           || (cntTab==1
               && p_مطابقت!=0
               && ALWAYS(p_مطابقت->pSTab!=0)
               && (p_مطابقت->pSTab->tabFlags & TF_Ephemeral)!=0
               && (pTab->tabFlags & TF_Ephemeral)==0)
          ){
            cntTab = 1;
            p_مطابقت = p_شے;
          }ورنہ{
            cntTab++;
          }
#ورنہ
          /* The (much more common) non-SQLITE_ALLOW_ROWID_IN_VIEW case is
          ** simpler since we require exactly one candidate, which will
          ** always be a non-VIEW
          */
          cntTab++;
          p_مطابقت = p_شے;
#اگر_ختم
        }
      }
      اگر( p_مطابقت ){
        p_اظہاریہ->i_جدول = p_مطابقت->iCursor;
        أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
        p_اظہاریہ->y.pTab = p_مطابقت->pSTab;
        اگر( (p_مطابقت->fg.jointype & (JT_بایاں|JT_LTORJ))!=0 ){
          اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_CanBeNull);
        }
        pSchema = p_اظہاریہ->y.pTab->pSchema;
      }
    } /* if( pSrcList ) */

#اگر !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT)
    /* If we have not already resolved the name, then maybe
    ** it is a new.* or old.* trigger argument reference.  Or
    ** maybe it is an excluded.* from an upsert.  Or maybe it is
    ** a reference in the RETURNING clause to a table being modified.
    */
    اگر( cnt==0 && zDb==0 ){
      pTab = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
      اگر( pParse->pTriggerTab!=0 ){
        صحیح op = pParse->eTriggerOp;
        أكد( op==TK_DELETE || op==TK_UPDATE || op==TK_داخل_کرو );
        اگر( pParse->bReturning ){
          اگر( (pNC->ncFlags & NC_UBaseReg)!=0
           && ALWAYS(zTab==0
                     || sqlite3StrICmp(zTab,pParse->pTriggerTab->zName)==0
                     || is_Valid_Schema_جدول_Name(zTab, pParse->pTriggerTab, 0))
          ){
            p_اظہاریہ->i_جدول = op!=TK_DELETE;
            pTab = pParse->pTriggerTab;
          }
        }ورنہ اگر( op!=TK_DELETE && zTab && sqlite3StrICmp("new",zTab) == 0 ){
          p_اظہاریہ->i_جدول = 1;
          pTab = pParse->pTriggerTab;
        }ورنہ اگر( op!=TK_داخل_کرو && zTab && sqlite3StrICmp("old",zTab)==0 ){
          p_اظہاریہ->i_جدول = 0;
          pTab = pParse->pTriggerTab;
        }
      }
#اگر_ختم /* SQLITE_OMIT_TRIGGER */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UPSERT
      اگر( (pNC->ncFlags & NC_UUpsert)!=0 && zTab!=0 ){
        Upsert *pUpsert = pNC->uNC.pUpsert;
        اگر( pUpsert && sqlite3StrICmp("excluded",zTab)==0 ){
          pTab = pUpsert->pUpsertSrc->a[0].pSTab;
          p_اظہاریہ->i_جدول = EXCLUDED_جدول_نمبر;
          اگر( اندر_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (خالی*)&p_اظہاریہ->y.pTab);
          }
        }
      }
#اگر_ختم /* SQLITE_OMIT_UPSERT */

      اگر( pTab ){
        صحیح i_ستون;
        pSchema = pTab->pSchema;
        cntTab++;
        i_ستون = sqlite3_عمود_اشاریہ(pTab, z_ستون);
        اگر( i_ستون>=0 ){
          اگر( pTab->iPKey==i_ستون ) i_ستون = -1;
        }ورنہ{
          اگر( sqlite3IsRowid(z_ستون) && VisibleRowid(pTab) ){
            i_ستون = -1;
          }ورنہ{
            i_ستون = pTab->n_ستون;
          }
        }
        اگر( i_ستون<pTab->n_ستون ){
          cnt++;
          p_مطابقت = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_UPSERT
          اگر( p_اظہاریہ->i_جدول==EXCLUDED_جدول_نمبر ){
            testcase( i_ستون==(-1) );
            أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
            اگر( اندر_RENAME_OBJECT ){
              p_اظہاریہ->i_عمود = i_ستون;
              p_اظہاریہ->y.pTab = pTab;
              e_New_اظہاریہ_Op = TK_عمود;
            }ورنہ{
              p_اظہاریہ->i_جدول = pNC->uNC.pUpsert->reg_ڈیٹا +
                 sqlite3_جدول_عمود_مقصد_Storage(pTab, i_ستون);
              e_New_اظہاریہ_Op = TK_رجسٹر;
            }
          }ورنہ
#اگر_ختم /* SQLITE_OMIT_UPSERT */
          {
            أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
            p_اظہاریہ->y.pTab = pTab;
            اگر( pParse->bReturning ){
              e_New_اظہاریہ_Op = TK_رجسٹر;
              p_اظہاریہ->op2 = TK_عمود;
              p_اظہاریہ->i_عمود = i_ستون;
              p_اظہاریہ->i_جدول = pNC->uNC.i_بنیاد_Reg + (pTab->n_ستون+1)*p_اظہاریہ->i_جدول +
                 sqlite3_جدول_عمود_مقصد_Storage(pTab, i_ستون) + 1;
            }ورنہ{
              p_اظہاریہ->i_عمود = (i16)i_ستون;
              e_New_اظہاریہ_Op = TK_TRIGGER;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
              اگر( i_ستون<0 ){
                p_اظہاریہ->aff_اظہاریہ = SQLITE_AFF_INTEGER;
              }ورنہ اگر( p_اظہاریہ->i_جدول==0 ){
                testcase( i_ستون==31 );
                testcase( i_ستون==32 );
                pParse->oldmask |= (i_ستون>=32 ? 0xffffffff : (((u32)1)<<i_ستون));
              }ورنہ{
                testcase( i_ستون==31 );
                testcase( i_ستون==32 );
                pParse->newmask |= (i_ستون>=32 ? 0xffffffff : (((u32)1)<<i_ستون));
              }
#اگر_ختم /* SQLITE_OMIT_TRIGGER */
            }
          }
        }
      }
    }
#اگر_ختم /* !defined(SQLITE_OMIT_TRIGGER) || !defined(SQLITE_OMIT_UPSERT) */

    /*
    ** Perhaps the name is a reference to the ROWID
    */
    اگر( cnt==0
     && cntTab>=1
     && p_مطابقت
     && (pNC->ncFlags & (NC_Idx_اظہاریہ|NC_Gen_ستون))==0
     && sqlite3IsRowid(z_ستون)
     && ALWAYS(VisibleRowid(p_مطابقت->pSTab) || p_مطابقت->fg.isNestedFrom)
    ){
      cnt = cntTab;
#اگر SQLITE_ALLOW_ROWID_اندر_VIEW+0==2
      اگر( p_مطابقت->pSTab!=0 && IsView(p_مطابقت->pSTab) ){
        e_New_اظہاریہ_Op = TK_NULL;
      }
#اگر_ختم
      اگر( p_مطابقت->fg.isNestedFrom==0 ) p_اظہاریہ->i_عمود = -1;
      p_اظہاریہ->aff_اظہاریہ = SQLITE_AFF_INTEGER;
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
    اگر( cnt==0
     && (pNC->ncFlags & NC_UEList)!=0
     && zTab==0
    ){
      pEList = pNC->uNC.pEList;
      أكد( pEList!=0 );
      برائے(j=0; j<pEList->n_اظہاریہ; j++){
        حرف *zAs = pEList->a[j].zEName;
        اگر( pEList->a[j].fg.eEName==ENAME_NAME
         && sqlite3_stricmp(zAs, z_ستون)==0
        ){
          اظہاریہ *pOrig;
          أكد( p_اظہاریہ->p_بایاں==0 && p_اظہاریہ->p_دایاں==0 );
          أكد( اظہاریہ_Use_XList(p_اظہاریہ)==0 || p_اظہاریہ->x.pList==0 );
          أكد( اظہاریہ_Use_XSelect(p_اظہاریہ)==0 || p_اظہاریہ->x.pSelect==0 );
          pOrig = pEList->a[j].p_اظہاریہ;
          اگر( (pNC->ncFlags&NC_AllowAgg)==0 && اظہاریہ_Has_Property(pOrig, EP_Agg) ){
            sqlite3_Error_پیغام(pParse, "misuse of aliased aggregate %s", zAs);
            واپس WRC_Abort;
          }
          اگر( اظہاریہ_Has_Property(pOrig, EP_Win)
           && ((pNC->ncFlags&NC_AllowWin)==0 || pNC!=p_چوٹی_NC )
          ){
            sqlite3_Error_پیغام(pParse, "misuse of aliased window function %s",zAs);
            واپس WRC_Abort;
          }
          اگر( sqlite3_اظہاریہ_سمتیہ_سائز(pOrig)!=1 ){
            sqlite3_Error_پیغام(pParse, "row value misused");
            واپس WRC_Abort;
          }
          resolveAlias(pParse, pEList, j, p_اظہاریہ, nSubquery);
          cnt = 1;
          p_مطابقت = 0;
          أكد( zTab==0 && zDb==0 );
          اگر( اندر_RENAME_OBJECT ){
            sqlite3RenameTokenRemap(pParse, 0, (خالی*)p_اظہاریہ);
          }
          جاؤ lookupname_انتہا;
        }
      }
    }

    /* Advance to the next name context.  The loop will exit when either
    ** we have a match (cnt>0) or when we run out of name contexts.
    */
    اگر( cnt ) توڑو;
    pNC = pNC->p_اگلا;
    nSubquery++;
  }جب( pNC );


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
  اگر( cnt==0 && zTab==0 ){
    أكد( p_اظہاریہ->op==TK_ID );
    اگر( اظہاریہ_Has_Property(p_اظہاریہ,EP_DblQuoted)
     && are_Double_Quoted_لڑیاں_Enabled(db, p_چوٹی_NC)
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
#اگر_تعریف_شدہ SQLITE_ENABLE_NORMALIZE
      sqlite3_Vdbe_جوڑو_Dblquote_Str(db, pParse->pVdbe, z_ستون);
#اگر_ختم
      p_اظہاریہ->op = TK_STRING;
      memset(&p_اظہاریہ->y, 0, حجم(p_اظہاریہ->y));
      واپس WRC_Prune;
    }
    اگر( sqlite3_اظہاریہ_Id_مقصد_True_False(p_اظہاریہ) ){
      واپس WRC_Prune;
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
  أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_xIsSelect|EP_Int_قدر) );
  اگر( cnt!=1 ){
    ثابت حرف *zErr;
    اگر( pFJMatch ){
      اگر( pFJMatch->n_اظہاریہ==cnt-1 ){
        اگر( اظہاریہ_Has_Property(p_اظہاریہ,EP_برگ) ){
          اظہاریہ_Clear_Property(p_اظہاریہ,EP_برگ);
        }ورنہ{
          sqlite3_اظہاریہ_Delete(db, p_اظہاریہ->p_بایاں);
          p_اظہاریہ->p_بایاں = 0;
          sqlite3_اظہاریہ_Delete(db, p_اظہاریہ->p_دایاں);
          p_اظہاریہ->p_دایاں = 0;
        }
        extendFJMatch(pParse, &pFJMatch, p_مطابقت, p_اظہاریہ->i_عمود);
        p_اظہاریہ->op = TK_FUNCTION;
        p_اظہاریہ->u.zToken = "coalesce";
        p_اظہاریہ->x.pList = pFJMatch;
        p_اظہاریہ->aff_اظہاریہ = SQLITE_AFF_DEFER;
        cnt = 1;
        جاؤ lookupname_انتہا;
      }ورنہ{
        sqlite3_اظہاریہ_List_Delete(db, pFJMatch);
        pFJMatch = 0;
      }
    }
    zErr = cnt==0 ? "no such column" : "ambiguous column name";
    اگر( zDb ){
      sqlite3_Error_پیغام(pParse, "%s: %s.%s.%s", zErr, zDb, zTab, z_ستون);
    }ورنہ اگر( zTab ){
      sqlite3_Error_پیغام(pParse, "%s: %s.%s", zErr, zTab, z_ستون);
    }ورنہ اگر( cnt==0 && اظہاریہ_Has_Property(p_دایاں,EP_DblQuoted) ){
      sqlite3_Error_پیغام(pParse, "%s: \"%s\" - should this be a"
                              " string literal in single-quotes?",
                              zErr, z_ستون);
    }ورنہ{
      sqlite3_Error_پیغام(pParse, "%s: %s", zErr, z_ستون);
    }
    sqlite3_ریکارڈ_Error_انحراف_کا_اظہاریہ(pParse->db, p_اظہاریہ);
    pParse->checkSchema = 1;
    p_چوٹی_NC->nNcErr++;
    e_New_اظہاریہ_Op = TK_NULL;
  }
  أكد( pFJMatch==0 );

  /* Remove all substructure from pExpr */
  اگر( !اظہاریہ_Has_Property(p_اظہاریہ,(EP_TokenOnly|EP_برگ)) ){
    sqlite3_اظہاریہ_Delete(db, p_اظہاریہ->p_بایاں);
    p_اظہاریہ->p_بایاں = 0;
    sqlite3_اظہاریہ_Delete(db, p_اظہاریہ->p_دایاں);
    p_اظہاریہ->p_دایاں = 0;
    اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_برگ);
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
  اگر( p_مطابقت ){
    اگر( p_اظہاریہ->i_عمود>=0 ){
      p_مطابقت->ستون_مستعمل |= sqlite3_اظہاریہ_ستون_مستعمل(p_اظہاریہ);
    }ورنہ{
      p_مطابقت->fg.rowid_مستعمل = 1;
    }
  }

  p_اظہاریہ->op = e_New_اظہاریہ_Op;
lookupname_انتہا:
  اگر( cnt==1 ){
    أكد( pNC!=0 );
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
    اگر( db->xAuth ){
      اگر( pFJMatch ){
        أكد( p_اظہاریہ->op==TK_FUNCTION );
        أكد( sqlite3_stricmp(p_اظہاریہ->u.zToken,"coalesce")==0 );
        أكد( p_اظہاریہ->x.pList==pFJMatch );
        أكد( pFJMatch->n_اظہاریہ>0 );
        برائے(i=0; i<pFJMatch->n_اظہاریہ; i++){
          أكد( pFJMatch->a[i].p_اظہاریہ->op==TK_عمود );
          sqlite3AuthRead(pParse,pFJMatch->a[i].p_اظہاریہ,pSchema,pNC->pSrcList);
        }
      }ورنہ اگر( p_اظہاریہ->op==TK_عمود || p_اظہاریہ->op==TK_TRIGGER ){
        sqlite3AuthRead(pParse, p_اظہاریہ, pSchema, pNC->pSrcList);
      }
    } 
#اگر_ختم
    /* Increment the nRef value on all name contexts from TopNC up to
    ** the point where the name matched. */
    برائے(;;){
      أكد( p_چوٹی_NC!=0 );
      p_چوٹی_NC->nRef++;
      اگر( p_چوٹی_NC==pNC ) توڑو;
      p_چوٹی_NC = p_چوٹی_NC->p_اگلا;
    }
    واپس WRC_Prune;
  } ورنہ {
    واپس WRC_Abort;
  }
}

/*
** Allocate and return a pointer to an expression to load the column iCol
** from datasource iSrc in SrcList pSrc.
*/
اظہاریہ *sqlite3_Create_عمود_اظہاریہ(sqlite3 *db, SrcList *pSrc, صحیح iSrc, صحیح i_ستون){
  اظہاریہ *p = sqlite3_اظہاریہ_Alloc(db, TK_عمود, 0, 0);
  اگر( p ){
    Src_شے *p_شے = &pSrc->a[iSrc];
    جدول *pTab;
    أكد( اظہاریہ_Use_YTab(p) );
    pTab = p->y.pTab = p_شے->pSTab;
    p->i_جدول = p_شے->iCursor;
    اگر( p->y.pTab->iPKey==i_ستون ){
      p->i_عمود = -1;
    }ورنہ{
      p->i_عمود = (ynVar)i_ستون;
      اگر( (pTab->tabFlags & TF_HasGenerated)!=0
       && (pTab->a_ستون[i_ستون].ستون_Flags & COLFLAG_GENERATED)!=0
      ){
        testcase( pTab->n_ستون==63 );
        testcase( pTab->n_ستون==64 );
        p_شے->ستون_مستعمل = pTab->n_ستون>=64 ? ALLBITS : MASKBIT(pTab->n_ستون)-1;
      }ورنہ{
        testcase( i_ستون==BMS );
        testcase( i_ستون==BMS-1 );
        p_شے->ستون_مستعمل |= ((Bitmask)1)<<(i_ستون>=BMS ? BMS-1 : i_ستون);
      }
    }
  }
  واپس p;
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
ساکن خالی notValidImpl(
   Parse *pParse,       /* Leave error message here */
   NameContext *pNC,    /* The name context */
   ثابت حرف *z_پیغام,    /* Type of error */
   اظہاریہ *p_اظہاریہ,         /* Invalidate this expression on error */
   اظہاریہ *pError         /* Associate error with this expression */
){
  ثابت حرف *z_اندر = "partial index WHERE clauses";
  اگر( pNC->ncFlags & NC_Idx_اظہاریہ )      z_اندر = "index expressions";
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CHECK
  ورنہ اگر( pNC->ncFlags & NC_IsCheck ) z_اندر = "CHECK constraints";
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_GENERATED_COLUMNS
  ورنہ اگر( pNC->ncFlags & NC_Gen_ستون ) z_اندر = "generated columns";
#اگر_ختم
  sqlite3_Error_پیغام(pParse, "%s prohibited in %s", z_پیغام, z_اندر);
  اگر( p_اظہاریہ ) p_اظہاریہ->op = TK_NULL;
  sqlite3_ریکارڈ_Error_انحراف_کا_اظہاریہ(pParse->db, pError);
}
#تعریف_کرو sqlite3ResolveNotValid(P,N,M,X,E,R) \
  أكد( ((X)&~(NC_IsCheck|NC_PartIdx|NC_Idx_اظہاریہ|NC_Gen_ستون))==0 ); \
  اگر( ((N)->ncFlags & (X))!=0 ) notValidImpl(P,N,M,E,R);

/*
** Expression p should encode a floating point value between 1.0 and 0.0.
** Return 134,217,728 (2^27) times this value.  Or return -1 if p is not
** a floating point value between 1.0 and 0.0.
*/
ساکن صحیح اظہاریہ_Probability(اظہاریہ *p){
  دوہرا r = -1.0;
  اگر( p->op!=TK_FLOAT ) واپس -1;
  أكد( !اظہاریہ_Has_Property(p, EP_Int_قدر) );
  sqlite3AtoF(p->u.zToken, &r);
  أكد( r>=0.0 );
  اگر( r>1.0 ) واپس -1;
  واپس (صحیح)(r*134217728.0);
}

/*
** Set the EP_SubtArg property on every expression inside of
** pList.  If any subexpression is actually a subquery, then
** also set the EP_SubtArg property on the first result-set
** column of that subquery.
*/
ساکن SQLITE_NOINLINE خالی resolve_سیٹ_اظہاریہ_Subtype_Arg(اظہاریہ_List *pList){
  صحیح nn, ii;
  nn = pList ? pList->n_اظہاریہ : 0;
  برائے(ii=0; ii<nn; ii++){
    اظہاریہ *p_اظہاریہ = pList->a[ii].p_اظہاریہ;
    جب( 1 /*exit-by-break*/ ){
      اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_SubtArg);
      اگر( p_اظہاریہ->op==TK_SELECT ){
        أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
        أكد( p_اظہاریہ->x.pSelect!=0 );
        resolve_سیٹ_اظہاریہ_Subtype_Arg(p_اظہاریہ->x.pSelect->pEList);
        توڑو;
      }
      اگر( p_اظہاریہ->op==TK_UPLUS ){
        p_اظہاریہ = p_اظہاریہ->p_بایاں;
        أكد( p_اظہاریہ!=0 );
      }ورنہ{
        توڑو;
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
ساکن صحیح resolve_اظہاریہ_قدم(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  NameContext *pNC;
  Parse *pParse;

  pNC = pWalker->u.pNC;
  أكد( pNC!=0 );
  pParse = pNC->pParse;
  أكد( pParse==pWalker->pParse );

#اگر_غیر_تعریف_شدہ NDEBUG
  اگر( pNC->pSrcList && pNC->pSrcList->nAlloc>0 ){
    SrcList *pSrcList = pNC->pSrcList;
    صحیح i;
    برائے(i=0; i<pNC->pSrcList->nSrc; i++){
      أكد( pSrcList->a[i].iCursor>=0 && pSrcList->a[i].iCursor<pParse->nTab);
    }
  }
#اگر_ختم
  انتخاب( p_اظہاریہ->op ){

    /* The special operator TK_ROW means use the rowid for the first
    ** column in the FROM clause.  This is used by the LIMIT and ORDER BY
    ** clause processing on UPDATE and DELETE statements, and by
    ** UPDATE ... FROM statement processing.
    */
    حالت TK_صف: {
      SrcList *pSrcList = pNC->pSrcList;
      Src_شے *p_شے;
      أكد( pSrcList && pSrcList->nSrc>=1 );
      p_شے = pSrcList->a;
      p_اظہاریہ->op = TK_عمود;
      أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
      p_اظہاریہ->y.pTab = p_شے->pSTab;
      p_اظہاریہ->i_جدول = p_شے->iCursor;
      p_اظہاریہ->i_عمود--;
      p_اظہاریہ->aff_اظہاریہ = SQLITE_AFF_INTEGER;
      توڑو;
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
    حالت TK_NOTNULL:
    حالت TK_ISNULL: {
      صحیح anRef[8];
      NameContext *p;
      صحیح i;
      برائے(i=0, p=pNC; p && i<Array_سائز(anRef); p=p->p_اگلا, i++){
        anRef[i] = p->nRef;
      }
      sqlite3_Walk_اظہاریہ(pWalker, p_اظہاریہ->p_بایاں);
      اگر( اندر_RENAME_OBJECT ) واپس WRC_Prune;
      اگر( sqlite3_اظہاریہ_Can_Be_Null(p_اظہاریہ->p_بایاں) ){
        /* The expression can be NULL.  So the optimization does not apply */
        واپس WRC_Prune;
      }

      برائے(i=0, p=pNC; p; p=p->p_اگلا, i++){
        اگر( (p->ncFlags & NC_Where)==0 ){
          واپس WRC_Prune;  /* Not in a WHERE clause.  Unsafe to optimize. */
        }
      }
      testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) );
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
#اگر TREETRACE_ENABLED
      اگر( sqlite3_درخت_Trace & 0x80000 ){
        sqlite3DebugPrintf(
           "NOT NULL strength reduction converts the following to %d:\n",
           p_اظہاریہ->op==TK_NOTNULL
        );
        sqlite3_Show_اظہاریہ(p_اظہاریہ);
      }
#اگر_ختم /* TREETRACE_ENABLED */
      p_اظہاریہ->u.i_قدر = (p_اظہاریہ->op==TK_NOTNULL);
      p_اظہاریہ->flags |= EP_Int_قدر;
      p_اظہاریہ->op = TK_INTEGER;
      برائے(i=0, p=pNC; p && i<Array_سائز(anRef); p=p->p_اگلا, i++){
        p->nRef = anRef[i];
      }
      sqlite3_اظہاریہ_Delete(pParse->db, p_اظہاریہ->p_بایاں);
      p_اظہاریہ->p_بایاں = 0;
      واپس WRC_Prune;
    }

    /* A column name:                    ID
    ** Or table name and column name:    ID.ID
    ** Or a database, table and column:  ID.ID.ID
    **
    ** The TK_ID and TK_OUT cases are combined so that there will only
    ** be one call to lookupName().  Then the compiler will in-line
    ** lookupName() for a size reduction and performance increase.
    */
    حالت TK_ID:
    حالت TK_DOT: {
      ثابت حرف *z_جدول;
      ثابت حرف *zDb;
      اظہاریہ *p_دایاں;

      اگر( p_اظہاریہ->op==TK_ID ){
        zDb = 0;
        z_جدول = 0;
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
        p_دایاں = p_اظہاریہ;
      }ورنہ{
        اظہاریہ *p_بایاں = p_اظہاریہ->p_بایاں;
        testcase( pNC->ncFlags & NC_Idx_اظہاریہ );
        testcase( pNC->ncFlags & NC_Gen_ستون );
        sqlite3ResolveNotValid(pParse, pNC, "the \".\" operator",
                               NC_Idx_اظہاریہ|NC_Gen_ستون, 0, p_اظہاریہ);
        p_دایاں = p_اظہاریہ->p_دایاں;
        اگر( p_دایاں->op==TK_ID ){
          zDb = 0;
        }ورنہ{
          أكد( p_دایاں->op==TK_DOT );
          أكد( !اظہاریہ_Has_Property(p_دایاں, EP_Int_قدر) );
          zDb = p_بایاں->u.zToken;
          p_بایاں = p_دایاں->p_بایاں;
          p_دایاں = p_دایاں->p_دایاں;
        }
        أكد( اظہاریہ_Use_UToken(p_بایاں) && اظہاریہ_Use_UToken(p_دایاں) );
        z_جدول = p_بایاں->u.zToken;
        أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
        اگر( اندر_RENAME_OBJECT ){
          sqlite3RenameTokenRemap(pParse, (خالی*)p_اظہاریہ, (خالی*)p_دایاں);
          sqlite3RenameTokenRemap(pParse, (خالی*)&p_اظہاریہ->y.pTab, (خالی*)p_بایاں);
        }
      }
      واپس lookupName(pParse, zDb, z_جدول, p_دایاں, pNC, p_اظہاریہ);
    }

    /* Resolve function names
    */
    حالت TK_FUNCTION: {
      اظہاریہ_List *pList;            /* The argument list */
      صحیح n;                      /* Number of arguments */
      صحیح no_such_func = 0;       /* True if no such function exists */
      صحیح wrong_عدد_دلائل = 0;     /* True if wrong number of arguments */
      صحیح is_agg = 0;             /* True if is an aggregate function */
      ثابت حرف *zId;            /* The function name. */
      FuncDef *pDef;              /* Information about the function */
      u8 enc = ENC(pParse->db);   /* The database encoding */
      صحیح savedAllowFlags = (pNC->ncFlags & (NC_AllowAgg | NC_AllowWin));
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      کھڑکی *pWin = (Is_کھڑکی_Func(p_اظہاریہ) ? p_اظہاریہ->y.pWin : 0);
#اگر_ختم
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_xIsSelect|EP_Int_قدر) );
      أكد( p_اظہاریہ->p_بایاں==0 || p_اظہاریہ->p_بایاں->op==TK_ترتیب );
      pList = p_اظہاریہ->x.pList;
      n = pList ? pList->n_اظہاریہ : 0;
      zId = p_اظہاریہ->u.zToken;
      pDef = sqlite3_ڈھونڈو_Function(pParse->db, zId, n, enc, 0);
      اگر( pDef==0 ){
        pDef = sqlite3_ڈھونڈو_Function(pParse->db, zId, -2, enc, 0);
        اگر( pDef==0 ){
          no_such_func = 1;
        }ورنہ{
          wrong_عدد_دلائل = 1;
        }
      }ورنہ{
        is_agg = pDef->xFinalize!=0;
        اگر( pDef->funcFlags & SQLITE_FUNC_UNLIKELY ){
          اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_Unlikely);
          اگر( n==2 ){
            p_اظہاریہ->i_جدول = اظہاریہ_Probability(pList->a[1].p_اظہاریہ);
            اگر( p_اظہاریہ->i_جدول<0 ){
              sqlite3_Error_پیغام(pParse,
                "second argument to %#T() must be a "
                "constant between 0.0 and 1.0", p_اظہاریہ);
              pNC->nNcErr++;
            }
          }ورنہ{
            /* EVIDENCE-OF: R-61304-29449 The unlikely(X) function is
            ** equivalent to likelihood(X, 0.0625).
            ** EVIDENCE-OF: R-01283-11636 The unlikely(X) function is
            ** short-hand for likelihood(X,0.0625).
            ** EVIDENCE-OF: R-36850-34127 The likely(X) function is short-hand
            ** for likelihood(X,0.9375).
            ** EVIDENCE-OF: R-53436-40973 The likely(X) function is equivalent
            ** to likelihood(X,0.9375). */
            /* TUNING: unlikely() probability is 0.0625.  likely() is 0.9375 */
            p_اظہاریہ->i_جدول = pDef->zName[0]=='u' ? 8388608 : 125829120;
          }            
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_AUTHORIZATION
        {
          صحیح auth = sqlite3AuthCheck(pParse, SQLITE_FUNCTION, 0,pDef->zName,0);
          اگر( auth!=SQLITE_OK ){
            اگر( auth==SQLITE_DENY ){
              sqlite3_Error_پیغام(pParse, "not authorized to use function: %#T",
                                      p_اظہاریہ);
              pNC->nNcErr++;
            }
            p_اظہاریہ->op = TK_NULL;
            واپس WRC_Prune;
          }
        }
#اگر_ختم

        /* If the function may call sqlite3_value_subtype(), then set the 
        ** EP_SubtArg flag on all of its argument expressions. This prevents
        ** where.c from replacing the expression with a value read from an
        ** index on the same expression, which will not have the correct 
        ** subtype. Also set the flag if the function expression itself is
        ** an EP_SubtArg expression. In this case subtypes are required as 
        ** the function may return a value with a subtype back to its 
        ** caller using sqlite3_result_value().  */
        اگر( (pDef->funcFlags & SQLITE_SUBTYPE) 
         || اظہاریہ_Has_Property(p_اظہاریہ, EP_SubtArg) 
        ){
          resolve_سیٹ_اظہاریہ_Subtype_Arg(pList);
        }

        اگر( pDef->funcFlags & (SQLITE_FUNC_CONSTANT|SQLITE_FUNC_SLOCHNG) ){
          /* For the purposes of the EP_ConstFunc flag, date and time
          ** functions and other functions that change slowly are considered
          ** constant because they are constant for the duration of one query.
          ** This allows them to be factored out of inner loops. */
          اظہاریہ_سیٹ_Property(p_اظہاریہ,EP_ConstFunc);
        }
        اگر( (pDef->funcFlags & SQLITE_FUNC_CONSTANT)==0 ){
          /* Clearly non-deterministic functions like random(), but also
          ** date/time functions that use 'now', and other functions like
          ** sqlite_version() that might change over time cannot be used
          ** in an index or generated column.  Curiously, they can be used
          ** in a CHECK constraint.  SQLServer, MySQL, and PostgreSQL all
          ** allow this. */
          sqlite3ResolveNotValid(pParse, pNC, "non-deterministic functions",
                                 NC_Idx_اظہاریہ|NC_PartIdx|NC_Gen_ستون, 0, p_اظہاریہ);
        }ورنہ{
          أكد( (NC_SelfRef & 0xff)==NC_SelfRef ); /* Must fit in 8 bits */
          p_اظہاریہ->op2 = pNC->ncFlags & NC_SelfRef;
        }
        اگر( (pDef->funcFlags & SQLITE_FUNC_اندرونی)!=0
         && pParse->nested==0
         && (pParse->db->mDbFlags & DBFLAG_اندرونی_Func)==0
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
        }ورنہ
        اگر( (pDef->funcFlags & (SQLITE_FUNC_براہ_راست|SQLITE_FUNC_UNSAFE))!=0
         && !اندر_RENAME_OBJECT
        ){
          اگر( pNC->ncFlags & NC_FromDDL ) اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_FromDDL);
          sqlite3_اظہاریہ_Function_Usable(pParse, p_اظہاریہ, pDef);
        }
      }

      اگر( 0==اندر_RENAME_OBJECT ){
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
        أكد( is_agg==0 || (pDef->funcFlags & SQLITE_FUNC_MINMAX)
          || (pDef->x_قدر==0 && pDef->xInverse==0)
          || (pDef->x_قدر && pDef->xInverse && pDef->xSFunc && pDef->xFinalize)
        );
        اگر( pDef && pDef->x_قدر==0 && pWin ){
          sqlite3_Error_پیغام(pParse,
              "%#T() may not be used as a window function", p_اظہاریہ
          );
          pNC->nNcErr++;
        }ورنہ اگر(
              (is_agg && (pNC->ncFlags & NC_AllowAgg)==0)
           || (is_agg && (pDef->funcFlags&SQLITE_FUNC_کھڑکی) && !pWin)
           || (is_agg && pWin && (pNC->ncFlags & NC_AllowWin)==0)
        ){
          ثابت حرف *zType;
          اگر( (pDef->funcFlags & SQLITE_FUNC_کھڑکی) || pWin ){
            zType = "window";
          }ورنہ{
            zType = "aggregate";
          }
          sqlite3_Error_پیغام(pParse, "misuse of %s function %#T()",zType,p_اظہاریہ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#ورنہ
        اگر( (is_agg && (pNC->ncFlags & NC_AllowAgg)==0) ){
          sqlite3_Error_پیغام(pParse,"misuse of aggregate function %#T()",p_اظہاریہ);
          pNC->nNcErr++;
          is_agg = 0;
        }
#اگر_ختم
        ورنہ اگر( no_such_func
              && (pParse->db->init.busy==0 ||
                  (no_such_func==2 && pParse->db->init.busy==2))
              /* Suppress "no such function" errors when reading
              ** the sqlite_schema table.  Except, do raise the error
              ** if init.busy is 2, meaning the schema parse is due
              ** to an ALTER TABLE ADD COLUMN statement, and the function
              ** is an internal-use-only function (no_such_func==2). */
#اگر_تعریف_شدہ SQLITE_ENABLE_UNKNOWN_SQL_FUNCTION
              && pParse->explain==0
#اگر_ختم
        ){
          sqlite3_Error_پیغام(pParse, "no such function: %#T", p_اظہاریہ);
          pNC->nNcErr++;
        }ورنہ اگر( wrong_عدد_دلائل ){
          sqlite3_Error_پیغام(pParse,"wrong number of arguments to function %#T()",
               p_اظہاریہ);
          pNC->nNcErr++;
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
        ورنہ اگر( is_agg==0 && اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
          sqlite3_Error_پیغام(pParse,
              "FILTER may not be used with non-aggregate %#T()",
              p_اظہاریہ
          );
          pNC->nNcErr++;
        }
#اگر_ختم
        ورنہ اگر( is_agg==0 && p_اظہاریہ->p_بایاں ){
          sqlite3_اظہاریہ_ترتیب_By_Aggregate_Error(pParse, p_اظہاریہ);
          pNC->nNcErr++;
        }
        اگر( is_agg ){
          /* Window functions may not be arguments of aggregate functions.
          ** Or arguments of other window functions. But aggregate functions
          ** may be arguments for window functions.  */
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
          pNC->ncFlags &= ~(NC_AllowWin | (!pWin ? NC_AllowAgg : 0));
#ورنہ
          pNC->ncFlags &= ~NC_AllowAgg;
#اگر_ختم
        }
      }
      ورنہ اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) || p_اظہاریہ->p_بایاں ){
        is_agg = 1;
      }
      sqlite3_Walk_اظہاریہ_List(pWalker, pList);
      اگر( is_agg ){
        اگر( p_اظہاریہ->p_بایاں ){
          أكد( p_اظہاریہ->p_بایاں->op==TK_ترتیب );
          أكد( اظہاریہ_Use_XList(p_اظہاریہ->p_بایاں) );
          sqlite3_Walk_اظہاریہ_List(pWalker, p_اظہاریہ->p_بایاں->x.pList);
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
        اگر( pWin && pParse->nErr==0 ){
          Select *pSel = pNC->pWinSelect;
          أكد( اظہاریہ_Use_YWin(p_اظہاریہ) && pWin==p_اظہاریہ->y.pWin );
          اگر( اندر_RENAME_OBJECT==0 ){
            sqlite3_کھڑکی_Update(pParse, pSel ? pSel->pWinDefn : 0, pWin, pDef);
            اگر( pParse->db->mallocFailed ) توڑو;
          }
          sqlite3_Walk_اظہاریہ_List(pWalker, pWin->p_تقسیم);
          sqlite3_Walk_اظہاریہ_List(pWalker, pWin->p_ترتیب_By);
          sqlite3_Walk_اظہاریہ(pWalker, pWin->pFilter);
          sqlite3_کھڑکی_Link(pSel, pWin);
          pNC->ncFlags |= NC_HasWin;
        }ورنہ
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */
        {
          NameContext *pNC2;          /* For looping up thru outer contexts */
          p_اظہاریہ->op = TK_AGG_FUNCTION;
          p_اظہاریہ->op2 = 0;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
          اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
            sqlite3_Walk_اظہاریہ(pWalker, p_اظہاریہ->y.pWin->pFilter);
          }
#اگر_ختم
          pNC2 = pNC;
          جب( pNC2
              && sqlite3ReferencesSrcList(pParse, p_اظہاریہ, pNC2->pSrcList)==0
          ){
            p_اظہاریہ->op2 += (1 + pNC2->nNestedSelect);
            pNC2 = pNC2->p_اگلا;
          }
          أكد( pDef!=0 || اندر_RENAME_OBJECT );
          اگر( pNC2 && pDef ){
            p_اظہاریہ->op2 += pNC2->nNestedSelect;
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
      واپس WRC_Prune;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
    حالت TK_EXISTS:
    حالت TK_SELECT:
#اگر_ختم
    حالت TK_اندر: {
      testcase( p_اظہاریہ->op==TK_اندر );
      testcase( p_اظہاریہ->op==TK_EXISTS );
      testcase( p_اظہاریہ->op==TK_SELECT );
      اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
        صحیح nRef = pNC->nRef;
        testcase( pNC->ncFlags & NC_IsCheck );
        testcase( pNC->ncFlags & NC_PartIdx );
        testcase( pNC->ncFlags & NC_Idx_اظہاریہ );
        testcase( pNC->ncFlags & NC_Gen_ستون );
        أكد( p_اظہاریہ->x.pSelect );
        اگر( p_اظہاریہ->op==TK_EXISTS )  pParse->bHasExists = 1;
        اگر( pNC->ncFlags & NC_SelfRef ){
          notValidImpl(pParse, pNC, "subqueries", p_اظہاریہ, p_اظہاریہ);
        }ورنہ{
          sqlite3WalkSelect(pWalker, p_اظہاریہ->x.pSelect);
        }
        أكد( pNC->nRef>=nRef );
        اگر( nRef!=pNC->nRef ){
          اظہاریہ_سیٹ_Property(p_اظہاریہ, EP_VarSelect);
          p_اظہاریہ->x.pSelect->selFlags |= SF_Correlated;
        }
        pNC->ncFlags |= NC_Subquery;
      }
      توڑو;
    }
    حالت TK_VARIABLE: {
      testcase( pNC->ncFlags & NC_IsCheck );
      testcase( pNC->ncFlags & NC_PartIdx );
      testcase( pNC->ncFlags & NC_Idx_اظہاریہ );
      testcase( pNC->ncFlags & NC_Gen_ستون );
      sqlite3ResolveNotValid(pParse, pNC, "parameters",
               NC_IsCheck|NC_PartIdx|NC_Idx_اظہاریہ|NC_Gen_ستون, p_اظہاریہ, p_اظہاریہ);
      توڑو;
    }
    حالت TK_IS:
    حالت TK_ISNOT: {
      اظہاریہ *p_دایاں = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ->p_دایاں);
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Reduced) );
      /* Handle special cases of "x IS TRUE", "x IS FALSE", "x IS NOT TRUE",
      ** and "x IS NOT FALSE". */
      اگر( ALWAYS(p_دایاں) && (p_دایاں->op==TK_ID || p_دایاں->op==TK_TRUEFALSE) ){
        صحیح rc = resolve_اظہاریہ_قدم(pWalker, p_دایاں);
        اگر( rc==WRC_Abort ) واپس WRC_Abort;
        اگر( p_دایاں->op==TK_TRUEFALSE ){
          p_اظہاریہ->op2 = p_اظہاریہ->op;
          p_اظہاریہ->op = TK_TRUTH;
          واپس WRC_Continue;
        }
      }
      /* no break */ deliberate_fall_through
    }
    حالت TK_BETWEEN:
    حالت TK_EQ:
    حالت TK_NE:
    حالت TK_LT:
    حالت TK_LE:
    حالت TK_GT:
    حالت TK_GE: {
      صحیح n_بایاں, n_دایاں;
      اگر( pParse->db->mallocFailed ) توڑو;
      أكد( p_اظہاریہ->p_بایاں!=0 );
      n_بایاں = sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ->p_بایاں);
      اگر( p_اظہاریہ->op==TK_BETWEEN ){
        أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
        n_دایاں = sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ->x.pList->a[0].p_اظہاریہ);
        اگر( n_دایاں==n_بایاں ){
          n_دایاں = sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ->x.pList->a[1].p_اظہاریہ);
        }
      }ورنہ{
        أكد( p_اظہاریہ->p_دایاں!=0 );
        n_دایاں = sqlite3_اظہاریہ_سمتیہ_سائز(p_اظہاریہ->p_دایاں);
      }
      اگر( n_بایاں!=n_دایاں ){
        testcase( p_اظہاریہ->op==TK_EQ );
        testcase( p_اظہاریہ->op==TK_NE );
        testcase( p_اظہاریہ->op==TK_LT );
        testcase( p_اظہاریہ->op==TK_LE );
        testcase( p_اظہاریہ->op==TK_GT );
        testcase( p_اظہاریہ->op==TK_GE );
        testcase( p_اظہاریہ->op==TK_IS );
        testcase( p_اظہاریہ->op==TK_ISNOT );
        testcase( p_اظہاریہ->op==TK_BETWEEN );
        sqlite3_Error_پیغام(pParse, "row value misused");
        sqlite3_ریکارڈ_Error_انحراف_کا_اظہاریہ(pParse->db, p_اظہاریہ);
      }
      توڑو;
    }
  }
  أكد( pParse->db->mallocFailed==0 || pParse->nErr!=0 );
  واپس pParse->nErr ? WRC_Abort : WRC_Continue;
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
ساکن صحیح resolveAsName(
  Parse *pParse,     /* Parsing context for error messages */
  اظہاریہ_List *pEList,  /* List of expressions to scan */
  اظہاریہ *pE           /* Expression we are trying to match */
){
  صحیح i;             /* Loop counter */

  غیر_مستعمل_PARAMETER(pParse);

  اگر( pE->op==TK_ID ){
    ثابت حرف *z_ستون;
    أكد( !اظہاریہ_Has_Property(pE, EP_Int_قدر) );
    z_ستون = pE->u.zToken;
    برائے(i=0; i<pEList->n_اظہاریہ; i++){
      اگر( pEList->a[i].fg.eEName==ENAME_NAME
       && sqlite3_stricmp(pEList->a[i].zEName, z_ستون)==0
      ){
        واپس i+1;
      }
    }
  }
  واپس 0;
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
ساکن صحیح resolve_ترتیب_By_رکن_مقصد_اظہاریہ_List(
  Parse *pParse,     /* Parsing context for error messages */
  Select *pSelect,   /* The SELECT statement with the ORDER BY clause */
  اظہاریہ *pE           /* The specific ORDER BY term */
){
  صحیح i;             /* Loop counter */
  اظہاریہ_List *pEList;  /* The columns of the result set */
  NameContext nc;    /* Name context for resolving pE */
  sqlite3 *db;       /* Database connection */
  صحیح rc;            /* Return code from subprocedures */
  u8 savedSuppErr;   /* Saved value of db->suppressErr */

  أكد( sqlite3_اظہاریہ_Is_Integer(pE, &i, 0, 1)==0 );
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
  rc = sqlite3_Resolve_اظہاریہ_نام(&nc, pE);
  db->suppressErr = savedSuppErr;
  اگر( rc ) واپس 0;

  /* Try to match the ORDER BY expression against an expression
  ** in the result set.  Return an 1-based index of the matching
  ** result-set entry.
  */
  برائے(i=0; i<pEList->n_اظہاریہ; i++){
    اگر( sqlite3_اظہاریہ_Compare(0, pEList->a[i].p_اظہاریہ, pE, -1)<2 ){
      واپس i+1;
    }
  }

  /* If no match, return 0. */
  واپس 0;
}

/*
** Generate an ORDER BY or GROUP BY term out-of-range error.
*/
ساکن خالی resolve_برآمد_کا_Range_Error(
  Parse *pParse,         /* The error context into which to write the error */
  ثابت حرف *zType,     /* "ORDER" or "GROUP" */
  صحیح i,                 /* The index (1-based) of the term out of range */
  صحیح mx,                /* Largest permissible value of i */
  اظہاریہ *pError           /* Associate the error with the expression */
){
  sqlite3_Error_پیغام(pParse,
    "%r %s BY term out of range - should be "
    "between 1 and %d", i, zType, mx);
  sqlite3_ریکارڈ_Error_انحراف_کا_اظہاریہ(pParse->db, pError);
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
ساکن صحیح resolve_Compound_ترتیب_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect       /* The SELECT statement containing the ORDER BY */
){
  صحیح i;
  اظہاریہ_List *p_ترتیب_By;
  اظہاریہ_List *pEList;
  sqlite3 *db;
  صحیح مزید_مقصد_Do = 1;

  p_ترتیب_By = pSelect->p_ترتیب_By;
  اگر( p_ترتیب_By==0 ) واپس 0;
  db = pParse->db;
  اگر( p_ترتیب_By->n_اظہاریہ>db->a_حد_بندی[SQLITE_حد_بندی_عمود] ){
    sqlite3_Error_پیغام(pParse, "too many terms in ORDER BY clause");
    واپس 1;
  }
  برائے(i=0; i<p_ترتیب_By->n_اظہاریہ; i++){
    p_ترتیب_By->a[i].fg.مکمل = 0;
  }
  pSelect->p_اگلا = 0;
  جب( pSelect->p_پیشگی ){
    pSelect->p_پیشگی->p_اگلا = pSelect;
    pSelect = pSelect->p_پیشگی;
  }
  جب( pSelect && مزید_مقصد_Do ){
    ساخت اظہاریہ_List_شے *p_شے;
    مزید_مقصد_Do = 0;
    pEList = pSelect->pEList;
    أكد( pEList!=0 );
    برائے(i=0, p_شے=p_ترتیب_By->a; i<p_ترتیب_By->n_اظہاریہ; i++, p_شے++){
      صحیح i_ستون = -1;
      اظہاریہ *pE, *pDup;
      اگر( p_شے->fg.مکمل ) جاری;
      pE = sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_شے->p_اظہاریہ);
      اگر( NEVER(pE==0) ) جاری;
      اگر( sqlite3_اظہاریہ_Is_Integer(pE, &i_ستون, 0, 1) ){
        اگر( i_ستون<=0 || i_ستون>pEList->n_اظہاریہ ){
          resolve_برآمد_کا_Range_Error(pParse, "ORDER", i+1, pEList->n_اظہاریہ, pE);
          واپس 1;
        }
      }ورنہ{
        i_ستون = resolveAsName(pParse, pEList, pE);
        اگر( i_ستون==0 ){
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
          pDup = sqlite3_اظہاریہ_Dup(db, pE, 0);
          اگر( !db->mallocFailed ){
            أكد(pDup);
            i_ستون = resolve_ترتیب_By_رکن_مقصد_اظہاریہ_List(pParse, pSelect, pDup);
            اگر( اندر_RENAME_OBJECT && i_ستون>0 ){
              resolve_ترتیب_By_رکن_مقصد_اظہاریہ_List(pParse, pSelect, pE);
            }
          }
          sqlite3_اظہاریہ_Delete(db, pDup);
        }
      }
      اگر( i_ستون>0 ){
        /* Convert the ORDER BY term into an integer column number iCol,
        ** taking care to preserve the COLLATE clause if it exists. */
        اگر( !اندر_RENAME_OBJECT ){
          اظہاریہ *pNew = sqlite3_اظہاریہ_Int32(db, i_ستون);
          اگر( pNew==0 ) واپس 1;
          اگر( p_شے->p_اظہاریہ==pE ){
            p_شے->p_اظہاریہ = pNew;
          }ورنہ{
            اظہاریہ *p_والد = p_شے->p_اظہاریہ;
            أكد( p_والد->op==TK_COLLATE );
            جب( p_والد->p_بایاں->op==TK_COLLATE ) p_والد = p_والد->p_بایاں;
            أكد( p_والد->p_بایاں==pE );
            p_والد->p_بایاں = pNew;
          }
          sqlite3_اظہاریہ_Delete(db, pE);
          p_شے->u.x.i_ترتیب_By_ستون = (u16)i_ستون;
        }
        p_شے->fg.مکمل = 1;
      }ورنہ{
        مزید_مقصد_Do = 1;
      }
    }
    pSelect = pSelect->p_اگلا;
  }
  برائے(i=0; i<p_ترتیب_By->n_اظہاریہ; i++){
    اگر( p_ترتیب_By->a[i].fg.مکمل==0 ){
      sqlite3_Error_پیغام(pParse, "%r ORDER BY term does not match any "
            "column in the result set", i+1);
      واپس 1;
    }
  }
  واپس 0;
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
صحیح sqlite3_Resolve_ترتیب_گروہ_By(
  Parse *pParse,        /* Parsing context.  Leave error messages here */
  Select *pSelect,      /* The SELECT statement containing the clause */
  اظہاریہ_List *p_ترتیب_By,   /* The ORDER BY or GROUP BY clause to be processed */
  ثابت حرف *zType     /* "ORDER" or "GROUP" */
){
  صحیح i;
  sqlite3 *db = pParse->db;
  اظہاریہ_List *pEList;
  ساخت اظہاریہ_List_شے *p_شے;

  اگر( p_ترتیب_By==0 || pParse->db->mallocFailed || اندر_RENAME_OBJECT ) واپس 0;
  اگر( p_ترتیب_By->n_اظہاریہ>db->a_حد_بندی[SQLITE_حد_بندی_عمود] ){
    sqlite3_Error_پیغام(pParse, "too many terms in %s BY clause", zType);
    واپس 1;
  }
  pEList = pSelect->pEList;
  أكد( pEList!=0 );  /* sqlite3SelectNew() guarantees this */
  برائے(i=0, p_شے=p_ترتیب_By->a; i<p_ترتیب_By->n_اظہاریہ; i++, p_شے++){
    اگر( p_شے->u.x.i_ترتیب_By_ستون ){
      اگر( p_شے->u.x.i_ترتیب_By_ستون>pEList->n_اظہاریہ ){
        resolve_برآمد_کا_Range_Error(pParse, zType, i+1, pEList->n_اظہاریہ, 0);
        واپس 1;
      }
      resolveAlias(pParse, pEList, p_شے->u.x.i_ترتیب_By_ستون-1, p_شے->p_اظہاریہ,0);
    }
  }
  واپس 0;
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
/*
** Walker callback for windowRemoveExprFromSelect().
*/
ساکن صحیح resolveRemoveWindowsCb(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  غیر_مستعمل_PARAMETER(pWalker);
  اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
    کھڑکی *pWin = p_اظہاریہ->y.pWin;
    sqlite3_کھڑکی_Unlink_From_Select(pWin);
  }
  واپس WRC_Continue;
}

/*
** Remove any Window objects owned by the expression pExpr from the
** Select.pWin list of Select object pSelect.
*/
ساکن خالی کھڑکی_Remove_اظہاریہ_From_Select(Select *pSelect, اظہاریہ *p_اظہاریہ){
  اگر( pSelect->pWin ){
    Walker sWalker;
    memset(&sWalker, 0, حجم(Walker));
    sWalker.x_اظہاریہ_Callback = resolveRemoveWindowsCb;
    sWalker.u.pSelect = pSelect;
    sqlite3_Walk_اظہاریہ(&sWalker, p_اظہاریہ);
  }
}
#ورنہ
# define کھڑکی_Remove_اظہاریہ_From_Select(a, b)
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */

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
ساکن صحیح resolve_ترتیب_گروہ_By(
  NameContext *pNC,     /* The name context of the SELECT statement */
  Select *pSelect,      /* The SELECT statement holding pOrderBy */
  اظہاریہ_List *p_ترتیب_By,   /* An ORDER BY or GROUP BY clause to resolve */
  ثابت حرف *zType     /* Either "ORDER" or "GROUP", as appropriate */
){
  صحیح i, j;                      /* Loop counters */
  صحیح i_ستون;                      /* Column number */
  ساخت اظہاریہ_List_شے *p_شے;   /* A term of the ORDER BY clause */
  Parse *pParse;                 /* Parsing context */
  صحیح n_نتیجہ;                   /* Number of terms in the result set */

  أكد( p_ترتیب_By!=0 );
  n_نتیجہ = pSelect->pEList->n_اظہاریہ;
  pParse = pNC->pParse;
  برائے(i=0, p_شے=p_ترتیب_By->a; i<p_ترتیب_By->n_اظہاریہ; i++, p_شے++){
    اظہاریہ *pE = p_شے->p_اظہاریہ;
    اظہاریہ *pE2 = sqlite3_اظہاریہ_Skip_Collate_And_Likely(pE);
    اگر( NEVER(pE2==0) ) جاری;
    اگر( zType[0]!='G' ){
      i_ستون = resolveAsName(pParse, pSelect->pEList, pE2);
      اگر( i_ستون>0 ){
        /* If an AS-name match is found, mark this ORDER BY column as being
        ** a copy of the iCol-th result-set column.  The subsequent call to
        ** sqlite3ResolveOrderGroupBy() will convert the expression to a
        ** copy of the iCol-th result-set expression. */
        p_شے->u.x.i_ترتیب_By_ستون = (u16)i_ستون;
        جاری;
      }
    }
    اگر( sqlite3_اظہاریہ_Is_Integer(pE2, &i_ستون, 0, 1) ){
      /* The ORDER BY term is an integer constant.  Again, set the column
      ** number so that sqlite3ResolveOrderGroupBy() will convert the
      ** order-by term to a copy of the result-set expression */
      اگر( i_ستون<1 || i_ستون>0xffff ){
        resolve_برآمد_کا_Range_Error(pParse, zType, i+1, n_نتیجہ, pE2);
        واپس 1;
      }
      p_شے->u.x.i_ترتیب_By_ستون = (u16)i_ستون;
      جاری;
    }

    /* Otherwise, treat the ORDER BY term as an ordinary expression */
    p_شے->u.x.i_ترتیب_By_ستون = 0;
    اگر( sqlite3_Resolve_اظہاریہ_نام(pNC, pE) ){
      واپس 1;
    }
    برائے(j=0; j<pSelect->pEList->n_اظہاریہ; j++){
      اگر( sqlite3_اظہاریہ_Compare(0, pE, pSelect->pEList->a[j].p_اظہاریہ, -1)==0 ){
        /* Since this expression is being changed into a reference
        ** to an identical expression in the result set, remove all Window
        ** objects belonging to the expression from the Select.pWin list. */
        کھڑکی_Remove_اظہاریہ_From_Select(pSelect, pE);
        p_شے->u.x.i_ترتیب_By_ستون = j+1;
      }
    }
  }
  واپس sqlite3_Resolve_ترتیب_گروہ_By(pParse, pSelect, p_ترتیب_By, zType);
}

/*
** Resolve names in the SELECT statement p and all of its descendants.
*/
ساکن صحیح resolve_Select_قدم(Walker *pWalker, Select *p){
  NameContext *pOuterNC;  /* Context that contains this SELECT */
  NameContext sNC;        /* Name context of this SELECT */
  صحیح isCompound;         /* True if p is a compound select */
  صحیح nCompound;          /* Number of compound terms processed so far */
  Parse *pParse;          /* Parsing context */
  صحیح i;                  /* Loop counter */
  اظہاریہ_List *p_گروہ_By;     /* The GROUP BY clause */
  Select *pLeftmost;      /* Left-most of SELECT of a compound */
  sqlite3 *db;            /* Database connection */
 

  أكد( p!=0 );
  اگر( p->selFlags & SF_Resolved ){
    واپس WRC_Prune;
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
  اگر( (p->selFlags & SF_Expanded)==0 ){
    sqlite3SelectPrep(pParse, p, pOuterNC);
    واپس pParse->nErr ? WRC_Abort : WRC_Prune;
  }

  isCompound = p->p_پیشگی!=0;
  nCompound = 0;
  pLeftmost = p;
  جب( p ){
    أكد( (p->selFlags & SF_Expanded)!=0 );
    أكد( (p->selFlags & SF_Resolved)==0 );
    p->selFlags |= SF_Resolved;

    /* Resolve the expressions in the LIMIT and OFFSET clauses. These
    ** are not allowed to refer to any names, so pass an empty NameContext.
    */
    memset(&sNC, 0, حجم(sNC));
    sNC.pParse = pParse;
    sNC.pWinSelect = p;
    اگر( sqlite3_Resolve_اظہاریہ_نام(&sNC, p->p_حد_بندی) ){
      واپس WRC_Abort;
    }

    /* If the SF_Converted flags is set, then this Select object was
    ** was created by the convertCompoundSelectToSubquery() function.
    ** In this case the ORDER BY clause (p->pOrderBy) should be resolved
    ** as if it were part of the sub-query, not the parent. This block
    ** moves the pOrderBy down to the sub-query. It will be moved back
    ** after the names have been resolved.  */
    اگر( p->selFlags & SF_Converted ){
      Select *pSub;
      أكد( p->pSrc->a[0].fg.isSubquery );
      أكد( p->pSrc->a[0].u4.pSubq!=0 );
      pSub = p->pSrc->a[0].u4.pSubq->pSelect;
      أكد( pSub!=0 );
      أكد( p->pSrc->nSrc==1 && p->p_ترتیب_By );
      أكد( pSub->p_پیشگی && pSub->p_ترتیب_By==0 );
      pSub->p_ترتیب_By = p->p_ترتیب_By;
      p->p_ترتیب_By = 0;
    }
 
    /* Recursively resolve names in all subqueries in the FROM clause
    */
    اگر( pOuterNC ) pOuterNC->nNestedSelect++;
    برائے(i=0; i<p->pSrc->nSrc; i++){
      Src_شے *p_شے = &p->pSrc->a[i];
      أكد( p_شے->zName!=0
              || p_شے->fg.isSubquery );  /* Test of tag-20240424-1*/
      اگر( p_شے->fg.isSubquery
       && (p_شے->u4.pSubq->pSelect->selFlags & SF_Resolved)==0
      ){
        صحیح nRef = pOuterNC ? pOuterNC->nRef : 0;
        ثابت حرف *zSavedContext = pParse->zAuthContext;

        اگر( p_شے->zName ) pParse->zAuthContext = p_شے->zName;
        sqlite3_Resolve_Select_نام(pParse, p_شے->u4.pSubq->pSelect, pOuterNC);
        pParse->zAuthContext = zSavedContext;
        اگر( pParse->nErr ) واپس WRC_Abort;
        أكد( db->mallocFailed==0 );

        /* If the number of references to the outer context changed when
        ** expressions in the sub-select were resolved, the sub-select
        ** is correlated. It is not required to check the refcount on any
        ** but the innermost outer context object, as lookupName() increments
        ** the refcount on all contexts between the current one and the
        ** context containing the column when it resolves a name. */
        اگر( pOuterNC ){
          أكد( p_شے->fg.isCorrelated==0 && pOuterNC->nRef>=nRef );
          p_شے->fg.isCorrelated = (pOuterNC->nRef>nRef);
        }
      }
    }
    اگر( pOuterNC && ALWAYS(pOuterNC->nNestedSelect>0) ){
      pOuterNC->nNestedSelect--;
    }
 
    /* Set up the local name-context to pass to sqlite3ResolveExprNames() to
    ** resolve the result-set expression list.
    */
    sNC.ncFlags = NC_AllowAgg|NC_AllowWin;
    sNC.pSrcList = p->pSrc;
    sNC.p_اگلا = pOuterNC;
 
    /* Resolve names in the result set. */
    اگر( sqlite3_Resolve_اظہاریہ_List_نام(&sNC, p->pEList) ) واپس WRC_Abort;
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* If there are no aggregate functions in the result-set, and no GROUP BY
    ** expression, do not allow aggregates in any of the other expressions.
    */
    أكد( (p->selFlags & SF_Aggregate)==0 );
    p_گروہ_By = p->p_گروہ_By;
    اگر( p_گروہ_By || (sNC.ncFlags & NC_HasAgg)!=0 ){
      أكد( NC_MinMaxAgg==SF_MinMaxAgg );
      أكد( NC_ترتیب_Agg==SF_ترتیب_By_Reqd );
      p->selFlags |= SF_Aggregate | (sNC.ncFlags&(NC_MinMaxAgg|NC_ترتیب_Agg));
    }ورنہ{
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
    اگر( p->pHaving ){
      اگر( (p->selFlags & SF_Aggregate)==0 ){
        sqlite3_Error_پیغام(pParse, "HAVING clause on a non-aggregate query");
        واپس WRC_Abort;
      }
      اگر( sqlite3_Resolve_اظہاریہ_نام(&sNC, p->pHaving) ) واپس WRC_Abort;
    }
    sNC.ncFlags |= NC_Where;
    اگر( sqlite3_Resolve_اظہاریہ_نام(&sNC, p->pWhere) ) واپس WRC_Abort;
    sNC.ncFlags &= ~NC_Where;

    /* Resolve names in table-valued-function arguments */
    برائے(i=0; i<p->pSrc->nSrc; i++){
      Src_شے *p_شے = &p->pSrc->a[i];
      اگر( p_شے->fg.isTabFunc
       && sqlite3_Resolve_اظہاریہ_List_نام(&sNC, p_شے->u1.pFuncArg)
      ){
        واپس WRC_Abort;
      }
    }

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( اندر_RENAME_OBJECT ){
      کھڑکی *pWin;
      برائے(pWin=p->pWinDefn; pWin; pWin=pWin->p_اگلا_Win){
        اگر( sqlite3_Resolve_اظہاریہ_List_نام(&sNC, pWin->p_ترتیب_By)
         || sqlite3_Resolve_اظہاریہ_List_نام(&sNC, pWin->p_تقسیم)
        ){
          واپس WRC_Abort;
        }
      }
    }
#اگر_ختم

    sNC.ncFlags |= NC_AllowAgg|NC_AllowWin;

    /* If this is a converted compound query, move the ORDER BY clause from
    ** the sub-query back to the parent query. At this point each term
    ** within the ORDER BY clause has been transformed to an integer value.
    ** These integers will be replaced by copies of the corresponding result
    ** set expressions by the call to resolveOrderGroupBy() below.  */
    اگر( p->selFlags & SF_Converted ){
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
    اگر( p->p_ترتیب_By!=0
     && isCompound<=nCompound  /* Defer right-most ORDER BY of a compound */
     && resolve_ترتیب_گروہ_By(&sNC, p, p->p_ترتیب_By, "ORDER")
    ){
      واپس WRC_Abort;
    }
    اگر( db->mallocFailed ){
      واپس WRC_Abort;
    }
    sNC.ncFlags &= ~NC_AllowWin;
 
    /* Resolve the GROUP BY clause.  At the same time, make sure
    ** the GROUP BY clause does not contain aggregate functions.
    */
    اگر( p_گروہ_By ){
      ساخت اظہاریہ_List_شے *p_شے;
   
      اگر( resolve_ترتیب_گروہ_By(&sNC, p, p_گروہ_By, "GROUP") || db->mallocFailed ){
        واپس WRC_Abort;
      }
      برائے(i=0, p_شے=p_گروہ_By->a; i<p_گروہ_By->n_اظہاریہ; i++, p_شے++){
        اگر( اظہاریہ_Has_Property(p_شے->p_اظہاریہ, EP_Agg) ){
          sqlite3_Error_پیغام(pParse, "aggregate functions are not allowed in "
              "the GROUP BY clause");
          واپس WRC_Abort;
        }
      }
    }

    /* If this is part of a compound SELECT, check that it has the right
    ** number of expressions in the select list. */
    اگر( p->p_اگلا && p->pEList->n_اظہاریہ!=p->p_اگلا->pEList->n_اظہاریہ ){
      sqlite3_Select_Wrong_عدد_Terms_Error(pParse, p->p_اگلا);
      واپس WRC_Abort;
    }

    /* If the SELECT statement contains ON clauses that were moved into
    ** the WHERE clause, go through and verify that none of the terms
    ** in the ON clauses reference tables to the right of the ON clause. */
    اگر( (p->selFlags & SF_On_مقصد_Where) ){
      sqlite3SelectCheckOnClauses(pParse, p);
      اگر( pParse->nErr ) واپس WRC_Abort;
    }

    /* Advance to the next term of the compound
    */
    p = p->p_پیشگی;
    nCompound++;
  }

  /* Resolve the ORDER BY on a compound SELECT after all terms of
  ** the compound have been resolved.
  */
  اگر( isCompound && resolve_Compound_ترتیب_By(pParse, pLeftmost) ){
    واپس WRC_Abort;
  }

  واپس WRC_Prune;
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
صحیح sqlite3_Resolve_اظہاریہ_نام(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  اظہاریہ *p_اظہاریہ             /* The expression to be analyzed. */
){
  صحیح savedHasAgg;
  Walker w;

  اگر( p_اظہاریہ==0 ) واپس SQLITE_OK;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
  w.pParse = pNC->pParse;
  w.x_اظہاریہ_Callback = resolve_اظہاریہ_قدم;
  w.xSelectCallback = (pNC->ncFlags & NC_NoSelect) ? 0 : resolve_Select_قدم;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
  w.pParse->n_اونچائی += p_اظہاریہ->n_اونچائی;
  اگر( sqlite3_اظہاریہ_Check_اونچائی(w.pParse, w.pParse->n_اونچائی) ){
    واپس SQLITE_ERROR;
  }
#اگر_ختم
  أكد( p_اظہاریہ!=0 );
  sqlite3_Walk_اظہاریہ_NN(&w, p_اظہاریہ);
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
  w.pParse->n_اونچائی -= p_اظہاریہ->n_اونچائی;
#اگر_ختم
  أكد( EP_Agg==NC_HasAgg );
  أكد( EP_Win==NC_HasWin );
  testcase( pNC->ncFlags & NC_HasAgg );
  testcase( pNC->ncFlags & NC_HasWin );
  اظہاریہ_سیٹ_Property(p_اظہاریہ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
  pNC->ncFlags |= savedHasAgg;
  واپس pNC->nNcErr>0 || w.pParse->nErr>0;
}

/*
** Resolve all names for all expression in an expression list.  This is
** just like sqlite3ResolveExprNames() except that it works for an expression
** list rather than a single expression.
**
** The return value is SQLITE_OK (0) for success or SQLITE_ERROR (1) for a
** failure.
*/
صحیح sqlite3_Resolve_اظہاریہ_List_نام(
  NameContext *pNC,       /* Namespace to resolve expressions in. */
  اظہاریہ_List *pList         /* The expression list to be analyzed. */
){
  صحیح i;
  صحیح savedHasAgg = 0;
  Walker w;
  اگر( pList==0 ) واپس SQLITE_OK;
  w.pParse = pNC->pParse;
  w.x_اظہاریہ_Callback = resolve_اظہاریہ_قدم;
  w.xSelectCallback = resolve_Select_قدم;
  w.xSelectCallback2 = 0;
  w.u.pNC = pNC;
  savedHasAgg = pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
  pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
  برائے(i=0; i<pList->n_اظہاریہ; i++){
    اظہاریہ *p_اظہاریہ = pList->a[i].p_اظہاریہ;
    اگر( p_اظہاریہ==0 ) جاری;
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
    w.pParse->n_اونچائی += p_اظہاریہ->n_اونچائی;
    اگر( sqlite3_اظہاریہ_Check_اونچائی(w.pParse, w.pParse->n_اونچائی) ){
      واپس SQLITE_ERROR;
    }
#اگر_ختم
    sqlite3_Walk_اظہاریہ_NN(&w, p_اظہاریہ);
#اگر SQLITE_MAX_اظہاریہ_گہرائی>0
    w.pParse->n_اونچائی -= p_اظہاریہ->n_اونچائی;
#اگر_ختم
    أكد( EP_Agg==NC_HasAgg );
    أكد( EP_Win==NC_HasWin );
    testcase( pNC->ncFlags & NC_HasAgg );
    testcase( pNC->ncFlags & NC_HasWin );
    اگر( pNC->ncFlags & (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg) ){
      اظہاریہ_سیٹ_Property(p_اظہاریہ, pNC->ncFlags & (NC_HasAgg|NC_HasWin) );
      savedHasAgg |= pNC->ncFlags &
                          (NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
      pNC->ncFlags &= ~(NC_HasAgg|NC_MinMaxAgg|NC_HasWin|NC_ترتیب_Agg);
    }
    اگر( w.pParse->nErr>0 ) واپس SQLITE_ERROR;
  }
  pNC->ncFlags |= savedHasAgg;
  واپس SQLITE_OK;
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
خالی sqlite3_Resolve_Select_نام(
  Parse *pParse,         /* The parser context */
  Select *p,             /* The SELECT statement being coded. */
  NameContext *pOuterNC  /* Name context for parent SELECT statement */
){
  Walker w;

  أكد( p!=0 );
  w.x_اظہاریہ_Callback = resolve_اظہاریہ_قدم;
  w.xSelectCallback = resolve_Select_قدم;
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
صحیح sqlite3ResolveSelfReference(
  Parse *pParse,   /* Parsing context */
  جدول *pTab,     /* The table being referenced, or NULL */
  صحیح type,        /* NC_IsCheck, NC_PartIdx, NC_IdxExpr, NC_GenCol, or 0 */
  اظہاریہ *p_اظہاریہ,     /* Expression to resolve.  May be NULL. */
  اظہاریہ_List *pList  /* Expression list to resolve.  May be NULL. */
){
  SrcList *pSrc;                  /* Fake SrcList for pParse->pNewTable */
  NameContext sNC;                /* Name context for pParse->pNewTable */
  صحیح rc;
  اتحاد {
    SrcList sSrc;
    u8 srcSpace[SZ_SRCLIST_1];     /* Memory space for the fake SrcList */
  } uSrc;

  أكد( type==0 || pTab!=0 );
  أكد( type==NC_IsCheck || type==NC_PartIdx || type==NC_Idx_اظہاریہ
          || type==NC_Gen_ستون || pTab==0 );
  memset(&sNC, 0, حجم(sNC));
  memset(&uSrc, 0, حجم(uSrc));
  pSrc = &uSrc.sSrc;
  اگر( pTab ){
    pSrc->nSrc = 1;
    pSrc->a[0].zName = pTab->zName;
    pSrc->a[0].pSTab = pTab;
    pSrc->a[0].iCursor = -1;
    اگر( pTab->pSchema!=pParse->db->aDb[1].pSchema ){
      /* Cause EP_FromDDL to be set on TK_FUNCTION nodes of non-TEMP
      ** schema elements */
      type |= NC_FromDDL;
    }
  }
  sNC.pParse = pParse;
  sNC.pSrcList = pSrc;
  sNC.ncFlags = type | NC_IsDDL;
  اگر( (rc = sqlite3_Resolve_اظہاریہ_نام(&sNC, p_اظہاریہ))!=SQLITE_OK ) واپس rc;
  اگر( pList ) rc = sqlite3_Resolve_اظہاریہ_List_نام(&sNC, pList);
  واپس rc;
}
