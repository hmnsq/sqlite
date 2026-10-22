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


#ถ้า !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
static จำนวนเต็ม walkWindowList(Walker *pWalker, Window *pList, จำนวนเต็ม bOneOnly){
  Window *pWin;
  สำหรับ(pWin=pList; pWin; pWin=pWin->pNextWin){
    จำนวนเต็ม rc;
    rc = sqlite3WalkExprList(pWalker, pWin->pOrderBy);
    ถ้า( rc ) คืน WRC_Abort;
    rc = sqlite3WalkExprList(pWalker, pWin->pPartition);
    ถ้า( rc ) คืน WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pFilter);
    ถ้า( rc ) คืน WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pStart);
    ถ้า( rc ) คืน WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pEnd);
    ถ้า( rc ) คืน WRC_Abort;
    ถ้า( bOneOnly ) bha_nga;
  }
  คืน WRC_Continue;
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
SQLITE_NOINLINE จำนวนเต็ม sqlite3WalkExprNN(Walker *pWalker, Expr *pExpr){
  จำนวนเต็ม rc;
  testcase( ExprHasProperty(pExpr, EP_TokenOnly) );
  testcase( ExprHasProperty(pExpr, EP_Reduced) );
  ขณะ(1){
    rc = pWalker->xExprCallback(pWalker, pExpr);
    ถ้า( rc ) คืน rc & WRC_Abort;
    ถ้า( !ExprHasProperty(pExpr,(EP_TokenOnly|EP_Leaf)) ){
      assert( pExpr->x.pList==0 || pExpr->pRight==0 );
      ถ้า( pExpr->pLeft && sqlite3WalkExprNN(pWalker, pExpr->pLeft) ){
        คืน WRC_Abort;
      }
      ถ้า( pExpr->pRight ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        pExpr = pExpr->pRight;
        continue;
      }มิฉะนั้น ถ้า( ExprUseXSelect(pExpr) ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        ถ้า( sqlite3WalkSelect(pWalker, pExpr->x.pSelect) ) คืน WRC_Abort;
      }มิฉะนั้น{
        ถ้า( pExpr->x.pList ){
          ถ้า( sqlite3WalkExprList(pWalker, pExpr->x.pList) ) คืน WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        ถ้า( ExprHasProperty(pExpr, EP_WinFunc) ){
          ถ้า( walkWindowList(pWalker, pExpr->y.pWin, 1) ) คืน WRC_Abort;
        }
#endif
      }
    }
    bha_nga;
  }
  คืน WRC_Continue;
}
จำนวนเต็ม sqlite3WalkExpr(Walker *pWalker, Expr *pExpr){
  คืน pExpr ? sqlite3WalkExprNN(pWalker,pExpr) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
จำนวนเต็ม sqlite3WalkExprList(Walker *pWalker, ExprList *p){
  จำนวนเต็ม i;
  sa_nracanaa ExprList_item *pItem;
  ถ้า( p ){
    สำหรับ(i=p->nExpr, pItem=p->a; i>0; i--, pItem++){
      ถ้า( sqlite3WalkExpr(pWalker, pItem->pExpr) ) คืน WRC_Abort;
    }
  }
  คืน WRC_Continue;
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
จำนวนเต็ม sqlite3WalkSelectExpr(Walker *pWalker, Select *p){
  ถ้า( sqlite3WalkExprList(pWalker, p->pEList) ) คืน WRC_Abort;
  ถ้า( sqlite3WalkExpr(pWalker, p->pWhere) ) คืน WRC_Abort;
  ถ้า( sqlite3WalkExprList(pWalker, p->pGroupBy) ) คืน WRC_Abort;
  ถ้า( sqlite3WalkExpr(pWalker, p->pHaving) ) คืน WRC_Abort;
  ถ้า( sqlite3WalkExprList(pWalker, p->pOrderBy) ) คืน WRC_Abort;
  ถ้า( sqlite3WalkExpr(pWalker, p->pLimit) ) คืน WRC_Abort;
#ถ้า !defined(SQLITE_OMIT_WINDOWFUNC)
  ถ้า( p->pWinDefn ){
    Parse *pParse;
    ถ้า( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3SelectPopWith
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      จำนวนเต็ม rc = walkWindowList(pWalker, p->pWinDefn, 0);
      คืน rc;
    }
  }
#endif
  คืน WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
จำนวนเต็ม sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  จำนวนเต็ม i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  ถ้า( ALWAYS(pSrc) ){
    สำหรับ(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      ถ้า( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        คืน WRC_Abort;
      }
      ถ้า( pItem->fg.isTabFunc
       && sqlite3WalkExprList(pWalker, pItem->u1.pFuncArg)
      ){
        คืน WRC_Abort;
      }
    }
  }
  คืน WRC_Continue;
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
จำนวนเต็ม sqlite3WalkSelect(Walker *pWalker, Select *p){
  จำนวนเต็ม rc;
  ถ้า( p==0 ) คืน WRC_Continue;
  ถ้า( pWalker->xSelectCallback==0 ) คืน WRC_Continue;
  do{
    rc = pWalker->xSelectCallback(pWalker, p);
    ถ้า( rc ) คืน rc & WRC_Abort;
    ถ้า( sqlite3WalkSelectExpr(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      คืน WRC_Abort;
    }
    ถ้า( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }ขณะ( p!=0 );
  คืน WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
จำนวนเต็ม sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  คืน WRC_Continue;
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
จำนวนเต็ม sqlite3ExprWalkNoop(Walker *NotUsed, Expr *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  คืน WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
จำนวนเต็ม sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  คืน WRC_Continue;
}
