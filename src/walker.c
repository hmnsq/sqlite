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
#అన్తర్భావ "sqliteInt.h"
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>


#ఒకవేళ !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
స్థిర పూర్ణాంకం walk_కిటికీ_List(Walker *pWalker, కిటికీ *pList, పూర్ణాంకం bOneOnly){
  కిటికీ *pWin;
  కొరకు(pWin=pList; pWin; pWin=pWin->p_తదుపరి_Win){
    పూర్ణాంకం rc;
    rc = sqlite3_Walk_సమాసం_List(pWalker, pWin->p_క్రమం_By);
    ఒకవేళ( rc ) తిరిగి WRC_Abort;
    rc = sqlite3_Walk_సమాసం_List(pWalker, pWin->p_విభజన);
    ఒకవేళ( rc ) తిరిగి WRC_Abort;
    rc = sqlite3_Walk_సమాసం(pWalker, pWin->pFilter);
    ఒకవేళ( rc ) తిరిగి WRC_Abort;
    rc = sqlite3_Walk_సమాసం(pWalker, pWin->p_ప్రారంభం);
    ఒకవేళ( rc ) తిరిగి WRC_Abort;
    rc = sqlite3_Walk_సమాసం(pWalker, pWin->p_చివర);
    ఒకవేళ( rc ) తిరిగి WRC_Abort;
    ఒకవేళ( bOneOnly ) ఆపు;
  }
  తిరిగి WRC_Continue;
}
#అయితే_ముగింపు

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
SQLITE_NOINLINE పూర్ణాంకం sqlite3_Walk_సమాసం_NN(Walker *pWalker, సమాసం *p_సమాసం){
  పూర్ణాంకం rc;
  testcase( సమాసం_Has_Property(p_సమాసం, EP_TokenOnly) );
  testcase( సమాసం_Has_Property(p_సమాసం, EP_Reduced) );
  అయితే(1){
    rc = pWalker->x_సమాసం_Callback(pWalker, p_సమాసం);
    ఒకవేళ( rc ) తిరిగి rc & WRC_Abort;
    ఒకవేళ( !సమాసం_Has_Property(p_సమాసం,(EP_TokenOnly|EP_ఆకు)) ){
      ప్రతిజ్ఞా( p_సమాసం->x.pList==0 || p_సమాసం->p_కుడి==0 );
      ఒకవేళ( p_సమాసం->p_ఎడమ && sqlite3_Walk_సమాసం_NN(pWalker, p_సమాసం->p_ఎడమ) ){
        తిరిగి WRC_Abort;
      }
      ఒకవేళ( p_సమాసం->p_కుడి ){
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_WinFunc) );
        p_సమాసం = p_సమాసం->p_కుడి;
        కొనసాగు;
      }లేకపోతే ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
        ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_WinFunc) );
        ఒకవేళ( sqlite3WalkSelect(pWalker, p_సమాసం->x.pSelect) ) తిరిగి WRC_Abort;
      }లేకపోతే{
        ఒకవేళ( p_సమాసం->x.pList ){
          ఒకవేళ( sqlite3_Walk_సమాసం_List(pWalker, p_సమాసం->x.pList) ) తిరిగి WRC_Abort;
        }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
        ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_WinFunc) ){
          ఒకవేళ( walk_కిటికీ_List(pWalker, p_సమాసం->y.pWin, 1) ) తిరిగి WRC_Abort;
        }
