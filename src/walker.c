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
#अन्तर्भाव "sqliteInt.h"
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>


#यदि !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
स्थिर पूर्णाङ्क walkWindowList(Walker *pWalker, Window *pList, पूर्णाङ्क bOneOnly){
  Window *pWin;
  प्रति(pWin=pList; pWin; pWin=pWin->pNextWin){
    पूर्णाङ्क rc;
    rc = sqlite3_Walk_अभिव्यक्ती_List(pWalker, pWin->pOrderBy);
    यदि( rc ) प्रत्यागम WRC_Abort;
    rc = sqlite3_Walk_अभिव्यक्ती_List(pWalker, pWin->pPartition);
    यदि( rc ) प्रत्यागम WRC_Abort;
    rc = sqlite3_Walk_अभिव्यक्ती(pWalker, pWin->pFilter);
    यदि( rc ) प्रत्यागम WRC_Abort;
    rc = sqlite3_Walk_अभिव्यक्ती(pWalker, pWin->pStart);
    यदि( rc ) प्रत्यागम WRC_Abort;
    rc = sqlite3_Walk_अभिव्यक्ती(pWalker, pWin->pEnd);
    यदि( rc ) प्रत्यागम WRC_Abort;
    यदि( bOneOnly ) भङ्ग;
  }
  प्रत्यागम WRC_Continue;
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
SQLITE_NOINLINE पूर्णाङ्क sqlite3_Walk_अभिव्यक्ती_NN(Walker *pWalker, अभिव्यक्ती *p_अभिव्यक्ती){
  पूर्णाङ्क rc;
  testcase( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_TokenOnly) );
  testcase( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Reduced) );
  यावत्(1){
    rc = pWalker->x_अभिव्यक्ती_Callback(pWalker, p_अभिव्यक्ती);
    यदि( rc ) प्रत्यागम rc & WRC_Abort;
    यदि( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती,(EP_TokenOnly|EP_Leaf)) ){
      प्रतिज्ञा( p_अभिव्यक्ती->x.pList==0 || p_अभिव्यक्ती->p_उजवा==0 );
      यदि( p_अभिव्यक्ती->p_डावा && sqlite3_Walk_अभिव्यक्ती_NN(pWalker, p_अभिव्यक्ती->p_डावा) ){
        प्रत्यागम WRC_Abort;
      }
      यदि( p_अभिव्यक्ती->p_उजवा ){
        प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_WinFunc) );
        p_अभिव्यक्ती = p_अभिव्यक्ती->p_उजवा;
        अनुवर्त;
      }अन्यथा यदि( अभिव्यक्ती_Use_XSelect(p_अभिव्यक्ती) ){
        प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_WinFunc) );
        यदि( sqlite3WalkSelect(pWalker, p_अभिव्यक्ती->x.pSelect) ) प्रत्यागम WRC_Abort;
      }अन्यथा{
        यदि( p_अभिव्यक्ती->x.pList ){
          यदि( sqlite3_Walk_अभिव्यक्ती_List(pWalker, p_अभिव्यक्ती->x.pList) ) प्रत्यागम WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        यदि( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_WinFunc) ){
          यदि( walkWindowList(pWalker, p_अभिव्यक्ती->y.pWin, 1) ) प्रत्यागम WRC_Abort;
        }
#endif
      }
    }
    भङ्ग;
  }
  प्रत्यागम WRC_Continue;
}
पूर्णाङ्क sqlite3_Walk_अभिव्यक्ती(Walker *pWalker, अभिव्यक्ती *p_अभिव्यक्ती){
  प्रत्यागम p_अभिव्यक्ती ? sqlite3_Walk_अभिव्यक्ती_NN(pWalker,p_अभिव्यक्ती) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
पूर्णाङ्क sqlite3_Walk_अभिव्यक्ती_List(Walker *pWalker, अभिव्यक्ती_List *p){
  पूर्णाङ्क i;
  संरचना अभिव्यक्ती_List_item *pItem;
  यदि( p ){
    प्रति(i=p->n_अभिव्यक्ती, pItem=p->a; i>0; i--, pItem++){
      यदि( sqlite3_Walk_अभिव्यक्ती(pWalker, pItem->p_अभिव्यक्ती) ) प्रत्यागम WRC_Abort;
    }
  }
  प्रत्यागम WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
शून्य sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
पूर्णाङ्क sqlite3_Walk_Select_अभिव्यक्ती(Walker *pWalker, Select *p){
  यदि( sqlite3_Walk_अभिव्यक्ती_List(pWalker, p->pEList) ) प्रत्यागम WRC_Abort;
  यदि( sqlite3_Walk_अभिव्यक्ती(pWalker, p->pWhere) ) प्रत्यागम WRC_Abort;
  यदि( sqlite3_Walk_अभिव्यक्ती_List(pWalker, p->pGroupBy) ) प्रत्यागम WRC_Abort;
  यदि( sqlite3_Walk_अभिव्यक्ती(pWalker, p->pHaving) ) प्रत्यागम WRC_Abort;
  यदि( sqlite3_Walk_अभिव्यक्ती_List(pWalker, p->pOrderBy) ) प्रत्यागम WRC_Abort;
  यदि( sqlite3_Walk_अभिव्यक्ती(pWalker, p->pLimit) ) प्रत्यागम WRC_Abort;
#यदि !defined(SQLITE_OMIT_WINDOWFUNC)
  यदि( p->pWinDefn ){
    Parse *pParse;
    यदि( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_काढा_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      पूर्णाङ्क rc = walkWindowList(pWalker, p->pWinDefn, 0);
      प्रत्यागम rc;
    }
  }
#endif
  प्रत्यागम WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
पूर्णाङ्क sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  पूर्णाङ्क i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  यदि( ALWAYS(pSrc) ){
    प्रति(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      यदि( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        प्रत्यागम WRC_Abort;
      }
      यदि( pItem->fg.isTabFunc
       && sqlite3_Walk_अभिव्यक्ती_List(pWalker, pItem->u1.pFuncArg)
      ){
        प्रत्यागम WRC_Abort;
      }
    }
  }
  प्रत्यागम WRC_Continue;
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
पूर्णाङ्क sqlite3WalkSelect(Walker *pWalker, Select *p){
  पूर्णाङ्क rc;
  यदि( p==0 ) प्रत्यागम WRC_Continue;
  यदि( pWalker->xSelectCallback==0 ) प्रत्यागम WRC_Continue;
  कुरु{
    rc = pWalker->xSelectCallback(pWalker, p);
    यदि( rc ) प्रत्यागम rc & WRC_Abort;
    यदि( sqlite3_Walk_Select_अभिव्यक्ती(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      प्रत्यागम WRC_Abort;
    }
    यदि( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }यावत्( p!=0 );
  प्रत्यागम WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
पूर्णाङ्क sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  प्रत्यागम WRC_Continue;
}
शून्य sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
पूर्णाङ्क sqlite3_अभिव्यक्ती_Walk_Noop(Walker *NotUsed, अभिव्यक्ती *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  प्रत्यागम WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
पूर्णाङ्क sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  प्रत्यागम WRC_Continue;
}
