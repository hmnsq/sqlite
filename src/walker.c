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


#အကယ်၍ !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
static ကိန်းပြည့် walkWindowList(Walker *pWalker, Window *pList, ကိန်းပြည့် bOneOnly){
  Window *pWin;
  အတွက်(pWin=pList; pWin; pWin=pWin->pNextWin){
    ကိန်းပြည့် rc;
    rc = sqlite3WalkExprList(pWalker, pWin->pOrderBy);
    အကယ်၍( rc ) ပြန်ပေး WRC_Abort;
    rc = sqlite3WalkExprList(pWalker, pWin->pPartition);
    အကယ်၍( rc ) ပြန်ပေး WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pFilter);
    အကယ်၍( rc ) ပြန်ပေး WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pStart);
    အကယ်၍( rc ) ပြန်ပေး WRC_Abort;
    rc = sqlite3WalkExpr(pWalker, pWin->pEnd);
    အကယ်၍( rc ) ပြန်ပေး WRC_Abort;
    အကယ်၍( bOneOnly ) bha_nga;
  }
  ပြန်ပေး WRC_Continue;
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
SQLITE_NOINLINE ကိန်းပြည့် sqlite3WalkExprNN(Walker *pWalker, Expr *pExpr){
  ကိန်းပြည့် rc;
  testcase( ExprHasProperty(pExpr, EP_TokenOnly) );
  testcase( ExprHasProperty(pExpr, EP_Reduced) );
  နေစဉ်(1){
    rc = pWalker->xExprCallback(pWalker, pExpr);
    အကယ်၍( rc ) ပြန်ပေး rc & WRC_Abort;
    အကယ်၍( !ExprHasProperty(pExpr,(EP_TokenOnly|EP_Leaf)) ){
      assert( pExpr->x.pList==0 || pExpr->pRight==0 );
      အကယ်၍( pExpr->pLeft && sqlite3WalkExprNN(pWalker, pExpr->pLeft) ){
        ပြန်ပေး WRC_Abort;
      }
      အကယ်၍( pExpr->pRight ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        pExpr = pExpr->pRight;
        continue;
      }မဟုတ်ပါက အကယ်၍( ExprUseXSelect(pExpr) ){
        assert( !ExprHasProperty(pExpr, EP_WinFunc) );
        အကယ်၍( sqlite3WalkSelect(pWalker, pExpr->x.pSelect) ) ပြန်ပေး WRC_Abort;
      }မဟုတ်ပါက{
        အကယ်၍( pExpr->x.pList ){
          အကယ်၍( sqlite3WalkExprList(pWalker, pExpr->x.pList) ) ပြန်ပေး WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        အကယ်၍( ExprHasProperty(pExpr, EP_WinFunc) ){
          အကယ်၍( walkWindowList(pWalker, pExpr->y.pWin, 1) ) ပြန်ပေး WRC_Abort;
        }
#endif
      }
    }
    bha_nga;
  }
  ပြန်ပေး WRC_Continue;
}
ကိန်းပြည့် sqlite3WalkExpr(Walker *pWalker, Expr *pExpr){
  ပြန်ပေး pExpr ? sqlite3WalkExprNN(pWalker,pExpr) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
ကိန်းပြည့် sqlite3WalkExprList(Walker *pWalker, ExprList *p){
  ကိန်းပြည့် i;
  sa_nracanaa ExprList_item *pItem;
  အကယ်၍( p ){
    အတွက်(i=p->nExpr, pItem=p->a; i>0; i--, pItem++){
      အကယ်၍( sqlite3WalkExpr(pWalker, pItem->pExpr) ) ပြန်ပေး WRC_Abort;
    }
  }
  ပြန်ပေး WRC_Continue;
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
ကိန်းပြည့် sqlite3WalkSelectExpr(Walker *pWalker, Select *p){
  အကယ်၍( sqlite3WalkExprList(pWalker, p->pEList) ) ပြန်ပေး WRC_Abort;
  အကယ်၍( sqlite3WalkExpr(pWalker, p->pWhere) ) ပြန်ပေး WRC_Abort;
  အကယ်၍( sqlite3WalkExprList(pWalker, p->pGroupBy) ) ပြန်ပေး WRC_Abort;
  အကယ်၍( sqlite3WalkExpr(pWalker, p->pHaving) ) ပြန်ပေး WRC_Abort;
  အကယ်၍( sqlite3WalkExprList(pWalker, p->pOrderBy) ) ပြန်ပေး WRC_Abort;
  အကယ်၍( sqlite3WalkExpr(pWalker, p->pLimit) ) ပြန်ပေး WRC_Abort;
#အကယ်၍ !defined(SQLITE_OMIT_WINDOWFUNC)
  အကယ်၍( p->pWinDefn ){
    Parse *pParse;
    အကယ်၍( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3SelectPopWith
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      ကိန်းပြည့် rc = walkWindowList(pWalker, p->pWinDefn, 0);
      ပြန်ပေး rc;
    }
  }
#endif
  ပြန်ပေး WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
ကိန်းပြည့် sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  ကိန်းပြည့် i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  အကယ်၍( ALWAYS(pSrc) ){
    အတွက်(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      အကယ်၍( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        ပြန်ပေး WRC_Abort;
      }
      အကယ်၍( pItem->fg.isTabFunc
       && sqlite3WalkExprList(pWalker, pItem->u1.pFuncArg)
      ){
        ပြန်ပေး WRC_Abort;
      }
    }
  }
  ပြန်ပေး WRC_Continue;
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
ကိန်းပြည့် sqlite3WalkSelect(Walker *pWalker, Select *p){
  ကိန်းပြည့် rc;
  အကယ်၍( p==0 ) ပြန်ပေး WRC_Continue;
  အကယ်၍( pWalker->xSelectCallback==0 ) ပြန်ပေး WRC_Continue;
  do{
    rc = pWalker->xSelectCallback(pWalker, p);
    အကယ်၍( rc ) ပြန်ပေး rc & WRC_Abort;
    အကယ်၍( sqlite3WalkSelectExpr(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      ပြန်ပေး WRC_Abort;
    }
    အကယ်၍( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }နေစဉ်( p!=0 );
  ပြန်ပေး WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
ကိန်းပြည့် sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  ပြန်ပေး WRC_Continue;
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
ကိန်းပြည့် sqlite3ExprWalkNoop(Walker *NotUsed, Expr *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ပြန်ပေး WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
ကိန်းပြည့် sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ပြန်ပေး WRC_Continue;
}
