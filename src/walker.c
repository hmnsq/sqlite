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


#אם !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
ساكن صحيح walkWindowList(Walker *pWalker, Window *pList, صحيح bOneOnly){
  Window *pWin;
  עבור(pWin=pList; pWin; pWin=pWin->pNextWin){
    صحيح rc;
    rc = sqlite3WalkExprList(pWalker, pWin->pOrderBy);
    אם( rc ) החזר WRC_Abort;
    rc = sqlite3WalkExprList(pWalker, pWin->pPartition);
    אם( rc ) החזר WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pFilter);
    אם( rc ) החזר WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pStart);
    אם( rc ) החזר WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pEnd);
    אם( rc ) החזר WRC_Abort;
    אם( bOneOnly ) اكسر;
  }
  החזר WRC_Continue;
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
SQLITE_NOINLINE صحيح sqlite3WalkExprNN(Walker *pWalker, Expr *pExpr){
  صحيح rc;
  testcase( ExprHasProperty(pExpr, EP_TokenOnly) );
  testcase( ExprHasProperty(pExpr, EP_Reduced) );
  כלעוד(1){
    rc = pWalker->xExprCallback(pWalker, pExpr);
    אם( rc ) החזר rc & WRC_Abort;
    אם( !ExprHasProperty(pExpr,(EP_TokenOnly|EP_Leaf)) ){
      أكد( pExpr->x.pList==0 || pExpr->pRight==0 );
      אם( pExpr->pLeft && sqlite3WalkExprNN(pWalker, pExpr->pLeft) ){
        החזר WRC_Abort;
      }
      אם( pExpr->pRight ){
        أكد( !ExprHasProperty(pExpr, EP_WinFunc) );
        pExpr = pExpr->pRight;
        استمر;
      }אחרת אם( ExprUseXSelect(pExpr) ){
        أكد( !ExprHasProperty(pExpr, EP_WinFunc) );
        אם( sqlite3WalkSelect(pWalker, pExpr->x.pSelect) ) החזר WRC_Abort;
      }אחרת{
        אם( pExpr->x.pList ){
          אם( sqlite3WalkExprList(pWalker, pExpr->x.pList) ) החזר WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        אם( ExprHasProperty(pExpr, EP_WinFunc) ){
          אם( walkWindowList(pWalker, pExpr->y.pWin, 1) ) החזר WRC_Abort;
        }
#endif
      }
    }
    اكسر;
  }
  החזר WRC_Continue;
}
صحيح sqlite3WalkExpr(Walker *pWalker, Expr *pExpr){
  החזר pExpr ? sqlite3WalkExprNN(pWalker,pExpr) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
صحيح sqlite3WalkExprList(Walker *pWalker, ExprList *p){
  صحيح i;
  بنية ExprList_item *pItem;
  אם( p ){
    עבור(i=p->nExpr, pItem=p->a; i>0; i--, pItem++){
      אם( sqlite3WalkExpr(pWalker, pItem->pExpr) ) החזר WRC_Abort;
    }
  }
  החזר WRC_Continue;
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
صحيح sqlite3WalkSelectExpr(Walker *pWalker, Select *p){
  אם( sqlite3WalkExprList(pWalker, p->pEList) ) החזר WRC_Abort;
  אם( sqlite3WalkExpr(pWalker, p->pWhere) ) החזר WRC_Abort;
  אם( sqlite3WalkExprList(pWalker, p->pGroupBy) ) החזר WRC_Abort;
  אם( sqlite3WalkExpr(pWalker, p->pHaving) ) החזר WRC_Abort;
  אם( sqlite3WalkExprList(pWalker, p->pOrderBy) ) החזר WRC_Abort;
  אם( sqlite3WalkExpr(pWalker, p->pLimit) ) החזר WRC_Abort;
#אם !defined(SQLITE_OMIT_WINDOWFUNC)
  אם( p->pWinDefn ){
    Parse *pParse;
    אם( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3SelectPopWith
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      صحيح rc = walkWindowList(pWalker, p->pWinDefn, 0);
      החזר rc;
    }
  }
#endif
  החזר WRC_Continue;
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
  אם( ALWAYS(pSrc) ){
    עבור(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      אם( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        החזר WRC_Abort;
      }
      אם( pItem->fg.isTabFunc
       && sqlite3WalkExprList(pWalker, pItem->u1.pFuncArg)
      ){
        החזר WRC_Abort;
      }
    }
  }
  החזר WRC_Continue;
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
  אם( p==0 ) החזר WRC_Continue;
  אם( pWalker->xSelectCallback==0 ) החזר WRC_Continue;
  עשה{
    rc = pWalker->xSelectCallback(pWalker, p);
    אם( rc ) החזר rc & WRC_Abort;
    אם( sqlite3WalkSelectExpr(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      החזר WRC_Abort;
    }
    אם( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }כלעוד( p!=0 );
  החזר WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
صحيح sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  החזר WRC_Continue;
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
صحيح sqlite3ExprWalkNoop(Walker *NotUsed, Expr *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  החזר WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
صحيح sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  החזר WRC_Continue;
}
