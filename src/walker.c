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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>


#ਜੇ !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
ਸਥਿਰ ਪੂਰਨ walk_ਖਿੜਕੀ_List(Walker *pWalker, ਖਿੜਕੀ *pList, ਪੂਰਨ bOneOnly){
  ਖਿੜਕੀ *pWin;
  ਲਈ(pWin=pList; pWin; pWin=pWin->p_ਅਗਲਾ_Win){
    ਪੂਰਨ rc;
    rc = sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, pWin->p_ਕ੍ਰਮ_By);
    ਜੇ( rc ) ਵਾਪਸ WRC_Abort;
    rc = sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, pWin->p_ਹਿੱਸੇ_ਕਰੋ);
    ਜੇ( rc ) ਵਾਪਸ WRC_Abort;
    rc = sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pWin->pFilter);
    ਜੇ( rc ) ਵਾਪਸ WRC_Abort;
    rc = sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pWin->p_ਸ਼ੁਰੂ);
    ਜੇ( rc ) ਵਾਪਸ WRC_Abort;
    rc = sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, pWin->p_ਸਿਰਾ);
    ਜੇ( rc ) ਵਾਪਸ WRC_Abort;
    ਜੇ( bOneOnly ) ਰੋਕੋ;
  }
  ਵਾਪਸ WRC_Continue;
}
#ਜੇ_ਅੰਤ

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
SQLITE_NOINLINE ਪੂਰਨ sqlite3_Walk_ਵਿਅੰਜਕ_NN(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਪੂਰਨ rc;
  testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_TokenOnly) );
  testcase( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Reduced) );
  ਜਦੋਂ(1){
    rc = pWalker->x_ਵਿਅੰਜਕ_Callback(pWalker, p_ਵਿਅੰਜਕ);
    ਜੇ( rc ) ਵਾਪਸ rc & WRC_Abort;
    ਜੇ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ,(EP_TokenOnly|EP_ਪੱਤੀ)) ){
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pList==0 || p_ਵਿਅੰਜਕ->p_ਸੱਜਾ==0 );
      ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ && sqlite3_Walk_ਵਿਅੰਜਕ_NN(pWalker, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) ){
        ਵਾਪਸ WRC_Abort;
      }
      ਜੇ( p_ਵਿਅੰਜਕ->p_ਸੱਜਾ ){
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) );
        p_ਵਿਅੰਜਕ = p_ਵਿਅੰਜਕ->p_ਸੱਜਾ;
        ਜਾਰੀ;
      }ਨਹੀਂਤਾਂ ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
        ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) );
        ਜੇ( sqlite3WalkSelect(pWalker, p_ਵਿਅੰਜਕ->x.pSelect) ) ਵਾਪਸ WRC_Abort;
      }ਨਹੀਂਤਾਂ{
        ਜੇ( p_ਵਿਅੰਜਕ->x.pList ){
          ਜੇ( sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, p_ਵਿਅੰਜਕ->x.pList) ) ਵਾਪਸ WRC_Abort;
        }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
        ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_WinFunc) ){
          ਜੇ( walk_ਖਿੜਕੀ_List(pWalker, p_ਵਿਅੰਜਕ->y.pWin, 1) ) ਵਾਪਸ WRC_Abort;
        }
#ਜੇ_ਅੰਤ
      }
    }
    ਰੋਕੋ;
  }
  ਵਾਪਸ WRC_Continue;
}
ਪੂਰਨ sqlite3_Walk_ਵਿਅੰਜਕ(Walker *pWalker, ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ){
  ਵਾਪਸ p_ਵਿਅੰਜਕ ? sqlite3_Walk_ਵਿਅੰਜਕ_NN(pWalker,p_ਵਿਅੰਜਕ) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
ਪੂਰਨ sqlite3_Walk_ਵਿਅੰਜਕ_List(Walker *pWalker, ਵਿਅੰਜਕ_List *p){
  ਪੂਰਨ i;
  ਬਣਤਰ ਵਿਅੰਜਕ_List_ਵਸਤੂ *p_ਵਸਤੂ;
  ਜੇ( p ){
    ਲਈ(i=p->n_ਵਿਅੰਜਕ, p_ਵਸਤੂ=p->a; i>0; i--, p_ਵਸਤੂ++){
      ਜੇ( sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, p_ਵਸਤੂ->p_ਵਿਅੰਜਕ) ) ਵਾਪਸ WRC_Abort;
    }
  }
  ਵਾਪਸ WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
ਖਾਲੀ sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
  ਅਣਵਰਤਿਆ_PARAMETER(pWalker);
  ਅਣਵਰਤਿਆ_PARAMETER(p);
  /* No-op */
}

