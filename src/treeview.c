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
  nếu( p==0 ){
    *pp = p = sqlite3_malloc64( sizeof(*p) );
    nếu( p==0 ) trả_về;
    memset(p, 0, sizeof(*p));
  }không_thì{
    p->iLevel++;
  }
  assert( moreToFollow==0 || moreToFollow==1 );
  nếu( p->iLevel<(số_nguyên)sizeof(p->bLine) ) p->bLine[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
static void sqlite3TreeViewPop(TreeView **pp){
  TreeView *p = *pp;
  nếu( p==0 ) trả_về;
  p->iLevel--;
  nếu( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
void sqlite3TreeViewLine(TreeView *p, const char *zFormat, ...){
  va_list ap;
  số_nguyên i;
  StrAccum acc;
  char zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, sizeof(zBuf), 0);
  nếu( p ){
    cho(i=0; i<p->iLevel && i<(số_nguyên)sizeof(p->bLine)-1; i++){
      sqlite3_str_append(&acc, p->bLine[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->bLine[i] ? "|-- " : "'-- ", 4);
  }
  nếu( zFormat!=0 ){
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
static void sqlite3TreeViewItem(TreeView *p, const char *zLabel,u8 moreFollows){
  sqlite3TreeViewPush(&p, moreFollows);
  sqlite3TreeViewLine(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
void sqlite3TreeViewColumnList(
  TreeView *pView,
  const Column *aCol,
  số_nguyên nCol,
  u8 moreToFollow
){
  số_nguyên i;
  sqlite3TreeViewPush(&pView, moreToFollow);
  sqlite3TreeViewLine(pView, "COLUMNS");
  cho(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    số_nguyên colMoreToFollow = i<(nCol - 1);
    sqlite3TreeViewPush(&pView, colMoreToFollow);
    sqlite3TreeViewLine(pView, 0);
    printf(" %s", aCol[i].zCnName);
    switch( aCol[i].eCType ){
      case COLTYPE_ANY:      printf(" ANY");        break;
      case COLTYPE_BLOB:     printf(" BLOB");       break;
      case COLTYPE_INT:      printf(" INT");        break;
      case COLTYPE_INTEGER:  printf(" INTEGER");    break;
      case COLTYPE_REAL:     printf(" REAL");       break;
      case COLTYPE_TEXT:     printf(" TEXT");       break;
      case COLTYPE_CUSTOM: {
        nếu( flg & COLFLAG_HASTYPE ){
          const char *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          break;
        }
      }
    }
    nếu( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    nếu( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    nếu( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    nếu( flg ) printf(" flags=%04x", flg);
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
  số_nguyên i;
  nếu( pWith==0 ) trả_về;
  nếu( pWith->nCte==0 ) trả_về;
  nếu( pWith->pOuter ){
    sqlite3TreeViewLine(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }không_thì{
    sqlite3TreeViewLine(pView, "WITH (0x%p)", pWith);
  }
  nếu( pWith->nCte>0 ){
    sqlite3TreeViewPush(&pView, moreToFollow);
    cho(i=0; i<pWith->nCte; i++){
      StrAccum x;
      char zLine[1000];
      const struct Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, zLine, sizeof(zLine), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      nếu( pCte->pCols && pCte->pCols->nExpr>0 ){
        char cSep = '(';
        số_nguyên j;
        cho(j=0; j<pCte->pCols->nExpr; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      nếu( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      nếu( pCte->pUse ){
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
  số_nguyên i;
  nếu( pSrc==0 ) trả_về;
  cho(i=0; i<pSrc->nSrc; i++){
    const SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    số_nguyên n = 0;
    char zLine[1000];
    sqlite3StrAccumInit(&x, 0, zLine, sizeof(zLine), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    nếu( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    nếu( (pItem->fg.jointype & (JT_LEFT|JT_RIGHT))==(JT_LEFT|JT_RIGHT) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }không_thì nếu( pItem->fg.jointype & JT_LEFT ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }không_thì nếu( pItem->fg.jointype & JT_RIGHT ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }không_thì nếu( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    nếu( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    nếu( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    nếu( pItem->fg.isCte ){
      static const char *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    nếu( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    nếu( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    nếu( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    nếu( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    nếu( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    nếu( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    nếu( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    nếu( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    nếu( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    nếu( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3TreeViewItem(pView, zLine, i<pSrc->nSrc-1);
    n = 0;
    nếu( pItem->fg.isSubquery ) n++;
    nếu( pItem->fg.isTabFunc ) n++;
    nếu( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    nếu( pItem->fg.isUsing ){
      sqlite3TreeViewIdList(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }không_thì nếu( pItem->u3.pOn!=0 ){
      sqlite3TreeViewItem(pView, "ON", (--n)>0);
      sqlite3TreeViewExpr(pView, pItem->u3.pOn, 0);
      sqlite3TreeViewPop(&pView);
    }
    nếu( pItem->fg.isSubquery ){
      assert( n==1 );
      nếu( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3TreeViewColumnList(pView, pTab->aCol, pTab->nCol, 1);
      }
      assert( (số_nguyên)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3TreeViewSelect(pView, pItem->u4.pSubq->pSelect, 0);
    }
    nếu( pItem->fg.isTabFunc ){
      sqlite3TreeViewExprList(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3TreeViewPop(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
void sqlite3TreeViewSelect(TreeView *pView, const Select *p, u8 moreToFollow){
  số_nguyên n = 0;
  số_nguyên cnt = 0;
  nếu( p==0 ){
    sqlite3TreeViewLine(pView, "nil-SELECT");
    trả_về;
  } 
  sqlite3TreeViewPush(&pView, moreToFollow);
  nếu( p->pWith ){
    sqlite3TreeViewWith(pView, p->pWith, 1);
    cnt = 1;
    sqlite3TreeViewPush(&pView, 1);
  }
  do{
    nếu( p->selFlags & SF_WhereBegin ){
      sqlite3TreeViewLine(pView, "sqlite3WhereBegin()");
    }không_thì{
      sqlite3TreeViewLine(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (số_nguyên)p->nSelectRow
      );
    }
    nếu( cnt++ ) sqlite3TreeViewPop(&pView);
    nếu( p->pPrior ){
      n = 1000;
    }không_thì{
      n = 0;
      nếu( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      nếu( p->pWhere ) n++;
      nếu( p->pGroupBy ) n++;
      nếu( p->pHaving ) n++;
      nếu( p->pOrderBy ) n++;
      nếu( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      nếu( p->pWin ) n++;
      nếu( p->pWinDefn ) n++;
#endif
    }
    nếu( p->pEList ){
      sqlite3TreeViewExprList(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    nếu( p->pWin ){
      Window *pX;
      sqlite3TreeViewPush(&pView, (n--)>0);
      sqlite3TreeViewLine(pView, "window-functions");
      cho(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3TreeViewWinFunc(pView, pX, pX->pNextWin!=0);
      }
      sqlite3TreeViewPop(&pView);
    }
#endif
    nếu( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3TreeViewPush(&pView, (n--)>0);
      sqlite3TreeViewLine(pView, "FROM");
      sqlite3TreeViewSrcList(pView, p->pSrc);
      sqlite3TreeViewPop(&pView);
    }
    nếu( p->pWhere ){
      sqlite3TreeViewItem(pView, "WHERE", (n--)>0);
      sqlite3TreeViewExpr(pView, p->pWhere, 0);
      sqlite3TreeViewPop(&pView);
    }
    nếu( p->pGroupBy ){
      sqlite3TreeViewExprList(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    nếu( p->pHaving ){
      sqlite3TreeViewItem(pView, "HAVING", (n--)>0);
      sqlite3TreeViewExpr(pView, p->pHaving, 0);
      sqlite3TreeViewPop(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    nếu( p->pWinDefn ){
      Window *pX;
      sqlite3TreeViewItem(pView, "WINDOW", (n--)>0);
      cho(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3TreeViewWindow(pView, pX, pX->pNextWin!=0);
      }
      sqlite3TreeViewPop(&pView);
    }
#endif
    nếu( p->pOrderBy ){
      sqlite3TreeViewExprList(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    nếu( p->pLimit ){
      sqlite3TreeViewItem(pView, "LIMIT", (n--)>0);
      sqlite3TreeViewExpr(pView, p->pLimit->pLeft, p->pLimit->pRight!=0);
      nếu( p->pLimit->pRight ){
        sqlite3TreeViewItem(pView, "OFFSET", 0);
        sqlite3TreeViewExpr(pView, p->pLimit->pRight, 0);
        sqlite3TreeViewPop(&pView);
      }
      sqlite3TreeViewPop(&pView);
    }
    nếu( p->pPrior ){
      const char *zOp = "UNION";
      switch( p->op ){
        case TK_ALL:         zOp = "UNION ALL";  break;
        case TK_INTERSECT:   zOp = "INTERSECT";  break;
        case TK_EXCEPT:      zOp = "EXCEPT";     break;
      }
      sqlite3TreeViewItem(pView, zOp, 1);
    }
    p = p->pPrior;
  }trong_khi( p!=0 );
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
    case TK_UNBOUNDED: {
      sqlite3TreeViewItem(pView, "UNBOUNDED", moreToFollow);
      sqlite3TreeViewPop(&pView);
      break;
    }
    case TK_CURRENT: {
      sqlite3TreeViewItem(pView, "CURRENT", moreToFollow);
      sqlite3TreeViewPop(&pView);
      break;
    }
    case TK_PRECEDING: {
      sqlite3TreeViewItem(pView, "PRECEDING", moreToFollow);
      sqlite3TreeViewExpr(pView, pExpr, 0);
      sqlite3TreeViewPop(&pView);
      break;
    }
    case TK_FOLLOWING: {
      sqlite3TreeViewItem(pView, "FOLLOWING", moreToFollow);
      sqlite3TreeViewExpr(pView, pExpr, 0);
      sqlite3TreeViewPop(&pView);
      break;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
void sqlite3TreeViewWindow(TreeView *pView, const Window *pWin, u8 more){
  số_nguyên nElement = 0;
  nếu( pWin==0 ) trả_về;
  nếu( pWin->pFilter ){
    sqlite3TreeViewItem(pView, "FILTER", 1);
    sqlite3TreeViewExpr(pView, pWin->pFilter, 0);
    sqlite3TreeViewPop(&pView);
    nếu( pWin->eFrmType==TK_FILTER ) trả_về;
  }
  sqlite3TreeViewPush(&pView, more);
  nếu( pWin->zName ){
    sqlite3TreeViewLine(pView, "OVER %s (%p)", pWin->zName, pWin);
  }không_thì{
    sqlite3TreeViewLine(pView, "OVER (%p)", pWin);
  }
  nếu( pWin->zBase )    nElement++;
  nếu( pWin->pOrderBy ) nElement++;
  nếu( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  nếu( pWin->eExclude ) nElement++;
  nếu( pWin->zBase ){
    sqlite3TreeViewPush(&pView, (--nElement)>0);
    sqlite3TreeViewLine(pView, "window: %s", pWin->zBase);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pWin->pPartition ){
    sqlite3TreeViewExprList(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  nếu( pWin->pOrderBy ){
    sqlite3TreeViewExprList(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  nếu( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    char zBuf[30];
    const char *zFrmType = "ROWS";
    nếu( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    nếu( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(sizeof(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3TreeViewItem(pView, zBuf, (--nElement)>0);
    sqlite3TreeViewBound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3TreeViewBound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pWin->eExclude ){
    char zBuf[30];
    const char *zExclude;
    switch( pWin->eExclude ){
      case TK_NO:      zExclude = "NO OTHERS";   break;
      case TK_CURRENT: zExclude = "CURRENT ROW"; break;
      case TK_GROUP:   zExclude = "GROUP";       break;
      case TK_TIES:    zExclude = "TIES";        break;
      default:
        sqlite3_snprintf(sizeof(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        break;
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
  nếu( pWin==0 ) trả_về;
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
  const char *zBinOp = 0;   /* Binary operator */
  const char *zUniOp = 0;   /* Unary operator */
  char zFlgs[200];
  sqlite3TreeViewPush(&pView, moreToFollow);
  nếu( pExpr==0 ){
    sqlite3TreeViewLine(pView, "nil");
    sqlite3TreeViewPop(&pView);
    trả_về;
  }
  nếu( pExpr->flags || pExpr->affExpr || pExpr->vvaFlags || pExpr->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, sizeof(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      pExpr->flags, pExpr->affExpr ? pExpr->affExpr : 'n');
    nếu( ExprHasProperty(pExpr, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", pExpr->w.iJoin);
    }
    nếu( ExprHasProperty(pExpr, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", pExpr->w.iJoin);
    }
    nếu( ExprHasProperty(pExpr, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    nếu( ExprHasVVAProperty(pExpr, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    nếu( pExpr->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", pExpr->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }không_thì{
    zFlgs[0] = 0;
  }
  switch( pExpr->op ){
    case TK_AGG_COLUMN: {
      sqlite3TreeViewLine(pView, "AGG{%d:%d}%s",
            pExpr->iTable, pExpr->iColumn, zFlgs);
      break;
    }
    case TK_COLUMN: {
      nếu( pExpr->iTable<0 ){
        /* This only happens when coding check constraints */
        char zOp2[16];
        nếu( pExpr->op2 ){
          sqlite3_snprintf(sizeof(zOp2),zOp2," op2=0x%02x",pExpr->op2);
        }không_thì{
          zOp2[0] = 0;
        }
        sqlite3TreeViewLine(pView, "COLUMN(%d)%s%s",
                                    pExpr->iColumn, zFlgs, zOp2);
      }không_thì{
        assert( ExprUseYTab(pExpr) );
        sqlite3TreeViewLine(pView, "{%d:%d} pTab=%p%s",
                        pExpr->iTable, pExpr->iColumn,
                        pExpr->y.pTab, zFlgs);
      }
      nếu( ExprHasProperty(pExpr, EP_FixedCol) ){
        sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      }
      break;
    }
    case TK_INTEGER: {
      nếu( pExpr->flags & EP_IntValue ){
        sqlite3TreeViewLine(pView, "%d", pExpr->u.iValue);
      }không_thì{
        sqlite3TreeViewLine(pView, "%s", pExpr->u.zToken);
      }
      break;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    case TK_FLOAT: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"%s", pExpr->u.zToken);
      break;
    }
#endif
    case TK_STRING: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"%Q", pExpr->u.zToken);
      break;
    }
    case TK_NULL: {
      sqlite3TreeViewLine(pView,"NULL");
      break;
    }
    case TK_TRUEFALSE: {
      sqlite3TreeViewLine(pView,"%s%s",
         sqlite3ExprTruthValue(pExpr) ? "TRUE" : "FALSE", zFlgs);
      break;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    case TK_BLOB: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"%s", pExpr->u.zToken);
      break;
    }
#endif
    case TK_VARIABLE: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"VARIABLE(%s,%d)",
                          pExpr->u.zToken, pExpr->iColumn);
      break;
    }
    case TK_REGISTER: {
      sqlite3TreeViewLine(pView,"REGISTER(%d)", pExpr->iTable);
      break;
    }
    case TK_ID: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"ID \"%w\"", pExpr->u.zToken);
      break;
    }
    case TK_ASTERISK: {
      sqlite3TreeViewLine(pView,"ASKTERISK");
      break;
    }
#ifndef SQLITE_OMIT_CAST
    case TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView,"CAST %Q", pExpr->u.zToken);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      break;
    }
#endif /* SQLITE_OMIT_CAST */
    case TK_LT:      zBinOp = "LT";     break;
    case TK_LE:      zBinOp = "LE";     break;
    case TK_GT:      zBinOp = "GT";     break;
    case TK_GE:      zBinOp = "GE";     break;
    case TK_NE:      zBinOp = "NE";     break;
    case TK_EQ:      zBinOp = "EQ";     break;
    case TK_IS:      zBinOp = "IS";     break;
    case TK_ISNOT:   zBinOp = "ISNOT";  break;
    case TK_AND:     zBinOp = "AND";    break;
    case TK_OR:      zBinOp = "OR";     break;
    case TK_PLUS:    zBinOp = "ADD";    break;
    case TK_STAR:    zBinOp = "MUL";    break;
    case TK_MINUS:   zBinOp = "SUB";    break;
    case TK_REM:     zBinOp = "REM";    break;
    case TK_BITAND:  zBinOp = "BITAND"; break;
    case TK_BITOR:   zBinOp = "BITOR";  break;
    case TK_SLASH:   zBinOp = "DIV";    break;
    case TK_LSHIFT:  zBinOp = "LSHIFT"; break;
    case TK_RSHIFT:  zBinOp = "RSHIFT"; break;
    case TK_CONCAT:  zBinOp = "CONCAT"; break;
    case TK_DOT:     zBinOp = "DOT";    break;
    case TK_LIMIT:   zBinOp = "LIMIT";  break;

    case TK_UMINUS:  zUniOp = "UMINUS"; break;
    case TK_UPLUS:   zUniOp = "UPLUS";  break;
    case TK_BITNOT:  zUniOp = "BITNOT"; break;
    case TK_NOT:     zUniOp = "NOT";    break;
    case TK_ISNULL:  zUniOp = "ISNULL"; break;
    case TK_NOTNULL: zUniOp = "NOTNULL"; break;

    case TK_TRUTH: {
      số_nguyên x;
      const char *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      assert( pExpr->op2==TK_IS || pExpr->op2==TK_ISNOT );
      assert( pExpr->pRight );
      assert( sqlite3ExprSkipCollateAndLikely(pExpr->pRight)->op
                  == TK_TRUEFALSE );
      x = (pExpr->op2==TK_ISNOT)*2 + sqlite3ExprTruthValue(pExpr->pRight);
      zUniOp = azOp[x];
      break;
    }

    case TK_SPAN: {
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView, "SPAN %Q", pExpr->u.zToken);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      break;
    }

    case TK_COLLATE: {
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
      break;
    }

    case TK_AGG_FUNCTION:
    case TK_FUNCTION: {
      ExprList *pFarg;       /* List of function arguments */
      Window *pWin;
      nếu( ExprHasProperty(pExpr, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }không_thì{
        assert( ExprUseXList(pExpr) );
        pFarg = pExpr->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(pExpr) ? pExpr->y.pWin : 0;
#không_thì
        pWin = 0;
#endif 
      }
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      nếu( pExpr->op==TK_AGG_FUNCTION ){
        sqlite3TreeViewLine(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             pExpr->op2, pExpr->u.zToken, zFlgs,
                             pExpr->pAggInfo ? pExpr->pAggInfo->selId : 0,
                             pExpr->iAgg, pExpr->pAggInfo);
      }không_thì nếu( pExpr->op2!=0 ){
        const char *zOp2;
        char zBuf[8];
        sqlite3_snprintf(sizeof(zBuf),zBuf,"0x%02x",pExpr->op2);
        zOp2 = zBuf;
        nếu( pExpr->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        nếu( pExpr->op2==NC_IdxExpr ) zOp2 = "NC_IdxExpr";
        nếu( pExpr->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        nếu( pExpr->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3TreeViewLine(pView, "FUNCTION %Q%s op2=%s",
                            pExpr->u.zToken, zFlgs, zOp2);
      }không_thì{
        sqlite3TreeViewLine(pView, "FUNCTION %Q%s", pExpr->u.zToken, zFlgs);
      }
      nếu( pFarg ){
        sqlite3TreeViewExprList(pView, pFarg, pWin!=0 || pExpr->pLeft, 0);
        nếu( pExpr->pLeft ){
          Expr *pOB = pExpr->pLeft;
          assert( pOB->op==TK_ORDER );
          assert( ExprUseXList(pOB) );
          sqlite3TreeViewExprList(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      nếu( pWin ){
        sqlite3TreeViewWindow(pView, pWin, 0);
      }
#endif
      break;
    }
    case TK_ORDER: {
      sqlite3TreeViewExprList(pView, pExpr->x.pList, 0, "ORDERBY");
      break;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_EXISTS: {
      assert( ExprUseXSelect(pExpr) );
      sqlite3TreeViewLine(pView, "EXISTS-expr flags=0x%x", pExpr->flags);
      sqlite3TreeViewSelect(pView, pExpr->x.pSelect, 0);
      break;
    }
    case TK_SELECT: {
      assert( ExprUseXSelect(pExpr) );
      sqlite3TreeViewLine(pView, "subquery-expr flags=0x%x", pExpr->flags);
      sqlite3TreeViewSelect(pView, pExpr->x.pSelect, 0);
      break;
    }
    case TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      char *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", pExpr->flags);
      nếu( pExpr->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",pExpr->iTable);
      nếu( ExprHasProperty(pExpr, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            pExpr->y.sub.regReturn, pExpr->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3TreeViewLine(pView, z);
      sqlite3_free(z);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 1);
      nếu( ExprUseXSelect(pExpr) ){
        sqlite3TreeViewSelect(pView, pExpr->x.pSelect, 0);
      }không_thì{
        sqlite3TreeViewExprList(pView, pExpr->x.pList, 0, 0);
      }
      break;
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
    case TK_BETWEEN: {
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
      break;
    }
    case TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3TreeViewLine(pView, "%s(%d)", 
          pExpr->iTable ? "NEW" : "OLD", pExpr->iColumn);
      break;
    }
    case TK_CASE: {
      sqlite3TreeViewLine(pView, "CASE");
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 1);
      assert( ExprUseXList(pExpr) );
      sqlite3TreeViewExprList(pView, pExpr->x.pList, 0, 0);
      break;
    }
#ifndef SQLITE_OMIT_TRIGGER
    case TK_RAISE: {
      const char *zType = "unk";
      switch( pExpr->affExpr ){
        case OE_Rollback:   zType = "rollback";  break;
        case OE_Abort:      zType = "abort";     break;
        case OE_Fail:       zType = "fail";      break;
        case OE_Ignore:     zType = "ignore";    break;
      }
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      sqlite3TreeViewLine(pView, "RAISE %s", zType);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      break;
    }
#endif
    case TK_MATCH: {
      sqlite3TreeViewLine(pView, "MATCH {%d:%d}%s",
                          pExpr->iTable, pExpr->iColumn, zFlgs);
      sqlite3TreeViewExpr(pView, pExpr->pRight, 0);
      break;
    }
    case TK_VECTOR: {
      char *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      assert( ExprUseXList(pExpr) );
      sqlite3TreeViewBareExprList(pView, pExpr->x.pList, z);
      sqlite3_free(z);
      break;
    }
    case TK_SELECT_COLUMN: {
      sqlite3TreeViewLine(pView, "SELECT-COLUMN %d of [0..%d]%s",
              pExpr->iColumn, pExpr->iTable-1,
              pExpr->pRight==pExpr->pLeft ? " (SELECT-owner)" : "");
      assert( ExprUseXSelect(pExpr->pLeft) );
      sqlite3TreeViewSelect(pView, pExpr->pLeft->x.pSelect, 0);
      break;
    }
    case TK_IF_NULL_ROW: {
      sqlite3TreeViewLine(pView, "IF-NULL-ROW %d", pExpr->iTable);
      sqlite3TreeViewExpr(pView, pExpr->pLeft, 0);
      break;
    }
    case TK_ERROR: {
      Expr tmp;
      sqlite3TreeViewLine(pView, "ERROR");
      tmp = *pExpr;
      tmp.op = pExpr->op2;
      sqlite3TreeViewExpr(pView, &tmp, 0);
      break;
    }
    case TK_ROW: {
      nếu( pExpr->iColumn<=0 ){
        sqlite3TreeViewLine(pView, "First FROM table rowid");
      }không_thì{
        sqlite3TreeViewLine(pView, "First FROM table column %d",
            pExpr->iColumn-1);
      }
      break;
    }
    default: {
      sqlite3TreeViewLine(pView, "op=%d", pExpr->op);
      break;
    }
  }
  nếu( zBinOp ){
    sqlite3TreeViewLine(pView, "%s%s", zBinOp, zFlgs);
    sqlite3TreeViewExpr(pView, pExpr->pLeft, 1);
    sqlite3TreeViewExpr(pView, pExpr->pRight, 0);
  }không_thì nếu( zUniOp ){
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
  const char *zLabel
){
  nếu( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  nếu( pList==0 ){
    sqlite3TreeViewLine(pView, "%s (empty)", zLabel);
  }không_thì{
    số_nguyên i;
    sqlite3TreeViewLine(pView, "%s", zLabel);
    cho(i=0; i<pList->nExpr; i++){
      số_nguyên j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      char *zName = pList->a[i].zEName;
      số_nguyên moreToFollow = i<pList->nExpr - 1;
      nếu( j || zName || sortFlags ){
        sqlite3TreeViewPush(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3TreeViewLine(pView, 0);
        nếu( zName ){
          switch( pList->a[i].fg.eEName ){
            default:
              fprintf(stdout, "AS %s ", zName);
              break;
            case ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              nếu( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              nếu( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              nếu( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              break;
            case ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              break;
          }
        }
        nếu( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        nếu( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }không_thì nếu( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3TreeViewExpr(pView, pList->a[i].pExpr, moreToFollow);
      nếu( j || zName || sortFlags ){
        sqlite3TreeViewPop(&pView);
      }
    }
  }
}
void sqlite3TreeViewExprList(
  TreeView *pView,
  const ExprList *pList,
  u8 moreToFollow,
  const char *zLabel
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
  const char *zLabel
){
  nếu( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  nếu( pList==0 ){
    sqlite3TreeViewLine(pView, "%s (empty)", zLabel);
  }không_thì{
    số_nguyên i;
    sqlite3TreeViewLine(pView, "%s", zLabel);
    cho(i=0; i<pList->nId; i++){
      char *zName = pList->a[i].zName;
      số_nguyên moreToFollow = i<pList->nId - 1;
      nếu( zName==0 ) zName = "(null)";
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
  const char *zLabel
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
  nếu( pUpsert==0 ) trả_về;
  sqlite3TreeViewPush(&pView, moreToFollow);
  trong_khi( pUpsert ){
    số_nguyên n;
    sqlite3TreeViewPush(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3TreeViewLine(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3TreeViewExprList(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3TreeViewExprList(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    nếu( pUpsert->pUpsertWhere ){
      sqlite3TreeViewItem(pView, "WHERE", (n--)>0);
      sqlite3TreeViewExpr(pView, pUpsert->pUpsertWhere, 0);
      sqlite3TreeViewPop(&pView);
    }
    sqlite3TreeViewPop(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3TreeViewPop(&pView);
}

#nếu TREETRACE_ENABLED
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
  số_nguyên n = 0;
  TreeView *pView = 0;
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, "DELETE");
  nếu( pWith ) n++;
  nếu( pTabList ) n++;
  nếu( pWhere ) n++;
  nếu( pOrderBy ) n++;
  nếu( pLimit ) n++;
  nếu( pTrigger ) n++;
  nếu( pWith ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewWith(pView, pWith, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pTabList ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "FROM");
    sqlite3TreeViewSrcList(pView, pTabList);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pWhere ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "WHERE");
    sqlite3TreeViewExpr(pView, pWhere, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pOrderBy ){
    sqlite3TreeViewExprList(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  nếu( pLimit ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "LIMIT");
    sqlite3TreeViewExpr(pView, pLimit, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pTrigger ){
    sqlite3TreeViewTrigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3TreeViewPop(&pView);
}
#endif /* TREETRACE_ENABLED */

#nếu TREETRACE_ENABLED
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
  số_nguyên onError,
  const Upsert *pUpsert,
  const Trigger *pTrigger
){
  TreeView *pView = 0;
  số_nguyên n = 0;
  const char *zLabel = "INSERT";
  switch( onError ){
    case OE_Replace:  zLabel = "REPLACE";             break;
    case OE_Ignore:   zLabel = "INSERT OR IGNORE";    break;
    case OE_Rollback: zLabel = "INSERT OR ROLLBACK";  break;
    case OE_Abort:    zLabel = "INSERT OR ABORT";     break;
    case OE_Fail:     zLabel = "INSERT OR FAIL";      break;
  }
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, zLabel);
  nếu( pWith ) n++;
  nếu( pTabList ) n++;
  nếu( pColumnList ) n++;
  nếu( pSelect ) n++;
  nếu( pExprList ) n++;
  nếu( pUpsert ) n++;
  nếu( pTrigger ) n++;
  nếu( pWith ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewWith(pView, pWith, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pTabList ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "INTO");
    sqlite3TreeViewSrcList(pView, pTabList);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pColumnList ){
    sqlite3TreeViewIdList(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  nếu( pSelect ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "DATA-SOURCE");
    sqlite3TreeViewSelect(pView, pSelect, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pExprList ){
    sqlite3TreeViewExprList(pView, pExprList, (--n)>0, "VALUES");
  }
  nếu( pUpsert ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "UPSERT");
    sqlite3TreeViewUpsert(pView, pUpsert, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pTrigger ){
    sqlite3TreeViewTrigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3TreeViewPop(&pView);
}
#endif /* TREETRACE_ENABLED */

#nếu TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
void sqlite3TreeViewUpdate(
  const With *pWith,
  const SrcList *pTabList,
  const ExprList *pChanges,
  const Expr *pWhere,
  số_nguyên onError,
  const ExprList *pOrderBy,
  const Expr *pLimit,
  const Upsert *pUpsert,
  const Trigger *pTrigger
){
  số_nguyên n = 0;
  TreeView *pView = 0;
  const char *zLabel = "UPDATE";
  switch( onError ){
    case OE_Replace:  zLabel = "UPDATE OR REPLACE";   break;
    case OE_Ignore:   zLabel = "UPDATE OR IGNORE";    break;
    case OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  break;
    case OE_Abort:    zLabel = "UPDATE OR ABORT";     break;
    case OE_Fail:     zLabel = "UPDATE OR FAIL";      break;
  }
  sqlite3TreeViewPush(&pView, 0);
  sqlite3TreeViewLine(pView, zLabel);
  nếu( pWith ) n++;
  nếu( pTabList ) n++;
  nếu( pChanges ) n++;
  nếu( pWhere ) n++;
  nếu( pOrderBy ) n++;
  nếu( pLimit ) n++;
  nếu( pUpsert ) n++;
  nếu( pTrigger ) n++;
  nếu( pWith ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewWith(pView, pWith, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pTabList ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "FROM");
    sqlite3TreeViewSrcList(pView, pTabList);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pChanges ){
    sqlite3TreeViewExprList(pView, pChanges, (--n)>0, "SET");
  }
  nếu( pWhere ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "WHERE");
    sqlite3TreeViewExpr(pView, pWhere, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pOrderBy ){
    sqlite3TreeViewExprList(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  nếu( pLimit ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "LIMIT");
    sqlite3TreeViewExpr(pView, pLimit, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pUpsert ){
    sqlite3TreeViewPush(&pView, (--n)>0);
    sqlite3TreeViewLine(pView, "UPSERT");
    sqlite3TreeViewUpsert(pView, pUpsert, 0);
    sqlite3TreeViewPop(&pView);
  }
  nếu( pTrigger ){
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
  số_nguyên cnt = 0;
  nếu( pStep==0 ) trả_về;
  sqlite3TreeViewPush(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  do{
    nếu( cnt++ && pStep->pNext==0 ){
      sqlite3TreeViewPop(&pView);
      sqlite3TreeViewPush(&pView, 0);
    }
    sqlite3TreeViewLine(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }trong_khi( showFullList && (pStep = pStep->pNext)!=0 );
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
  số_nguyên cnt = 0;
  nếu( pTrigger==0 ) trả_về;
  sqlite3TreeViewPush(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  do{
    nếu( cnt++ && pTrigger->pNext==0 ){
      sqlite3TreeViewPop(&pView);
      sqlite3TreeViewPush(&pView, 0);
    }
    sqlite3TreeViewLine(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3TreeViewPush(&pView, 0);
    sqlite3TreeViewTriggerStep(pView, pTrigger->step_list, 0, 1);
    sqlite3TreeViewPop(&pView);
  }trong_khi( showFullList && (pTrigger = pTrigger->pNext)!=0 );
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
