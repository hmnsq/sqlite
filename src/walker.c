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


#ಆದರೆ !defined(SQLITE_OMIT_WINDOWFUNC)
/*
** Walk all expressions linked into the list of Window objects passed
** as the second argument.
*/
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ walk_ಕಿಟಕಿ_List(Walker *pWalker, ಕಿಟಕಿ *pList, ಪೂರ್ಣಾಂಕ bOneOnly){
  ಕಿಟಕಿ *pWin;
  ಗಾಗಿ(pWin=pList; pWin; pWin=pWin->p_ಮುಂದಿನ_Win){
    ಪೂರ್ಣಾಂಕ rc;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pWin->p_ಕ್ರಮ_By);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ WRC_Abort;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, pWin->p_ವಿಭಜನೆ);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ WRC_Abort;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pWin->pFilter);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ WRC_Abort;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pWin->p_ಆರಂಭ);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ WRC_Abort;
    rc = sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, pWin->p_ಕೊನೆ);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ WRC_Abort;
    ಆದರೆ( bOneOnly ) ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}
#ಆದರೆ_ಅಂತ್ಯ

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
SQLITE_NOINLINE ಪೂರ್ಣಾಂಕ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಪೂರ್ಣಾಂಕ rc;
  testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly) );
  testcase( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Reduced) );
  ಆಗ(1){
    rc = pWalker->x_ಅಭಿವ್ಯಕ್ತಿ_Callback(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc & WRC_Abort;
    ಆದರೆ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ,(EP_TokenOnly|EP_ಎಲೆ)) ){
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList==0 || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==0 );
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ && sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) ){
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) );
        p_ಅಭಿವ್ಯಕ್ತಿ = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ;
        ಮುಂದುವರಿ;
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) );
        ಆದರೆ( sqlite3WalkSelect(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect) ) ಹಿಂತಿರುಗಿ WRC_Abort;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList ){
          ಆದರೆ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList) ) ಹಿಂತಿರುಗಿ WRC_Abort;
        }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
        ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_WinFunc) ){
          ಆದರೆ( walk_ಕಿಟಕಿ_List(pWalker, p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin, 1) ) ಹಿಂತಿರುಗಿ WRC_Abort;
        }
#ಆದರೆ_ಅಂತ್ಯ
      }
    }
    ನಿಲ್ಲಿಸು;
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}
ಪೂರ್ಣಾಂಕ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ){
  ಹಿಂತಿರುಗಿ p_ಅಭಿವ್ಯಕ್ತಿ ? sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_NN(pWalker,p_ಅಭಿವ್ಯಕ್ತಿ) : WRC_Continue;
}

/*
** Call sqlite3WalkExpr() for every expression in list p or until
** an abort request is seen.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(Walker *pWalker, ಅಭಿವ್ಯಕ್ತಿ_List *p){
  ಪೂರ್ಣಾಂಕ i;
  ರಚನೆ ಅಭಿವ್ಯಕ್ತಿ_List_ಅಂಶ *p_ಅಂಶ;
  ಆದರೆ( p ){
    ಗಾಗಿ(i=p->n_ಅಭಿವ್ಯಕ್ತಿ, p_ಅಂಶ=p->a; i>0; i--, p_ಅಂಶ++){
      ಆದರೆ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p_ಅಂಶ->p_ಅಭಿವ್ಯಕ್ತಿ) ) ಹಿಂತಿರುಗಿ WRC_Abort;
    }
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** This is a no-op callback for Walker->xSelectCallback2.  If this
** callback is set, then the Select->pWinDefn list is traversed.
*/
ಶೂನ್ಯ sqlite3WalkWinDefnDummyCallback(Walker *pWalker, Select *p){
  ಬಳಸದ_PARAMETER(pWalker);
  ಬಳಸದ_PARAMETER(p);
  /* No-op */
}

