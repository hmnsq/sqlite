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


#만약 !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
static 정수 walkWindowList(Walker *pWalker, Window *pList, 정수 bOneOnly){
  Window *pWin;
  위해(pWin=pList; pWin; pWin=pWin->pNextWin){
    정수 rc;
    rc = sqlite3WalkExprList(pWalker, pWin->pOrderBy);
    만약( rc ) 반환 WRC_Abort;
    rc = sqlite3WalkExprList(pWalker, pWin->pPartition);
    만약( rc ) 반환 WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pFilter);
    만약( rc ) 반환 WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pStart);
    만약( rc ) 반환 WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pEnd);
    만약( rc ) 반환 WRC_Abort;
    만약( bOneOnly ) break;
  }
  반환 WRC_Continue;
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
SQLITE_NOINLINE 정수 sqlite3WalkExprNN(Walker *pWalker, Expr *pExpr){
  정수 rc;
  testcase( ExprHasProperty(pExpr, EP_TokenOnly) );
  testcase( ExprHasProperty(pExpr, EP_Reduced) );
  동안(1){
    rc = pWalker->xExprCallback(pWalker, pExpr);
    만약( rc ) 반환 rc & WRC_Abort;
    만약( !ExprHasProperty(pExpr,(EP_TokenOnly|EP_Leaf)) ){
      assert( pExpr->x.pList==0 || pExpr->pRight==0 );
      만약( pExpr->pLeft && sqlite3WalkExprNN(pWalker, pExpr->pLeft) ){
        반환 WRC_Abort;
      }
      만약( pExpr->pRight ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        pExpr = pExpr->pRight;
        continue;
      }아니면 만약( ExprUseXSelect(pExpr) ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        만약( sqlite3WalkSelect(pWalker, pExpr->x.pSelect) ) 반환 WRC_Abort;
      }아니면{
        만약( pExpr->x.pList ){
          만약( sqlite3WalkExprList(pWalker, pExpr->x.pList) ) 반환 WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        만약( ExprHasProperty(pExpr, EP_WinFunc) ){
          만약( walkWindowList(pWalker, pExpr->y.pWin, 1) ) 반환 WRC_Abort;
        }
#endif
      }
    }
    break;
  }
  반환 WRC_Continue;
}
정수 sqlite3WalkExpr(Walker *pWalker, Expr *pExpr){
  반환 pExpr ? sqlite3WalkExprNN(pWalker,pExpr) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
정수 sqlite3WalkExprList(Walker *pWalker, ExprList *p){
  정수 i;
  struct ExprList_item *pItem;
  만약( p ){
    위해(i=p->nExpr, pItem=p->a; i>0; i--, pItem++){
      만약( sqlite3WalkExpr(pWalker, pItem->pExpr) ) 반환 WRC_Abort;
    }
  }
  반환 WRC_Continue;
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
정수 sqlite3WalkSelectExpr(Walker *pWalker, Select *p){
  만약( sqlite3WalkExprList(pWalker, p->pEList) ) 반환 WRC_Abort;
  만약( sqlite3WalkExpr(pWalker, p->pWhere) ) 반환 WRC_Abort;
  만약( sqlite3WalkExprList(pWalker, p->pGroupBy) ) 반환 WRC_Abort;
  만약( sqlite3WalkExpr(pWalker, p->pHaving) ) 반환 WRC_Abort;
  만약( sqlite3WalkExprList(pWalker, p->pOrderBy) ) 반환 WRC_Abort;
  만약( sqlite3WalkExpr(pWalker, p->pLimit) ) 반환 WRC_Abort;
#만약 !defined(SQLITE_OMIT_WINDOWFUNC)
  만약( p->pWinDefn ){
    Parse *pParse;
    만약( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3SelectPopWith
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      정수 rc = walkWindowList(pWalker, p->pWinDefn, 0);
      반환 rc;
    }
  }
#endif
  반환 WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
정수 sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  정수 i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  만약( ALWAYS(pSrc) ){
    위해(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      만약( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        반환 WRC_Abort;
      }
      만약( pItem->fg.isTabFunc
       && sqlite3WalkExprList(pWalker, pItem->u1.pFuncArg)
      ){
        반환 WRC_Abort;
      }
    }
  }
  반환 WRC_Continue;
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
정수 sqlite3WalkSelect(Walker *pWalker, Select *p){
  정수 rc;
  만약( p==0 ) 반환 WRC_Continue;
  만약( pWalker->xSelectCallback==0 ) 반환 WRC_Continue;
  do{
    rc = pWalker->xSelectCallback(pWalker, p);
    만약( rc ) 반환 rc & WRC_Abort;
    만약( sqlite3WalkSelectExpr(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      반환 WRC_Abort;
    }
    만약( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }동안( p!=0 );
  반환 WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
정수 sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  반환 WRC_Continue;
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
정수 sqlite3ExprWalkNoop(Walker *NotUsed, Expr *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  반환 WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
정수 sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  반환 WRC_Continue;
}
