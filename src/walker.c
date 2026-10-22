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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>


#जर !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
स्थिर पूर्णांक walkWindowList(Walker *pWalker, Window *pList, पूर्णांक bOneOnly){
  Window *pWin;
  साठी(pWin=pList; pWin; pWin=pWin->pNextWin){
    पूर्णांक rc;
    rc = sqlite3_Walk_अभिव्यक्ती_List(pWalker, pWin->pOrderBy);
    जर( rc ) परत WRC_Abort;
    rc = sqlite3_Walk_अभिव्यक्ती_List(pWalker, pWin->pPartition);
    जर( rc ) परत WRC_Abort;
    rc = sqlite3_Walk_अभिव्यक्ती(pWalker, pWin->pFilter);
    जर( rc ) परत WRC_Abort;
    rc = sqlite3_Walk_अभिव्यक्ती(pWalker, pWin->pStart);
    जर( rc ) परत WRC_Abort;
    rc = sqlite3_Walk_अभिव्यक्ती(pWalker, pWin->pEnd);
    जर( rc ) परत WRC_Abort;
    जर( bOneOnly ) थांबा;
  }
  परत WRC_Continue;
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
SQLITE_NOINLINE पूर्णांक sqlite3_Walk_अभिव्यक्ती_NN(Walker *pWalker, अभिव्यक्ती *p_अभिव्यक्ती){
  पूर्णांक rc;
  testcase( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_TokenOnly) );
  testcase( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Reduced) );
  पर्यंत(1){
    rc = pWalker->x_अभिव्यक्ती_Callback(pWalker, p_अभिव्यक्ती);
    जर( rc ) परत rc & WRC_Abort;
    जर( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती,(EP_TokenOnly|EP_Leaf)) ){
      प्रतिज्ञा( p_अभिव्यक्ती->x.pList==0 || p_अभिव्यक्ती->p_उजवा==0 );
      जर( p_अभिव्यक्ती->p_डावा && sqlite3_Walk_अभिव्यक्ती_NN(pWalker, p_अभिव्यक्ती->p_डावा) ){
        परत WRC_Abort;
      }
      जर( p_अभिव्यक्ती->p_उजवा ){
        प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_WinFunc) );
        p_अभिव्यक्ती = p_अभिव्यक्ती->p_उजवा;
        सुरूठेवा;
      }अन्यथा जर( अभिव्यक्ती_Use_XSelect(p_अभिव्यक्ती) ){
        प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_WinFunc) );
        जर( sqlite3WalkSelect(pWalker, p_अभिव्यक्ती->x.pSelect) ) परत WRC_Abort;
      }अन्यथा{
        जर( p_अभिव्यक्ती->x.pList ){
          जर( sqlite3_Walk_अभिव्यक्ती_List(pWalker, p_अभिव्यक्ती->x.pList) ) परत WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_WinFunc) ){
          जर( walkWindowList(pWalker, p_अभिव्यक्ती->y.pWin, 1) ) परत WRC_Abort;
        }
#endif
      }
    }
    थांबा;
  }
  परत WRC_Continue;
}
पूर्णांक sqlite3_Walk_अभिव्यक्ती(Walker *pWalker, अभिव्यक्ती *p_अभिव्यक्ती){
  परत p_अभिव्यक्ती ? sqlite3_Walk_अभिव्यक्ती_NN(pWalker,p_अभिव्यक्ती) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
पूर्णांक sqlite3_Walk_अभिव्यक्ती_List(Walker *pWalker, अभिव्यक्ती_List *p){
  पूर्णांक i;
  संरचना अभिव्यक्ती_List_item *pItem;
  जर( p ){
    साठी(i=p->n_अभिव्यक्ती, pItem=p->a; i>0; i--, pItem++){
      जर( sqlite3_Walk_अभिव्यक्ती(pWalker, pItem->p_अभिव्यक्ती) ) परत WRC_Abort;
    }
  }
  परत WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
शून्य sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
पूर्णांक sqlite3_Walk_Select_अभिव्यक्ती(Walker *pWalker, Select *p){
  जर( sqlite3_Walk_अभिव्यक्ती_List(pWalker, p->pEList) ) परत WRC_Abort;
  जर( sqlite3_Walk_अभिव्यक्ती(pWalker, p->pWhere) ) परत WRC_Abort;
  जर( sqlite3_Walk_अभिव्यक्ती_List(pWalker, p->pGroupBy) ) परत WRC_Abort;
  जर( sqlite3_Walk_अभिव्यक्ती(pWalker, p->pHaving) ) परत WRC_Abort;
  जर( sqlite3_Walk_अभिव्यक्ती_List(pWalker, p->pOrderBy) ) परत WRC_Abort;
  जर( sqlite3_Walk_अभिव्यक्ती(pWalker, p->pLimit) ) परत WRC_Abort;
#जर !defined(SQLITE_OMIT_WINDOWFUNC)
  जर( p->pWinDefn ){
    Parse *pParse;
    जर( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_काढा_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      पूर्णांक rc = walkWindowList(pWalker, p->pWinDefn, 0);
      परत rc;
    }
  }
#endif
  परत WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
पूर्णांक sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  पूर्णांक i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  जर( ALWAYS(pSrc) ){
    साठी(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      जर( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        परत WRC_Abort;
      }
      जर( pItem->fg.isTabFunc
       && sqlite3_Walk_अभिव्यक्ती_List(pWalker, pItem->u1.pFuncArg)
      ){
        परत WRC_Abort;
      }
    }
  }
  परत WRC_Continue;
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
पूर्णांक sqlite3WalkSelect(Walker *pWalker, Select *p){
  पूर्णांक rc;
  जर( p==0 ) परत WRC_Continue;
  जर( pWalker->xSelectCallback==0 ) परत WRC_Continue;
  करा{
    rc = pWalker->xSelectCallback(pWalker, p);
    जर( rc ) परत rc & WRC_Abort;
    जर( sqlite3_Walk_Select_अभिव्यक्ती(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      परत WRC_Abort;
    }
    जर( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }पर्यंत( p!=0 );
  परत WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
पूर्णांक sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  परत WRC_Continue;
}
शून्य sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
पूर्णांक sqlite3_अभिव्यक्ती_Walk_Noop(Walker *NotUsed, अभिव्यक्ती *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  परत WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
पूर्णांक sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  परत WRC_Continue;
}