/*
** Walk all expressions associated with SELECT statement p.  Do
** not invoke the SELECT callback on p, but do (of course) invoke
** any expr callbacks and SELECT callbacks that come from subqueries.
** Return WRC_Abort or WRC_Continue.
*/
ਪੂਰਨ sqlite3_Walk_Select_ਵਿਅੰਜਕ(Walker *pWalker, Select *p){
  ਜੇ( sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, p->pEList) ) ਵਾਪਸ WRC_Abort;
  ਜੇ( sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, p->pWhere) ) ਵਾਪਸ WRC_Abort;
  ਜੇ( sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, p->p_ਸਮੂਹ_By) ) ਵਾਪਸ WRC_Abort;
  ਜੇ( sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, p->pHaving) ) ਵਾਪਸ WRC_Abort;
  ਜੇ( sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, p->p_ਕ੍ਰਮ_By) ) ਵਾਪਸ WRC_Abort;
  ਜੇ( sqlite3_Walk_ਵਿਅੰਜਕ(pWalker, p->p_ਹੱਦਬੰਦੀ) ) ਵਾਪਸ WRC_Abort;
#ਜੇ !defined(SQLITE_OMIT_WINDOWFUNC)
  ਜੇ( p->pWinDefn ){
    Parse *pParse;
    ਜੇ( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && ਵਿੱਚ_RENAME_OBJECT)
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_ਕੱਢੋ_With
#ਜੇ_ਅੰਤ
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      ਪੂਰਨ rc = walk_ਖਿੜਕੀ_List(pWalker, p->pWinDefn, 0);
      ਵਾਪਸ rc;
    }
  }
#ਜੇ_ਅੰਤ
  ਵਾਪਸ WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
ਪੂਰਨ sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  ਪੂਰਨ i;
  Src_ਵਸਤੂ *p_ਵਸਤੂ;

  pSrc = p->pSrc;
  ਜੇ( ALWAYS(pSrc) ){
    ਲਈ(i=pSrc->nSrc, p_ਵਸਤੂ=pSrc->a; i>0; i--, p_ਵਸਤੂ++){
      ਜੇ( p_ਵਸਤੂ->fg.isSubquery
       && sqlite3WalkSelect(pWalker, p_ਵਸਤੂ->u4.pSubq->pSelect)
      ){
        ਵਾਪਸ WRC_Abort;
      }
      ਜੇ( p_ਵਸਤੂ->fg.isTabFunc
       && sqlite3_Walk_ਵਿਅੰਜਕ_List(pWalker, p_ਵਸਤੂ->u1.pFuncArg)
      ){
        ਵਾਪਸ WRC_Abort;
      }
    }
  }
  ਵਾਪਸ WRC_Continue;
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
ਪੂਰਨ sqlite3WalkSelect(Walker *pWalker, Select *p){
  ਪੂਰਨ rc;
  ਜੇ( p==0 ) ਵਾਪਸ WRC_Continue;
  ਜੇ( pWalker->xSelectCallback==0 ) ਵਾਪਸ WRC_Continue;
  ਕਰੋ{
    rc = pWalker->xSelectCallback(pWalker, p);
    ਜੇ( rc ) ਵਾਪਸ rc & WRC_Abort;
    ਜੇ( sqlite3_Walk_Select_ਵਿਅੰਜਕ(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      ਵਾਪਸ WRC_Abort;
    }
    ਜੇ( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->p_ਪੂਰਵ;
  }ਜਦੋਂ( p!=0 );
  ਵਾਪਸ WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
ਪੂਰਨ sqlite3_Walker_ਡੂੰਘਾਈ_Increase(Walker *pWalker, Select *pSelect){
  ਅਣਵਰਤਿਆ_PARAMETER(pSelect);
  pWalker->walker_ਡੂੰਘਾਈ++;
  ਵਾਪਸ WRC_Continue;
}
ਖਾਲੀ sqlite3_Walker_ਡੂੰਘਾਈ_Decrease(Walker *pWalker, Select *pSelect){
  ਅਣਵਰਤਿਆ_PARAMETER(pSelect);
  pWalker->walker_ਡੂੰਘਾਈ--;
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
ਪੂਰਨ sqlite3_ਵਿਅੰਜਕ_Walk_Noop(Walker *Not_ਵਰਤਿਆ, ਵਿਅੰਜਕ *NotUsed2){
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  ਵਾਪਸ WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
ਪੂਰਨ sqlite3SelectWalkNoop(Walker *Not_ਵਰਤਿਆ, Select *NotUsed2){
  ਅਣਵਰਤਿਆ_PARAMETER2(Not_ਵਰਤਿਆ, NotUsed2);
  ਵਾਪਸ WRC_Continue;
}
