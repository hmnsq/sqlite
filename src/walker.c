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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>


#ಯದಿ !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ walkWindowList(Walker *pWalker, Window *pList, ಪೂರ್ಣಾಙ್ಕ bOneOnly){
  Window *pWin;
  ಪ್ರತಿ(pWin=pList; pWin; pWin=pWin->pNextWin){
    ಪೂರ್ಣಾಙ್ಕ rc;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pWin->pOrderBy);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pWin->pPartition);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pWin->pFilter);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pWin->pStart);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pWin->pEnd);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    ಯದಿ( bOneOnly ) ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
SQLITE_NOINLINE ಪೂರ್ಣಾಙ್ಕ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಙ್ಕ rc;
  testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly) );
  testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced) );
  ಯಾವತ್(1){
    rc = pWalker->x_ಅಭಿವ್ಯಕ್ತಿ_Callback(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc & WRC_Abort;
    ಯದಿ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,(EP_TokenOnly|EP_Leaf)) ){
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==0 || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==0 );
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ && sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) ){
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) );
        p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        ಅನುವರ್ತ;
      }ಅನ್ಯಥಾ ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) );
        ಯದಿ( sqlite3WalkSelect(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }ಅನ್ಯಥಾ{
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList ){
          ಯದಿ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
          ಯದಿ( walkWindowList(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin, 1) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
        }
#endif
      }
    }
    ಭಙ್ಗ;
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}
ಪೂರ್ಣಾಙ್ಕ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪ್ರತ್ಯಾಗಮ p_ಅಭಿವ್ಯಕ್ತಿ ? sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(pWalker,p_ಅಭಿವ್ಯಕ್ತಿ) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ_List *p){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಸಂರಚನಾ ಅಭಿವ್ಯಕ್ತಿ_List_item *pItem;
  ಯದಿ( p ){
    ಪ್ರತಿ(i=p->n_ಅಭಿವ್ಯಕ್ತಿ, pItem=p->a; i>0; i--, pItem++){
      ಯದಿ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pItem->p_ಅಭಿವ್ಯಕ್ತಿ) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
ಶೂನ್ಯ sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_Walk_Select_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, Select *p){
  ಯದಿ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p->pEList) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  ಯದಿ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p->pWhere) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  ಯದಿ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p->pGroupBy) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  ಯದಿ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p->pHaving) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  ಯದಿ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p->pOrderBy) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
  ಯದಿ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p->pLimit) ) ಪ್ರತ್ಯಾಗಮ WRC_Abort;
#ಯದಿ !defined(SQLITE_OMIT_WINDOWFUNC)
  ಯದಿ( p->pWinDefn ){
    Parse *pParse;
    ಯದಿ( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_ತೆಗೆ_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      ಪೂರ್ಣಾಙ್ಕ rc = walkWindowList(pWalker, p->pWinDefn, 0);
      ಪ್ರತ್ಯಾಗಮ rc;
    }
  }
#endif
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  ಪೂರ್ಣಾಙ್ಕ i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  ಯದಿ( ALWAYS(pSrc) ){
    ಪ್ರತಿ(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      ಯದಿ( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
      ಯದಿ( pItem->fg.isTabFunc
       && sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pItem->u1.pFuncArg)
      ){
        ಪ್ರತ್ಯಾಗಮ WRC_Abort;
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
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
ಪೂರ್ಣಾಙ್ಕ sqlite3WalkSelect(Walker *pWalker, Select *p){
  ಪೂರ್ಣಾಙ್ಕ rc;
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಯದಿ( pWalker->xSelectCallback==0 ) ಪ್ರತ್ಯಾಗಮ WRC_Continue;
  ಕುರು{
    rc = pWalker->xSelectCallback(pWalker, p);
    ಯದಿ( rc ) ಪ್ರತ್ಯಾಗಮ rc & WRC_Abort;
    ಯದಿ( sqlite3_Walk_Select_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      ಪ್ರತ್ಯಾಗಮ WRC_Abort;
    }
    ಯದಿ( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }ಯಾವತ್( p!=0 );
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}
ಶೂನ್ಯ sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
ಪೂರ್ಣಾಙ್ಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop(Walker *NotUsed, ಅಭಿವ್ಯಕ್ತಿ *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
ಪೂರ್ಣಾಙ್ಕ sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  ಪ್ರತ್ಯಾಗಮ WRC_Continue;
}