#అయితే_ముగింపు
      }
    }
    ఆపు;
  }
  తిరిగి WRC_Continue;
}
పూర్ణాంకం sqlite3_Walk_సమాసం(Walker *pWalker, సమాసం *p_సమాసం){
  తిరిగి p_సమాసం ? sqlite3_Walk_సమాసం_NN(pWalker,p_సమాసం) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
పూర్ణాంకం sqlite3_Walk_సమాసం_List(Walker *pWalker, సమాసం_List *p){
  పూర్ణాంకం i;
  నిర్మాణం సమాసం_List_అంశం *p_అంశం;
  ఒకవేళ( p ){
    కొరకు(i=p->n_సమాసం, p_అంశం=p->a; i>0; i--, p_అంశం++){
      ఒకవేళ( sqlite3_Walk_సమాసం(pWalker, p_అంశం->p_సమాసం) ) తిరిగి WRC_Abort;
    }
  }
  తిరిగి WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
శూన్య sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
  వాడని_PARAMETER(pWalker);
  వాడని_PARAMETER(p);
  /* No-op */
}

/*
** Walk all expressions associated with SELECT statement p.  Do
** not invoke the SELECT callback on p, but do (of course) invoke
** any expr callbacks and SELECT callbacks that come from subqueries.
** Return WRC_Abort or WRC_Continue.
*/
పూర్ణాంకం sqlite3_Walk_Select_సమాసం(Walker *pWalker, Select *p){
  ఒకవేళ( sqlite3_Walk_సమాసం_List(pWalker, p->pEList) ) తిరిగి WRC_Abort;
  ఒకవేళ( sqlite3_Walk_సమాసం(pWalker, p->pWhere) ) తిరిగి WRC_Abort;
  ఒకవేళ( sqlite3_Walk_సమాసం_List(pWalker, p->p_సమూహం_By) ) తిరిగి WRC_Abort;
  ఒకవేళ( sqlite3_Walk_సమాసం(pWalker, p->pHaving) ) తిరిగి WRC_Abort;
  ఒకవేళ( sqlite3_Walk_సమాసం_List(pWalker, p->p_క్రమం_By) ) తిరిగి WRC_Abort;
  ఒకవేళ( sqlite3_Walk_సమాసం(pWalker, p->p_హద్దు) ) తిరిగి WRC_Abort;
#ఒకవేళ !defined(SQLITE_OMIT_WINDOWFUNC)
  ఒకవేళ( p->pWinDefn ){
    Parse *pParse;
    ఒకవేళ( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && లోపల_RENAME_OBJECT)
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_తీయి_With
#అయితే_ముగింపు
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      పూర్ణాంకం rc = walk_కిటికీ_List(pWalker, p->pWinDefn, 0);
      తిరిగి rc;
    }
  }
#అయితే_ముగింపు
  తిరిగి WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
పూర్ణాంకం sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  పూర్ణాంకం i;
  Src_అంశం *p_అంశం;

  pSrc = p->pSrc;
  ఒకవేళ( ALWAYS(pSrc) ){
    కొరకు(i=pSrc->nSrc, p_అంశం=pSrc->a; i>0; i--, p_అంశం++){
      ఒకవేళ( p_అంశం->fg.isSubquery
       && sqlite3WalkSelect(pWalker, p_అంశం->u4.pSubq->pSelect)
      ){
        తిరిగి WRC_Abort;
      }
      ఒకవేళ( p_అంశం->fg.isTabFunc
       && sqlite3_Walk_సమాసం_List(pWalker, p_అంశం->u1.pFuncArg)
      ){
        తిరిగి WRC_Abort;
      }
    }
  }
  తిరిగి WRC_Continue;
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
పూర్ణాంకం sqlite3WalkSelect(Walker *pWalker, Select *p){
  పూర్ణాంకం rc;
  ఒకవేళ( p==0 ) తిరిగి WRC_Continue;
  ఒకవేళ( pWalker->xSelectCallback==0 ) తిరిగి WRC_Continue;
  చేయి{
    rc = pWalker->xSelectCallback(pWalker, p);
    ఒకవేళ( rc ) తిరిగి rc & WRC_Abort;
    ఒకవేళ( sqlite3_Walk_Select_సమాసం(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      తిరిగి WRC_Abort;
    }
    ఒకవేళ( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->p_పూర్వ;
  }అయితే( p!=0 );
  తిరిగి WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
పూర్ణాంకం sqlite3_Walker_లోతు_Increase(Walker *pWalker, Select *pSelect){
  వాడని_PARAMETER(pSelect);
  pWalker->walker_లోతు++;
  తిరిగి WRC_Continue;
}
శూన్య sqlite3_Walker_లోతు_Decrease(Walker *pWalker, Select *pSelect){
  వాడని_PARAMETER(pSelect);
  pWalker->walker_లోతు--;
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
పూర్ణాంకం sqlite3_సమాసం_Walk_Noop(Walker *Not_వాడిన, సమాసం *NotUsed2){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  తిరిగి WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
పూర్ణాంకం sqlite3SelectWalkNoop(Walker *Not_వాడిన, Select *NotUsed2){
  వాడని_PARAMETER2(Not_వాడిన, NotUsed2);
  తిరిగి WRC_Continue;
}
