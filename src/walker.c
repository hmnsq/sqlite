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
ساكن صحيح walk_کھڑکی_List(Walker *pWalker, کھڑکی *pList, صحيح bOneOnly){
  کھڑکی *pWin;
  لكل(pWin=pList; pWin; pWin=pWin->p_اگلا_Win){
    صحيح rc;
    rc = sqlite3_Walk_اظہاریہ_List(pWalker, pWin->p_ترتیب_By);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_اظہاریہ_List(pWalker, pWin->p_حصے_کرو);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_اظہاریہ(pWalker, pWin->pFilter);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_اظہاریہ(pWalker, pWin->p_شروع);
    إذا( rc ) أرجع WRC_Abort;
    rc = sqlite3_Walk_اظہاریہ(pWalker, pWin->p_سرا);
    إذا( rc ) أرجع WRC_Abort;
    إذا( bOneOnly ) اكسر;
  }
  أرجع WRC_Continue;
}
#جے_انت

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
SQLITE_NOINLINE صحيح sqlite3_Walk_اظہاریہ_NN(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  صحيح rc;
  testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly) );
  testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_Reduced) );
  طالما(1){
    rc = pWalker->x_اظہاریہ_Callback(pWalker, p_اظہاریہ);
    إذا( rc ) أرجع rc & WRC_Abort;
    إذا( !اظہاریہ_Has_Property(p_اظہاریہ,(EP_TokenOnly|EP_پتی)) ){
      أكد( p_اظہاریہ->x.pList==0 || p_اظہاریہ->p_دایاں==0 );
      إذا( p_اظہاریہ->p_بایاں && sqlite3_Walk_اظہاریہ_NN(pWalker, p_اظہاریہ->p_بایاں) ){
        أرجع WRC_Abort;
      }
      إذا( p_اظہاریہ->p_دایاں ){
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) );
        p_اظہاریہ = p_اظہاریہ->p_دایاں;
        استمر;
      }وإلا إذا( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) );
        إذا( sqlite3WalkSelect(pWalker, p_اظہاریہ->x.pSelect) ) أرجع WRC_Abort;
      }وإلا{
        إذا( p_اظہاریہ->x.pList ){
          إذا( sqlite3_Walk_اظہاریہ_List(pWalker, p_اظہاریہ->x.pList) ) أرجع WRC_Abort;
        }
#جے_بنا_تعریف SQLITE_OMIT_WINDOWFUNC
        إذا( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
          إذا( walk_کھڑکی_List(pWalker, p_اظہاریہ->y.pWin, 1) ) أرجع WRC_Abort;
        }
#جے_انت
      }
    }
    اكسر;
  }
  أرجع WRC_Continue;
}
صحيح sqlite3_Walk_اظہاریہ(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  أرجع p_اظہاریہ ? sqlite3_Walk_اظہاریہ_NN(pWalker,p_اظہاریہ) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
صحيح sqlite3_Walk_اظہاریہ_List(Walker *pWalker, اظہاریہ_List *p){
  صحيح i;
  بنية اظہاریہ_List_شے *p_شے;
  إذا( p ){
    لكل(i=p->n_اظہاریہ, p_شے=p->a; i>0; i--, p_شے++){
      إذا( sqlite3_Walk_اظہاریہ(pWalker, p_شے->p_اظہاریہ) ) أرجع WRC_Abort;
    }
  }
  أرجع WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
فراغ sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
  بن_ورتیا_PARAMETER(pWalker);
  بن_ورتیا_PARAMETER(p);
  /* No-op */
}

/*
** Walk all expressions associated with SELECT statement p.  Do
** not invoke the SELECT callback on p, but do (of course) invoke
** any expr callbacks and SELECT callbacks that come from subqueries.
** Return WRC_Abort or WRC_Continue.
*/
صحيح sqlite3_Walk_Select_اظہاریہ(Walker *pWalker, Select *p){
  إذا( sqlite3_Walk_اظہاریہ_List(pWalker, p->pEList) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_اظہاریہ(pWalker, p->pWhere) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_اظہاریہ_List(pWalker, p->p_گروہ_By) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_اظہاریہ(pWalker, p->pHaving) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_اظہاریہ_List(pWalker, p->p_ترتیب_By) ) أرجع WRC_Abort;
  إذا( sqlite3_Walk_اظہاریہ(pWalker, p->p_حد_بندی) ) أرجع WRC_Abort;
#إذا !defined(SQLITE_OMIT_WINDOWFUNC)
  إذا( p->pWinDefn ){
    Parse *pParse;
    إذا( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && وچ_RENAME_OBJECT)
#جے_بنا_تعریف SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_نکالو_With
#جے_انت
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      صحيح rc = walk_کھڑکی_List(pWalker, p->pWinDefn, 0);
      أرجع rc;
    }
  }
#جے_انت
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
  Src_شے *p_شے;

  pSrc = p->pSrc;
  إذا( ALWAYS(pSrc) ){
    لكل(i=pSrc->nSrc, p_شے=pSrc->a; i>0; i--, p_شے++){
      إذا( p_شے->fg.isSubquery
       && sqlite3WalkSelect(pWalker, p_شے->u4.pSubq->pSelect)
      ){
        أرجع WRC_Abort;
      }
      إذا( p_شے->fg.isTabFunc
       && sqlite3_Walk_اظہاریہ_List(pWalker, p_شے->u1.pFuncArg)
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
    إذا( sqlite3_Walk_Select_اظہاریہ(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      أرجع WRC_Abort;
    }
    إذا( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->p_پہلوں;
  }طالما( p!=0 );
  أرجع WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
صحيح sqlite3_Walker_ڈونگھائی_Increase(Walker *pWalker, Select *pSelect){
  بن_ورتیا_PARAMETER(pSelect);
  pWalker->walker_ڈونگھائی++;
  أرجع WRC_Continue;
}
فراغ sqlite3_Walker_ڈونگھائی_Decrease(Walker *pWalker, Select *pSelect){
  بن_ورتیا_PARAMETER(pSelect);
  pWalker->walker_ڈونگھائی--;
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
صحيح sqlite3_اظہاریہ_Walk_Noop(Walker *Not_ورتیا, اظہاریہ *NotUsed2){
  بن_ورتیا_PARAMETER2(Not_ورتیا, NotUsed2);
  أرجع WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
صحيح sqlite3SelectWalkNoop(Walker *Not_ورتیا, Select *NotUsed2){
  بن_ورتیا_PARAMETER2(Not_ورتیا, NotUsed2);
  أرجع WRC_Continue;
}
