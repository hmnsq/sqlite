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
#समावेश "sqliteInt.h"
#समावेश <stdlib.h>
#समावेश <string.h>


#अगर !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
अटल पूर्णांक walkWindowList(Walker *pWalker, Window *pList, पूर्णांक bOneOnly){
  Window *pWin;
  क्रम(pWin=pList; pWin; pWin=pWin->p_अग्र_Win){
    पूर्णांक rc;
    rc = sqlite3_Walk_व्यंजक_List(pWalker, pWin->pOrderBy);
    अगर( rc ) वापस WRC_Abort;
    rc = sqlite3_Walk_व्यंजक_List(pWalker, pWin->pPartition);
    अगर( rc ) वापस WRC_Abort;
    rc = sqlite3_Walk_व्यंजक(pWalker, pWin->pFilter);
    अगर( rc ) वापस WRC_Abort;
    rc = sqlite3_Walk_व्यंजक(pWalker, pWin->pStart);
    अगर( rc ) वापस WRC_Abort;
    rc = sqlite3_Walk_व्यंजक(pWalker, pWin->pEnd);
    अगर( rc ) वापस WRC_Abort;
    अगर( bOneOnly ) अवरोध;
  }
  वापस WRC_Continue;
}
#पूर्ण_अगर

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
SQLITE_NOINLINE पूर्णांक sqlite3_Walk_व्यंजक_NN(Walker *pWalker, व्यंजक *p_व्यंजक){
  पूर्णांक rc;
  testcase( व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly) );
  testcase( व्यंजक_Has_Property(p_व्यंजक, EP_Reduced) );
  जबतक(1){
    rc = pWalker->x_व्यंजक_Callback(pWalker, p_व्यंजक);
    अगर( rc ) वापस rc & WRC_Abort;
    अगर( !व्यंजक_Has_Property(p_व्यंजक,(EP_TokenOnly|EP_Leaf)) ){
      निश्चित( p_व्यंजक->x.pList==0 || p_व्यंजक->p_दायाँ==0 );
      अगर( p_व्यंजक->p_बायाँ && sqlite3_Walk_व्यंजक_NN(pWalker, p_व्यंजक->p_बायाँ) ){
        वापस WRC_Abort;
      }
      अगर( p_व्यंजक->p_दायाँ ){
        निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) );
        p_व्यंजक = p_व्यंजक->p_दायाँ;
        जारी;
      }अन्यथा अगर( व्यंजक_Use_XSelect(p_व्यंजक) ){
        निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) );
        अगर( sqlite3WalkSelect(pWalker, p_व्यंजक->x.pSelect) ) वापस WRC_Abort;
      }अन्यथा{
        अगर( p_व्यंजक->x.pList ){
          अगर( sqlite3_Walk_व्यंजक_List(pWalker, p_व्यंजक->x.pList) ) वापस WRC_Abort;
        }
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
        अगर( व्यंजक_Has_Property(p_व्यंजक, EP_WinFunc) ){
          अगर( walkWindowList(pWalker, p_व्यंजक->y.pWin, 1) ) वापस WRC_Abort;
        }
#पूर्ण_अगर
      }
    }
    अवरोध;
  }
  वापस WRC_Continue;
}
पूर्णांक sqlite3_Walk_व्यंजक(Walker *pWalker, व्यंजक *p_व्यंजक){
  वापस p_व्यंजक ? sqlite3_Walk_व्यंजक_NN(pWalker,p_व्यंजक) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
पूर्णांक sqlite3_Walk_व्यंजक_List(Walker *pWalker, व्यंजक_List *p){
  पूर्णांक i;
  काष्ठा व्यंजक_List_item *pItem;
  अगर( p ){
    क्रम(i=p->n_व्यंजक, pItem=p->a; i>0; i--, pItem++){
      अगर( sqlite3_Walk_व्यंजक(pWalker, pItem->p_व्यंजक) ) वापस WRC_Abort;
    }
  }
  वापस WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
व्योम sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
पूर्णांक sqlite3_Walk_Select_व्यंजक(Walker *pWalker, Select *p){
  अगर( sqlite3_Walk_व्यंजक_List(pWalker, p->pEList) ) वापस WRC_Abort;
  अगर( sqlite3_Walk_व्यंजक(pWalker, p->pWhere) ) वापस WRC_Abort;
  अगर( sqlite3_Walk_व्यंजक_List(pWalker, p->pGroupBy) ) वापस WRC_Abort;
  अगर( sqlite3_Walk_व्यंजक(pWalker, p->pHaving) ) वापस WRC_Abort;
  अगर( sqlite3_Walk_व्यंजक_List(pWalker, p->pOrderBy) ) वापस WRC_Abort;
  अगर( sqlite3_Walk_व्यंजक(pWalker, p->pLimit) ) वापस WRC_Abort;
#अगर !defined(SQLITE_OMIT_WINDOWFUNC)
  अगर( p->pWinDefn ){
    Parse *pParse;
    अगर( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#अगर_अघोषित SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_निकालो_With
#पूर्ण_अगर
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      पूर्णांक rc = walkWindowList(pWalker, p->pWinDefn, 0);
      वापस rc;
    }
  }
#पूर्ण_अगर
  वापस WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
पूर्णांक sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  पूर्णांक i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  अगर( ALWAYS(pSrc) ){
    क्रम(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      अगर( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        वापस WRC_Abort;
      }
      अगर( pItem->fg.isTabFunc
       && sqlite3_Walk_व्यंजक_List(pWalker, pItem->u1.pFuncArg)
      ){
        वापस WRC_Abort;
      }
    }
  }
  वापस WRC_Continue;
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
पूर्णांक sqlite3WalkSelect(Walker *pWalker, Select *p){
  पूर्णांक rc;
  अगर( p==0 ) वापस WRC_Continue;
  अगर( pWalker->xSelectCallback==0 ) वापस WRC_Continue;
  करो{
    rc = pWalker->xSelectCallback(pWalker, p);
    अगर( rc ) वापस rc & WRC_Abort;
    अगर( sqlite3_Walk_Select_व्यंजक(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      वापस WRC_Abort;
    }
    अगर( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }जबतक( p!=0 );
  वापस WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
पूर्णांक sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  वापस WRC_Continue;
}
व्योम sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
पूर्णांक sqlite3_व्यंजक_Walk_Noop(Walker *NotUsed, व्यंजक *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  वापस WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
पूर्णांक sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  वापस WRC_Continue;
}
