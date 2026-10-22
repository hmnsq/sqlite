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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>


#ଯଦି !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା walkWindowList(Walker *pWalker, Window *pList, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bOneOnly){
  Window *pWin;
  ପାଇଁ(pWin=pList; pWin; pWin=pWin->pNextWin){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
    rc = sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pWin->pOrderBy);
    ଯଦି( rc ) ଫେରନ୍ତୁ WRC_Abort;
    rc = sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pWin->pPartition);
    ଯଦି( rc ) ଫେରନ୍ତୁ WRC_Abort;
    rc = sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pWin->pFilter);
    ଯଦି( rc ) ଫେରନ୍ତୁ WRC_Abort;
    rc = sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pWin->pStart);
    ଯଦି( rc ) ଫେରନ୍ତୁ WRC_Abort;
    rc = sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pWin->pEnd);
    ଯଦି( rc ) ଫେରନ୍ତୁ WRC_Abort;
    ଯଦି( bOneOnly ) ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
SQLITE_NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Walk_ପଦସମଷ୍ଟି_NN(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_TokenOnly) );
  testcase( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Reduced) );
  ଯେତେବେଳେ(1){
    rc = pWalker->x_ପଦସମଷ୍ଟି_Callback(pWalker, p_ପଦସମଷ୍ଟି);
    ଯଦି( rc ) ଫେରନ୍ତୁ rc & WRC_Abort;
    ଯଦି( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି,(EP_TokenOnly|EP_Leaf)) ){
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pList==0 || p_ପଦସମଷ୍ଟି->p_ଡାହାଣ==0 );
      ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ && sqlite3_Walk_ପଦସମଷ୍ଟି_NN(pWalker, p_ପଦସମଷ୍ଟି->p_ବାମ) ){
        ଫେରନ୍ତୁ WRC_Abort;
      }
      ଯଦି( p_ପଦସମଷ୍ଟି->p_ଡାହାଣ ){
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) );
        p_ପଦସମଷ୍ଟି = p_ପଦସମଷ୍ଟି->p_ଡାହାଣ;
        ଜାରି;
      }ଅନ୍ୟଥା ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
        ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) );
        ଯଦି( sqlite3WalkSelect(pWalker, p_ପଦସମଷ୍ଟି->x.pSelect) ) ଫେରନ୍ତୁ WRC_Abort;
      }ଅନ୍ୟଥା{
        ଯଦି( p_ପଦସମଷ୍ଟି->x.pList ){
          ଯଦି( sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, p_ପଦସମଷ୍ଟି->x.pList) ) ଫେରନ୍ତୁ WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_WinFunc) ){
          ଯଦି( walkWindowList(pWalker, p_ପଦସମଷ୍ଟି->y.pWin, 1) ) ଫେରନ୍ତୁ WRC_Abort;
        }
#endif
      }
    }
    ଭାଙ୍ଗ;
  }
  ଫେରନ୍ତୁ WRC_Continue;
}
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Walk_ପଦସମଷ୍ଟି(Walker *pWalker, ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି){
  ଫେରନ୍ତୁ p_ପଦସମଷ୍ଟି ? sqlite3_Walk_ପଦସମଷ୍ଟି_NN(pWalker,p_ପଦସମଷ୍ଟି) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Walk_ପଦସମଷ୍ଟି_List(Walker *pWalker, ପଦସମଷ୍ଟି_List *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ସଂରଚନା ପଦସମଷ୍ଟି_List_item *pItem;
  ଯଦି( p ){
    ପାଇଁ(i=p->n_ପଦସମଷ୍ଟି, pItem=p->a; i>0; i--, pItem++){
      ଯଦି( sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, pItem->p_ପଦସମଷ୍ଟି) ) ଫେରନ୍ତୁ WRC_Abort;
    }
  }
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
ଶୂନ୍ୟ sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_Walk_Select_ପଦସମଷ୍ଟି(Walker *pWalker, Select *p){
  ଯଦି( sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, p->pEList) ) ଫେରନ୍ତୁ WRC_Abort;
  ଯଦି( sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, p->pWhere) ) ଫେରନ୍ତୁ WRC_Abort;
  ଯଦି( sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, p->pGroupBy) ) ଫେରନ୍ତୁ WRC_Abort;
  ଯଦି( sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, p->pHaving) ) ଫେରନ୍ତୁ WRC_Abort;
  ଯଦି( sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, p->pOrderBy) ) ଫେରନ୍ତୁ WRC_Abort;
  ଯଦି( sqlite3_Walk_ପଦସମଷ୍ଟି(pWalker, p->pLimit) ) ଫେରନ୍ତୁ WRC_Abort;
#ଯଦି !defined(SQLITE_OMIT_WINDOWFUNC)
  ଯଦି( p->pWinDefn ){
    Parse *pParse;
    ଯଦି( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_କାଢ଼_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = walkWindowList(pWalker, p->pWinDefn, 0);
      ଫେରନ୍ତୁ rc;
    }
  }
#endif
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  ଯଦି( ALWAYS(pSrc) ){
    ପାଇଁ(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      ଯଦି( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        ଫେରନ୍ତୁ WRC_Abort;
      }
      ଯଦି( pItem->fg.isTabFunc
       && sqlite3_Walk_ପଦସମଷ୍ଟି_List(pWalker, pItem->u1.pFuncArg)
      ){
        ଫେରନ୍ତୁ WRC_Abort;
      }
    }
  }
  ଫେରନ୍ତୁ WRC_Continue;
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3WalkSelect(Walker *pWalker, Select *p){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
  ଯଦି( p==0 ) ଫେରନ୍ତୁ WRC_Continue;
  ଯଦି( pWalker->xSelectCallback==0 ) ଫେରନ୍ତୁ WRC_Continue;
  କର{
    rc = pWalker->xSelectCallback(pWalker, p);
    ଯଦି( rc ) ଫେରନ୍ତୁ rc & WRC_Abort;
    ଯଦି( sqlite3_Walk_Select_ପଦସମଷ୍ଟି(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      ଫେରନ୍ତୁ WRC_Abort;
    }
    ଯଦି( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }ଯେତେବେଳେ( p!=0 );
  ଫେରନ୍ତୁ WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  ଫେରନ୍ତୁ WRC_Continue;
}
ଶୂନ୍ୟ sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3_ପଦସମଷ୍ଟି_Walk_Noop(Walker *NotUsed, ପଦସମଷ୍ଟି *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ଫେରନ୍ତୁ WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
ପୂର୍ଣ୍ଣସଂଖ୍ୟା sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ଫେରନ୍ତୁ WRC_Continue;
}