/*
** Walk all expressions associated with SELECT statement p.  Do
** not invoke the SELECT callback on p, but do (of course) invoke
** any expr callbacks and SELECT callbacks that come from subqueries.
** Return WRC_Abort or WRC_Continue.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Walk_Select_ಅಭಿವ್ಯಕ್ತಿ(Walker *pWalker, Select *p){
  ಆದರೆ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p->pEList) ) ಹಿಂತಿರುಗಿ WRC_Abort;
  ಆದರೆ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p->pWhere) ) ಹಿಂತಿರುಗಿ WRC_Abort;
  ಆದರೆ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p->p_ಗುಂಪು_By) ) ಹಿಂತಿರುಗಿ WRC_Abort;
  ಆದರೆ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p->pHaving) ) ಹಿಂತಿರುಗಿ WRC_Abort;
  ಆದರೆ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p->p_ಕ್ರಮ_By) ) ಹಿಂತಿರುಗಿ WRC_Abort;
  ಆದರೆ( sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p->p_ಎಲ್ಲೆ) ) ಹಿಂತಿರುಗಿ WRC_Abort;
#ಆದರೆ !defined(SQLITE_OMIT_WINDOWFUNC)
  ಆದರೆ( p->pWinDefn ){
    Parse *pParse;
    ಆದರೆ( pWalker->xSelectCallback2==sqlite3WalkWinDefnDummyCallback
     || ((pParse = pWalker->pParse)!=0 && ಒಳ_RENAME_OBJECT)
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CTE
     || pWalker->xSelectCallback2==sqlite3_Select_ತೆಗೆ_With
#ಆದರೆ_ಅಂತ್ಯ
    ){
      /* The following may return WRC_Abort if there are unresolvable
      ** symbols (e.g. a table that does not exist) in a window definition. */
      ಪೂರ್ಣಾಂಕ rc = walk_ಕಿಟಕಿ_List(pWalker, p->pWinDefn, 0);
      ಹಿಂತಿರುಗಿ rc;
    }
  }
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** Walk the parse trees associated with all subqueries in the
** FROM clause of SELECT statement p.  Do not invoke the select
** callback on p, but do invoke it on each FROM clause subquery
** and on any subqueries further down in the tree.  Return 
** WRC_Abort or WRC_Continue;
*/
ಪೂರ್ಣಾಂಕ sqlite3WalkSelectFrom(Walker *pWalker, Select *p){
  SrcList *pSrc;
  ಪೂರ್ಣಾಂಕ i;
  Src_ಅಂಶ *p_ಅಂಶ;

  pSrc = p->pSrc;
  ಆದರೆ( ALWAYS(pSrc) ){
    ಗಾಗಿ(i=pSrc->nSrc, p_ಅಂಶ=pSrc->a; i>0; i--, p_ಅಂಶ++){
      ಆದರೆ( p_ಅಂಶ->fg.isSubquery
       && sqlite3WalkSelect(pWalker, p_ಅಂಶ->u4.pSubq->pSelect)
      ){
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
      ಆದರೆ( p_ಅಂಶ->fg.isTabFunc
       && sqlite3_Walk_ಅಭಿವ್ಯಕ್ತಿ_List(pWalker, p_ಅಂಶ->u1.pFuncArg)
      ){
        ಹಿಂತಿರುಗಿ WRC_Abort;
      }
    }
  }
  ಹಿಂತಿರುಗಿ WRC_Continue;
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
ಪೂರ್ಣಾಂಕ sqlite3WalkSelect(Walker *pWalker, Select *p){
  ಪೂರ್ಣಾಂಕ rc;
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಆದರೆ( pWalker->xSelectCallback==0 ) ಹಿಂತಿರುಗಿ WRC_Continue;
  ಮಾಡು{
    rc = pWalker->xSelectCallback(pWalker, p);
    ಆದರೆ( rc ) ಹಿಂತಿರುಗಿ rc & WRC_Abort;
    ಆದರೆ( sqlite3_Walk_Select_ಅಭಿವ್ಯಕ್ತಿ(pWalker, p)
     || sqlite3WalkSelectFrom(pWalker, p)
    ){
      ಹಿಂತಿರುಗಿ WRC_Abort;
    }
    ಆದರೆ( pWalker->xSelectCallback2 ){
      pWalker->xSelectCallback2(pWalker, p);
    }
    p = p->p_ಪೂರ್ವ;
  }ಆಗ( p!=0 );
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/* Increase the walkerDepth when entering a subquery, and
** decrease when leaving the subquery.
*/
ಪೂರ್ಣಾಂಕ sqlite3_Walker_ಆಳ_Increase(Walker *pWalker, Select *pSelect){
  ಬಳಸದ_PARAMETER(pSelect);
  pWalker->walker_ಆಳ++;
  ಹಿಂತಿರುಗಿ WRC_Continue;
}
ಶೂನ್ಯ sqlite3_Walker_ಆಳ_Decrease(Walker *pWalker, Select *pSelect){
  ಬಳಸದ_PARAMETER(pSelect);
  pWalker->walker_ಆಳ--;
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
ಪೂರ್ಣಾಂಕ sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Walk_Noop(Walker *Not_ಬಳಸಿದ, ಅಭಿವ್ಯಕ್ತಿ *NotUsed2){
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  ಹಿಂತಿರುಗಿ WRC_Continue;
}

/*
** No-op routine for the parse-tree walker for SELECT statements.
** subquery in the parser tree.
*/
ಪೂರ್ಣಾಂಕ sqlite3SelectWalkNoop(Walker *Not_ಬಳಸಿದ, Select *NotUsed2){
  ಬಳಸದ_PARAMETER2(Not_ಬಳಸಿದ, NotUsed2);
  ಹಿಂತಿರುಗಿ WRC_Continue;
}
