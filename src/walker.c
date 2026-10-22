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


#නම් !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
static පූර්ණ_සංඛ්‍යා walkWindowList(Walker *pWalker, Window *pList, පූර්ණ_සංඛ්‍යා bOneOnly){
  Window *pWin;
  සඳහා(pWin=pList; pWin; pWin=pWin->pNextWin){
    පූර්ණ_සංඛ්‍යා rc;
    rc = sqlite3WalkExprList(pWalker, pWin->pOrderBy);
    නම්( rc ) ආපසු WRC_Abort;
    rc = sqlite3WalkExprList(pWalker, pWin->pPartition);
    නම්( rc ) ආපසු WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pFilter);
    නම්( rc ) ආපසු WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pStart);
    නම්( rc ) ආපසු WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pEnd);
    නම්( rc ) ආපසු WRC_Abort;
    නම්( bOneOnly ) bha_nga;
  }
  ආපසු WRC_Continue;
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
SQLITE_NOINLINE පූර්ණ_සංඛ්‍යා sqlite3WalkExprNN(Walker *pWalker, Expr *pExpr){
  පූර්ණ_සංඛ්‍යා rc;
  testcase( ExprHasProperty(pExpr, EP_TokenOnly) );
  testcase( ExprHasProperty(pExpr, EP_Reduced) );
  අතරතුර(1){
    rc = pWalker->xExprCallback(pWalker, pExpr);
    නම්( rc ) ආපසු rc & WRC_Abort;
    නම්( !ExprHasProperty(pExpr,(EP_TokenOnly|EP_Leaf)) ){
      assert( pExpr->x.pList==0 || pExpr->pRight==0 );
      නම්( pExpr->pLeft && sqlite3WalkExprNN(pWalker, pExpr->pLeft) ){
        ආපසු WRC_Abort;
      }
      නම්( pExpr->pRight ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        pExpr = pExpr->pRight;
        continue;
      }නැතහොත් නම්( ExprUseXSelect(pExpr) ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        නම්( sqlite3WalkSelect(pWalker, pExpr->x.pSelect) ) ආපසු WRC_Abort;
      }නැතහොත්{
        නම්( pExpr->x.pList ){
          නම්( sqlite3WalkExprList(pWalker, pExpr->x.pList) ) ආපසු WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        නම්( ExprHasProperty(pExpr, EP_WinFunc) ){
          නම්( walkWindowList(pWalker, pExpr->y.pWin, 1) ) ආපසු WRC_Abort;
        }
#endif
      }
    }
    bha_nga;
  }
  ආපසු WRC_Continue;
}
පූර්ණ_සංඛ්‍යා sqlite3WalkExpr(Walker *pWalker, Expr *pExpr){
  ආපසු pExpr ? sqlite3WalkExprNN(pWalker,pExpr) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
පූර්ණ_සංඛ්‍යා sqlite3WalkExprList(Walker *pWalker, ExprList *p){
  පූර්ණ_සංඛ්‍යා i;
  sa_nracanaa ExprList_item *pItem;
  නම්( p ){
    සඳහා(i=p->nExpr, pItem=p->a; i>0; i--, pItem++){
      නම්( sqlite3WalkExpr(pWalker, pItem->pExpr) ) ආපසු WRC_Abort;
    }
  }
  ආපසු WRC_Continue;
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
පූර්ණ_සංඛ්‍යා sqlite3WalkSelectExpr(Walker *pWalker, Select *p){
  නම්( sqlite3WalkExprList(pWalker, p->pEList) ) ආපසු WRC_Abort;
  නම්( sqlite3WalkExpr(pWalker, p->pWhere) ) ආපසු WRC_Abort;
  නම්( sqlite3WalkExprList(pWalker, p->pGroupBy) ) ආපසු WRC_Abort;
  නම්( sqlite3WalkExpr(pWalker, p->pHaving) ) ආපසු WRC_Abort;
  නම්( sqlite3WalkExprList(pWalker, p->pOrderBy) ) ආපසු WRC_Abort;
  නම්( sqlite3WalkExpr(pWalker, p->pLimit) ) ආපසු WRC_Abort;
#නම් !defined(SQLITE_OMIT_WINDOWFUNC)
  නම්( p->pWinDefn ){
    Parse *pParse;
    නම්( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3SelectPopWith
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      පූර්ණ_සංඛ්‍යා rc = walkWindowList(pWalker, p->pWinDefn, 0);
      ආපසු rc;
    }
  }
#endif
  ආපසු WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
පූර්ණ_සංඛ්‍යා sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  පූර්ණ_සංඛ්‍යා i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  නම්( ALWAYS(pSrc) ){
    සඳහා(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      නම්( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        ආපසු WRC_Abort;
      }
      නම්( pItem->fg.isTabFunc
       && sqlite3WalkExprList(pWalker, pItem->u1.pFuncArg)
      ){
        ආපසු WRC_Abort;
      }
    }
  }
  ආපසු WRC_Continue;
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
පූර්ණ_සංඛ්‍යා sqlite3WalkSelect(Walker *pWalker, Select *p){
  පූර්ණ_සංඛ්‍යා rc;
  නම්( p==0 ) ආපසු WRC_Continue;
  නම්( pWalker->xSelectCallback==0 ) ආපසු WRC_Continue;
  do{
    rc = pWalker->xSelectCallback(pWalker, p);
    නම්( rc ) ආපසු rc & WRC_Abort;
    නම්( sqlite3WalkSelectExpr(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      ආපසු WRC_Abort;
    }
    නම්( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }අතරතුර( p!=0 );
  ආපසු WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
පූර්ණ_සංඛ්‍යා sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  ආපසු WRC_Continue;
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
පූර්ණ_සංඛ්‍යා sqlite3ExprWalkNoop(Walker *NotUsed, Expr *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ආපසු WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
පූර්ණ_සංඛ්‍යා sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ආපසු WRC_Continue;
}
