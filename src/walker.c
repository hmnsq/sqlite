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
#ضمن "sqliteInt.h"
#ضمن <stdlib.h>
#ضمن <string.h>


#إذا !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
ساكن صحيح walkWindowList(Walker *pWalker, Window *pList, صحيح bOneOnly){
  Window *pWin;
  لكل(pWin=pList; pWin; pWin=pWin->pNextWin){
    صحيح rc;
    rc = sqlite3_Walk_دەربڕین_List(pWalker, pWin->pOrderBy);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_دەربڕین_List(pWalker, pWin->pPartition);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_دەربڕین(pWalker, pWin->pFilter);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_دەربڕین(pWalker, pWin->pStart);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_دەربڕین(pWalker, pWin->pEnd);
    إذا( rc ) أرجع WRC_Abort;
    إذا( bOneOnly ) اكسر;
  }
  أرجع WRC_Continue;
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
SQLITE_NOINLINE صحيح sqlite3_Walk_دەربڕین_NN(Walker *pWalker, دەربڕین *p_دەربڕین){
  صحيح rc;
  testcase( دەربڕین_Has_Property(p_دەربڕین, EP_TokenOnly) );
  testcase( دەربڕین_Has_Property(p_دەربڕین, EP_Reduced) );
  طالما(1){
    rc = pWalker->x_دەربڕین_Callback(pWalker, p_دەربڕین);
    إذا( rc ) أرجع rc & WRC_Abort;
    إذا( !دەربڕین_Has_Property(p_دەربڕین,(EP_TokenOnly|EP_Leaf)) ){
      أكد( p_دەربڕین->x.pList==0 || p_دەربڕین->p_ڕاست==0 );
      إذا( p_دەربڕین->p_چەپ && sqlite3_Walk_دەربڕین_NN(pWalker, p_دەربڕین->p_چەپ) ){
        أرجع WRC_Abort;
      }
      إذا( p_دەربڕین->p_ڕاست ){
        أكد( !دەربڕین_Has_Property(p_دەربڕین, EP_WinFunc) );
        p_دەربڕین = p_دەربڕین->p_ڕاست;
        استمر;
      }وإلا إذا( دەربڕین_Use_XSelect(p_دەربڕین) ){
        أكد( !دەربڕین_Has_Property(p_دەربڕین, EP_WinFunc) );
        إذا( sqlite3WalkSelect(pWalker, p_دەربڕین->x.pSelect) ) أرجع WRC_Abort;
      }وإلا{
        إذا( p_دەربڕین->x.pList ){
          إذا( sqlite3_Walk_دەربڕین_List(pWalker, p_دەربڕین->x.pList) ) أرجع WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        إذا( دەربڕین_Has_Property(p_دەربڕین, EP_WinFunc) ){
          إذا( walkWindowList(pWalker, p_دەربڕین->y.pWin, 1) ) أرجع WRC_Abort;
        }
#endif
      }
    }
    اكسر;
  }
  أرجع WRC_Continue;
}
صحيح sqlite3_Walk_دەربڕین(Walker *pWalker, دەربڕین *p_دەربڕین){
  أرجع p_دەربڕین ? sqlite3_Walk_دەربڕین_NN(pWalker,p_دەربڕین) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
صحيح sqlite3_Walk_دەربڕین_List(Walker *pWalker, دەربڕین_List *p){
  صحيح i;
  بنية دەربڕین_List_item *pItem;
  إذا( p ){
    لكل(i=p->n_دەربڕین, pItem=p->a; i>0; i--, pItem++){
      إذا( sqlite3_Walk_دەربڕین(pWalker, pItem->p_دەربڕین) ) أرجع WRC_Abort;
    }
  }
  أرجع WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
فراغ sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
صحيح sqlite3_Walk_Select_دەربڕین(Walker *pWalker, Select *p){
  إذا( sqlite3_Walk_دەربڕین_List(pWalker, p->pEList) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_دەربڕین(pWalker, p->pWhere) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_دەربڕین_List(pWalker, p->pGroupBy) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_دەربڕین(pWalker, p->pHaving) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_دەربڕین_List(pWalker, p->pOrderBy) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_دەربڕین(pWalker, p->pLimit) ) أرجع WRC_Abort;
#إذا !defined(SQLITE_OMIT_WINDOWFUNC)
  إذا( p->pWinDefn ){
    Parse *pParse;
    إذا( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_دەرهێنان_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      صحيح rc = walkWindowList(pWalker, p->pWinDefn, 0);
      أرجع rc;
    }
  }
#endif
  أرجع WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
صحيح sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  صحيح i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  إذا( ALWAYS(pSrc) ){
    لكل(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      إذا( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        أرجع WRC_Abort;
      }
      إذا( pItem->fg.isTabFunc
       && sqlite3_Walk_دەربڕین_List(pWalker, pItem->u1.pFuncArg)
      ){
        أرجع WRC_Abort;
      }
    }
  }
  أرجع WRC_Continue;
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
صحيح sqlite3WalkSelect(Walker *pWalker, Select *p){
  صحيح rc;
  إذا( p==0 ) أرجع WRC_Continue;
  إذا( pWalker->xSelectCallback==0 ) أرجع WRC_Continue;
  افعل{
    rc = pWalker->xSelectCallback(pWalker, p);
    إذا( rc ) أرجع rc & WRC_Abort;
    إذا( sqlite3_Walk_Select_دەربڕین(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      أرجع WRC_Abort;
    }
    إذا( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }طالما( p!=0 );
  أرجع WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
صحيح sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  أرجع WRC_Continue;
}
فراغ sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
صحيح sqlite3_دەربڕین_Walk_Noop(Walker *NotUsed, دەربڕین *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  أرجع WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
صحيح sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  أرجع WRC_Continue;
}
