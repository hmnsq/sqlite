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
#અન્તર્ભાવ "sqliteInt.h"
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>


#જો !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
સ્થિર પૂર્ણાંક walkWindowList(Walker *pWalker, Window *pList, પૂર્ણાંક bOneOnly){
  Window *pWin;
  માટે(pWin=pList; pWin; pWin=pWin->pNextWin){
    પૂર્ણાંક rc;
    rc = sqlite3_Walk_પદાવલિ_List(pWalker, pWin->pOrderBy);
    જો( rc ) પાછા WRC_Abort;
    rc = sqlite3_Walk_પદાવલિ_List(pWalker, pWin->pPartition);
    જો( rc ) પાછા WRC_Abort;
    rc = sqlite3_Walk_પદાવલિ(pWalker, pWin->pFilter);
    જો( rc ) પાછા WRC_Abort;
    rc = sqlite3_Walk_પદાવલિ(pWalker, pWin->pStart);
    જો( rc ) પાછા WRC_Abort;
    rc = sqlite3_Walk_પદાવલિ(pWalker, pWin->pEnd);
    જો( rc ) પાછા WRC_Abort;
    જો( bOneOnly ) તોડો;
  }
  પાછા WRC_Continue;
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
SQLITE_NOINLINE પૂર્ણાંક sqlite3_Walk_પદાવલિ_NN(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  પૂર્ણાંક rc;
  testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_TokenOnly) );
  testcase( પદાવલિ_Has_Property(p_પદાવલિ, EP_Reduced) );
  જ્યારે(1){
    rc = pWalker->x_પદાવલિ_Callback(pWalker, p_પદાવલિ);
    જો( rc ) પાછા rc & WRC_Abort;
    જો( !પદાવલિ_Has_Property(p_પદાવલિ,(EP_TokenOnly|EP_Leaf)) ){
      પ્રતિજ્ઞા( p_પદાવલિ->x.pList==0 || p_પદાવલિ->p_જમણું==0 );
      જો( p_પદાવલિ->p_ડાબું && sqlite3_Walk_પદાવલિ_NN(pWalker, p_પદાવલિ->p_ડાબું) ){
        પાછા WRC_Abort;
      }
      જો( p_પદાવલિ->p_જમણું ){
        પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) );
        p_પદાવલિ = p_પદાવલિ->p_જમણું;
        ચાલુ;
      }નહીંતર જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
        પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) );
        જો( sqlite3WalkSelect(pWalker, p_પદાવલિ->x.pSelect) ) પાછા WRC_Abort;
      }નહીંતર{
        જો( p_પદાવલિ->x.pList ){
          જો( sqlite3_Walk_પદાવલિ_List(pWalker, p_પદાવલિ->x.pList) ) પાછા WRC_Abort;
        }
#ifndef SQLITE_OMIT_WINDOWFUNC
        જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_WinFunc) ){
          જો( walkWindowList(pWalker, p_પદાવલિ->y.pWin, 1) ) પાછા WRC_Abort;
        }
#endif
      }
    }
    તોડો;
  }
  પાછા WRC_Continue;
}
પૂર્ણાંક sqlite3_Walk_પદાવલિ(Walker *pWalker, પદાવલિ *p_પદાવલિ){
  પાછા p_પદાવલિ ? sqlite3_Walk_પદાવલિ_NN(pWalker,p_પદાવલિ) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
પૂર્ણાંક sqlite3_Walk_પદાવલિ_List(Walker *pWalker, પદાવલિ_List *p){
  પૂર્ણાંક i;
  સંરચના પદાવલિ_List_item *pItem;
  જો( p ){
    માટે(i=p->n_પદાવલિ, pItem=p->a; i>0; i--, pItem++){
      જો( sqlite3_Walk_પદાવલિ(pWalker, pItem->p_પદાવલિ) ) પાછા WRC_Abort;
    }
  }
  પાછા WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
શૂન્ય sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
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
પૂર્ણાંક sqlite3_Walk_Select_પદાવલિ(Walker *pWalker, Select *p){
  જો( sqlite3_Walk_પદાવલિ_List(pWalker, p->pEList) ) પાછા WRC_Abort;
  જો( sqlite3_Walk_પદાવલિ(pWalker, p->pWhere) ) પાછા WRC_Abort;
  જો( sqlite3_Walk_પદાવલિ_List(pWalker, p->pGroupBy) ) પાછા WRC_Abort;
  જો( sqlite3_Walk_પદાવલિ(pWalker, p->pHaving) ) પાછા WRC_Abort;
  જો( sqlite3_Walk_પદાવલિ_List(pWalker, p->pOrderBy) ) પાછા WRC_Abort;
  જો( sqlite3_Walk_પદાવલિ(pWalker, p->pLimit) ) પાછા WRC_Abort;
#જો !defined(SQLITE_OMIT_WINDOWFUNC)
  જો( p->pWinDefn ){
    Parse *pParse;
    જો( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && IN_RENAME_OBJECT)
#ifndef SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_કાઢો_With
#endif
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      પૂર્ણાંક rc = walkWindowList(pWalker, p->pWinDefn, 0);
      પાછા rc;
    }
  }
#endif
  પાછા WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
પૂર્ણાંક sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  પૂર્ણાંક i;
  SrcItem *pItem;

  pSrc = p->pSrc;
  જો( ALWAYS(pSrc) ){
    માટે(i=pSrc->nSrc, pItem=pSrc->a; i>0; i--, pItem++){
      જો( pItem->fg.isSubquery
       && sqlite3WalkSelect(pWalker, pItem->u4.pSubq->pSelect)
      ){
        પાછા WRC_Abort;
      }
      જો( pItem->fg.isTabFunc
       && sqlite3_Walk_પદાવલિ_List(pWalker, pItem->u1.pFuncArg)
      ){
        પાછા WRC_Abort;
      }
    }
  }
  પાછા WRC_Continue;
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
પૂર્ણાંક sqlite3WalkSelect(Walker *pWalker, Select *p){
  પૂર્ણાંક rc;
  જો( p==0 ) પાછા WRC_Continue;
  જો( pWalker->xSelectCallback==0 ) પાછા WRC_Continue;
  કરો{
    rc = pWalker->xSelectCallback(pWalker, p);
    જો( rc ) પાછા rc & WRC_Abort;
    જો( sqlite3_Walk_Select_પદાવલિ(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      પાછા WRC_Abort;
    }
    જો( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->pPrior;
  }જ્યારે( p!=0 );
  પાછા WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
પૂર્ણાંક sqlite3WalkerDepthIncrease(Walker *pWalker, Select *pSelect){
  UNUSED_PARAMETER(pSelect);
  pWalker->walkerDepth++;
  પાછા WRC_Continue;
}
શૂન્ય sqlite3WalkerDepthDecrease(Walker *pWalker, Select *pSelect){
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
પૂર્ણાંક sqlite3_પદાવલિ_Walk_Noop(Walker *NotUsed, પદાવલિ *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  પાછા WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
પૂર્ણાંક sqlite3SelectWalkNoop(Walker *NotUsed, Select *NotUsed2){
  UNUSED_PARAMETER2(NotUsed, NotUsed2);
  પાછા WRC_Continue;
}
