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
#شامل "sqliteInt.h"
#شامل <stdlib.h>
#شامل <string.h>


#اگر !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
ایستا صحیح walkWindowList(Walker *pWalker, Window *pList, صحیح bOneOnly){
  Window *pWin;
  برای(pWin=pList; pWin; pWin=pWin->pNextWin){
    صحیح rc;
    rc = sqlite3_Walk_عبارت_List(pWalker, pWin->pOrderBy);
    اگر( rc ) بازگشت WRC_Abort;
    rc = sqlite3_Walk_عبارت_List(pWalker, pWin->pPartition);
    اگر( rc ) بازگشت WRC_Abort;
    rc = sqlite3_Walk_عبارت(pWalker, pWin->pFilter);
    اگر( rc ) بازگشت WRC_Abort;
    rc = sqlite3_Walk_عبارت(pWalker, pWin->pStart);
    اگر( rc ) بازگشت WRC_Abort;
    rc = sqlite3_Walk_عبارت(pWalker, pWin->pEnd);
    اگر( rc ) بازگشت WRC_Abort;
    اگر( bOneOnly ) بشکن;
  }
  بازگشت WRC_Continue;
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
SQLITE_NOINLINE صحیح sqlite3_Walk_عبارت_NN(Walker *pWalker, عبارت *p_عبارت){
  صحیح rc;
  testcase( عبارت_Has_Property(p_عبارت, EP_TokenOnly) );
  testcase( عبارت_Has_Property(p_عبارت, EP_Reduced) );
  وقتی(1){
    rc = pWalker->x_عبارت_Callback(pWalker, p_عبارت);
    اگر( rc ) بازگشت rc & WRC_Abort;
    اگر( !عبارت_Has_Property(p_عبارت,(EP_TokenOnly|EP_Leaf)) ){
      أكد( p_عبارت->x.pList==0 || p_عبارت->p_راست==0 );
      اگر( p_عبارت->p_چپ && sqlite3_Walk_عبارت_NN(pWalker, p_عبارت->p_چپ) ){
        بازگشت WRC_Abort;
      }
      اگر( p_عبارت->p_راست ){
        أكد( !عبارت_Has_Property(p_عبارت, EP_WinFunc) );
        p_عبارت = p_عبارت->p_راست;
        ادامه;
      }وگرنه اگر( عبارت_Use_XSelect(p_عبارت) ){
        أكد( !عبارت_Has_Property(p_عبارت, EP_WinFunc) );
        اگر( sqlite3WalkSelect(pWalker, p_عبارت->x.pSelect) ) بازگشت WRC_Abort;
      }وگرنه{
        اگر( p_عبارت->x.pList ){
          اگر( sqlite3_Walk_عبارت_List(pWalker, p_عبارت->x.pList) ) بازگشت WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        اگر( عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
          اگر( walkWindowList(pWalker, p_عبارت->y.pWin, 1) ) بازگشت WRC_Abort;
        }
#endif
      }
    }
    بشکن;
  }
  بازگشت WRC_Continue;
}
صحیح sqlite3_Walk_عبارت(Walker *pWalker, عبارت *p_عبارت){
  بازگشت p_عبارت ? sqlite3_Walk_عبارت_NN(pWalker,p_عبارت) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
صحیح sqlite3_Walk_عبارت_List(Walker *pWalker, عبارت_List *p){
  صحیح i;
  ساختار عبارت_List_item *pItem;
  اگر( p ){
    برای(i=p->n_عبارت, pItem=p->a; i>0; i--, pItem++){
      اگر( sqlite3_Walk_عبارت(pWalker, pItem->p_عبارت) ) بازگشت WRC_Abort;
    }
  }
  بازگشت WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
تهی sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
صحیح sqlite3_Walk_Select_عبارت(Walker *pWalker, Select *p){
  اگر( sqlite3_Walk_عبارت_List(pWalker, p->pEList) ) بازگشت WRC_Abort;
  اگر( sqlite3_Walk_عبارت(pWalker, p->pWhere) ) بازگشت WRC_Abort;
  اگر( sqlite3_Walk_عبارت_List(pWalker, p->pGroupBy) ) بازگشت WRC_Abort;
  اگر( sqlite3_Walk_عبارت(pWalker, p->pHaving) ) بازگشت WRC_Abort;
  اگر( sqlite3_Walk_عبارت_List(pWalker, p->pOrderBy) ) بازگشت WRC_Abort;
  اگر( sqlite3_Walk_عبارت(pWalker, p->pLimit) ) بازگشت WRC_Abort;
#اگر !defined(SQLITE_OMIT_WINDOWFUNC)
  اگر( p->pWinDefn ){
    Parse *pParse;
    اگر( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_برداشت_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      صحیح rc = walkWindowList(pWalker, p->pWinDefn, 0);
      بازگشت rc;
    }
  }
#endif
  بازگشت WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
صحیح sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  صحیح i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  اگر( ALWAYS(pSrc) ){
    برای(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      اگر( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        بازگشت WRC_Abort;
      }
      اگر( pItem->fg.isTabFunc
       && sqlite3_Walk_عبارت_List(pWalker, pItem->u1.pFuncArg)
      ){
        بازگشت WRC_Abort;
      }
    }
  }
  بازگشت WRC_Continue;
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
صحیح sqlite3WalkSelect(Walker *pWalker, Select *p){
  صحیح rc;
  اگر( p==0 ) بازگشت WRC_Continue;
  اگر( pWalker->xSelectCallback==0 ) بازگشت WRC_Continue;
  انجامبده{
    rc = pWalker->xSelectCallback(pWalker, p);
    اگر( rc ) بازگشت rc & WRC_Abort;
    اگر( sqlite3_Walk_Select_عبارت(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      بازگشت WRC_Abort;
    }
    اگر( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }وقتی( p!=0 );
  بازگشت WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
صحیح sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  بازگشت WRC_Continue;
}
تهی sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
صحیح sqlite3_عبارت_Walk_Noop(Walker *NotUsed, عبارت *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  بازگشت WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
صحیح sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  بازگشت WRC_Continue;
}
