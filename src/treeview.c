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
#ضمن "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
ساكن فراغ sqlite3TreeViewPush(TreeView **pp, u8 moreToFollow){
  TreeView *p = *pp;
  ܐܢ( p==0 ){
    *pp = p = sqlite3_malloc64( حجم(*p) );
    ܐܢ( p==0 ) ܦܢܐ;
    memset(p, 0, حجم(*p));
  }ܐܚܪܢܐ{
    p->iLevel++;
  }
  أكد( moreToFollow==0 || moreToFollow==1 );
  ܐܢ( p->iLevel<(صحيح)حجم(p->bLine) ) p->bLine[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
ساكن فراغ sqlite3TreeViewPop(TreeView **pp){
  TreeView *p = *pp;
  ܐܢ( p==0 ) ܦܢܐ;
  p->iLevel--;
  ܐܢ( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
فراغ sqlite3TreeViewLine(TreeView *p, ثابت حرف *zFormat, ...){
  va_list ap;
  صحيح i;
  StrAccum acc;
  حرف zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
  ܐܢ( p ){
    لكل(i=0; i<p->iLevel && i<(صحيح)حجم(p->bLine)-1; i++){
      sqlite3_str_append(&acc, p->bLine[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->bLine[i] ? "|-- " : "'-- ", 4);
  }
  ܐܢ( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    أكد( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
ساكن فراغ sqlite3TreeViewItem(TreeView *p, ثابت حرف *zLabel,u8 moreFollows){
  sqlite3TreeViewPush(&p, moreFollows);
  sqlite3TreeViewLine(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
فراغ sqlite3TreeViewColumnList(
  TreeView *pView,
  ثابت Column *aCol,
  صحيح nCol,
  u8 moreToFollow
){
  صحيح i;
  sqlite3TreeViewPush(&pView, moreToFollow);
  sqlite3TreeViewLine(pView, "COLUMNS");
  لكل(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    صحيح colMoreToFollow = i<(nCol - 1);
    sqlite3TreeViewPush(&pView, colMoreToFollow);
    sqlite3TreeViewLine(pView, 0);
    printf(" %s", aCol[i].zCnName);
    بدل( aCol[i].eCType ){
      حالة COLTYPE_ANY:      printf(" ANY");        اكسر;
      حالة COLTYPE_BLOB:     printf(" BLOB");       اكسر;
      حالة COLTYPE_INT:      printf(" INT");        اكسر;
      حالة COLTYPE_INTEGER:  printf(" INTEGER");    اكسر;
      حالة COLTYPE_REAL:     printf(" REAL");       اكسر;
      حالة COLTYPE_TEXT:     printf(" TEXT");       اكسر;
      حالة COLTYPE_CUSTOM: {
        ܐܢ( flg & COLFLAG_HASTYPE ){
          ثابت حرف *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          اكسر;
        }
      }
    }
    ܐܢ( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    ܐܢ( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    ܐܢ( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    ܐܢ( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3TreeViewPop(&pView);
  }
  sqlite3TreeViewPop(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
فراغ sqlite3TreeViewWith(TreeView *pView, ثابت With *pWith, u8 moreToFollow){
  صحيح i;
  ܐܢ( pWith==0 ) ܦܢܐ;
  ܐܢ( pWith->nCte==0 ) ܦܢܐ;
  ܐܢ( pWith->pOuter ){
    sqlite3TreeViewLine(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }ܐܚܪܢܐ{
    sqlite3TreeViewLine(pView, "WITH (0x%p)", pWith);
  }
  ܐܢ( pWith->nCte>0 ){
    sqlite3TreeViewPush(&pView, moreToFollow);
    لكل(i=0; i<pWith->nCte; i++){
      StrAccum x;
      حرف zLine[1000];
      ثابت بنية Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, zLine, حجم(zLine), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      ܐܢ( pCte->pCols && pCte->pCols->nExpr>0 ){
        حرف cSep = '(';
        صحيح j;
        لكل(j=0; j<pCte->pCols->nExpr; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      ܐܢ( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      ܐܢ( pCte->pUse ){
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
فراغ sqlite3TreeViewSrcList(TreeView *pView, ثابت SrcList *pSrc){
  صحيح i;
  ܐܢ( pSrc==0 ) ܦܢܐ;
  لكل(i=0; i<pSrc->nSrc; i++){
    ثابت SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    صحيح n = 0;
    حرف zLine[1000];
    sqlite3StrAccumInit(&x, 0, zLine, حجم(zLine), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    ܐܢ( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    ܐܢ( (pItem->fg.jointype & (JT_LEFT|JT_RIGHT))==(JT_LEFT|JT_RIGHT) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }ܐܚܪܢܐ ܐܢ( pItem->fg.jointype & JT_LEFT ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }ܐܚܪܢܐ ܐܢ( pItem->fg.jointype & JT_RIGHT ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }ܐܚܪܢܐ ܐܢ( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    ܐܢ( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    ܐܢ( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ܐܢ( pItem->fg.isCte ){
      ساكن ثابت حرف *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    ܐܢ( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    ܐܢ( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    ܐܢ( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    ܐܢ( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    ܐܢ( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    ܐܢ( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    ܐܢ( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    ܐܢ( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    ܐܢ( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    ܐܢ( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3TreeViewItem(pView, zLine, i<pSrc->nSrc-1);
    n = 0;
    ܐܢ( pItem->fg.isSubquery ) n++;
    ܐܢ( pItem->fg.isTabFunc ) n++;
    ܐܢ( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    ܐܢ( pItem->fg.isUsing ){
      sqlite3TreeViewIdList(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }ܐܚܪܢܐ ܐܢ( pItem->u3.pOn!=0 ){
      sqlite3TreeViewItem(pView, "ON", (--n)>0);
      sqlite3TreeViewExpr(pView, pItem->u3.pOn, 0);
      sqlite3TreeViewPop(&pView);
    }
    ܐܢ( pItem->fg.isSubquery ){
      أكد( n==1 );
      ܐܢ( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3TreeViewColumnList(pView, pTab->aCol, pTab->nCol, 1);
      }
      أكد( (صحيح)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3TreeViewSelect(pView, pItem->u4.pSubq->pSelect, 0);
    }
    ܐܢ( pItem->fg.isTabFunc ){
      sqlite3TreeViewExprList(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3TreeViewPop(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
فراغ sqlite3TreeViewSelect(TreeView *pView, ثابت Select *p, u8 moreToFollow){
  صحيح n = 0;
  صحيح cnt = 0;
  ܐܢ( p==0 ){
    sqlite3TreeViewLine(pView, "nil-SELECT");
    ܦܢܐ;
  } 
  sqlite3TreeViewPush(&pView, moreToFollow);
  ܐܢ( p->pWith ){
    sqlite3TreeViewWith(pView, p->pWith, 1);
    cnt = 1;
    sqlite3TreeViewPush(&pView, 1);
  }
  ܥܒܕ{
    ܐܢ( p->selFlags & SF_WhereBegin ){
      sqlite3TreeViewLine(pView, "sqlite3WhereBegin()");
    }ܐܚܪܢܐ{
      sqlite3TreeViewLine(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (صحيح)p->nSelectRow
      );
    }
    ܐܢ( cnt++ ) sqlite3TreeViewPop(&pView);
    ܐܢ( p->pPrior ){
      n = 1000;
    }ܐܚܪܢܐ{
      n = 0;
      ܐܢ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      ܐܢ( p->pWhere ) n++;
      ܐܢ( p->pGroupBy ) n++;
      ܐܢ( p->pHaving ) n++;
      ܐܢ( p->pOrderBy ) n++;
      ܐܢ( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      ܐܢ( p->pWin ) n++;
      ܐܢ( p->pWinDefn ) n++;
#endif
    }
    ܐܢ( p->pEList ){
      sqlite3TreeViewExprList(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    ܐܢ( p->pWin ){
      Window *pX;
      sqlite3TreeViewPush(&pView, (n--)>0);
      sqlite3TreeViewLine(pView, "window-functions");
      لكل(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3TreeViewWinFunc(pView, pX, pX->pNextWin!=0);
      }
      sqlite3TreeViewPop(&pView);
    }
#endif
    ܐܢ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3TreeViewPush(&pView, (n--)>0);
      sqlite3TreeViewLine(pView, "FROM");
      sqlite3TreeViewSrcList(pView, p->pSrc);
      sqlite3TreeViewPop(&pView);
    }
    ܐܢ( p->pWhere ){
      sqlite3TreeViewItem(pView, "WHERE", (n--)>0);
      sqlite3TreeViewExpr(pView, p->pWhere, 0);
      sqlite3TreeViewPop(&pView);
    }
    ܐܢ( p->pGroupBy ){
      sqlite3TreeViewExprList(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    ܐܢ( p->pHaving ){
      sqlite3TreeViewItem(pView, "HAVING", (n--)>0);
      sqlite3TreeViewExpr(pView, p->pHaving, 0);
      sqlite3TreeViewPop(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ܐܢ( p->pWinDefn ){
      Window *pX;
      sqlite3TreeViewItem(pView, "WINDOW", (n--)>0);
      لكل(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3TreeViewWindow(pView, pX, pX->pNextWin!=0);
      }
      sqlite3TreeViewPop(&pView);
    }
#endif
    ܐܢ( p->pOrderBy ){
      sqlite3TreeViewExprList(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    ܐܢ( p->pLimit ){
      sqlite3TreeViewItem(pView, "LIMIT", (n--)>0);
      sqlite3TreeViewExpr(pView, p->pLimit->pLeft, p->pLimit->pRight!=0);
      ܐܢ( p->pLimit->pRight ){
        sqlite3TreeViewItem(pView, "OFFSET", 0);
        sqlite3TreeViewExpr(pView, p->pLimit->pRight, 0);
        sqlite3TreeViewPop(&pView);
      }
      sqlite3TreeViewPop(&pView);
    }
    ܐܢ( p->pPrior ){
      ثابت حرف *zOp = "UNION";
      بدل( p->op ){
        حالة TK_ALL:         zOp = "UNION ALL";  اكسر;
        حالة TK_INTERSECT:   zOp = "INTERSECT";  اكسر;
        حالة TK_EXCEPT:      zOp = "EXCEPT";     اكسر;
      }
      sqlite3TreeViewItem(pView, zOp, 1);
    }
    p = p->pPrior;
  }ܟܡܐ( p!=0 );
  sqlite3TreeViewPop(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
فراغ sqlite3TreeViewBound(
  TreeView *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  Expr *pExpr,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  بدل( eBound ){
    حالة TK_UNBOUNDED: {
      sqlite3TreeViewItem(pView, "UNBOUNDED", moreToFollow);
      sqlite3TreeViewPop(&pView);
      اكسر;
    }
    حالة TK_CURRENT: {
      sqlite3TreeViewItem(pView, "CURRENT", moreToFollow);
      sqlite3TreeViewPop(&pView);
      اكسر;
    }
    حالة TK_PRECEDING: {
      sqlite3TreeViewItem(pView, "PRECEDING", moreToFollow);
      sqlite3TreeViewExpr(pView, pExpr, 0);
      sqlite3TreeViewPop(&pView);
      اكسر;
    }
    حالة TK_FOLLOWING: {
      sqlite3TreeViewItem(pView, "FOLLOWING", moreToFollow);
      sqlite3TreeViewExpr(pView, pExpr, 0);
      sqlite3TreeViewPop(&pView);
      اكسر;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
فراغ sqlite3TreeViewWindow(TreeView *pView, ثابت Window *pWin, u8 more){
  صحيح nElement = 0;
  ܐܢ( pWin==0 ) ܦܢܐ;
  ܐܢ( pWin->pFilter ){
    sqlite3TreeViewItem(pView, "FILTER", 1);
    sqlite3TreeViewExpr(pView, pWin->pFilter, 0);
    sqlite3TreeViewPop(&pView);
    ܐܢ( pWin->eFrmType==TK_FILTER ) ܦܢܐ;
  }
  sqlite3TreeViewPush(&pView, more);
  ܐܢ( pWin->zName ){
    sqlite3TreeViewLine(pView, "OVER %s (%p)", pWin->zName, pWin);
  }ܐܚܪܢܐ{
    sqlite3TreeViewLine(pView, "OVER (%p)", pWin);
  }
  ܐܢ( pWin->zBase )    nElement++;
  ܐܢ( pWin->pOrderBy ) nElement++;
  ܐܢ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  ܐܢ( pWin->eExclude ) nElement++;
  ܐܢ( pWin->zBase ){
    sqlite3TreeViewPush(&pView, (--nElement)>0);
    sqlite3TreeViewLine(pView, "window: %s", pWin->zBase);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pWin->pPartition ){
    sqlite3TreeViewExprList(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  ܐܢ( pWin->pOrderBy ){
    sqlite3TreeViewExprList(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  ܐܢ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    حرف zBuf[30];
    ثابت حرف *zFrmType = "ROWS";
    ܐܢ( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    ܐܢ( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(حجم(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3TreeViewItem(pView, zBuf, (--nElement)>0);
    sqlite3TreeViewBound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3TreeViewBound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pWin->eExclude ){
    حرف zBuf[30];
    ثابت حرف *zExclude;
    بدل( pWin->eExclude ){
      حالة TK_NO:      zExclude = "NO OTHERS";   اكسر;
      حالة TK_CURRENT: zExclude = "CURRENT ROW"; اكسر;
      حالة TK_GROUP:   zExclude = "GROUP";       اكسر;
      حالة TK_TIES:    zExclude = "TIES";        اكسر;
      افتراضي:
        sqlite3_snprintf(حجم(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        اكسر;
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
فراغ sqlite3TreeViewWinFunc(TreeView *pView, ثابت Window *pWin, u8 more){
  ܐܢ( pWin==0 ) ܦܢܐ;
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
فراغ sqlite3TreeViewExpr(TreeView *pView, ثابت Expr *pExpr, u8 moreToFollow){
  ثابت حرف *zBinOp = 0;   /* Binary operator */
  ثابت حرف *zUniOp = 0;   /* Unary operator */
  حرف zFlgs[200];
  sqlite3TreeViewPush(&pView, moreToFollow);
  ܐܢ( pExpr==0 ){
    sqlite3TreeViewLine(pView, "nil");
    sqlite3TreeViewPop(&pView);
    ܦܢܐ;
  }
  ܐܢ( pExpr->flags || pExpr->affExpr || pExpr->vvaFlags || pExpr->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, حجم(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      pExpr->flags, pExpr->affExpr ? pExpr->affExpr : 'n');
    ܐܢ( ExprHasProperty(pExpr, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", pExpr->w.iJoin);
    }
    ܐܢ( ExprHasProperty(pExpr, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", pExpr->w.iJoin);
    }
    ܐܢ( ExprHasProperty(pExpr, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ܐܢ( ExprHasVVAProperty(pExpr, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    ܐܢ( pExpr->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", pExpr->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }ܐܚܪܢܐ{
    zFlgs[0] = 0;
  }
  بدل( pExpr->op ){
    حالة TK_AGG_COLUMN: {
      sqlite3TreeViewLine(pView, "AGG{%d:%d}%s",
            pExpr->iTable, pExpr->iColumn, zFlgs);
      اكسر;
    }
    حالة TK_COLUMN: {
      ܐܢ( pExpr->iTable<0 ){
        /* This only happens when coding check constraints */
        حرف zOp2[16];
        ܐܢ( pExpr->op2 ){
          sqlite3_snprintf(حجم(zOp2),zOp2," op2=0x%02x",pExpr->op2);
        }ܐܚܪܢܐ{
          zOp2[0] = 0;
        }
        sqlite3TreeViewLine(pView, "COLUMN(%d)%s%s",
                                    pExpr->iColumn, zFlgs, zOp2);
      }ܐܚܪܢܐ{
        أكد( ExprUseYTab(pExpr) );
        sqlite3TreeViewLine(pView, "{%d:%d} pTab=%p%s",
                        pExpr->iTable, pExpr->iColumn,
                        pExpr->y.pTab, zFlgs);
      }
      ܐܢ( ExprHasProperty(pExpr, EP_FixedCol) ){
        sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      }
      اكسر;
    }
    حالة TK_INTEGER: {
      ܐܢ( pExpr->flags & EP_IntValue ){
        sqlite3TreeViewLine(pView, "%d", pExpr->u.iValue);
      }ܐܚܪܢܐ{
        sqlite3TreeViewLine(pView, "%s", pExpr->u.zToken);
      }
      اكسر;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    حالة TK_FLOAT: {
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"%s", pExpr->u.zToken);
      اكسر;
    }
#endif
    حالة TK_STRING: {
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"%Q", pExpr->u.zToken);
      اكسر;
    }
    حالة TK_NULL: {
      sqlite3TreeViewLine(pView,"NULL");
      اكسر;
    }
    حالة TK_TRUEFALSE: {
      sqlite3TreeViewLine(pView,"%s%s",
         sqlite3ExprTruthValue(pExpr) ? "TRUE" : "FALSE", zFlgs);
      اكسر;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    حالة TK_BLOB: {
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"%s", pExpr->u.zToken);
      اكسر;
    }
#endif
    حالة TK_VARIABLE: {
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"VARIABLE(%s,%d)",
                          pExpr->u.zToken, pExpr->iColumn);
      اكسر;
    }
    حالة TK_REGISTER: {
      sqlite3TreeViewLine(pView,"REGISTER(%d)", pExpr->iTable);
      اكسر;
    }
    حالة TK_ID: {
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"ID \"%w\"", pExpr->u.zToken);
      اكسر;
    }
    حالة TK_ASTERISK: {
      sqlite3TreeViewLine(pView,"ASKTERISK");
      اكسر;
    }
#ifndef SQLITE_OMIT_CAST
    حالة TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"CAST %Q", pExpr->u.zToken);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      اكسر;
    }
#endif /* SQLITE_OMIT_CAST */
    حالة TK_LT:      zBinOp = "LT";     اكسر;
    حالة TK_LE:      zBinOp = "LE";     اكسر;
    حالة TK_GT:      zBinOp = "GT";     اكسر;
    حالة TK_GE:      zBinOp = "GE";     اكسر;
    حالة TK_NE:      zBinOp = "NE";     اكسر;
    حالة TK_EQ:      zBinOp = "EQ";     اكسر;
    حالة TK_IS:      zBinOp = "IS";     اكسر;
    حالة TK_ISNOT:   zBinOp = "ISNOT";  اكسر;
    حالة TK_AND:     zBinOp = "AND";    اكسر;
    حالة TK_OR:      zBinOp = "OR";     اكسر;
    حالة TK_PLUS:    zBinOp = "ADD";    اكسر;
    حالة TK_STAR:    zBinOp = "MUL";    اكسر;
    حالة TK_MINUS:   zBinOp = "SUB";    اكسر;
    حالة TK_REM:     zBinOp = "REM";    اكسر;
    حالة TK_BITAND:  zBinOp = "BITAND"; اكسر;
    حالة TK_BITOR:   zBinOp = "BITOR";  اكسر;
    حالة TK_SLASH:   zBinOp = "DIV";    اكسر;
    حالة TK_LSHIFT:  zBinOp = "LSHIFT"; اكسر;
    حالة TK_RSHIFT:  zBinOp = "RSHIFT"; اكسر;
    حالة TK_CONCAT:  zBinOp = "CONCAT"; اكسر;
    حالة TK_DOT:     zBinOp = "DOT";    اكسر;
    حالة TK_LIMIT:   zBinOp = "LIMIT";  اكسر;

    حالة TK_UMINUS:  zUniOp = "UMINUS"; اكسر;
    حالة TK_UPLUS:   zUniOp = "UPLUS";  اكسر;
    حالة TK_BITNOT:  zUniOp = "BITNOT"; اكسر;
    حالة TK_NOT:     zUniOp = "NOT";    اكسر;
    حالة TK_ISNULL:  zUniOp = "ISNULL"; اكسر;
    حالة TK_NOTNULL: zUniOp = "NOTNULL"; اكسر;

    حالة TK_TRUTH: {
      صحيح x;
      ثابت حرف *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      أكد( pExpr->op2==TK_IS || pExpr->op2==TK_ISNOT );
      أكد( pExpr->pRight );
      أكد( sqlite3ExprSkipCollateAndLikely(pExpr->pRight)->op
                  == TK_TRUEFALSE );
      x = (pExpr->op2==TK_ISNOT)*2 + sqlite3ExprTruthValue(pExpr->pRight);
      zUniOp = azOp[x];
      اكسر;
    }

    حالة TK_SPAN: {
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView, "SPAN %Q", pExpr->u.zToken);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      اكسر;
    }

    حالة TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView, "%sCOLLATE %Q%s",
        !ExprHasProperty(pExpr, EP_Collate) ? "SOFT-" : "",
        pExpr->u.zToken, zFlgs);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      اكسر;
    }

    حالة TK_AGG_FUNCTION:
    حالة TK_FUNCTION: {
      ExprList *pFarg;       /* List of function arguments */
      Window *pWin;
      ܐܢ( ExprHasProperty(pExpr, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }ܐܚܪܢܐ{
        أكد( ExprUseXList(pExpr) );
        pFarg = pExpr->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(pExpr) ? pExpr->y.pWin : 0;
#ܐܚܪܢܐ
        pWin = 0;
#endif 
      }
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      ܐܢ( pExpr->op==TK_AGG_FUNCTION ){
        sqlite3TreeViewLine(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             pExpr->op2, pExpr->u.zToken, zFlgs,
                             pExpr->pAggInfo ? pExpr->pAggInfo->selId : 0,
                             pExpr->iAgg, pExpr->pAggInfo);
      }ܐܚܪܢܐ ܐܢ( pExpr->op2!=0 ){
        ثابت حرف *zOp2;
        حرف zBuf[8];
        sqlite3_snprintf(حجم(zBuf),zBuf,"0x%02x",pExpr->op2);
        zOp2 = zBuf;
        ܐܢ( pExpr->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        ܐܢ( pExpr->op2==NC_IdxExpr ) zOp2 = "NC_IdxExpr";
        ܐܢ( pExpr->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        ܐܢ( pExpr->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3TreeViewLine(pView, "FUNCTION %Q%s op2=%s",
                            pExpr->u.zToken, zFlgs, zOp2);
      }ܐܚܪܢܐ{
        sqlite3TreeViewLine(pView, "FUNCTION %Q%s", pExpr->u.zToken, zFlgs);
      }
      ܐܢ( pFarg ){
        sqlite3TreeViewExprList(pView, pFarg, pWin!=0 || pExpr->pLeft, 0);
        ܐܢ( pExpr->pLeft ){
          Expr *pOB = pExpr->pLeft;
          أكد( pOB->op==TK_ORDER );
          أكد( ExprUseXList(pOB) );
          sqlite3TreeViewExprList(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ܐܢ( pWin ){
        sqlite3TreeViewWindow(pView, pWin, 0);
      }
#endif
      اكسر;
    }
    حالة TK_ORDER: {
      sqlite3TreeViewExprList(pView, pExpr->x.pList, 0, "ORDERBY");
      اكسر;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS: {
      أكد( ExprUseXSelect(pExpr) );
      sqlite3TreeViewLine(pView, "EXISTS-expr flags=0x%x", pExpr->flags);
      sqlite3TreeViewSelect(pView, pExpr->x.pSelect, 0);
      اكسر;
    }
    حالة TK_SELECT: {
      أكد( ExprUseXSelect(pExpr) );
      sqlite3TreeViewLine(pView, "subquery-expr flags=0x%x", pExpr->flags);
      sqlite3TreeViewSelect(pView, pExpr->x.pSelect, 0);
      اكسر;
    }
    حالة TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      حرف *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", pExpr->flags);
      ܐܢ( pExpr->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",pExpr->iTable);
      ܐܢ( ExprHasProperty(pExpr, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            pExpr->y.sub.regReturn, pExpr->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3TreeViewLine(pView, z);
      sqlite3_free(z);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 1);
      ܐܢ( ExprUseXSelect(pExpr) ){
        sqlite3TreeViewSelect(pView, pExpr->x.pSelect, 0);
      }ܐܚܪܢܐ{
        sqlite3TreeViewExprList(pView, pExpr->x.pList, 0, 0);
      }
      اكسر;
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
    حالة TK_BETWEEN: {
      ثابت Expr *pX, *pY, *pZ;
      pX = pExpr->pLeft;
      أكد( ExprUseXList(pExpr) );
      أكد( pExpr->x.pList->nExpr==2 );
      pY = pExpr->x.pList->a[0].pExpr;
      pZ = pExpr->x.pList->a[1].pExpr;
      sqlite3TreeViewLine(pView, "BETWEEN%s", zFlgs);
      sqlite3TreeViewExpr(pView, pX, 1);
      sqlite3TreeViewExpr(pView, pY, 1);
      sqlite3TreeViewExpr(pView, pZ, 0);
      اكسر;
    }
    حالة TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3TreeViewLine(pView, "%s(%d)", 
          pExpr->iTable ? "NEW" : "OLD", pExpr->iColumn);
      اكسر;
    }
    حالة TK_CASE: {
      sqlite3TreeViewLine(pView, "CASE");
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 1);
      أكد( ExprUseXList(pExpr) );
      sqlite3TreeViewExprList(pView, pExpr->x.pList, 0, 0);
      اكسر;
    }
#ifndef SQLITE_OMIT_TRIGGER
    حالة TK_RAISE: {
      ثابت حرف *zType = "unk";
      بدل( pExpr->affExpr ){
        حالة OE_Rollback:   zType = "rollback";  اكسر;
        حالة OE_Abort:      zType = "abort";     اكسر;
        حالة OE_Fail:       zType = "fail";      اكسر;
        حالة OE_Ignore:     zType = "ignore";    اكسر;
      }
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView, "RAISE %s", zType);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      اكسر;
    }
#endif
    حالة TK_MATCH: {
      sqlite3TreeViewLine(pView, "MATCH {%d:%d}%s",
                          pExpr->iTable, pExpr->iColumn, zFlgs);
      sqlite3TreeViewExpr(pView, pExpr->pRight, 0);
      اكسر;
    }
    حالة TK_VECTOR: {
      حرف *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      أكد( ExprUseXList(pExpr) );
      sqlite3TreeViewBareExprList(pView, pExpr->x.pList, z);
      sqlite3_free(z);
      اكسر;
    }
    حالة TK_SELECT_COLUMN: {
      sqlite3TreeViewLine(pView, "SELECT-COLUMN %d of [0..%d]%s",
              pExpr->iColumn, pExpr->iTable-1,
              pExpr->pRight==pExpr->pLeft ? " (SELECT-owner)" : "");
      أكد( ExprUseXSelect(pExpr->pLeft) );
      sqlite3TreeViewSelect(pView, pExpr->pLeft->x.pSelect, 0);
      اكسر;
    }
    حالة TK_IF_NULL_ROW: {
      sqlite3TreeViewLine(pView, "IF-NULL-ROW %d", pExpr->iTable);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      اكسر;
    }
    حالة TK_ERROR: {
      Expr tmp;
      sqlite3TreeViewLine(pView, "ERROR");
      tmp = *pExpr;
      tmp.op = pExpr->op2;
      sqlite3TreeViewExpr(pView, &tmp, 0);
      اكسر;
    }
    حالة TK_ROW: {
      ܐܢ( pExpr->iColumn<=0 ){
        sqlite3TreeViewLine(pView, "First FROM table rowid");
      }ܐܚܪܢܐ{
        sqlite3TreeViewLine(pView, "First FROM table column %d",
            pExpr->iColumn-1);
      }
      اكسر;
    }
    افتراضي: {
      sqlite3TreeViewLine(pView, "op=%d", pExpr->op);
      اكسر;
    }
  }
  ܐܢ( zBinOp ){
    sqlite3TreeViewLine(pView, "%s%s", zBinOp, zFlgs);
    sqlite3TreeViewExpr(pView, pExpr->pLeft, 1);
    sqlite3TreeViewExpr(pView, pExpr->pRight, 0);
  }ܐܚܪܢܐ ܐܢ( zUniOp ){
    sqlite3TreeViewLine(pView, "%s%s", zUniOp, zFlgs);
   sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
  }
  sqlite3TreeViewPop(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
فراغ sqlite3TreeViewBareExprList(
  TreeView *pView,
  ثابت ExprList *pList,
  ثابت حرف *zLabel
){
  ܐܢ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ܐܢ( pList==0 ){
    sqlite3TreeViewLine(pView, "%s (empty)", zLabel);
  }ܐܚܪܢܐ{
    صحيح i;
    sqlite3TreeViewLine(pView, "%s", zLabel);
    لكل(i=0; i<pList->nExpr; i++){
      صحيح j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      حرف *zName = pList->a[i].zEName;
      صحيح moreToFollow = i<pList->nExpr - 1;
      ܐܢ( j || zName || sortFlags ){
        sqlite3TreeViewPush(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3TreeViewLine(pView, 0);
        ܐܢ( zName ){
          بدل( pList->a[i].fg.eEName ){
            افتراضي:
              fprintf(stdout, "AS %s ", zName);
              اكسر;
            حالة ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              ܐܢ( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              ܐܢ( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              ܐܢ( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              اكسر;
            حالة ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              اكسر;
          }
        }
        ܐܢ( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        ܐܢ( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }ܐܚܪܢܐ ܐܢ( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3TreeViewExpr(pView, pList->a[i].pExpr, moreToFollow);
      ܐܢ( j || zName || sortFlags ){
        sqlite3TreeViewPop(&pView);
      }
    }
  }
}
فراغ sqlite3TreeViewExprList(
  TreeView *pView,
  ثابت ExprList *pList,
  u8 moreToFollow,
  ثابت حرف *zLabel
){
  sqlite3TreeViewPush(&pView, moreToFollow);
  sqlite3TreeViewBareExprList(pView, pList, zLabel);
  sqlite3TreeViewPop(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
فراغ sqlite3TreeViewBareIdList(
  TreeView *pView,
  ثابت IdList *pList,
  ثابت حرف *zLabel
){
  ܐܢ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ܐܢ( pList==0 ){
    sqlite3TreeViewLine(pView, "%s (empty)", zLabel);
  }ܐܚܪܢܐ{
    صحيح i;
    sqlite3TreeViewLine(pView, "%s", zLabel);
    لكل(i=0; i<pList->nId; i++){
      حرف *zName = pList->a[i].zName;
      صحيح moreToFollow = i<pList->nId - 1;
      ܐܢ( zName==0 ) zName = "(null)";
      sqlite3TreeViewPush(&pView, moreToFollow);
      sqlite3TreeViewLine(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3TreeViewPop(&pView);
    }
  }
}
فراغ sqlite3TreeViewIdList(
  TreeView *pView,
  ثابت IdList *pList,
  u8 moreToFollow,
  ثابت حرف *zLabel
){
  sqlite3TreeViewPush(&pView, moreToFollow);
  sqlite3TreeViewBareIdList(pView, pList, zLabel);
  sqlite3TreeViewPop(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
فراغ sqlite3TreeViewUpsert(
  TreeView *pView,
  ثابت Upsert *pUpsert,
  u8 moreToFollow
){
  ܐܢ( pUpsert==0 ) ܦܢܐ;
  sqlite3TreeViewPush(&pView, moreToFollow);
  ܟܡܐ( pUpsert ){
    صحيح n;
    sqlite3TreeViewPush(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3TreeViewLine(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3TreeViewExprList(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3TreeViewExprList(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    ܐܢ( pUpsert->pUpsertWhere ){
      sqlite3TreeViewItem(pView, "WHERE", (n--)>0);
      sqlite3TreeViewExpr(pView, pUpsert->pUpsertWhere, 0);
      sqlite3TreeViewPop(&pView);
    }
    sqlite3TreeViewPop(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3TreeViewPop(&pView);
}

#ܐܢ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
فراغ sqlite3TreeViewDelete(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت Expr *pWhere,
  ثابت ExprList *pOrderBy,
  ثابت Expr *pLimit,
  ثابت Trigger *pTrigger
){
  صحيح n = 0;
  TreeView *pView = 0;
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, "DELETE");
  ܐܢ( pWith ) n++;
  ܐܢ( pTabList ) n++;
  ܐܢ( pWhere ) n++;
  ܐܢ( pOrderBy ) n++;
  ܐܢ( pLimit ) n++;
  ܐܢ( pTrigger ) n++;
  ܐܢ( pWith ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewWith(pView, pWith, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pTabList ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "FROM");
    sqlite3TreeViewSrcList(pView, pTabList);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pWhere ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "WHERE");
    sqlite3TreeViewExpr(pView, pWhere, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pOrderBy ){
    sqlite3TreeViewExprList(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  ܐܢ( pLimit ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "LIMIT");
    sqlite3TreeViewExpr(pView, pLimit, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pTrigger ){
    sqlite3TreeViewTrigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3TreeViewPop(&pView);
}
#endif /* TREETRACE_ENABLED */

#ܐܢ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
فراغ sqlite3TreeViewInsert(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت IdList *pColumnList,
  ثابت Select *pSelect,
  ثابت ExprList *pExprList,
  صحيح onError,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  TreeView *pView = 0;
  صحيح n = 0;
  ثابت حرف *zLabel = "INSERT";
  بدل( onError ){
    حالة OE_Replace:  zLabel = "REPLACE";             اكسر;
    حالة OE_Ignore:   zLabel = "INSERT OR IGNORE";    اكسر;
    حالة OE_Rollback: zLabel = "INSERT OR ROLLBACK";  اكسر;
    حالة OE_Abort:    zLabel = "INSERT OR ABORT";     اكسر;
    حالة OE_Fail:     zLabel = "INSERT OR FAIL";      اكسر;
  }
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, zLabel);
  ܐܢ( pWith ) n++;
  ܐܢ( pTabList ) n++;
  ܐܢ( pColumnList ) n++;
  ܐܢ( pSelect ) n++;
  ܐܢ( pExprList ) n++;
  ܐܢ( pUpsert ) n++;
  ܐܢ( pTrigger ) n++;
  ܐܢ( pWith ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewWith(pView, pWith, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pTabList ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "INTO");
    sqlite3TreeViewSrcList(pView, pTabList);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pColumnList ){
    sqlite3TreeViewIdList(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  ܐܢ( pSelect ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "DATA-SOURCE");
    sqlite3TreeViewSelect(pView, pSelect, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pExprList ){
    sqlite3TreeViewExprList(pView, pExprList, (--n)>0, "VALUES");
  }
  ܐܢ( pUpsert ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "UPSERT");
    sqlite3TreeViewUpsert(pView, pUpsert, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pTrigger ){
    sqlite3TreeViewTrigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3TreeViewPop(&pView);
}
#endif /* TREETRACE_ENABLED */

#ܐܢ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
فراغ sqlite3TreeViewUpdate(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت ExprList *pChanges,
  ثابت Expr *pWhere,
  صحيح onError,
  ثابت ExprList *pOrderBy,
  ثابت Expr *pLimit,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  صحيح n = 0;
  TreeView *pView = 0;
  ثابت حرف *zLabel = "UPDATE";
  بدل( onError ){
    حالة OE_Replace:  zLabel = "UPDATE OR REPLACE";   اكسر;
    حالة OE_Ignore:   zLabel = "UPDATE OR IGNORE";    اكسر;
    حالة OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  اكسر;
    حالة OE_Abort:    zLabel = "UPDATE OR ABORT";     اكسر;
    حالة OE_Fail:     zLabel = "UPDATE OR FAIL";      اكسر;
  }
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, zLabel);
  ܐܢ( pWith ) n++;
  ܐܢ( pTabList ) n++;
  ܐܢ( pChanges ) n++;
  ܐܢ( pWhere ) n++;
  ܐܢ( pOrderBy ) n++;
  ܐܢ( pLimit ) n++;
  ܐܢ( pUpsert ) n++;
  ܐܢ( pTrigger ) n++;
  ܐܢ( pWith ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewWith(pView, pWith, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pTabList ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "FROM");
    sqlite3TreeViewSrcList(pView, pTabList);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pChanges ){
    sqlite3TreeViewExprList(pView, pChanges, (--n)>0, "SET");
  }
  ܐܢ( pWhere ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "WHERE");
    sqlite3TreeViewExpr(pView, pWhere, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pOrderBy ){
    sqlite3TreeViewExprList(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  ܐܢ( pLimit ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "LIMIT");
    sqlite3TreeViewExpr(pView, pLimit, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pUpsert ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "UPSERT");
    sqlite3TreeViewUpsert(pView, pUpsert, 0);
    sqlite3TreeViewPop(&pView);
  }
  ܐܢ( pTrigger ){
    sqlite3TreeViewTrigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3TreeViewPop(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
فراغ sqlite3TreeViewTriggerStep(
  TreeView *pView,
  ثابت TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  صحيح cnt = 0;
  ܐܢ( pStep==0 ) ܦܢܐ;
  sqlite3TreeViewPush(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  ܥܒܕ{
    ܐܢ( cnt++ && pStep->pNext==0 ){
      sqlite3TreeViewPop(&pView);
      sqlite3TreeViewPush(&pView, 0);
    }
    sqlite3TreeViewLine(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }ܟܡܐ( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3TreeViewPop(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
فراغ sqlite3TreeViewTrigger(
  TreeView *pView,
  ثابت Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  صحيح cnt = 0;
  ܐܢ( pTrigger==0 ) ܦܢܐ;
  sqlite3TreeViewPush(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  ܥܒܕ{
    ܐܢ( cnt++ && pTrigger->pNext==0 ){
      sqlite3TreeViewPop(&pView);
      sqlite3TreeViewPush(&pView, 0);
    }
    sqlite3TreeViewLine(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3TreeViewPush(&pView, 0);
    sqlite3TreeViewTriggerStep(pView, pTrigger->step_list, 0, 1);
    sqlite3TreeViewPop(&pView);
  }ܟܡܐ( showFullList && (pTrigger = pTrigger->pNext)!=0 );
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
فراغ sqlite3ShowExpr(ثابت Expr *p){ sqlite3TreeViewExpr(0,p,0); }
فراغ sqlite3ShowExprList(ثابت ExprList *p){ sqlite3TreeViewExprList(0,p,0,0);}
فراغ sqlite3ShowIdList(ثابت IdList *p){ sqlite3TreeViewIdList(0,p,0,0); }
فراغ sqlite3ShowSrcList(ثابت SrcList *p){
  TreeView *pView = 0;
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, "SRCLIST");
  sqlite3TreeViewSrcList(pView,p);
  sqlite3TreeViewPop(&pView);
}
فراغ sqlite3ShowSelect(ثابت Select *p){ sqlite3TreeViewSelect(0,p,0); }
فراغ sqlite3ShowWith(ثابت With *p){ sqlite3TreeViewWith(0,p,0); }
فراغ sqlite3ShowUpsert(ثابت Upsert *p){ sqlite3TreeViewUpsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
فراغ sqlite3ShowTriggerStep(ثابت TriggerStep *p){
  sqlite3TreeViewTriggerStep(0,p,0,0);
}
فراغ sqlite3ShowTriggerStepList(ثابت TriggerStep *p){
  sqlite3TreeViewTriggerStep(0,p,0,1);
}
فراغ sqlite3ShowTrigger(ثابت Trigger *p){ sqlite3TreeViewTrigger(0,p,0,0); }
فراغ sqlite3ShowTriggerList(ثابت Trigger *p){ sqlite3TreeViewTrigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
فراغ sqlite3ShowWindow(ثابت Window *p){ sqlite3TreeViewWindow(0,p,0); }
فراغ sqlite3ShowWinFunc(ثابت Window *p){ sqlite3TreeViewWinFunc(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
