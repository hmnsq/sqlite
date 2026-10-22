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
ساکن صحیح walk_کھڑکی_List(Walker *pWalker, کھڑکی *pList, صحیح bOneOnly){
  کھڑکی *pWin;
  برائے(pWin=pList; pWin; pWin=pWin->p_اگلا_Win){
    صحیح rc;
    rc = sqlite3_Walk_اظہاریہ_List(pWalker, pWin->p_ترتیب_By);
    اگر( rc ) واپس WRC_Abort;
    rc = sqlite3_Walk_اظہاریہ_List(pWalker, pWin->p_تقسیم);
    اگر( rc ) واپس WRC_Abort;
    rc = sqlite3_Walk_اظہاریہ(pWalker, pWin->pFilter);
    اگر( rc ) واپس WRC_Abort;
    rc = sqlite3_Walk_اظہاریہ(pWalker, pWin->p_آغاز);
    اگر( rc ) واپس WRC_Abort;
    rc = sqlite3_Walk_اظہاریہ(pWalker, pWin->p_انتہا);
    اگر( rc ) واپس WRC_Abort;
    اگر( bOneOnly ) توڑو;
  }
  واپس WRC_Continue;
}
#اگر_ختم

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
SQLITE_NOINLINE صحیح sqlite3_Walk_اظہاریہ_NN(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  صحیح rc;
  testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly) );
  testcase( اظہاریہ_Has_Property(p_اظہاریہ, EP_Reduced) );
  جب(1){
    rc = pWalker->x_اظہاریہ_Callback(pWalker, p_اظہاریہ);
    اگر( rc ) واپس rc & WRC_Abort;
    اگر( !اظہاریہ_Has_Property(p_اظہاریہ,(EP_TokenOnly|EP_برگ)) ){
      أكد( p_اظہاریہ->x.pList==0 || p_اظہاریہ->p_دایاں==0 );
      اگر( p_اظہاریہ->p_بایاں && sqlite3_Walk_اظہاریہ_NN(pWalker, p_اظہاریہ->p_بایاں) ){
        واپس WRC_Abort;
      }
      اگر( p_اظہاریہ->p_دایاں ){
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) );
        p_اظہاریہ = p_اظہاریہ->p_دایاں;
        جاری;
      }ورنہ اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
        أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) );
        اگر( sqlite3WalkSelect(pWalker, p_اظہاریہ->x.pSelect) ) واپس WRC_Abort;
      }ورنہ{
        اگر( p_اظہاریہ->x.pList ){
          اگر( sqlite3_Walk_اظہاریہ_List(pWalker, p_اظہاریہ->x.pList) ) واپس WRC_Abort;
        }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
        اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_WinFunc) ){
          اگر( walk_کھڑکی_List(pWalker, p_اظہاریہ->y.pWin, 1) ) واپس WRC_Abort;
        }
#اگر_ختم
      }
    }
    توڑو;
  }
  واپس WRC_Continue;
}
صحیح sqlite3_Walk_اظہاریہ(Walker *pWalker, اظہاریہ *p_اظہاریہ){
  واپس p_اظہاریہ ? sqlite3_Walk_اظہاریہ_NN(pWalker,p_اظہاریہ) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
صحیح sqlite3_Walk_اظہاریہ_List(Walker *pWalker, اظہاریہ_List *p){
  صحیح i;
  ساخت اظہاریہ_List_شے *p_شے;
  اگر( p ){
    برائے(i=p->n_اظہاریہ, p_شے=p->a; i>0; i--, p_شے++){
      اگر( sqlite3_Walk_اظہاریہ(pWalker, p_شے->p_اظہاریہ) ) واپس WRC_Abort;
    }
  }
  واپس WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
خالی sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
  غیر_مستعمل_PARAMETER(pWalker);
  غیر_مستعمل_PARAMETER(p);
  /* No-op */
}

/*
** Walk all expressions associated with SELECT statement p.  Do
** not invoke the SELECT callback on p, but do (of course) invoke
** any expr callbacks and SELECT callbacks that come from subqueries.
** Return WRC_Abort or WRC_Continue.
*/
صحیح sqlite3_Walk_Select_اظہاریہ(Walker *pWalker, Select *p){
  اگر( sqlite3_Walk_اظہاریہ_List(pWalker, p->pEList) ) واپس WRC_Abort;
  اگر( sqlite3_Walk_اظہاریہ(pWalker, p->pWhere) ) واپس WRC_Abort;
  اگر( sqlite3_Walk_اظہاریہ_List(pWalker, p->p_گروہ_By) ) واپس WRC_Abort;
  اگر( sqlite3_Walk_اظہاریہ(pWalker, p->pHaving) ) واپس WRC_Abort;
  اگر( sqlite3_Walk_اظہاریہ_List(pWalker, p->p_ترتیب_By) ) واپس WRC_Abort;
  اگر( sqlite3_Walk_اظہاریہ(pWalker, p->p_حد_بندی) ) واپس WRC_Abort;
#اگر !defined(SQLITE_OMIT_WINDOWFUNC)
  اگر( p->pWinDefn ){
    Parse *pParse;
    اگر( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && اندر_RENAME_OBJECT)
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_نکالو_With
#اگر_ختم
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      صحیح rc = walk_کھڑکی_List(pWalker, p->pWinDefn, 0);
      واپس rc;
    }
  }
#اگر_ختم
  واپس WRC_Continue;
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
  Src_شے *p_شے;

  pSrc = p->pSrc;
  اگر( ALWAYS(pSrc) ){
    برائے(i=pSrc->nSrc, p_شے=pSrc->a; i>0; i--, p_شے++){
      اگر( p_شے->fg.isSubquery
       && sqlite3WalkSelect(pWalker, p_شے->u4.pSubq->pSelect)
      ){
        واپس WRC_Abort;
      }
      اگر( p_شے->fg.isTabFunc
       && sqlite3_Walk_اظہاریہ_List(pWalker, p_شے->u1.pFuncArg)
      ){
        واپس WRC_Abort;
      }
    }
  }
  واپس WRC_Continue;
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
  اگر( p==0 ) واپس WRC_Continue;
  اگر( pWalker->xSelectCallback==0 ) واپس WRC_Continue;
  کرو{
    rc = pWalker->xSelectCallback(pWalker, p);
    اگر( rc ) واپس rc & WRC_Abort;
    اگر( sqlite3_Walk_Select_اظہاریہ(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      واپس WRC_Abort;
    }
    اگر( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->p_پیشگی;
  }جب( p!=0 );
  واپس WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
صحیح sqlite3_Walker_گہرائی_Increase(Walker *pWalker, Select *pSelect){
  غیر_مستعمل_PARAMETER(pSelect);
  pWalker->walker_گہرائی++;
  واپس WRC_Continue;
}
خالی sqlite3_Walker_گہرائی_Decrease(Walker *pWalker, Select *pSelect){
  غیر_مستعمل_PARAMETER(pSelect);
  pWalker->walker_گہرائی--;
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
صحیح sqlite3_اظہاریہ_Walk_Noop(Walker *Not_مستعمل, اظہاریہ *NotUsed2){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  واپس WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
صحیح sqlite3SelectWalkNoop(Walker *Not_مستعمل, Select *NotUsed2){
  غیر_مستعمل_PARAMETER2(Not_مستعمل, NotUsed2);
  واپس WRC_Continue;
}
