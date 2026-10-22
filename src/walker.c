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
#include "sqliteInt.h"
#include <stdlib.h>
#include <string.h>


#ако !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
static цяло walkWindowList(Walker *pWalker, Window *pList, цяло bOneOnly){
  Window *pWin;
  за(pWin=pList; pWin; pWin=pWin->pNextWin){
    цяло rc;
    rc = sqlite3WalkExprList(pWalker, pWin->pOrderBy);
    ако( rc ) върни WRC_Abort;
    rc = sqlite3WalkExprList(pWalker, pWin->pPartition);
    ако( rc ) върни WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pFilter);
    ако( rc ) върни WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pStart);
    ако( rc ) върни WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pEnd);
    ако( rc ) върни WRC_Abort;
    ако( bOneOnly ) break;
  }
  върни WRC_Continue;
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
SQLITE_NOINLINE цяло sqlite3WalkExprNN(Walker *pWalker, Expr *pExpr){
  цяло rc;
  testcase( ExprHasProperty(pExpr, EP_TokenOnly) );
  testcase( ExprHasProperty(pExpr, EP_Reduced) );
  докато(1){
    rc = pWalker->xExprCallback(pWalker, pExpr);
    ако( rc ) върни rc & WRC_Abort;
    ако( !ExprHasProperty(pExpr,(EP_TokenOnly|EP_Leaf)) ){
      assert( pExpr->x.pList==0 || pExpr->pRight==0 );
      ако( pExpr->pLeft && sqlite3WalkExprNN(pWalker, pExpr->pLeft) ){
        върни WRC_Abort;
      }
      ако( pExpr->pRight ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        pExpr = pExpr->pRight;
        continue;
      }иначе ако( ExprUseXSelect(pExpr) ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        ако( sqlite3WalkSelect(pWalker, pExpr->x.pSelect) ) върни WRC_Abort;
      }иначе{
        ако( pExpr->x.pList ){
          ако( sqlite3WalkExprList(pWalker, pExpr->x.pList) ) върни WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        ако( ExprHasProperty(pExpr, EP_WinFunc) ){
          ако( walkWindowList(pWalker, pExpr->y.pWin, 1) ) върни WRC_Abort;
        }
#endif
      }
    }
    break;
  }
  върни WRC_Continue;
}
цяло sqlite3WalkExpr(Walker *pWalker, Expr *pExpr){
  върни pExpr ? sqlite3WalkExprNN(pWalker,pExpr) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
цяло sqlite3WalkExprList(Walker *pWalker, ExprList *p){
  цяло i;
  struct ExprList_item *pItem;
  ако( p ){
    за(i=p->nExpr, pItem=p->a; i>0; i--, pItem++){
      ако( sqlite3WalkExpr(pWalker, pItem->pExpr) ) върни WRC_Abort;
    }
  }
  върни WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
void sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
цяло sqlite3WalkSelectExpr(Walker *pWalker, Select *p){
  ако( sqlite3WalkExprList(pWalker, p->pEList) ) върни WRC_Abort;
  ако( sqlite3WalkExpr(pWalker, p->pWhere) ) върни WRC_Abort;
  ако( sqlite3WalkExprList(pWalker, p->pGroupBy) ) върни WRC_Abort;
  ако( sqlite3WalkExpr(pWalker, p->pHaving) ) върни WRC_Abort;
  ако( sqlite3WalkExprList(pWalker, p->pOrderBy) ) върни WRC_Abort;
  ако( sqlite3WalkExpr(pWalker, p->pLimit) ) върни WRC_Abort;
#ако !defined(SQLITE_OMIT_WINDOWFUNC)
  ако( p->pWinDefn ){
    Parse *pParse;
    ако( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3SelectPopWith
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      цяло rc = walkWindowList(pWalker, p->pWinDefn, 0);
      върни rc;
    }
  }
#endif
  върни WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
цяло sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  цяло i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  ако( ALWAYS(pSrc) ){
    за(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      ако( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        върни WRC_Abort;
      }
      ако( pItem->fg.isTabFunc
       && sqlite3WalkExprList(pWalker, pItem->u1.pFuncArg)
      ){
        върни WRC_Abort;
      }
    }
  }
  върни WRC_Continue;
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
цяло sqlite3WalkSelect(Walker *pWalker, Select *p){
  цяло rc;
  ако( p==0 ) върни WRC_Continue;
  ако( pWalker->xSelectCallback==0 ) върни WRC_Continue;
  do{
    rc = pWalker->xSelectCallback(pWalker, p);
    ако( rc ) върни rc & WRC_Abort;
    ако( sqlite3WalkSelectExpr(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      върни WRC_Abort;
    }
    ако( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }докато( p!=0 );
  върни WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
цяло sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  върни WRC_Continue;
}
void sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
цяло sqlite3ExprWalkNoop(Walker *NotUsed, Expr *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  върни WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
цяло sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  върни WRC_Continue;
}
