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
ساكن صحيح walk_پنجره_List(Walker *pWalker, پنجره *pList, صحيح bOneOnly){
  پنجره *pWin;
  لكل(pWin=pList; pWin; pWin=pWin->p_بعدی_Win){
    صحيح rc;
    rc = sqlite3_Walk_عبارت_List(pWalker, pWin->p_ترتیب_By);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_عبارت_List(pWalker, pWin->p_افراز);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_عبارت(pWalker, pWin->pFilter);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_عبارت(pWalker, pWin->p_شروع);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_عبارت(pWalker, pWin->p_انتها);
    إذا( rc ) أرجع WRC_Abort;
    إذا( bOneOnly ) اكسر;
  }
  أرجع WRC_Continue;
}
#پایان_اگر

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
SQLITE_NOINLINE صحيح sqlite3_Walk_عبارت_NN(Walker *pWalker, عبارت *p_عبارت){
  صحيح rc;
  testcase( عبارت_Has_Property(p_عبارت, EP_TokenOnly) );
  testcase( عبارت_Has_Property(p_عبارت, EP_Reduced) );
  طالما(1){
    rc = pWalker->x_عبارت_Callback(pWalker, p_عبارت);
    إذا( rc ) أرجع rc & WRC_Abort;
    إذا( !عبارت_Has_Property(p_عبارت,(EP_TokenOnly|EP_برگ)) ){
      أكد( p_عبارت->x.pList==0 || p_عبارت->p_راست==0 );
      إذا( p_عبارت->p_چپ && sqlite3_Walk_عبارت_NN(pWalker, p_عبارت->p_چپ) ){
        أرجع WRC_Abort;
      }
      إذا( p_عبارت->p_راست ){
        أكد( !عبارت_Has_Property(p_عبارت, EP_WinFunc) );
        p_عبارت = p_عبارت->p_راست;
        استمر;
      }وإلا إذا( عبارت_Use_XSelect(p_عبارت) ){
        أكد( !عبارت_Has_Property(p_عبارت, EP_WinFunc) );
        إذا( sqlite3WalkSelect(pWalker, p_عبارت->x.pSelect) ) أرجع WRC_Abort;
      }وإلا{
        إذا( p_عبارت->x.pList ){
          إذا( sqlite3_Walk_عبارت_List(pWalker, p_عبارت->x.pList) ) أرجع WRC_Abort;
        }
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
        إذا( عبارت_Has_Property(p_عبارت, EP_WinFunc) ){
          إذا( walk_پنجره_List(pWalker, p_عبارت->y.pWin, 1) ) أرجع WRC_Abort;
        }
#پایان_اگر
      }
    }
    اكسر;
  }
  أرجع WRC_Continue;
}
صحيح sqlite3_Walk_عبارت(Walker *pWalker, عبارت *p_عبارت){
  أرجع p_عبارت ? sqlite3_Walk_عبارت_NN(pWalker,p_عبارت) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
صحيح sqlite3_Walk_عبارت_List(Walker *pWalker, عبارت_List *p){
  صحيح i;
  بنية عبارت_List_قلم *p_قلم;
  إذا( p ){
    لكل(i=p->n_عبارت, p_قلم=p->a; i>0; i--, p_قلم++){
      إذا( sqlite3_Walk_عبارت(pWalker, p_قلم->p_عبارت) ) أرجع WRC_Abort;
    }
  }
  أرجع WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
فراغ sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
  استفادهنشده_PARAMETER(pWalker);
  استفادهنشده_PARAMETER(p);
  /* No-op */
}

/*
** Walk all expressions associated with SELECT statement p.  Do
** not invoke the SELECT callback on p, but do (of course) invoke
** any expr callbacks and SELECT callbacks that come from subqueries.
** Return WRC_Abort or WRC_Continue.
*/
صحيح sqlite3_Walk_Select_عبارت(Walker *pWalker, Select *p){
  إذا( sqlite3_Walk_عبارت_List(pWalker, p->pEList) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_عبارت(pWalker, p->pWhere) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_عبارت_List(pWalker, p->p_گروه_By) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_عبارت(pWalker, p->pHaving) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_عبارت_List(pWalker, p->p_ترتیب_By) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_عبارت(pWalker, p->p_حد) ) أرجع WRC_Abort;
#إذا !defined(SQLITE_OMIT_WINDOWFUNC)
  إذا( p->pWinDefn ){
    Parse *pParse;
    إذا( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && درون_RENAME_OBJECT)
#اگر_تعریف_نشده SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_برداشت_With
#پایان_اگر
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      صحيح rc = walk_پنجره_List(pWalker, p->pWinDefn, 0);
      أرجع rc;
    }
  }
#پایان_اگر
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
  Src_قلم *p_قلم;

  pSrc = p->pSrc;
  إذا( ALWAYS(pSrc) ){
    لكل(i=pSrc->nSrc, p_قلم=pSrc->a; i>0; i--, p_قلم++){
      إذا( p_قلم->fg.isSubquery
       && sqlite3WalkSelect(pWalker, p_قلم->u4.pSubq->pSelect)
      ){
        أرجع WRC_Abort;
      }
      إذا( p_قلم->fg.isTabFunc
       && sqlite3_Walk_عبارت_List(pWalker, p_قلم->u1.pFuncArg)
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
    إذا( sqlite3_Walk_Select_عبارت(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      أرجع WRC_Abort;
    }
    إذا( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->p_پیشین;
  }طالما( p!=0 );
  أرجع WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
صحيح sqlite3_Walker_عمق_Increase(Walker *pWalker, Select *pSelect){
  استفادهنشده_PARAMETER(pSelect);
  pWalker->walker_عمق++;
  أرجع WRC_Continue;
}
فراغ sqlite3_Walker_عمق_Decrease(Walker *pWalker, Select *pSelect){
  استفادهنشده_PARAMETER(pSelect);
  pWalker->walker_عمق--;
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
صحيح sqlite3_عبارت_Walk_Noop(Walker *Not_استفاده_شده, عبارت *NotUsed2){
  استفادهنشده_PARAMETER2(Not_استفاده_شده, NotUsed2);
  أرجع WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
صحيح sqlite3SelectWalkNoop(Walker *Not_استفاده_شده, Select *NotUsed2){
  استفادهنشده_PARAMETER2(Not_استفاده_شده, NotUsed2);
  أرجع WRC_Continue;
}
