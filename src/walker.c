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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>


#எனில் !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
நிலையான முழு walkWindowList(Walker *pWalker, Window *pList, முழு bOneOnly){
  Window *pWin;
  க்கு(pWin=pList; pWin; pWin=pWin->pNextWin){
    முழு rc;
    rc = sqlite3_Walk_கோவை_List(pWalker, pWin->pOrderBy);
    எனில்( rc ) திரும்பு WRC_Abort;
    rc = sqlite3_Walk_கோவை_List(pWalker, pWin->pPartition);
    எனில்( rc ) திரும்பு WRC_Abort;
    rc = sqlite3_Walk_கோவை(pWalker, pWin->pFilter);
    எனில்( rc ) திரும்பு WRC_Abort;
    rc = sqlite3_Walk_கோவை(pWalker, pWin->pStart);
    எனில்( rc ) திரும்பு WRC_Abort;
    rc = sqlite3_Walk_கோவை(pWalker, pWin->pEnd);
    எனில்( rc ) திரும்பு WRC_Abort;
    எனில்( bOneOnly ) நிறுத்து;
  }
  திரும்பு WRC_Continue;
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
SQLITE_NOINLINE முழு sqlite3_Walk_கோவை_NN(Walker *pWalker, கோவை *p_கோவை){
  முழு rc;
  testcase( கோவை_Has_Property(p_கோவை, EP_TokenOnly) );
  testcase( கோவை_Has_Property(p_கோவை, EP_Reduced) );
  போது(1){
    rc = pWalker->x_கோவை_Callback(pWalker, p_கோவை);
    எனில்( rc ) திரும்பு rc & WRC_Abort;
    எனில்( !கோவை_Has_Property(p_கோவை,(EP_TokenOnly|EP_Leaf)) ){
      ப்ரதிஜ்ஞா( p_கோவை->x.pList==0 || p_கோவை->p_வலது==0 );
      எனில்( p_கோவை->p_இடது && sqlite3_Walk_கோவை_NN(pWalker, p_கோவை->p_இடது) ){
        திரும்பு WRC_Abort;
      }
      எனில்( p_கோவை->p_வலது ){
        ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_WinFunc) );
        p_கோவை = p_கோவை->p_வலது;
        தொடர்;
      }இல்லையெனில் எனில்( கோவை_Use_XSelect(p_கோவை) ){
        ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_WinFunc) );
        எனில்( sqlite3WalkSelect(pWalker, p_கோவை->x.pSelect) ) திரும்பு WRC_Abort;
      }இல்லையெனில்{
        எனில்( p_கோவை->x.pList ){
          எனில்( sqlite3_Walk_கோவை_List(pWalker, p_கோவை->x.pList) ) திரும்பு WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        எனில்( கோவை_Has_Property(p_கோவை, EP_WinFunc) ){
          எனில்( walkWindowList(pWalker, p_கோவை->y.pWin, 1) ) திரும்பு WRC_Abort;
        }
#endif
      }
    }
    நிறுத்து;
  }
  திரும்பு WRC_Continue;
}
முழு sqlite3_Walk_கோவை(Walker *pWalker, கோவை *p_கோவை){
  திரும்பு p_கோவை ? sqlite3_Walk_கோவை_NN(pWalker,p_கோவை) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
முழு sqlite3_Walk_கோவை_List(Walker *pWalker, கோவை_List *p){
  முழு i;
  கட்டமைப்பு கோவை_List_item *pItem;
  எனில்( p ){
    க்கு(i=p->n_கோவை, pItem=p->a; i>0; i--, pItem++){
      எனில்( sqlite3_Walk_கோவை(pWalker, pItem->p_கோவை) ) திரும்பு WRC_Abort;
    }
  }
  திரும்பு WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
வெறுமை sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
முழு sqlite3_Walk_Select_கோவை(Walker *pWalker, Select *p){
  எனில்( sqlite3_Walk_கோவை_List(pWalker, p->pEList) ) திரும்பு WRC_Abort;
  எனில்( sqlite3_Walk_கோவை(pWalker, p->pWhere) ) திரும்பு WRC_Abort;
  எனில்( sqlite3_Walk_கோவை_List(pWalker, p->pGroupBy) ) திரும்பு WRC_Abort;
  எனில்( sqlite3_Walk_கோவை(pWalker, p->pHaving) ) திரும்பு WRC_Abort;
  எனில்( sqlite3_Walk_கோவை_List(pWalker, p->pOrderBy) ) திரும்பு WRC_Abort;
  எனில்( sqlite3_Walk_கோவை(pWalker, p->pLimit) ) திரும்பு WRC_Abort;
#எனில் !defined(SQLITE_OMIT_WINDOWFUNC)
  எனில்( p->pWinDefn ){
    Parse *pParse;
    எனில்( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_எடு_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      முழு rc = walkWindowList(pWalker, p->pWinDefn, 0);
      திரும்பு rc;
    }
  }
#endif
  திரும்பு WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
முழு sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  முழு i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  எனில்( ALWAYS(pSrc) ){
    க்கு(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      எனில்( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        திரும்பு WRC_Abort;
      }
      எனில்( pItem->fg.isTabFunc
       && sqlite3_Walk_கோவை_List(pWalker, pItem->u1.pFuncArg)
      ){
        திரும்பு WRC_Abort;
      }
    }
  }
  திரும்பு WRC_Continue;
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
முழு sqlite3WalkSelect(Walker *pWalker, Select *p){
  முழு rc;
  எனில்( p==0 ) திரும்பு WRC_Continue;
  எனில்( pWalker->xSelectCallback==0 ) திரும்பு WRC_Continue;
  செய்{
    rc = pWalker->xSelectCallback(pWalker, p);
    எனில்( rc ) திரும்பு rc & WRC_Abort;
    எனில்( sqlite3_Walk_Select_கோவை(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      திரும்பு WRC_Abort;
    }
    எனில்( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }போது( p!=0 );
  திரும்பு WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
முழு sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  திரும்பு WRC_Continue;
}
வெறுமை sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
முழு sqlite3_கோவை_Walk_Noop(Walker *NotUsed, கோவை *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  திரும்பு WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
முழு sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  திரும்பு WRC_Continue;
}
