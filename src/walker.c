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


#ပေႃး !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
static puur_naa_nka walkWindowList(Walker *pWalker, Window *pList, puur_naa_nka bOneOnly){
  Window *pWin;
  for(pWin=pList; pWin; pWin=pWin->pNextWin){
    puur_naa_nka rc;
    rc = sqlite3WalkExprList(pWalker, pWin->pOrderBy);
    ပေႃး( rc ) ပွၵ်ႈ WRC_Abort;
    rc = sqlite3WalkExprList(pWalker, pWin->pPartition);
    ပေႃး( rc ) ပွၵ်ႈ WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pFilter);
    ပေႃး( rc ) ပွၵ်ႈ WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pStart);
    ပေႃး( rc ) ပွၵ်ႈ WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pEnd);
    ပေႃး( rc ) ပွၵ်ႈ WRC_Abort;
    ပေႃး( bOneOnly ) bha_nga;
  }
  ပွၵ်ႈ WRC_Continue;
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
SQLITE_NOINLINE puur_naa_nka sqlite3WalkExprNN(Walker *pWalker, Expr *pExpr){
  puur_naa_nka rc;
  testcase( ExprHasProperty(pExpr, EP_TokenOnly) );
  testcase( ExprHasProperty(pExpr, EP_Reduced) );
  ၶၢဝ်း(1){
    rc = pWalker->xExprCallback(pWalker, pExpr);
    ပေႃး( rc ) ပွၵ်ႈ rc & WRC_Abort;
    ပေႃး( !ExprHasProperty(pExpr,(EP_TokenOnly|EP_Leaf)) ){
      assert( pExpr->x.pList==0 || pExpr->pRight==0 );
      ပေႃး( pExpr->pLeft && sqlite3WalkExprNN(pWalker, pExpr->pLeft) ){
        ပွၵ်ႈ WRC_Abort;
      }
      ပေႃး( pExpr->pRight ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        pExpr = pExpr->pRight;
        continue;
      }ပေႃးဢမ်ႇ ပေႃး( ExprUseXSelect(pExpr) ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        ပေႃး( sqlite3WalkSelect(pWalker, pExpr->x.pSelect) ) ပွၵ်ႈ WRC_Abort;
      }ပေႃးဢမ်ႇ{
        ပေႃး( pExpr->x.pList ){
          ပေႃး( sqlite3WalkExprList(pWalker, pExpr->x.pList) ) ပွၵ်ႈ WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        ပေႃး( ExprHasProperty(pExpr, EP_WinFunc) ){
          ပေႃး( walkWindowList(pWalker, pExpr->y.pWin, 1) ) ပွၵ်ႈ WRC_Abort;
        }
#endif
      }
    }
    bha_nga;
  }
  ပွၵ်ႈ WRC_Continue;
}
puur_naa_nka sqlite3WalkExpr(Walker *pWalker, Expr *pExpr){
  ပွၵ်ႈ pExpr ? sqlite3WalkExprNN(pWalker,pExpr) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
puur_naa_nka sqlite3WalkExprList(Walker *pWalker, ExprList *p){
  puur_naa_nka i;
  sa_nracanaa ExprList_item *pItem;
  ပေႃး( p ){
    for(i=p->nExpr, pItem=p->a; i>0; i--, pItem++){
      ပေႃး( sqlite3WalkExpr(pWalker, pItem->pExpr) ) ပွၵ်ႈ WRC_Abort;
    }
  }
  ပွၵ်ႈ WRC_Continue;
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
puur_naa_nka sqlite3WalkSelectExpr(Walker *pWalker, Select *p){
  ပေႃး( sqlite3WalkExprList(pWalker, p->pEList) ) ပွၵ်ႈ WRC_Abort;
  ပေႃး( sqlite3WalkExpr(pWalker, p->pWhere) ) ပွၵ်ႈ WRC_Abort;
  ပေႃး( sqlite3WalkExprList(pWalker, p->pGroupBy) ) ပွၵ်ႈ WRC_Abort;
  ပေႃး( sqlite3WalkExpr(pWalker, p->pHaving) ) ပွၵ်ႈ WRC_Abort;
  ပေႃး( sqlite3WalkExprList(pWalker, p->pOrderBy) ) ပွၵ်ႈ WRC_Abort;
  ပေႃး( sqlite3WalkExpr(pWalker, p->pLimit) ) ပွၵ်ႈ WRC_Abort;
#ပေႃး !defined(SQLITE_OMIT_WINDOWFUNC)
  ပေႃး( p->pWinDefn ){
    Parse *pParse;
    ပေႃး( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3SelectPopWith
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      puur_naa_nka rc = walkWindowList(pWalker, p->pWinDefn, 0);
      ပွၵ်ႈ rc;
    }
  }
#endif
  ပွၵ်ႈ WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
puur_naa_nka sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  puur_naa_nka i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  ပေႃး( ALWAYS(pSrc) ){
    for(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      ပေႃး( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        ပွၵ်ႈ WRC_Abort;
      }
      ပေႃး( pItem->fg.isTabFunc
       && sqlite3WalkExprList(pWalker, pItem->u1.pFuncArg)
      ){
        ပွၵ်ႈ WRC_Abort;
      }
    }
  }
  ပွၵ်ႈ WRC_Continue;
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
puur_naa_nka sqlite3WalkSelect(Walker *pWalker, Select *p){
  puur_naa_nka rc;
  ပေႃး( p==0 ) ပွၵ်ႈ WRC_Continue;
  ပေႃး( pWalker->xSelectCallback==0 ) ပွၵ်ႈ WRC_Continue;
  do{
    rc = pWalker->xSelectCallback(pWalker, p);
    ပေႃး( rc ) ပွၵ်ႈ rc & WRC_Abort;
    ပေႃး( sqlite3WalkSelectExpr(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      ပွၵ်ႈ WRC_Abort;
    }
    ပေႃး( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }ၶၢဝ်း( p!=0 );
  ပွၵ်ႈ WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
puur_naa_nka sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  ပွၵ်ႈ WRC_Continue;
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
puur_naa_nka sqlite3ExprWalkNoop(Walker *NotUsed, Expr *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ပွၵ်ႈ WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
puur_naa_nka sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ပွၵ်ႈ WRC_Continue;
}
