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
#অন্তর্ভাৱ "sqliteInt.h"
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <string.h>


#যদি !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
স্থির পূর্ণাঙ্ক walkWindowList(Walker *pWalker, Window *pList, পূর্ণাঙ্ক bOneOnly){
  Window *pWin;
  বাবে(pWin=pList; pWin; pWin=pWin->pNextWin){
    পূর্ণাঙ্ক rc;
    rc = sqlite3_Walk_ৰাশিমালা_List(pWalker, pWin->pOrderBy);
    যদি( rc ) ঘূৰাই WRC_Abort;
    rc = sqlite3_Walk_ৰাশিমালা_List(pWalker, pWin->pPartition);
    যদি( rc ) ঘূৰাই WRC_Abort;
    rc = sqlite3_Walk_ৰাশিমালা(pWalker, pWin->pFilter);
    যদি( rc ) ঘূৰাই WRC_Abort;
    rc = sqlite3_Walk_ৰাশিমালা(pWalker, pWin->pStart);
    যদি( rc ) ঘূৰাই WRC_Abort;
    rc = sqlite3_Walk_ৰাশিমালা(pWalker, pWin->pEnd);
    যদি( rc ) ঘূৰাই WRC_Abort;
    যদি( bOneOnly ) ভঙ্গ;
  }
  ঘূৰাই WRC_Continue;
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
SQLITE_NOINLINE পূর্ণাঙ্ক sqlite3_Walk_ৰাশিমালা_NN(Walker *pWalker, ৰাশিমালা *p_ৰাশিমালা){
  পূর্ণাঙ্ক rc;
  testcase( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_TokenOnly) );
  testcase( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Reduced) );
  যেতিয়ালৈ(1){
    rc = pWalker->x_ৰাশিমালা_Callback(pWalker, p_ৰাশিমালা);
    যদি( rc ) ঘূৰাই rc & WRC_Abort;
    যদি( !ৰাশিমালা_Has_Property(p_ৰাশিমালা,(EP_TokenOnly|EP_Leaf)) ){
      প্রতিজ্ঞা( p_ৰাশিমালা->x.pList==0 || p_ৰাশিমালা->p_ডান==0 );
      যদি( p_ৰাশিমালা->p_বাম && sqlite3_Walk_ৰাশিমালা_NN(pWalker, p_ৰাশিমালা->p_বাম) ){
        ঘূৰাই WRC_Abort;
      }
      যদি( p_ৰাশিমালা->p_ডান ){
        প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_WinFunc) );
        p_ৰাশিমালা = p_ৰাশিমালা->p_ডান;
        অনুৱর্ত;
      }অন্যথা যদি( ৰাশিমালা_Use_XSelect(p_ৰাশিমালা) ){
        প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_WinFunc) );
        যদি( sqlite3WalkSelect(pWalker, p_ৰাশিমালা->x.pSelect) ) ঘূৰাই WRC_Abort;
      }অন্যথা{
        যদি( p_ৰাশিমালা->x.pList ){
          যদি( sqlite3_Walk_ৰাশিমালা_List(pWalker, p_ৰাশিমালা->x.pList) ) ঘূৰাই WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        যদি( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_WinFunc) ){
          যদি( walkWindowList(pWalker, p_ৰাশিমালা->y.pWin, 1) ) ঘূৰাই WRC_Abort;
        }
#endif
      }
    }
    ভঙ্গ;
  }
  ঘূৰাই WRC_Continue;
}
পূর্ণাঙ্ক sqlite3_Walk_ৰাশিমালা(Walker *pWalker, ৰাশিমালা *p_ৰাশিমালা){
  ঘূৰাই p_ৰাশিমালা ? sqlite3_Walk_ৰাশিমালা_NN(pWalker,p_ৰাশিমালা) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
পূর্ণাঙ্ক sqlite3_Walk_ৰাশিমালা_List(Walker *pWalker, ৰাশিমালা_List *p){
  পূর্ণাঙ্ক i;
  সংরচনা ৰাশিমালা_List_item *pItem;
  যদি( p ){
    বাবে(i=p->n_ৰাশিমালা, pItem=p->a; i>0; i--, pItem++){
      যদি( sqlite3_Walk_ৰাশিমালা(pWalker, pItem->p_ৰাশিমালা) ) ঘূৰাই WRC_Abort;
    }
  }
  ঘূৰাই WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
শূন্য sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
পূর্ণাঙ্ক sqlite3_Walk_Select_ৰাশিমালা(Walker *pWalker, Select *p){
  যদি( sqlite3_Walk_ৰাশিমালা_List(pWalker, p->pEList) ) ঘূৰাই WRC_Abort;
  যদি( sqlite3_Walk_ৰাশিমালা(pWalker, p->pWhere) ) ঘূৰাই WRC_Abort;
  যদি( sqlite3_Walk_ৰাশিমালা_List(pWalker, p->pGroupBy) ) ঘূৰাই WRC_Abort;
  যদি( sqlite3_Walk_ৰাশিমালা(pWalker, p->pHaving) ) ঘূৰাই WRC_Abort;
  যদি( sqlite3_Walk_ৰাশিমালা_List(pWalker, p->pOrderBy) ) ঘূৰাই WRC_Abort;
  যদি( sqlite3_Walk_ৰাশিমালা(pWalker, p->pLimit) ) ঘূৰাই WRC_Abort;
#যদি !defined(SQLITE_OMIT_WINDOWFUNC)
  যদি( p->pWinDefn ){
    Parse *pParse;
    যদি( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_বেৰ_কৰো_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      পূর্ণাঙ্ক rc = walkWindowList(pWalker, p->pWinDefn, 0);
      ঘূৰাই rc;
    }
  }
#endif
  ঘূৰাই WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
পূর্ণাঙ্ক sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  পূর্ণাঙ্ক i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  যদি( ALWAYS(pSrc) ){
    বাবে(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      যদি( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        ঘূৰাই WRC_Abort;
      }
      যদি( pItem->fg.isTabFunc
       && sqlite3_Walk_ৰাশিমালা_List(pWalker, pItem->u1.pFuncArg)
      ){
        ঘূৰাই WRC_Abort;
      }
    }
  }
  ঘূৰাই WRC_Continue;
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
পূর্ণাঙ্ক sqlite3WalkSelect(Walker *pWalker, Select *p){
  পূর্ণাঙ্ক rc;
  যদি( p==0 ) ঘূৰাই WRC_Continue;
  যদি( pWalker->xSelectCallback==0 ) ঘূৰাই WRC_Continue;
  কৰা{
    rc = pWalker->xSelectCallback(pWalker, p);
    যদি( rc ) ঘূৰাই rc & WRC_Abort;
    যদি( sqlite3_Walk_Select_ৰাশিমালা(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      ঘূৰাই WRC_Abort;
    }
    যদি( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }যেতিয়ালৈ( p!=0 );
  ঘূৰাই WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
পূর্ণাঙ্ক sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  ঘূৰাই WRC_Continue;
}
শূন্য sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
পূর্ণাঙ্ক sqlite3_ৰাশিমালা_Walk_Noop(Walker *NotUsed, ৰাশিমালা *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ঘূৰাই WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
পূর্ণাঙ্ক sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ঘূৰাই WRC_Continue;
}
