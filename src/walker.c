/*
** 2008 August 16
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This file contains routines used for walking the parser tree for
** an SQL statement.
*/
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>


#എങ്കിൽ !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ walkWindowList(Walker *pWalker, Window *pList, പൂർണ്ണസംഖ്യ bOneOnly){
  Window *pWin;
  വേണ്ടി(pWin=pList; pWin; pWin=pWin->pNextWin){
    പൂർണ്ണസംഖ്യ rc;
    rc = sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, pWin->pOrderBy);
    എങ്കിൽ( rc ) തിരികെ WRC_Abort;
    rc = sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, pWin->pPartition);
    എങ്കിൽ( rc ) തിരികെ WRC_Abort;
    rc = sqlite3_Walk_വ്യഞ്ജകം(pWalker, pWin->pFilter);
    എങ്കിൽ( rc ) തിരികെ WRC_Abort;
    rc = sqlite3_Walk_വ്യഞ്ജകം(pWalker, pWin->pStart);
    എങ്കിൽ( rc ) തിരികെ WRC_Abort;
    rc = sqlite3_Walk_വ്യഞ്ജകം(pWalker, pWin->pEnd);
    എങ്കിൽ( rc ) തിരികെ WRC_Abort;
    എങ്കിൽ( bOneOnly ) നിർത്തുക;
  }
  തിരികെ WRC_Continue;
}
#endif

/*
** Walk an expression tree.  Invoke the callback once for each node
** of the expression, while descending.  (In other words, the callback
** is invoked before visiting children.)
**
** The return value from the callback should be one of the WRC_*
** constants to specify how to proceed with the walk.
**
**    WRC_Continue      Continue descending down the tree.
**
**    WRC_Prune         Do not descend into child nodes, but allow
**                      the walk to continue with sibling nodes.
**
**    WRC_Abort         Do no more callbacks.  Unwind the stack and
**                      return from the top-level walk call.
**
** The return value from this routine is WRC_Abort to abandon the tree walk
** and WRC_Continue to continue.
*/
SQLITE_NOINLINE പൂർണ്ണസംഖ്യ sqlite3_Walk_വ്യഞ്ജകം_NN(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  പൂർണ്ണസംഖ്യ rc;
  testcase( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_TokenOnly) );
  testcase( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Reduced) );
  ആകുമ്പോൾ(1){
    rc = pWalker->x_വ്യഞ്ജകം_Callback(pWalker, p_വ്യഞ്ജകം);
    എങ്കിൽ( rc ) തിരികെ rc & WRC_Abort;
    എങ്കിൽ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,(EP_TokenOnly|EP_Leaf)) ){
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pList==0 || p_വ്യഞ്ജകം->p_വലത്==0 );
      എങ്കിൽ( p_വ്യഞ്ജകം->p_ഇടത് && sqlite3_Walk_വ്യഞ്ജകം_NN(pWalker, p_വ്യഞ്ജകം->p_ഇടത്) ){
        തിരികെ WRC_Abort;
      }
      എങ്കിൽ( p_വ്യഞ്ജകം->p_വലത് ){
        പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) );
        p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->p_വലത്;
        തുടരുക;
      }അല്ലെങ്കിൽ എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
        പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) );
        എങ്കിൽ( sqlite3WalkSelect(pWalker, p_വ്യഞ്ജകം->x.pSelect) ) തിരികെ WRC_Abort;
      }അല്ലെങ്കിൽ{
        എങ്കിൽ( p_വ്യഞ്ജകം->x.pList ){
          എങ്കിൽ( sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, p_വ്യഞ്ജകം->x.pList) ) തിരികെ WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_WinFunc) ){
          എങ്കിൽ( walkWindowList(pWalker, p_വ്യഞ്ജകം->y.pWin, 1) ) തിരികെ WRC_Abort;
        }
