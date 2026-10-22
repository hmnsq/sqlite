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


#जइ !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
स्थिर पूर्णाङ्क walkWindowList(Walker *pWalker, Window *pList, पूर्णाङ्क bOneOnly){
  Window *pWin;
  प्रति(pWin=pList; pWin; pWin=pWin->pNextWin){
    पूर्णाङ्क rc;
    rc = sqlite3_Walk_व्यञ्जकम्_List(pWalker, pWin->pOrderBy);
    जइ( rc ) प्रत्यागम WRC_Abort;
    rc = sqlite3_Walk_व्यञ्जकम्_List(pWalker, pWin->pPartition);
    जइ( rc ) प्रत्यागम WRC_Abort;
    rc = sqlite3_Walk_व्यञ्जकम्(pWalker, pWin->pFilter);
    जइ( rc ) प्रत्यागम WRC_Abort;
    rc = sqlite3_Walk_व्यञ्जकम्(pWalker, pWin->pStart);
    जइ( rc ) प्रत्यागम WRC_Abort;
    rc = sqlite3_Walk_व्यञ्जकम्(pWalker, pWin->pEnd);
    जइ( rc ) प्रत्यागम WRC_Abort;
    जइ( bOneOnly ) भङ्ग;
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
SQLITE_NOINLINE पूर्णाङ्क sqlite3_Walk_व्यञ्जकम्_NN(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  पूर्णाङ्क rc;
  testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_TokenOnly) );
  testcase( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Reduced) );
  जाव(1){
    rc = pWalker->x_व्यञ्जकम्_Callback(pWalker, p_व्यञ्जकम्);
    जइ( rc ) प्रत्यागम rc & WRC_Abort;
    जइ( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्,(EP_TokenOnly|EP_Leaf)) ){
      प्रतिज्ञा( p_व्यञ्जकम्->x.pList==0 || p_व्यञ्जकम्->p_दक्षिणः==0 );
      जइ( p_व्यञ्जकम्->p_वामः && sqlite3_Walk_व्यञ्जकम्_NN(pWalker, p_व्यञ्जकम्->p_वामः) ){
        प्रत्यागम WRC_Abort;
      }
      जइ( p_व्यञ्जकम्->p_दक्षिणः ){
        प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) );
        p_व्यञ्जकम् = p_व्यञ्जकम्->p_दक्षिणः;
        अनुवर्त;
      }अण्णहा जइ( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
        प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) );
        जइ( sqlite3WalkSelect(pWalker, p_व्यञ्जकम्->x.pSelect) ) प्रत्यागम WRC_Abort;
      }अण्णहा{
        जइ( p_व्यञ्जकम्->x.pList ){
          जइ( sqlite3_Walk_व्यञ्जकम्_List(pWalker, p_व्यञ्जकम्->x.pList) ) प्रत्यागम WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_WinFunc) ){
          जइ( walkWindowList(pWalker, p_व्यञ्जकम्->y.pWin, 1) ) प्रत्यागम WRC_Abort;
        }
#endif
      }
    }
    भङ्ग;
  }
  प्रत्यागम WRC_Continue;
}
पूर्णाङ्क sqlite3_Walk_व्यञ्जकम्(Walker *pWalker, व्यञ्जकम् *p_व्यञ्जकम्){
  प्रत्यागम p_व्यञ्जकम् ? sqlite3_Walk_व्यञ्जकम्_NN(pWalker,p_व्यञ्जकम्) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
पूर्णाङ्क sqlite3_Walk_व्यञ्जकम्_List(Walker *pWalker, व्यञ्जकम्_List *p){
  पूर्णाङ्क i;
  संरचना व्यञ्जकम्_List_item *pItem;
  जइ( p ){
    प्रति(i=p->n_व्यञ्जकम्, pItem=p->a; i>0; i--, pItem++){
      जइ( sqlite3_Walk_व्यञ्जकम्(pWalker, pItem->p_व्यञ्जकम्) ) प्रत्यागम WRC_Abort;
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
पूर्णाङ्क sqlite3_Walk_Select_व्यञ्जकम्(Walker *pWalker, Select *p){
  जइ( sqlite3_Walk_व्यञ्जकम्_List(pWalker, p->pEList) ) प्रत्यागम WRC_Abort;
  जइ( sqlite3_Walk_व्यञ्जकम्(pWalker, p->pWhere) ) प्रत्यागम WRC_Abort;
  जइ( sqlite3_Walk_व्यञ्जकम्_List(pWalker, p->pGroupBy) ) प्रत्यागम WRC_Abort;
  जइ( sqlite3_Walk_व्यञ्जकम्(pWalker, p->pHaving) ) प्रत्यागम WRC_Abort;
  जइ( sqlite3_Walk_व्यञ्जकम्_List(pWalker, p->pOrderBy) ) प्रत्यागम WRC_Abort;
  जइ( sqlite3_Walk_व्यञ्जकम्(pWalker, p->pLimit) ) प्रत्यागम WRC_Abort;
#जइ !defined(SQLITE_OMIT_WINDOWFUNC)
  जइ( p->pWinDefn ){
    Parse *pParse;
    जइ( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_निष्कासय_With
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
  जइ( ALWAYS(pSrc) ){
    प्रति(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      जइ( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        प्रत्यागम WRC_Abort;
      }
      जइ( pItem->fg.isTabFunc
       && sqlite3_Walk_व्यञ्जकम्_List(pWalker, pItem->u1.pFuncArg)
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
  जइ( p==0 ) प्रत्यागम WRC_Continue;
  जइ( pWalker->xSelectCallback==0 ) प्रत्यागम WRC_Continue;
  करेहि{
    rc = pWalker->xSelectCallback(pWalker, p);
    जइ( rc ) प्रत्यागम rc & WRC_Abort;
    जइ( sqlite3_Walk_Select_व्यञ्जकम्(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      प्रत्यागम WRC_Abort;
    }
    जइ( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }जाव( p!=0 );
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
पूर्णाङ्क sqlite3_व्यञ्जकम्_Walk_Noop(Walker *NotUsed, व्यञ्जकम् *NotUsed2){
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
