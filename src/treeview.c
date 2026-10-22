/*
** 2015-06-08
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains C code to implement the TreeView debugging routines.
** These routines print a parse tree to standard output for debugging and
** analysis. 
**
** The interfaces in this file is only available when compiling
** with SQLITE_DEBUG.
*/
#include "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
static void sqlite3TreeViewPush(TreeView **pp, u8 moreToFollow){
  TreeView *p = *pp;
  ᬬᬾᬦ᭄( p==0 ){
    *pp = p = sqlite3_malloc64( parimaa_na(*p) );
    ᬬᬾᬦ᭄( p==0 ) ᬯᬮᬶ;
    memset(p, 0, parimaa_na(*p));
  }ᬮᬶᬬ{
    p->iLevel++;
  }
  assert( moreToFollow==0 || moreToFollow==1 );
  ᬬᬾᬦ᭄( p->iLevel<(puur_naa_nka)parimaa_na(p->bLine) ) p->bLine[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
static void sqlite3TreeViewPop(TreeView **pp){
  TreeView *p = *pp;
  ᬬᬾᬦ᭄( p==0 ) ᬯᬮᬶ;
  p->iLevel--;
  ᬬᬾᬦ᭄( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
void sqlite3TreeViewLine(TreeView *p, const var_na *zFormat, ...){
  va_list ap;
  puur_naa_nka i;
  StrAccum acc;
  var_na zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, parimaa_na(zBuf), 0);
  ᬬᬾᬦ᭄( p ){
    for(i=0; i<p->iLevel && i<(puur_naa_nka)parimaa_na(p->bLine)-1; i++){
      sqlite3_str_append(&acc, p->bLine[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->bLine[i] ? "|-- " : "'-- ", 4);
  }
  ᬬᬾᬦ᭄( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    assert( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
static void sqlite3TreeViewItem(TreeView *p, const var_na *zLabel,u8 moreFollows){
  sqlite3TreeViewPush(&p, moreFollows);
  sqlite3TreeViewLine(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
void sqlite3TreeViewColumnList(
  TreeView *pView,
  const Column *aCol,
  puur_naa_nka nCol,
  u8 moreToFollow
){
  puur_naa_nka i;
  sqlite3TreeViewPush(&pView, moreToFollow);
  sqlite3TreeViewLine(pView, "COLUMNS");
  for(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    puur_naa_nka colMoreToFollow = i<(nCol - 1);
    sqlite3TreeViewPush(&pView, colMoreToFollow);
    sqlite3TreeViewLine(pView, 0);
    printf(" %s", aCol[i].zCnName);
    switch( aCol[i].eCType ){
      pak_sa COLTYPE_ANY:      printf(" ANY");        bha_nga;
      pak_sa COLTYPE_BLOB:     printf(" BLOB");       bha_nga;
      pak_sa COLTYPE_INT:      printf(" INT");        bha_nga;
      pak_sa COLTYPE_INTEGER:  printf(" INTEGER");    bha_nga;
      pak_sa COLTYPE_REAL:     printf(" REAL");       bha_nga;
      pak_sa COLTYPE_TEXT:     printf(" TEXT");       bha_nga;
      pak_sa COLTYPE_CUSTOM: {
        ᬬᬾᬦ᭄( flg & COLFLAG_HASTYPE ){
          const var_na *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          bha_nga;
        }
      }
    }
    ᬬᬾᬦ᭄( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    ᬬᬾᬦ᭄( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    ᬬᬾᬦ᭄( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    ᬬᬾᬦ᭄( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3TreeViewPop(&pView);
  }
  sqlite3TreeViewPop(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
void sqlite3TreeViewWith(TreeView *pView, const With *pWith, u8 moreToFollow){
  puur_naa_nka i;
  ᬬᬾᬦ᭄( pWith==0 ) ᬯᬮᬶ;
  ᬬᬾᬦ᭄( pWith->nCte==0 ) ᬯᬮᬶ;
  ᬬᬾᬦ᭄( pWith->pOuter ){
    sqlite3TreeViewLine(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }ᬮᬶᬬ{
    sqlite3TreeViewLine(pView, "WITH (0x%p)", pWith);
  }
  ᬬᬾᬦ᭄( pWith->nCte>0 ){
    sqlite3TreeViewPush(&pView, moreToFollow);
    for(i=0; i<pWith->nCte; i++){
      StrAccum x;
      var_na zLine[1000];
      const sa_nracanaa Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, zLine, parimaa_na(zLine), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      ᬬᬾᬦ᭄( pCte->pCols && pCte->pCols->nExpr>0 ){
        var_na cSep = '(';
        puur_naa_nka j;
        for(j=0; j<pCte->pCols->nExpr; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      ᬬᬾᬦ᭄( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      ᬬᬾᬦ᭄( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3TreeViewItem(pView, zLine, i<pWith->nCte-1);
      sqlite3TreeViewSelect(pView, pCte->pSelect, 0);
      sqlite3TreeViewPop(&pView);
    }
    sqlite3TreeViewPop(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
void sqlite3TreeViewSrcList(TreeView *pView, const SrcList *pSrc){
  puur_naa_nka i;
  ᬬᬾᬦ᭄( pSrc==0 ) ᬯᬮᬶ;
  for(i=0; i<pSrc->nSrc; i++){
    const SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    puur_naa_nka n = 0;
    var_na zLine[1000];
    sqlite3StrAccumInit(&x, 0, zLine, parimaa_na(zLine), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    ᬬᬾᬦ᭄( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    ᬬᬾᬦ᭄( (pItem->fg.jointype & (JT_LEFT|JT_RIGHT))==(JT_LEFT|JT_RIGHT) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }ᬮᬶᬬ ᬬᬾᬦ᭄( pItem->fg.jointype & JT_LEFT ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }ᬮᬶᬬ ᬬᬾᬦ᭄( pItem->fg.jointype & JT_RIGHT ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }ᬮᬶᬬ ᬬᬾᬦ᭄( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    ᬬᬾᬦ᭄( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    ᬬᬾᬦ᭄( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ᬬᬾᬦ᭄( pItem->fg.isCte ){
      static const var_na *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    ᬬᬾᬦ᭄( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    ᬬᬾᬦ᭄( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    ᬬᬾᬦ᭄( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    ᬬᬾᬦ᭄( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    ᬬᬾᬦ᭄( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    ᬬᬾᬦ᭄( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    ᬬᬾᬦ᭄( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    ᬬᬾᬦ᭄( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    ᬬᬾᬦ᭄( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    ᬬᬾᬦ᭄( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3TreeViewItem(pView, zLine, i<pSrc->nSrc-1);
    n = 0;
    ᬬᬾᬦ᭄( pItem->fg.isSubquery ) n++;
    ᬬᬾᬦ᭄( pItem->fg.isTabFunc ) n++;
    ᬬᬾᬦ᭄( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    ᬬᬾᬦ᭄( pItem->fg.isUsing ){
      sqlite3TreeViewIdList(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }ᬮᬶᬬ ᬬᬾᬦ᭄( pItem->u3.pOn!=0 ){
      sqlite3TreeViewItem(pView, "ON", (--n)>0);
      sqlite3TreeViewExpr(pView, pItem->u3.pOn, 0);
      sqlite3TreeViewPop(&pView);
    }
    ᬬᬾᬦ᭄( pItem->fg.isSubquery ){
      assert( n==1 );
      ᬬᬾᬦ᭄( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3TreeViewColumnList(pView, pTab->aCol, pTab->nCol, 1);
      }
      assert( (puur_naa_nka)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3TreeViewSelect(pView, pItem->u4.pSubq->pSelect, 0);
    }
    ᬬᬾᬦ᭄( pItem->fg.isTabFunc ){
      sqlite3TreeViewExprList(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3TreeViewPop(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
void sqlite3TreeViewSelect(TreeView *pView, const Select *p, u8 moreToFollow){
  puur_naa_nka n = 0;
  puur_naa_nka cnt = 0;
  ᬬᬾᬦ᭄( p==0 ){
    sqlite3TreeViewLine(pView, "nil-SELECT");
    ᬯᬮᬶ;
  } 
  sqlite3TreeViewPush(&pView, moreToFollow);
  ᬬᬾᬦ᭄( p->pWith ){
    sqlite3TreeViewWith(pView, p->pWith, 1);
    cnt = 1;
    sqlite3TreeViewPush(&pView, 1);
  }
  do{
    ᬬᬾᬦ᭄( p->selFlags & SF_WhereBegin ){
      sqlite3TreeViewLine(pView, "sqlite3WhereBegin()");
    }ᬮᬶᬬ{
      sqlite3TreeViewLine(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (puur_naa_nka)p->nSelectRow
      );
    }
    ᬬᬾᬦ᭄( cnt++ ) sqlite3TreeViewPop(&pView);
    ᬬᬾᬦ᭄( p->pPrior ){
      n = 1000;
    }ᬮᬶᬬ{
      n = 0;
      ᬬᬾᬦ᭄( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      ᬬᬾᬦ᭄( p->pWhere ) n++;
      ᬬᬾᬦ᭄( p->pGroupBy ) n++;
      ᬬᬾᬦ᭄( p->pHaving ) n++;
      ᬬᬾᬦ᭄( p->pOrderBy ) n++;
      ᬬᬾᬦ᭄( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      ᬬᬾᬦ᭄( p->pWin ) n++;
      ᬬᬾᬦ᭄( p->pWinDefn ) n++;
#endif
    }
    ᬬᬾᬦ᭄( p->pEList ){
      sqlite3TreeViewExprList(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    ᬬᬾᬦ᭄( p->pWin ){
      Window *pX;
      sqlite3TreeViewPush(&pView, (n--)>0);
      sqlite3TreeViewLine(pView, "window-functions");
      for(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3TreeViewWinFunc(pView, pX, pX->pNextWin!=0);
      }
      sqlite3TreeViewPop(&pView);
    }
#endif
    ᬬᬾᬦ᭄( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3TreeViewPush(&pView, (n--)>0);
      sqlite3TreeViewLine(pView, "FROM");
      sqlite3TreeViewSrcList(pView, p->pSrc);
      sqlite3TreeViewPop(&pView);
    }
    ᬬᬾᬦ᭄( p->pWhere ){
      sqlite3TreeViewItem(pView, "WHERE", (n--)>0);
      sqlite3TreeViewExpr(pView, p->pWhere, 0);
      sqlite3TreeViewPop(&pView);
    }
    ᬬᬾᬦ᭄( p->pGroupBy ){
      sqlite3TreeViewExprList(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    ᬬᬾᬦ᭄( p->pHaving ){
      sqlite3TreeViewItem(pView, "HAVING", (n--)>0);
      sqlite3TreeViewExpr(pView, p->pHaving, 0);
      sqlite3TreeViewPop(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ᬬᬾᬦ᭄( p->pWinDefn ){
      Window *pX;
      sqlite3TreeViewItem(pView, "WINDOW", (n--)>0);
      for(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3TreeViewWindow(pView, pX, pX->pNextWin!=0);
      }
      sqlite3TreeViewPop(&pView);
    }
#endif
    ᬬᬾᬦ᭄( p->pOrderBy ){
      sqlite3TreeViewExprList(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    ᬬᬾᬦ᭄( p->pLimit ){
      sqlite3TreeViewItem(pView, "LIMIT", (n--)>0);
      sqlite3TreeViewExpr(pView, p->pLimit->pLeft, p->pLimit->pRight!=0);
      ᬬᬾᬦ᭄( p->pLimit->pRight ){
        sqlite3TreeViewItem(pView, "OFFSET", 0);
        sqlite3TreeViewExpr(pView, p->pLimit->pRight, 0);
        sqlite3TreeViewPop(&pView);
      }
      sqlite3TreeViewPop(&pView);
    }
    ᬬᬾᬦ᭄( p->pPrior ){
      const var_na *zOp = "UNION";
      switch( p->op ){
        pak_sa TK_ALL:         zOp = "UNION ALL";  bha_nga;
        pak_sa TK_INTERSECT:   zOp = "INTERSECT";  bha_nga;
        pak_sa TK_EXCEPT:      zOp = "EXCEPT";     bha_nga;
      }
      sqlite3TreeViewItem(pView, zOp, 1);
    }
    p = p->pPrior;
  }ᬲᭂᬤᭂᬮ( p!=0 );
  sqlite3TreeViewPop(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
void sqlite3TreeViewBound(
  TreeView *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  Expr *pExpr,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  switch( eBound ){
    pak_sa TK_UNBOUNDED: {
      sqlite3TreeViewItem(pView, "UNBOUNDED", moreToFollow);
      sqlite3TreeViewPop(&pView);
      bha_nga;
    }
    pak_sa TK_CURRENT: {
      sqlite3TreeViewItem(pView, "CURRENT", moreToFollow);
      sqlite3TreeViewPop(&pView);
      bha_nga;
    }
    pak_sa TK_PRECEDING: {
      sqlite3TreeViewItem(pView, "PRECEDING", moreToFollow);
      sqlite3TreeViewExpr(pView, pExpr, 0);
      sqlite3TreeViewPop(&pView);
      bha_nga;
    }
    pak_sa TK_FOLLOWING: {
      sqlite3TreeViewItem(pView, "FOLLOWING", moreToFollow);
      sqlite3TreeViewExpr(pView, pExpr, 0);
      sqlite3TreeViewPop(&pView);
      bha_nga;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
void sqlite3TreeViewWindow(TreeView *pView, const Window *pWin, u8 more){
  puur_naa_nka nElement = 0;
  ᬬᬾᬦ᭄( pWin==0 ) ᬯᬮᬶ;
  ᬬᬾᬦ᭄( pWin->pFilter ){
    sqlite3TreeViewItem(pView, "FILTER", 1);
    sqlite3TreeViewExpr(pView, pWin->pFilter, 0);
    sqlite3TreeViewPop(&pView);
    ᬬᬾᬦ᭄( pWin->eFrmType==TK_FILTER ) ᬯᬮᬶ;
  }
  sqlite3TreeViewPush(&pView, more);
  ᬬᬾᬦ᭄( pWin->zName ){
    sqlite3TreeViewLine(pView, "OVER %s (%p)", pWin->zName, pWin);
  }ᬮᬶᬬ{
    sqlite3TreeViewLine(pView, "OVER (%p)", pWin);
  }
  ᬬᬾᬦ᭄( pWin->zBase )    nElement++;
  ᬬᬾᬦ᭄( pWin->pOrderBy ) nElement++;
  ᬬᬾᬦ᭄( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  ᬬᬾᬦ᭄( pWin->eExclude ) nElement++;
  ᬬᬾᬦ᭄( pWin->zBase ){
    sqlite3TreeViewPush(&pView, (--nElement)>0);
    sqlite3TreeViewLine(pView, "window: %s", pWin->zBase);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pWin->pPartition ){
    sqlite3TreeViewExprList(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  ᬬᬾᬦ᭄( pWin->pOrderBy ){
    sqlite3TreeViewExprList(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  ᬬᬾᬦ᭄( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    var_na zBuf[30];
    const var_na *zFrmType = "ROWS";
    ᬬᬾᬦ᭄( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    ᬬᬾᬦ᭄( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(parimaa_na(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3TreeViewItem(pView, zBuf, (--nElement)>0);
    sqlite3TreeViewBound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3TreeViewBound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pWin->eExclude ){
    var_na zBuf[30];
    const var_na *zExclude;
    switch( pWin->eExclude ){
      pak_sa TK_NO:      zExclude = "NO OTHERS";   bha_nga;
      pak_sa TK_CURRENT: zExclude = "CURRENT ROW"; bha_nga;
      pak_sa TK_GROUP:   zExclude = "GROUP";       bha_nga;
      pak_sa TK_TIES:    zExclude = "TIES";        bha_nga;
      default:
        sqlite3_snprintf(parimaa_na(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        bha_nga;
    }
    sqlite3TreeViewPush(&pView, 0);
    sqlite3TreeViewLine(pView, "EXCLUDE %s", zExclude);
    sqlite3TreeViewPop(&pView);
  }
  sqlite3TreeViewPop(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
void sqlite3TreeViewWinFunc(TreeView *pView, const Window *pWin, u8 more){
  ᬬᬾᬦ᭄( pWin==0 ) ᬯᬮᬶ;
  sqlite3TreeViewPush(&pView, more);
  sqlite3TreeViewLine(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3TreeViewWindow(pView, pWin, 0);
  sqlite3TreeViewPop(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
void sqlite3TreeViewExpr(TreeView *pView, const Expr *pExpr, u8 moreToFollow){
  const var_na *zBinOp = 0;   /* Binary operator */
  const var_na *zUniOp = 0;   /* Unary operator */
  var_na zFlgs[200];
  sqlite3TreeViewPush(&pView, moreToFollow);
  ᬬᬾᬦ᭄( pExpr==0 ){
    sqlite3TreeViewLine(pView, "nil");
    sqlite3TreeViewPop(&pView);
    ᬯᬮᬶ;
  }
  ᬬᬾᬦ᭄( pExpr->flags || pExpr->affExpr || pExpr->vvaFlags || pExpr->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, parimaa_na(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      pExpr->flags, pExpr->affExpr ? pExpr->affExpr : 'n');
    ᬬᬾᬦ᭄( ExprHasProperty(pExpr, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", pExpr->w.iJoin);
    }
    ᬬᬾᬦ᭄( ExprHasProperty(pExpr, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", pExpr->w.iJoin);
    }
    ᬬᬾᬦ᭄( ExprHasProperty(pExpr, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ᬬᬾᬦ᭄( ExprHasVVAProperty(pExpr, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    ᬬᬾᬦ᭄( pExpr->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", pExpr->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }ᬮᬶᬬ{
    zFlgs[0] = 0;
  }
  switch( pExpr->op ){
    pak_sa TK_AGG_COLUMN: {
      sqlite3TreeViewLine(pView, "AGG{%d:%d}%s",
            pExpr->iTable, pExpr->iColumn, zFlgs);
      bha_nga;
    }
    pak_sa TK_COLUMN: {
      ᬬᬾᬦ᭄( pExpr->iTable<0 ){
        /* This only happens when coding check constraints */
        var_na zOp2[16];
        ᬬᬾᬦ᭄( pExpr->op2 ){
          sqlite3_snprintf(parimaa_na(zOp2),zOp2," op2=0x%02x",pExpr->op2);
        }ᬮᬶᬬ{
          zOp2[0] = 0;
        }
        sqlite3TreeViewLine(pView, "COLUMN(%d)%s%s",
                                    pExpr->iColumn, zFlgs, zOp2);
      }ᬮᬶᬬ{
        assert( ExprUseYTab(pExpr) );
        sqlite3TreeViewLine(pView, "{%d:%d} pTab=%p%s",
                        pExpr->iTable, pExpr->iColumn,
                        pExpr->y.pTab, zFlgs);
      }
      ᬬᬾᬦ᭄( ExprHasProperty(pExpr, EP_FixedCol) ){
        sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      }
      bha_nga;
    }
    pak_sa TK_INTEGER: {
      ᬬᬾᬦ᭄( pExpr->flags & EP_IntValue ){
        sqlite3TreeViewLine(pView, "%d", pExpr->u.iValue);
      }ᬮᬶᬬ{
        sqlite3TreeViewLine(pView, "%s", pExpr->u.zToken);
      }
      bha_nga;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    pak_sa TK_FLOAT: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"%s", pExpr->u.zToken);
      bha_nga;
    }
#endif
    pak_sa TK_STRING: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"%Q", pExpr->u.zToken);
      bha_nga;
    }
    pak_sa TK_NULL: {
      sqlite3TreeViewLine(pView,"NULL");
      bha_nga;
    }
    pak_sa TK_TRUEFALSE: {
      sqlite3TreeViewLine(pView,"%s%s",
         sqlite3ExprTruthValue(pExpr) ? "TRUE" : "FALSE", zFlgs);
      bha_nga;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    pak_sa TK_BLOB: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"%s", pExpr->u.zToken);
      bha_nga;
    }
#endif
    pak_sa TK_VARIABLE: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"VARIABLE(%s,%d)",
                          pExpr->u.zToken, pExpr->iColumn);
      bha_nga;
    }
    pak_sa TK_REGISTER: {
      sqlite3TreeViewLine(pView,"REGISTER(%d)", pExpr->iTable);
      bha_nga;
    }
    pak_sa TK_ID: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"ID \"%w\"", pExpr->u.zToken);
      bha_nga;
    }
    pak_sa TK_ASTERISK: {
      sqlite3TreeViewLine(pView,"ASKTERISK");
      bha_nga;
    }
#ifndef SQLITE_OMIT_CAST
    pak_sa TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"CAST %Q", pExpr->u.zToken);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      bha_nga;
    }
#endif /* SQLITE_OMIT_CAST */
    pak_sa TK_LT:      zBinOp = "LT";     bha_nga;
    pak_sa TK_LE:      zBinOp = "LE";     bha_nga;
    pak_sa TK_GT:      zBinOp = "GT";     bha_nga;
    pak_sa TK_GE:      zBinOp = "GE";     bha_nga;
    pak_sa TK_NE:      zBinOp = "NE";     bha_nga;
    pak_sa TK_EQ:      zBinOp = "EQ";     bha_nga;
    pak_sa TK_IS:      zBinOp = "IS";     bha_nga;
    pak_sa TK_ISNOT:   zBinOp = "ISNOT";  bha_nga;
    pak_sa TK_AND:     zBinOp = "AND";    bha_nga;
    pak_sa TK_OR:      zBinOp = "OR";     bha_nga;
    pak_sa TK_PLUS:    zBinOp = "ADD";    bha_nga;
    pak_sa TK_STAR:    zBinOp = "MUL";    bha_nga;
    pak_sa TK_MINUS:   zBinOp = "SUB";    bha_nga;
    pak_sa TK_REM:     zBinOp = "REM";    bha_nga;
    pak_sa TK_BITAND:  zBinOp = "BITAND"; bha_nga;
    pak_sa TK_BITOR:   zBinOp = "BITOR";  bha_nga;
    pak_sa TK_SLASH:   zBinOp = "DIV";    bha_nga;
    pak_sa TK_LSHIFT:  zBinOp = "LSHIFT"; bha_nga;
    pak_sa TK_RSHIFT:  zBinOp = "RSHIFT"; bha_nga;
    pak_sa TK_CONCAT:  zBinOp = "CONCAT"; bha_nga;
    pak_sa TK_DOT:     zBinOp = "DOT";    bha_nga;
    pak_sa TK_LIMIT:   zBinOp = "LIMIT";  bha_nga;

    pak_sa TK_UMINUS:  zUniOp = "UMINUS"; bha_nga;
    pak_sa TK_UPLUS:   zUniOp = "UPLUS";  bha_nga;
    pak_sa TK_BITNOT:  zUniOp = "BITNOT"; bha_nga;
    pak_sa TK_NOT:     zUniOp = "NOT";    bha_nga;
    pak_sa TK_ISNULL:  zUniOp = "ISNULL"; bha_nga;
    pak_sa TK_NOTNULL: zUniOp = "NOTNULL"; bha_nga;

    pak_sa TK_TRUTH: {
      puur_naa_nka x;
      const var_na *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      assert( pExpr->op2==TK_IS || pExpr->op2==TK_ISNOT );
      assert( pExpr->pRight );
      assert( sqlite3ExprSkipCollateAndLikely(pExpr->pRight)->op
                  == TK_TRUEFALSE );
      x = (pExpr->op2==TK_ISNOT)*2 + sqlite3ExprTruthValue(pExpr->pRight);
      zUniOp = azOp[x];
      bha_nga;
    }

    pak_sa TK_SPAN: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView, "SPAN %Q", pExpr->u.zToken);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      bha_nga;
    }

    pak_sa TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView, "%sCOLLATE %Q%s",
        !ExprHasProperty(pExpr, EP_Collate) ? "SOFT-" : "",
        pExpr->u.zToken, zFlgs);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      bha_nga;
    }

    pak_sa TK_AGG_FUNCTION:
    pak_sa TK_FUNCTION: {
      ExprList *pFarg;       /* List of function arguments */
      Window *pWin;
      ᬬᬾᬦ᭄( ExprHasProperty(pExpr, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }ᬮᬶᬬ{
        assert( ExprUseXList(pExpr) );
        pFarg = pExpr->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(pExpr) ? pExpr->y.pWin : 0;
#ᬮᬶᬬ
        pWin = 0;
#endif 
      }
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      ᬬᬾᬦ᭄( pExpr->op==TK_AGG_FUNCTION ){
        sqlite3TreeViewLine(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             pExpr->op2, pExpr->u.zToken, zFlgs,
                             pExpr->pAggInfo ? pExpr->pAggInfo->selId : 0,
                             pExpr->iAgg, pExpr->pAggInfo);
      }ᬮᬶᬬ ᬬᬾᬦ᭄( pExpr->op2!=0 ){
        const var_na *zOp2;
        var_na zBuf[8];
        sqlite3_snprintf(parimaa_na(zBuf),zBuf,"0x%02x",pExpr->op2);
        zOp2 = zBuf;
        ᬬᬾᬦ᭄( pExpr->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        ᬬᬾᬦ᭄( pExpr->op2==NC_IdxExpr ) zOp2 = "NC_IdxExpr";
        ᬬᬾᬦ᭄( pExpr->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        ᬬᬾᬦ᭄( pExpr->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3TreeViewLine(pView, "FUNCTION %Q%s op2=%s",
                            pExpr->u.zToken, zFlgs, zOp2);
      }ᬮᬶᬬ{
        sqlite3TreeViewLine(pView, "FUNCTION %Q%s", pExpr->u.zToken, zFlgs);
      }
      ᬬᬾᬦ᭄( pFarg ){
        sqlite3TreeViewExprList(pView, pFarg, pWin!=0 || pExpr->pLeft, 0);
        ᬬᬾᬦ᭄( pExpr->pLeft ){
          Expr *pOB = pExpr->pLeft;
          assert( pOB->op==TK_ORDER );
          assert( ExprUseXList(pOB) );
          sqlite3TreeViewExprList(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ᬬᬾᬦ᭄( pWin ){
        sqlite3TreeViewWindow(pView, pWin, 0);
      }
#endif
      bha_nga;
    }
    pak_sa TK_ORDER: {
      sqlite3TreeViewExprList(pView, pExpr->x.pList, 0, "ORDERBY");
      bha_nga;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    pak_sa TK_EXISTS: {
      assert( ExprUseXSelect(pExpr) );
      sqlite3TreeViewLine(pView, "EXISTS-expr flags=0x%x", pExpr->flags);
      sqlite3TreeViewSelect(pView, pExpr->x.pSelect, 0);
      bha_nga;
    }
    pak_sa TK_SELECT: {
      assert( ExprUseXSelect(pExpr) );
      sqlite3TreeViewLine(pView, "subquery-expr flags=0x%x", pExpr->flags);
      sqlite3TreeViewSelect(pView, pExpr->x.pSelect, 0);
      bha_nga;
    }
    pak_sa TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      var_na *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", pExpr->flags);
      ᬬᬾᬦ᭄( pExpr->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",pExpr->iTable);
      ᬬᬾᬦ᭄( ExprHasProperty(pExpr, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            pExpr->y.sub.regReturn, pExpr->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3TreeViewLine(pView, z);
      sqlite3_free(z);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 1);
      ᬬᬾᬦ᭄( ExprUseXSelect(pExpr) ){
        sqlite3TreeViewSelect(pView, pExpr->x.pSelect, 0);
      }ᬮᬶᬬ{
        sqlite3TreeViewExprList(pView, pExpr->x.pList, 0, 0);
      }
      bha_nga;
    }
#endif /* SQLITE_OMIT_SUBQUERY */

    /*
    **    x BETWEEN y AND z
    **
    ** This is equivalent to
    **
    **    x>=y AND x<=z
    **
    ** X is stored in pExpr->pLeft.
    ** Y is stored in pExpr->pList->a[0].pExpr.
    ** Z is stored in pExpr->pList->a[1].pExpr.
    */
    pak_sa TK_BETWEEN: {
      const Expr *pX, *pY, *pZ;
      pX = pExpr->pLeft;
      assert( ExprUseXList(pExpr) );
      assert( pExpr->x.pList->nExpr==2 );
      pY = pExpr->x.pList->a[0].pExpr;
      pZ = pExpr->x.pList->a[1].pExpr;
      sqlite3TreeViewLine(pView, "BETWEEN%s", zFlgs);
      sqlite3TreeViewExpr(pView, pX, 1);
      sqlite3TreeViewExpr(pView, pY, 1);
      sqlite3TreeViewExpr(pView, pZ, 0);
      bha_nga;
    }
    pak_sa TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3TreeViewLine(pView, "%s(%d)", 
          pExpr->iTable ? "NEW" : "OLD", pExpr->iColumn);
      bha_nga;
    }
    pak_sa TK_CASE: {
      sqlite3TreeViewLine(pView, "CASE");
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 1);
      assert( ExprUseXList(pExpr) );
      sqlite3TreeViewExprList(pView, pExpr->x.pList, 0, 0);
      bha_nga;
    }
#ifndef SQLITE_OMIT_TRIGGER
    pak_sa TK_RAISE: {
      const var_na *zType = "unk";
      switch( pExpr->affExpr ){
        pak_sa OE_Rollback:   zType = "rollback";  bha_nga;
        pak_sa OE_Abort:      zType = "abort";     bha_nga;
        pak_sa OE_Fail:       zType = "fail";      bha_nga;
        pak_sa OE_Ignore:     zType = "ignore";    bha_nga;
      }
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView, "RAISE %s", zType);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      bha_nga;
    }
#endif
    pak_sa TK_MATCH: {
      sqlite3TreeViewLine(pView, "MATCH {%d:%d}%s",
                          pExpr->iTable, pExpr->iColumn, zFlgs);
      sqlite3TreeViewExpr(pView, pExpr->pRight, 0);
      bha_nga;
    }
    pak_sa TK_VECTOR: {
      var_na *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      assert( ExprUseXList(pExpr) );
      sqlite3TreeViewBareExprList(pView, pExpr->x.pList, z);
      sqlite3_free(z);
      bha_nga;
    }
    pak_sa TK_SELECT_COLUMN: {
      sqlite3TreeViewLine(pView, "SELECT-COLUMN %d of [0..%d]%s",
              pExpr->iColumn, pExpr->iTable-1,
              pExpr->pRight==pExpr->pLeft ? " (SELECT-owner)" : "");
      assert( ExprUseXSelect(pExpr->pLeft) );
      sqlite3TreeViewSelect(pView, pExpr->pLeft->x.pSelect, 0);
      bha_nga;
    }
    pak_sa TK_IF_NULL_ROW: {
      sqlite3TreeViewLine(pView, "IF-NULL-ROW %d", pExpr->iTable);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      bha_nga;
    }
    pak_sa TK_ERROR: {
      Expr tmp;
      sqlite3TreeViewLine(pView, "ERROR");
      tmp = *pExpr;
      tmp.op = pExpr->op2;
      sqlite3TreeViewExpr(pView, &tmp, 0);
      bha_nga;
    }
    pak_sa TK_ROW: {
      ᬬᬾᬦ᭄( pExpr->iColumn<=0 ){
        sqlite3TreeViewLine(pView, "First FROM table rowid");
      }ᬮᬶᬬ{
        sqlite3TreeViewLine(pView, "First FROM table column %d",
            pExpr->iColumn-1);
      }
      bha_nga;
    }
    default: {
      sqlite3TreeViewLine(pView, "op=%d", pExpr->op);
      bha_nga;
    }
  }
  ᬬᬾᬦ᭄( zBinOp ){
    sqlite3TreeViewLine(pView, "%s%s", zBinOp, zFlgs);
    sqlite3TreeViewExpr(pView, pExpr->pLeft, 1);
    sqlite3TreeViewExpr(pView, pExpr->pRight, 0);
  }ᬮᬶᬬ ᬬᬾᬦ᭄( zUniOp ){
    sqlite3TreeViewLine(pView, "%s%s", zUniOp, zFlgs);
   sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
  }
  sqlite3TreeViewPop(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
void sqlite3TreeViewBareExprList(
  TreeView *pView,
  const ExprList *pList,
  const var_na *zLabel
){
  ᬬᬾᬦ᭄( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ᬬᬾᬦ᭄( pList==0 ){
    sqlite3TreeViewLine(pView, "%s (empty)", zLabel);
  }ᬮᬶᬬ{
    puur_naa_nka i;
    sqlite3TreeViewLine(pView, "%s", zLabel);
    for(i=0; i<pList->nExpr; i++){
      puur_naa_nka j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      var_na *zName = pList->a[i].zEName;
      puur_naa_nka moreToFollow = i<pList->nExpr - 1;
      ᬬᬾᬦ᭄( j || zName || sortFlags ){
        sqlite3TreeViewPush(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3TreeViewLine(pView, 0);
        ᬬᬾᬦ᭄( zName ){
          switch( pList->a[i].fg.eEName ){
            default:
              fprintf(stdout, "AS %s ", zName);
              bha_nga;
            pak_sa ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              ᬬᬾᬦ᭄( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              ᬬᬾᬦ᭄( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              ᬬᬾᬦ᭄( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              bha_nga;
            pak_sa ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              bha_nga;
          }
        }
        ᬬᬾᬦ᭄( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        ᬬᬾᬦ᭄( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }ᬮᬶᬬ ᬬᬾᬦ᭄( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3TreeViewExpr(pView, pList->a[i].pExpr, moreToFollow);
      ᬬᬾᬦ᭄( j || zName || sortFlags ){
        sqlite3TreeViewPop(&pView);
      }
    }
  }
}
void sqlite3TreeViewExprList(
  TreeView *pView,
  const ExprList *pList,
  u8 moreToFollow,
  const var_na *zLabel
){
  sqlite3TreeViewPush(&pView, moreToFollow);
  sqlite3TreeViewBareExprList(pView, pList, zLabel);
  sqlite3TreeViewPop(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
void sqlite3TreeViewBareIdList(
  TreeView *pView,
  const IdList *pList,
  const var_na *zLabel
){
  ᬬᬾᬦ᭄( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ᬬᬾᬦ᭄( pList==0 ){
    sqlite3TreeViewLine(pView, "%s (empty)", zLabel);
  }ᬮᬶᬬ{
    puur_naa_nka i;
    sqlite3TreeViewLine(pView, "%s", zLabel);
    for(i=0; i<pList->nId; i++){
      var_na *zName = pList->a[i].zName;
      puur_naa_nka moreToFollow = i<pList->nId - 1;
      ᬬᬾᬦ᭄( zName==0 ) zName = "(null)";
      sqlite3TreeViewPush(&pView, moreToFollow);
      sqlite3TreeViewLine(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3TreeViewPop(&pView);
    }
  }
}
void sqlite3TreeViewIdList(
  TreeView *pView,
  const IdList *pList,
  u8 moreToFollow,
  const var_na *zLabel
){
  sqlite3TreeViewPush(&pView, moreToFollow);
  sqlite3TreeViewBareIdList(pView, pList, zLabel);
  sqlite3TreeViewPop(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
void sqlite3TreeViewUpsert(
  TreeView *pView,
  const Upsert *pUpsert,
  u8 moreToFollow
){
  ᬬᬾᬦ᭄( pUpsert==0 ) ᬯᬮᬶ;
  sqlite3TreeViewPush(&pView, moreToFollow);
  ᬲᭂᬤᭂᬮ( pUpsert ){
    puur_naa_nka n;
    sqlite3TreeViewPush(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3TreeViewLine(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3TreeViewExprList(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3TreeViewExprList(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    ᬬᬾᬦ᭄( pUpsert->pUpsertWhere ){
      sqlite3TreeViewItem(pView, "WHERE", (n--)>0);
      sqlite3TreeViewExpr(pView, pUpsert->pUpsertWhere, 0);
      sqlite3TreeViewPop(&pView);
    }
    sqlite3TreeViewPop(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3TreeViewPop(&pView);
}

#ᬬᬾᬦ᭄ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
void sqlite3TreeViewDelete(
  const With *pWith,
  const SrcList *pTabList,
  const Expr *pWhere,
  const ExprList *pOrderBy,
  const Expr *pLimit,
  const Trigger *pTrigger
){
  puur_naa_nka n = 0;
  TreeView *pView = 0;
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, "DELETE");
  ᬬᬾᬦ᭄( pWith ) n++;
  ᬬᬾᬦ᭄( pTabList ) n++;
  ᬬᬾᬦ᭄( pWhere ) n++;
  ᬬᬾᬦ᭄( pOrderBy ) n++;
  ᬬᬾᬦ᭄( pLimit ) n++;
  ᬬᬾᬦ᭄( pTrigger ) n++;
  ᬬᬾᬦ᭄( pWith ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewWith(pView, pWith, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pTabList ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "FROM");
    sqlite3TreeViewSrcList(pView, pTabList);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pWhere ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "WHERE");
    sqlite3TreeViewExpr(pView, pWhere, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pOrderBy ){
    sqlite3TreeViewExprList(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  ᬬᬾᬦ᭄( pLimit ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "LIMIT");
    sqlite3TreeViewExpr(pView, pLimit, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pTrigger ){
    sqlite3TreeViewTrigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3TreeViewPop(&pView);
}
#endif /* TREETRACE_ENABLED */

#ᬬᬾᬦ᭄ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
void sqlite3TreeViewInsert(
  const With *pWith,
  const SrcList *pTabList,
  const IdList *pColumnList,
  const Select *pSelect,
  const ExprList *pExprList,
  puur_naa_nka onError,
  const Upsert *pUpsert,
  const Trigger *pTrigger
){
  TreeView *pView = 0;
  puur_naa_nka n = 0;
  const var_na *zLabel = "INSERT";
  switch( onError ){
    pak_sa OE_Replace:  zLabel = "REPLACE";             bha_nga;
    pak_sa OE_Ignore:   zLabel = "INSERT OR IGNORE";    bha_nga;
    pak_sa OE_Rollback: zLabel = "INSERT OR ROLLBACK";  bha_nga;
    pak_sa OE_Abort:    zLabel = "INSERT OR ABORT";     bha_nga;
    pak_sa OE_Fail:     zLabel = "INSERT OR FAIL";      bha_nga;
  }
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, zLabel);
  ᬬᬾᬦ᭄( pWith ) n++;
  ᬬᬾᬦ᭄( pTabList ) n++;
  ᬬᬾᬦ᭄( pColumnList ) n++;
  ᬬᬾᬦ᭄( pSelect ) n++;
  ᬬᬾᬦ᭄( pExprList ) n++;
  ᬬᬾᬦ᭄( pUpsert ) n++;
  ᬬᬾᬦ᭄( pTrigger ) n++;
  ᬬᬾᬦ᭄( pWith ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewWith(pView, pWith, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pTabList ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "INTO");
    sqlite3TreeViewSrcList(pView, pTabList);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pColumnList ){
    sqlite3TreeViewIdList(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  ᬬᬾᬦ᭄( pSelect ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "DATA-SOURCE");
    sqlite3TreeViewSelect(pView, pSelect, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pExprList ){
    sqlite3TreeViewExprList(pView, pExprList, (--n)>0, "VALUES");
  }
  ᬬᬾᬦ᭄( pUpsert ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "UPSERT");
    sqlite3TreeViewUpsert(pView, pUpsert, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pTrigger ){
    sqlite3TreeViewTrigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3TreeViewPop(&pView);
}
#endif /* TREETRACE_ENABLED */

#ᬬᬾᬦ᭄ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
void sqlite3TreeViewUpdate(
  const With *pWith,
  const SrcList *pTabList,
  const ExprList *pChanges,
  const Expr *pWhere,
  puur_naa_nka onError,
  const ExprList *pOrderBy,
  const Expr *pLimit,
  const Upsert *pUpsert,
  const Trigger *pTrigger
){
  puur_naa_nka n = 0;
  TreeView *pView = 0;
  const var_na *zLabel = "UPDATE";
  switch( onError ){
    pak_sa OE_Replace:  zLabel = "UPDATE OR REPLACE";   bha_nga;
    pak_sa OE_Ignore:   zLabel = "UPDATE OR IGNORE";    bha_nga;
    pak_sa OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  bha_nga;
    pak_sa OE_Abort:    zLabel = "UPDATE OR ABORT";     bha_nga;
    pak_sa OE_Fail:     zLabel = "UPDATE OR FAIL";      bha_nga;
  }
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, zLabel);
  ᬬᬾᬦ᭄( pWith ) n++;
  ᬬᬾᬦ᭄( pTabList ) n++;
  ᬬᬾᬦ᭄( pChanges ) n++;
  ᬬᬾᬦ᭄( pWhere ) n++;
  ᬬᬾᬦ᭄( pOrderBy ) n++;
  ᬬᬾᬦ᭄( pLimit ) n++;
  ᬬᬾᬦ᭄( pUpsert ) n++;
  ᬬᬾᬦ᭄( pTrigger ) n++;
  ᬬᬾᬦ᭄( pWith ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewWith(pView, pWith, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pTabList ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "FROM");
    sqlite3TreeViewSrcList(pView, pTabList);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pChanges ){
    sqlite3TreeViewExprList(pView, pChanges, (--n)>0, "SET");
  }
  ᬬᬾᬦ᭄( pWhere ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "WHERE");
    sqlite3TreeViewExpr(pView, pWhere, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pOrderBy ){
    sqlite3TreeViewExprList(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  ᬬᬾᬦ᭄( pLimit ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "LIMIT");
    sqlite3TreeViewExpr(pView, pLimit, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pUpsert ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "UPSERT");
    sqlite3TreeViewUpsert(pView, pUpsert, 0);
    sqlite3TreeViewPop(&pView);
  }
  ᬬᬾᬦ᭄( pTrigger ){
    sqlite3TreeViewTrigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3TreeViewPop(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
void sqlite3TreeViewTriggerStep(
  TreeView *pView,
  const TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  puur_naa_nka cnt = 0;
  ᬬᬾᬦ᭄( pStep==0 ) ᬯᬮᬶ;
  sqlite3TreeViewPush(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  do{
    ᬬᬾᬦ᭄( cnt++ && pStep->pNext==0 ){
      sqlite3TreeViewPop(&pView);
      sqlite3TreeViewPush(&pView, 0);
    }
    sqlite3TreeViewLine(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }ᬲᭂᬤᭂᬮ( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3TreeViewPop(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
void sqlite3TreeViewTrigger(
  TreeView *pView,
  const Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  puur_naa_nka cnt = 0;
  ᬬᬾᬦ᭄( pTrigger==0 ) ᬯᬮᬶ;
  sqlite3TreeViewPush(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  do{
    ᬬᬾᬦ᭄( cnt++ && pTrigger->pNext==0 ){
      sqlite3TreeViewPop(&pView);
      sqlite3TreeViewPush(&pView, 0);
    }
    sqlite3TreeViewLine(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3TreeViewPush(&pView, 0);
    sqlite3TreeViewTriggerStep(pView, pTrigger->step_list, 0, 1);
    sqlite3TreeViewPop(&pView);
  }ᬲᭂᬤᭂᬮ( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3TreeViewPop(&pView);
}
#endif /* SQLITE_OMIT_TRIGGER */
  

/*
** These simplified versions of the tree-view routines omit unnecessary
** parameters.  These variants are intended to be used from a symbolic
** debugger, such as "gdb", during interactive debugging sessions.
**
** This routines are given external linkage so that they will always be
** accessible to the debugging, and to avoid warnings about unused
** functions.  But these routines only exist in debugging builds, so they
** do not contaminate the interface.
**
** See Also:
**
**     sqlite3ShowWhereTerm() in where.c
*/
void sqlite3ShowExpr(const Expr *p){ sqlite3TreeViewExpr(0,p,0); }
void sqlite3ShowExprList(const ExprList *p){ sqlite3TreeViewExprList(0,p,0,0);}
void sqlite3ShowIdList(const IdList *p){ sqlite3TreeViewIdList(0,p,0,0); }
void sqlite3ShowSrcList(const SrcList *p){
  TreeView *pView = 0;
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, "SRCLIST");
  sqlite3TreeViewSrcList(pView,p);
  sqlite3TreeViewPop(&pView);
}
void sqlite3ShowSelect(const Select *p){ sqlite3TreeViewSelect(0,p,0); }
void sqlite3ShowWith(const With *p){ sqlite3TreeViewWith(0,p,0); }
void sqlite3ShowUpsert(const Upsert *p){ sqlite3TreeViewUpsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
void sqlite3ShowTriggerStep(const TriggerStep *p){
  sqlite3TreeViewTriggerStep(0,p,0,0);
}
void sqlite3ShowTriggerStepList(const TriggerStep *p){
  sqlite3TreeViewTriggerStep(0,p,0,1);
}
void sqlite3ShowTrigger(const Trigger *p){ sqlite3TreeViewTrigger(0,p,0,0); }
void sqlite3ShowTriggerList(const Trigger *p){ sqlite3TreeViewTrigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
void sqlite3ShowWindow(const Window *p){ sqlite3TreeViewWindow(0,p,0); }
void sqlite3ShowWinFunc(const Window *p){ sqlite3TreeViewWinFunc(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