#endif
      }
    }
    നിർത്തുക;
  }
  തിരികെ WRC_Continue;
}
പൂർണ്ണസംഖ്യ sqlite3_Walk_വ്യഞ്ജകം(Walker *pWalker, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  തിരികെ p_വ്യഞ്ജകം ? sqlite3_Walk_വ്യഞ്ജകം_NN(pWalker,p_വ്യഞ്ജകം) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
പൂർണ്ണസംഖ്യ sqlite3_Walk_വ്യഞ്ജകം_List(Walker *pWalker, വ്യഞ്ജകം_List *p){
  പൂർണ്ണസംഖ്യ i;
  ഘടന വ്യഞ്ജകം_List_item *pItem;
  എങ്കിൽ( p ){
    വേണ്ടി(i=p->n_വ്യഞ്ജകം, pItem=p->a; i>0; i--, pItem++){
      എങ്കിൽ( sqlite3_Walk_വ്യഞ്ജകം(pWalker, pItem->p_വ്യഞ്ജകം) ) തിരികെ WRC_Abort;
    }
  }
  തിരികെ WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
ശൂന്യം sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
  UNUSED_PARAMETER(pWalker);
  UNUSED_PARAMETER(p);
  /* No-op */
}

/*
** Walk all expressions associated with SELECT statement p.  Do
** not invoke the SELECT callback on p, but do (of course) invoke
** any expr callbacks and SELECT callbacks that come from subqueries.
** Return WRC_Abort or WRC_Continue.
*/
പൂർണ്ണസംഖ്യ sqlite3_Walk_Select_വ്യഞ്ജകം(Walker *pWalker, Select *p){
  എങ്കിൽ( sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, p->pEList) ) തിരികെ WRC_Abort;
  എങ്കിൽ( sqlite3_Walk_വ്യഞ്ജകം(pWalker, p->pWhere) ) തിരികെ WRC_Abort;
  എങ്കിൽ( sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, p->pGroupBy) ) തിരികെ WRC_Abort;
  എങ്കിൽ( sqlite3_Walk_വ്യഞ്ജകം(pWalker, p->pHaving) ) തിരികെ WRC_Abort;
  എങ്കിൽ( sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, p->pOrderBy) ) തിരികെ WRC_Abort;
  എങ്കിൽ( sqlite3_Walk_വ്യഞ്ജകം(pWalker, p->pLimit) ) തിരികെ WRC_Abort;
#എങ്കിൽ !defined(SQLITE_OMIT_WINDOWFUNC)
  എങ്കിൽ( p->pWinDefn ){
    Parse *pParse;
    എങ്കിൽ( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_എടുക്കുക_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      പൂർണ്ണസംഖ്യ rc = walkWindowList(pWalker, p->pWinDefn, 0);
      തിരികെ rc;
    }
  }
#endif
  തിരികെ WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
പൂർണ്ണസംഖ്യ sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  പൂർണ്ണസംഖ്യ i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  എങ്കിൽ( ALWAYS(pSrc) ){
    വേണ്ടി(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      എങ്കിൽ( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        തിരികെ WRC_Abort;
      }
      എങ്കിൽ( pItem->fg.isTabFunc
       && sqlite3_Walk_വ്യഞ്ജകം_List(pWalker, pItem->u1.pFuncArg)
      ){
        തിരികെ WRC_Abort;
      }
    }
  }
  തിരികെ WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in Select statement p.
** Invoke sqlite3WalkSelect() for subqueries in the FROM clause and
** on the compound select chain, p->pPrior. 
**
** If it is not NULL, the xSelectCallback() callback is invoked before
** the walk of the expressions and FROM clause. The xSelectCallback2()
** method is invoked following the walk of the expressions and FROM clause,
** but only if both xSelectCallback and xSelectCallback2 are both non-NULL
** and if the expressions and FROM clause both return WRC_Continue;
**
** Return WRC_Continue under normal conditions.  Return WRC_Abort if
** there is an abort request.
**
** If the Walker does not have an xSelectCallback() then this routine
** is a no-op returning WRC_Continue.
*/
പൂർണ്ണസംഖ്യ sqlite3WalkSelect(Walker *pWalker, Select *p){
  പൂർണ്ണസംഖ്യ rc;
  എങ്കിൽ( p==0 ) തിരികെ WRC_Continue;
  എങ്കിൽ( pWalker->xSelectCallback==0 ) തിരികെ WRC_Continue;
  ചെയ്യുക{
    rc = pWalker->xSelectCallback(pWalker, p);
    എങ്കിൽ( rc ) തിരികെ rc & WRC_Abort;
    എങ്കിൽ( sqlite3_Walk_Select_വ്യഞ്ജകം(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      തിരികെ WRC_Abort;
    }
    എങ്കിൽ( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }ആകുമ്പോൾ( p!=0 );
  തിരികെ WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
പൂർണ്ണസംഖ്യ sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  തിരികെ WRC_Continue;
}
ശൂന്യം sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth--;
}


/*
** No-op routine for the parse-tree walker.
**
** When this routine is the Walker.xExprCallback then expression trees
** are walked without any actions being taken at each node.  Presumably,
** when this routine is used for Walker.xExprCallback then 
** Walker.xSelectCallback is set to do something useful for every 
** subquery in the parser tree.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Walk_Noop(Walker *NotUsed, വ്യഞ്ജകം *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  തിരികെ WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
പൂർണ്ണസംഖ്യ sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  തിരികെ WRC_Continue;
}
