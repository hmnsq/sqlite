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
static void sqlite3_पुनो_View_इतुलाक(पुनो_View **pp, u8 moreToFollow){
  पुनो_View *p = *pp;
  if( p==0 ){
    *pp = p = sqlite3_malloc64( sizeof(*p) );
    if( p==0 ) return;
    memset(p, 0, sizeof(*p));
  }else{
    p->iLevel++;
  }
  assert( moreToFollow==0 || moreToFollow==1 );
  if( p->iLevel<(int)sizeof(p->b_लिन्या) ) p->b_लिन्या[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
static void sqlite3_पुनो_View_आलिसिन(पुनो_View **pp){
  पुनो_View *p = *pp;
  if( p==0 ) return;
  p->iLevel--;
  if( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
void sqlite3_पुनो_View_लिन्या(पुनो_View *p, const char *zFormat, ...){
  va_list ap;
  int i;
  StrAccum acc;
  char zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, sizeof(zBuf), 0);
  if( p ){
    for(i=0; i<p->iLevel && i<(int)sizeof(p->b_लिन्या)-1; i++){
      sqlite3_str_append(&acc, p->b_लिन्या[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_लिन्या[i] ? "|-- " : "'-- ", 4);
  }
  if( zFormat!=0 ){
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
static void sqlite3_पुनो_View_Item(पुनो_View *p, const char *zLabel,u8 moreFollows){
  sqlite3_पुनो_View_इतुलाक(&p, moreFollows);
  sqlite3_पुनो_View_लिन्या(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
void sqlite3_पुनो_View_Column_List(
  पुनो_View *pView,
  const Column *aCol,
  int nCol,
  u8 moreToFollow
){
  int i;
  sqlite3_पुनो_View_इतुलाक(&pView, moreToFollow);
  sqlite3_पुनो_View_लिन्या(pView, "COLUMNS");
  for(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    int colMoreToFollow = i<(nCol - 1);
    sqlite3_पुनो_View_इतुलाक(&pView, colMoreToFollow);
    sqlite3_पुनो_View_लिन्या(pView, 0);
    printf(" %s", aCol[i].zCnName);
    switch( aCol[i].eCType ){
      case COLTYPE_ANY:      printf(" ANY");        break;
      case COLTYPE_BLOB:     printf(" BLOB");       break;
      case COLTYPE_INT:      printf(" INT");        break;
      case COLTYPE_INTEGER:  printf(" INTEGER");    break;
      case COLTYPE_REAL:     printf(" REAL");       break;
      case COLTYPE_TEXT:     printf(" TEXT");       break;
      case COLTYPE_CUSTOM: {
        if( flg & COLFLAG_HASTYPE ){
          const char *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          break;
        }
      }
    }
    if( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    if( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    if( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    if( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  sqlite3_पुनो_View_आलिसिन(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
void sqlite3_पुनो_View_With(पुनो_View *pView, const With *pWith, u8 moreToFollow){
  int i;
  if( pWith==0 ) return;
  if( pWith->nCte==0 ) return;
  if( pWith->pOuter ){
    sqlite3_पुनो_View_लिन्या(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }else{
    sqlite3_पुनो_View_लिन्या(pView, "WITH (0x%p)", pWith);
  }
  if( pWith->nCte>0 ){
    sqlite3_पुनो_View_इतुलाक(&pView, moreToFollow);
    for(i=0; i<pWith->nCte; i++){
      StrAccum x;
      char z_लिन्या[1000];
      const struct Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_लिन्या, sizeof(z_लिन्या), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      if( pCte->pCols && pCte->pCols->n_पाहायाग>0 ){
        char cSep = '(';
        int j;
        for(j=0; j<pCte->pCols->n_पाहायाग; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      if( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      if( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_पुनो_View_Item(pView, z_लिन्या, i<pWith->nCte-1);
      sqlite3_पुनो_View_Select(pView, pCte->pSelect, 0);
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
void sqlite3_पुनो_View_Src_List(पुनो_View *pView, const SrcList *pSrc){
  int i;
  if( pSrc==0 ) return;
  for(i=0; i<pSrc->nSrc; i++){
    const SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    int n = 0;
    char z_लिन्या[1000];
    sqlite3StrAccumInit(&x, 0, z_लिन्या, sizeof(z_लिन्या), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    if( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    if( (pItem->fg.jointype & (JT_कालिवा|JT_कानान))==(JT_कालिवा|JT_कानान) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }else if( pItem->fg.jointype & JT_कालिवा ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }else if( pItem->fg.jointype & JT_कानान ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }else if( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    if( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    if( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    if( pItem->fg.isCte ){
      static const char *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    if( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    if( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    if( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    if( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    if( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    if( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    if( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    if( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    if( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    if( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_पुनो_View_Item(pView, z_लिन्या, i<pSrc->nSrc-1);
    n = 0;
    if( pItem->fg.isSubquery ) n++;
    if( pItem->fg.isTabFunc ) n++;
    if( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    if( pItem->fg.isUsing ){
      sqlite3_पुनो_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }else if( pItem->u3.pOn!=0 ){
      sqlite3_पुनो_View_Item(pView, "ON", (--n)>0);
      sqlite3_पुनो_View_पाहायाग(pView, pItem->u3.pOn, 0);
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
    if( pItem->fg.isSubquery ){
      assert( n==1 );
      if( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_पुनो_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      assert( (int)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_पुनो_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    if( pItem->fg.isTabFunc ){
      sqlite3_पुनो_View_पाहायाग_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
void sqlite3_पुनो_View_Select(पुनो_View *pView, const Select *p, u8 moreToFollow){
  int n = 0;
  int cnt = 0;
  if( p==0 ){
    sqlite3_पुनो_View_लिन्या(pView, "nil-SELECT");
    return;
  } 
  sqlite3_पुनो_View_इतुलाक(&pView, moreToFollow);
  if( p->pWith ){
    sqlite3_पुनो_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_पुनो_View_इतुलाक(&pView, 1);
  }
  do{
    if( p->selFlags & SF_WhereBegin ){
      sqlite3_पुनो_View_लिन्या(pView, "sqlite3WhereBegin()");
    }else{
      sqlite3_पुनो_View_लिन्या(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (int)p->nSelectRow
      );
    }
    if( cnt++ ) sqlite3_पुनो_View_आलिसिन(&pView);
    if( p->pPrior ){
      n = 1000;
    }else{
      n = 0;
      if( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      if( p->pWhere ) n++;
      if( p->pGroupBy ) n++;
      if( p->pHaving ) n++;
      if( p->pOrderBy ) n++;
      if( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      if( p->pWin ) n++;
      if( p->pWinDefn ) n++;
#endif
    }
    if( p->pEList ){
      sqlite3_पुनो_View_पाहायाग_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    if( p->pWin ){
      Window *pX;
      sqlite3_पुनो_View_इतुलाक(&pView, (n--)>0);
      sqlite3_पुनो_View_लिन्या(pView, "window-functions");
      for(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_पुनो_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
#endif
    if( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_पुनो_View_इतुलाक(&pView, (n--)>0);
      sqlite3_पुनो_View_लिन्या(pView, "FROM");
      sqlite3_पुनो_View_Src_List(pView, p->pSrc);
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
    if( p->pWhere ){
      sqlite3_पुनो_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_पुनो_View_पाहायाग(pView, p->pWhere, 0);
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
    if( p->pGroupBy ){
      sqlite3_पुनो_View_पाहायाग_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    if( p->pHaving ){
      sqlite3_पुनो_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_पुनो_View_पाहायाग(pView, p->pHaving, 0);
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    if( p->pWinDefn ){
      Window *pX;
      sqlite3_पुनो_View_Item(pView, "WINDOW", (n--)>0);
      for(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_पुनो_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
#endif
    if( p->pOrderBy ){
      sqlite3_पुनो_View_पाहायाग_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    if( p->pLimit ){
      sqlite3_पुनो_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_पुनो_View_पाहायाग(pView, p->pLimit->p_कालिवा, p->pLimit->p_कानान!=0);
      if( p->pLimit->p_कानान ){
        sqlite3_पुनो_View_Item(pView, "OFFSET", 0);
        sqlite3_पुनो_View_पाहायाग(pView, p->pLimit->p_कानान, 0);
        sqlite3_पुनो_View_आलिसिन(&pView);
      }
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
    if( p->pPrior ){
      const char *zOp = "UNION";
      switch( p->op ){
        case TK_ALL:         zOp = "UNION ALL";  break;
        case TK_INTERSECT:   zOp = "INTERSECT";  break;
        case TK_EXCEPT:      zOp = "EXCEPT";     break;
      }
      sqlite3_पुनो_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }while( p!=0 );
  sqlite3_पुनो_View_आलिसिन(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
void sqlite3_पुनो_View_Bound(
  पुनो_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  पाहायाग *p_पाहायाग,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  switch( eBound ){
    case TK_UNBOUNDED: {
      sqlite3_पुनो_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_पुनो_View_आलिसिन(&pView);
      break;
    }
    case TK_CURRENT: {
      sqlite3_पुनो_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_पुनो_View_आलिसिन(&pView);
      break;
    }
    case TK_PRECEDING: {
      sqlite3_पुनो_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग, 0);
      sqlite3_पुनो_View_आलिसिन(&pView);
      break;
    }
    case TK_FOLLOWING: {
      sqlite3_पुनो_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग, 0);
      sqlite3_पुनो_View_आलिसिन(&pView);
      break;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
void sqlite3_पुनो_View_Window(पुनो_View *pView, const Window *pWin, u8 more){
  int nElement = 0;
  if( pWin==0 ) return;
  if( pWin->pFilter ){
    sqlite3_पुनो_View_Item(pView, "FILTER", 1);
    sqlite3_पुनो_View_पाहायाग(pView, pWin->pFilter, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
    if( pWin->eFrmType==TK_FILTER ) return;
  }
  sqlite3_पुनो_View_इतुलाक(&pView, more);
  if( pWin->zName ){
    sqlite3_पुनो_View_लिन्या(pView, "OVER %s (%p)", pWin->zName, pWin);
  }else{
    sqlite3_पुनो_View_लिन्या(pView, "OVER (%p)", pWin);
  }
  if( pWin->zBase )    nElement++;
  if( pWin->pOrderBy ) nElement++;
  if( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  if( pWin->eExclude ) nElement++;
  if( pWin->zBase ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--nElement)>0);
    sqlite3_पुनो_View_लिन्या(pView, "window: %s", pWin->zBase);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pWin->pPartition ){
    sqlite3_पुनो_View_पाहायाग_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  if( pWin->pOrderBy ){
    sqlite3_पुनो_View_पाहायाग_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  if( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    char zBuf[30];
    const char *zFrmType = "ROWS";
    if( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    if( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(sizeof(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_पुनो_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_पुनो_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_पुनो_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pWin->eExclude ){
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
    sqlite3_पुनो_View_इतुलाक(&pView, 0);
    sqlite3_पुनो_View_लिन्या(pView, "EXCLUDE %s", zExclude);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  sqlite3_पुनो_View_आलिसिन(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
void sqlite3_पुनो_View_Win_Func(पुनो_View *pView, const Window *pWin, u8 more){
  if( pWin==0 ) return;
  sqlite3_पुनो_View_इतुलाक(&pView, more);
  sqlite3_पुनो_View_लिन्या(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_पुनो_View_Window(pView, pWin, 0);
  sqlite3_पुनो_View_आलिसिन(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
void sqlite3_पुनो_View_पाहायाग(पुनो_View *pView, const पाहायाग *p_पाहायाग, u8 moreToFollow){
  const char *zBinOp = 0;   /* Binary operator */
  const char *zUniOp = 0;   /* Unary operator */
  char zFlgs[200];
  sqlite3_पुनो_View_इतुलाक(&pView, moreToFollow);
  if( p_पाहायाग==0 ){
    sqlite3_पुनो_View_लिन्या(pView, "nil");
    sqlite3_पुनो_View_आलिसिन(&pView);
    return;
  }
  if( p_पाहायाग->flags || p_पाहायाग->aff_पाहायाग || p_पाहायाग->vvaFlags || p_पाहायाग->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, sizeof(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_पाहायाग->flags, p_पाहायाग->aff_पाहायाग ? p_पाहायाग->aff_पाहायाग : 'n');
    if( पाहायाग_Has_Property(p_पाहायाग, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_पाहायाग->w.iJoin);
    }
    if( पाहायाग_Has_Property(p_पाहायाग, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_पाहायाग->w.iJoin);
    }
    if( पाहायाग_Has_Property(p_पाहायाग, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    if( पाहायाग_Has_VVAProperty(p_पाहायाग, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    if( p_पाहायाग->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_पाहायाग->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }else{
    zFlgs[0] = 0;
  }
  switch( p_पाहायाग->op ){
    case TK_AGG_COLUMN: {
      sqlite3_पुनो_View_लिन्या(pView, "AGG{%d:%d}%s",
            p_पाहायाग->iTable, p_पाहायाग->iColumn, zFlgs);
      break;
    }
    case TK_COLUMN: {
      if( p_पाहायाग->iTable<0 ){
        /* This only happens when coding check constraints */
        char zOp2[16];
        if( p_पाहायाग->op2 ){
          sqlite3_snprintf(sizeof(zOp2),zOp2," op2=0x%02x",p_पाहायाग->op2);
        }else{
          zOp2[0] = 0;
        }
        sqlite3_पुनो_View_लिन्या(pView, "COLUMN(%d)%s%s",
                                    p_पाहायाग->iColumn, zFlgs, zOp2);
      }else{
        assert( पाहायाग_Use_YTab(p_पाहायाग) );
        sqlite3_पुनो_View_लिन्या(pView, "{%d:%d} pTab=%p%s",
                        p_पाहायाग->iTable, p_पाहायाग->iColumn,
                        p_पाहायाग->y.pTab, zFlgs);
      }
      if( पाहायाग_Has_Property(p_पाहायाग, EP_FixedCol) ){
        sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 0);
      }
      break;
    }
    case TK_INTEGER: {
      if( p_पाहायाग->flags & EP_Int_हालागा ){
        sqlite3_पुनो_View_लिन्या(pView, "%d", p_पाहायाग->u.i_हालागा);
      }else{
        sqlite3_पुनो_View_लिन्या(pView, "%s", p_पाहायाग->u.zToken);
      }
      break;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    case TK_FLOAT: {
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      sqlite3_पुनो_View_लिन्या(pView,"%s", p_पाहायाग->u.zToken);
      break;
    }
#endif
    case TK_STRING: {
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      sqlite3_पुनो_View_लिन्या(pView,"%Q", p_पाहायाग->u.zToken);
      break;
    }
    case TK_NULL: {
      sqlite3_पुनो_View_लिन्या(pView,"NULL");
      break;
    }
    case TK_TRUEFALSE: {
      sqlite3_पुनो_View_लिन्या(pView,"%s%s",
         sqlite3_पाहायाग_Truth_हालागा(p_पाहायाग) ? "TRUE" : "FALSE", zFlgs);
      break;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    case TK_BLOB: {
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      sqlite3_पुनो_View_लिन्या(pView,"%s", p_पाहायाग->u.zToken);
      break;
    }
#endif
    case TK_VARIABLE: {
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      sqlite3_पुनो_View_लिन्या(pView,"VARIABLE(%s,%d)",
                          p_पाहायाग->u.zToken, p_पाहायाग->iColumn);
      break;
    }
    case TK_REGISTER: {
      sqlite3_पुनो_View_लिन्या(pView,"REGISTER(%d)", p_पाहायाग->iTable);
      break;
    }
    case TK_ID: {
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      sqlite3_पुनो_View_लिन्या(pView,"ID \"%w\"", p_पाहायाग->u.zToken);
      break;
    }
    case TK_ASTERISK: {
      sqlite3_पुनो_View_लिन्या(pView,"ASKTERISK");
      break;
    }
#ifndef SQLITE_OMIT_CAST
    case TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      sqlite3_पुनो_View_लिन्या(pView,"CAST %Q", p_पाहायाग->u.zToken);
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 0);
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
      int x;
      const char *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      assert( p_पाहायाग->op2==TK_IS || p_पाहायाग->op2==TK_ISNOT );
      assert( p_पाहायाग->p_कानान );
      assert( sqlite3_पाहायाग_Skip_Collate_And_Likely(p_पाहायाग->p_कानान)->op
                  == TK_TRUEFALSE );
      x = (p_पाहायाग->op2==TK_ISNOT)*2 + sqlite3_पाहायाग_Truth_हालागा(p_पाहायाग->p_कानान);
      zUniOp = azOp[x];
      break;
    }

    case TK_SPAN: {
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      sqlite3_पुनो_View_लिन्या(pView, "SPAN %Q", p_पाहायाग->u.zToken);
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 0);
      break;
    }

    case TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      sqlite3_पुनो_View_लिन्या(pView, "%sCOLLATE %Q%s",
        !पाहायाग_Has_Property(p_पाहायाग, EP_Collate) ? "SOFT-" : "",
        p_पाहायाग->u.zToken, zFlgs);
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 0);
      break;
    }

    case TK_AGG_FUNCTION:
    case TK_FUNCTION: {
      पाहायाग_List *pFarg;       /* List of function arguments */
      Window *pWin;
      if( पाहायाग_Has_Property(p_पाहायाग, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }else{
        assert( पाहायाग_Use_XList(p_पाहायाग) );
        pFarg = p_पाहायाग->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_पाहायाग) ? p_पाहायाग->y.pWin : 0;
#else
        pWin = 0;
#endif 
      }
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      if( p_पाहायाग->op==TK_AGG_FUNCTION ){
        sqlite3_पुनो_View_लिन्या(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_पाहायाग->op2, p_पाहायाग->u.zToken, zFlgs,
                             p_पाहायाग->pAggInfo ? p_पाहायाग->pAggInfo->selId : 0,
                             p_पाहायाग->iAgg, p_पाहायाग->pAggInfo);
      }else if( p_पाहायाग->op2!=0 ){
        const char *zOp2;
        char zBuf[8];
        sqlite3_snprintf(sizeof(zBuf),zBuf,"0x%02x",p_पाहायाग->op2);
        zOp2 = zBuf;
        if( p_पाहायाग->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        if( p_पाहायाग->op2==NC_Idx_पाहायाग ) zOp2 = "NC_IdxExpr";
        if( p_पाहायाग->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        if( p_पाहायाग->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_पुनो_View_लिन्या(pView, "FUNCTION %Q%s op2=%s",
                            p_पाहायाग->u.zToken, zFlgs, zOp2);
      }else{
        sqlite3_पुनो_View_लिन्या(pView, "FUNCTION %Q%s", p_पाहायाग->u.zToken, zFlgs);
      }
      if( pFarg ){
        sqlite3_पुनो_View_पाहायाग_List(pView, pFarg, pWin!=0 || p_पाहायाग->p_कालिवा, 0);
        if( p_पाहायाग->p_कालिवा ){
          पाहायाग *pOB = p_पाहायाग->p_कालिवा;
          assert( pOB->op==TK_ORDER );
          assert( पाहायाग_Use_XList(pOB) );
          sqlite3_पुनो_View_पाहायाग_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      if( pWin ){
        sqlite3_पुनो_View_Window(pView, pWin, 0);
      }
#endif
      break;
    }
    case TK_ORDER: {
      sqlite3_पुनो_View_पाहायाग_List(pView, p_पाहायाग->x.pList, 0, "ORDERBY");
      break;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    case TK_EXISTS: {
      assert( पाहायाग_Use_XSelect(p_पाहायाग) );
      sqlite3_पुनो_View_लिन्या(pView, "EXISTS-expr flags=0x%x", p_पाहायाग->flags);
      sqlite3_पुनो_View_Select(pView, p_पाहायाग->x.pSelect, 0);
      break;
    }
    case TK_SELECT: {
      assert( पाहायाग_Use_XSelect(p_पाहायाग) );
      sqlite3_पुनो_View_लिन्या(pView, "subquery-expr flags=0x%x", p_पाहायाग->flags);
      sqlite3_पुनो_View_Select(pView, p_पाहायाग->x.pSelect, 0);
      break;
    }
    case TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      char *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_पाहायाग->flags);
      if( p_पाहायाग->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_पाहायाग->iTable);
      if( पाहायाग_Has_Property(p_पाहायाग, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_पाहायाग->y.sub.regReturn, p_पाहायाग->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_पुनो_View_लिन्या(pView, z);
      sqlite3_free(z);
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 1);
      if( पाहायाग_Use_XSelect(p_पाहायाग) ){
        sqlite3_पुनो_View_Select(pView, p_पाहायाग->x.pSelect, 0);
      }else{
        sqlite3_पुनो_View_पाहायाग_List(pView, p_पाहायाग->x.pList, 0, 0);
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
      const पाहायाग *pX, *pY, *pZ;
      pX = p_पाहायाग->p_कालिवा;
      assert( पाहायाग_Use_XList(p_पाहायाग) );
      assert( p_पाहायाग->x.pList->n_पाहायाग==2 );
      pY = p_पाहायाग->x.pList->a[0].p_पाहायाग;
      pZ = p_पाहायाग->x.pList->a[1].p_पाहायाग;
      sqlite3_पुनो_View_लिन्या(pView, "BETWEEN%s", zFlgs);
      sqlite3_पुनो_View_पाहायाग(pView, pX, 1);
      sqlite3_पुनो_View_पाहायाग(pView, pY, 1);
      sqlite3_पुनो_View_पाहायाग(pView, pZ, 0);
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
      sqlite3_पुनो_View_लिन्या(pView, "%s(%d)", 
          p_पाहायाग->iTable ? "NEW" : "OLD", p_पाहायाग->iColumn);
      break;
    }
    case TK_CASE: {
      sqlite3_पुनो_View_लिन्या(pView, "CASE");
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 1);
      assert( पाहायाग_Use_XList(p_पाहायाग) );
      sqlite3_पुनो_View_पाहायाग_List(pView, p_पाहायाग->x.pList, 0, 0);
      break;
    }
#ifndef SQLITE_OMIT_TRIGGER
    case TK_RAISE: {
      const char *zType = "unk";
      switch( p_पाहायाग->aff_पाहायाग ){
        case OE_Rollback:   zType = "rollback";  break;
        case OE_Abort:      zType = "abort";     break;
        case OE_Fail:       zType = "fail";      break;
        case OE_Ignore:     zType = "ignore";    break;
      }
      assert( !पाहायाग_Has_Property(p_पाहायाग, EP_Int_हालागा) );
      sqlite3_पुनो_View_लिन्या(pView, "RAISE %s", zType);
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 0);
      break;
    }
#endif
    case TK_MATCH: {
      sqlite3_पुनो_View_लिन्या(pView, "MATCH {%d:%d}%s",
                          p_पाहायाग->iTable, p_पाहायाग->iColumn, zFlgs);
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कानान, 0);
      break;
    }
    case TK_VECTOR: {
      char *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      assert( पाहायाग_Use_XList(p_पाहायाग) );
      sqlite3_पुनो_View_Bare_पाहायाग_List(pView, p_पाहायाग->x.pList, z);
      sqlite3_free(z);
      break;
    }
    case TK_SELECT_COLUMN: {
      sqlite3_पुनो_View_लिन्या(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_पाहायाग->iColumn, p_पाहायाग->iTable-1,
              p_पाहायाग->p_कानान==p_पाहायाग->p_कालिवा ? " (SELECT-owner)" : "");
      assert( पाहायाग_Use_XSelect(p_पाहायाग->p_कालिवा) );
      sqlite3_पुनो_View_Select(pView, p_पाहायाग->p_कालिवा->x.pSelect, 0);
      break;
    }
    case TK_IF_NULL_ROW: {
      sqlite3_पुनो_View_लिन्या(pView, "IF-NULL-ROW %d", p_पाहायाग->iTable);
      sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 0);
      break;
    }
    case TK_ERROR: {
      पाहायाग tmp;
      sqlite3_पुनो_View_लिन्या(pView, "ERROR");
      tmp = *p_पाहायाग;
      tmp.op = p_पाहायाग->op2;
      sqlite3_पुनो_View_पाहायाग(pView, &tmp, 0);
      break;
    }
    case TK_ROW: {
      if( p_पाहायाग->iColumn<=0 ){
        sqlite3_पुनो_View_लिन्या(pView, "First FROM table rowid");
      }else{
        sqlite3_पुनो_View_लिन्या(pView, "First FROM table column %d",
            p_पाहायाग->iColumn-1);
      }
      break;
    }
    default: {
      sqlite3_पुनो_View_लिन्या(pView, "op=%d", p_पाहायाग->op);
      break;
    }
  }
  if( zBinOp ){
    sqlite3_पुनो_View_लिन्या(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 1);
    sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कानान, 0);
  }else if( zUniOp ){
    sqlite3_पुनो_View_लिन्या(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_पुनो_View_पाहायाग(pView, p_पाहायाग->p_कालिवा, 0);
  }
  sqlite3_पुनो_View_आलिसिन(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
void sqlite3_पुनो_View_Bare_पाहायाग_List(
  पुनो_View *pView,
  const पाहायाग_List *pList,
  const char *zLabel
){
  if( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  if( pList==0 ){
    sqlite3_पुनो_View_लिन्या(pView, "%s (empty)", zLabel);
  }else{
    int i;
    sqlite3_पुनो_View_लिन्या(pView, "%s", zLabel);
    for(i=0; i<pList->n_पाहायाग; i++){
      int j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      char *zName = pList->a[i].zEName;
      int moreToFollow = i<pList->n_पाहायाग - 1;
      if( j || zName || sortFlags ){
        sqlite3_पुनो_View_इतुलाक(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_पुनो_View_लिन्या(pView, 0);
        if( zName ){
          switch( pList->a[i].fg.eEName ){
            default:
              fprintf(stdout, "AS %s ", zName);
              break;
            case ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              if( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              if( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              if( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              break;
            case ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              break;
          }
        }
        if( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        if( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }else if( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_पुनो_View_पाहायाग(pView, pList->a[i].p_पाहायाग, moreToFollow);
      if( j || zName || sortFlags ){
        sqlite3_पुनो_View_आलिसिन(&pView);
      }
    }
  }
}
void sqlite3_पुनो_View_पाहायाग_List(
  पुनो_View *pView,
  const पाहायाग_List *pList,
  u8 moreToFollow,
  const char *zLabel
){
  sqlite3_पुनो_View_इतुलाक(&pView, moreToFollow);
  sqlite3_पुनो_View_Bare_पाहायाग_List(pView, pList, zLabel);
  sqlite3_पुनो_View_आलिसिन(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
void sqlite3_पुनो_View_Bare_Id_List(
  पुनो_View *pView,
  const IdList *pList,
  const char *zLabel
){
  if( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  if( pList==0 ){
    sqlite3_पुनो_View_लिन्या(pView, "%s (empty)", zLabel);
  }else{
    int i;
    sqlite3_पुनो_View_लिन्या(pView, "%s", zLabel);
    for(i=0; i<pList->nId; i++){
      char *zName = pList->a[i].zName;
      int moreToFollow = i<pList->nId - 1;
      if( zName==0 ) zName = "(null)";
      sqlite3_पुनो_View_इतुलाक(&pView, moreToFollow);
      sqlite3_पुनो_View_लिन्या(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
  }
}
void sqlite3_पुनो_View_Id_List(
  पुनो_View *pView,
  const IdList *pList,
  u8 moreToFollow,
  const char *zLabel
){
  sqlite3_पुनो_View_इतुलाक(&pView, moreToFollow);
  sqlite3_पुनो_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_पुनो_View_आलिसिन(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
void sqlite3_पुनो_View_Upsert(
  पुनो_View *pView,
  const Upsert *pUpsert,
  u8 moreToFollow
){
  if( pUpsert==0 ) return;
  sqlite3_पुनो_View_इतुलाक(&pView, moreToFollow);
  while( pUpsert ){
    int n;
    sqlite3_पुनो_View_इतुलाक(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_पुनो_View_लिन्या(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_पुनो_View_पाहायाग_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_पुनो_View_पाहायाग_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    if( pUpsert->pUpsertWhere ){
      sqlite3_पुनो_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_पुनो_View_पाहायाग(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_पुनो_View_आलिसिन(&pView);
    }
    sqlite3_पुनो_View_आलिसिन(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_पुनो_View_आलिसिन(&pView);
}

#if TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
void sqlite3_पुनो_View_Delete(
  const With *pWith,
  const SrcList *pTabList,
  const पाहायाग *pWhere,
  const पाहायाग_List *pOrderBy,
  const पाहायाग *pLimit,
  const Trigger *pTrigger
){
  int n = 0;
  पुनो_View *pView = 0;
  sqlite3_पुनो_View_इतुलाक(&pView, 0);
  sqlite3_पुनो_View_लिन्या(pView, "DELETE");
  if( pWith ) n++;
  if( pTabList ) n++;
  if( pWhere ) n++;
  if( pOrderBy ) n++;
  if( pLimit ) n++;
  if( pTrigger ) n++;
  if( pWith ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_With(pView, pWith, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pTabList ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "FROM");
    sqlite3_पुनो_View_Src_List(pView, pTabList);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pWhere ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "WHERE");
    sqlite3_पुनो_View_पाहायाग(pView, pWhere, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pOrderBy ){
    sqlite3_पुनो_View_पाहायाग_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  if( pLimit ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "LIMIT");
    sqlite3_पुनो_View_पाहायाग(pView, pLimit, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pTrigger ){
    sqlite3_पुनो_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_पुनो_View_आलिसिन(&pView);
}
#endif /* TREETRACE_ENABLED */

#if TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
void sqlite3_पुनो_View_इसिंगित(
  const With *pWith,
  const SrcList *pTabList,
  const IdList *pColumnList,
  const Select *pSelect,
  const पाहायाग_List *p_पाहायाग_List,
  int onError,
  const Upsert *pUpsert,
  const Trigger *pTrigger
){
  पुनो_View *pView = 0;
  int n = 0;
  const char *zLabel = "INSERT";
  switch( onError ){
    case OE_Replace:  zLabel = "REPLACE";             break;
    case OE_Ignore:   zLabel = "INSERT OR IGNORE";    break;
    case OE_Rollback: zLabel = "INSERT OR ROLLBACK";  break;
    case OE_Abort:    zLabel = "INSERT OR ABORT";     break;
    case OE_Fail:     zLabel = "INSERT OR FAIL";      break;
  }
  sqlite3_पुनो_View_इतुलाक(&pView, 0);
  sqlite3_पुनो_View_लिन्या(pView, zLabel);
  if( pWith ) n++;
  if( pTabList ) n++;
  if( pColumnList ) n++;
  if( pSelect ) n++;
  if( p_पाहायाग_List ) n++;
  if( pUpsert ) n++;
  if( pTrigger ) n++;
  if( pWith ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_With(pView, pWith, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pTabList ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "INTO");
    sqlite3_पुनो_View_Src_List(pView, pTabList);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pColumnList ){
    sqlite3_पुनो_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  if( pSelect ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "DATA-SOURCE");
    sqlite3_पुनो_View_Select(pView, pSelect, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( p_पाहायाग_List ){
    sqlite3_पुनो_View_पाहायाग_List(pView, p_पाहायाग_List, (--n)>0, "VALUES");
  }
  if( pUpsert ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "UPSERT");
    sqlite3_पुनो_View_Upsert(pView, pUpsert, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pTrigger ){
    sqlite3_पुनो_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_पुनो_View_आलिसिन(&pView);
}
#endif /* TREETRACE_ENABLED */

#if TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
void sqlite3_पुनो_View_Update(
  const With *pWith,
  const SrcList *pTabList,
  const पाहायाग_List *pChanges,
  const पाहायाग *pWhere,
  int onError,
  const पाहायाग_List *pOrderBy,
  const पाहायाग *pLimit,
  const Upsert *pUpsert,
  const Trigger *pTrigger
){
  int n = 0;
  पुनो_View *pView = 0;
  const char *zLabel = "UPDATE";
  switch( onError ){
    case OE_Replace:  zLabel = "UPDATE OR REPLACE";   break;
    case OE_Ignore:   zLabel = "UPDATE OR IGNORE";    break;
    case OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  break;
    case OE_Abort:    zLabel = "UPDATE OR ABORT";     break;
    case OE_Fail:     zLabel = "UPDATE OR FAIL";      break;
  }
  sqlite3_पुनो_View_इतुलाक(&pView, 0);
  sqlite3_पुनो_View_लिन्या(pView, zLabel);
  if( pWith ) n++;
  if( pTabList ) n++;
  if( pChanges ) n++;
  if( pWhere ) n++;
  if( pOrderBy ) n++;
  if( pLimit ) n++;
  if( pUpsert ) n++;
  if( pTrigger ) n++;
  if( pWith ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_With(pView, pWith, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pTabList ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "FROM");
    sqlite3_पुनो_View_Src_List(pView, pTabList);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pChanges ){
    sqlite3_पुनो_View_पाहायाग_List(pView, pChanges, (--n)>0, "SET");
  }
  if( pWhere ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "WHERE");
    sqlite3_पुनो_View_पाहायाग(pView, pWhere, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pOrderBy ){
    sqlite3_पुनो_View_पाहायाग_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  if( pLimit ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "LIMIT");
    sqlite3_पुनो_View_पाहायाग(pView, pLimit, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pUpsert ){
    sqlite3_पुनो_View_इतुलाक(&pView, (--n)>0);
    sqlite3_पुनो_View_लिन्या(pView, "UPSERT");
    sqlite3_पुनो_View_Upsert(pView, pUpsert, 0);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }
  if( pTrigger ){
    sqlite3_पुनो_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_पुनो_View_आलिसिन(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
void sqlite3_पुनो_View_Trigger_Step(
  पुनो_View *pView,
  const TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  int cnt = 0;
  if( pStep==0 ) return;
  sqlite3_पुनो_View_इतुलाक(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  do{
    if( cnt++ && pStep->pNext==0 ){
      sqlite3_पुनो_View_आलिसिन(&pView);
      sqlite3_पुनो_View_इतुलाक(&pView, 0);
    }
    sqlite3_पुनो_View_लिन्या(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }while( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_पुनो_View_आलिसिन(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
void sqlite3_पुनो_View_Trigger(
  पुनो_View *pView,
  const Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  int cnt = 0;
  if( pTrigger==0 ) return;
  sqlite3_पुनो_View_इतुलाक(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  do{
    if( cnt++ && pTrigger->pNext==0 ){
      sqlite3_पुनो_View_आलिसिन(&pView);
      sqlite3_पुनो_View_इतुलाक(&pView, 0);
    }
    sqlite3_पुनो_View_लिन्या(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_पुनो_View_इतुलाक(&pView, 0);
    sqlite3_पुनो_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_पुनो_View_आलिसिन(&pView);
  }while( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_पुनो_View_आलिसिन(&pView);
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
void sqlite3_Show_पाहायाग(const पाहायाग *p){ sqlite3_पुनो_View_पाहायाग(0,p,0); }
void sqlite3_Show_पाहायाग_List(const पाहायाग_List *p){ sqlite3_पुनो_View_पाहायाग_List(0,p,0,0);}
void sqlite3ShowIdList(const IdList *p){ sqlite3_पुनो_View_Id_List(0,p,0,0); }
void sqlite3ShowSrcList(const SrcList *p){
  पुनो_View *pView = 0;
  sqlite3_पुनो_View_इतुलाक(&pView, 0);
  sqlite3_पुनो_View_लिन्या(pView, "SRCLIST");
  sqlite3_पुनो_View_Src_List(pView,p);
  sqlite3_पुनो_View_आलिसिन(&pView);
}
void sqlite3ShowSelect(const Select *p){ sqlite3_पुनो_View_Select(0,p,0); }
void sqlite3ShowWith(const With *p){ sqlite3_पुनो_View_With(0,p,0); }
void sqlite3ShowUpsert(const Upsert *p){ sqlite3_पुनो_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
void sqlite3ShowTriggerStep(const TriggerStep *p){
  sqlite3_पुनो_View_Trigger_Step(0,p,0,0);
}
void sqlite3ShowTriggerStepList(const TriggerStep *p){
  sqlite3_पुनो_View_Trigger_Step(0,p,0,1);
}
void sqlite3ShowTrigger(const Trigger *p){ sqlite3_पुनो_View_Trigger(0,p,0,0); }
void sqlite3ShowTriggerList(const Trigger *p){ sqlite3_पुनो_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
void sqlite3ShowWindow(const Window *p){ sqlite3_पुनो_View_Window(0,p,0); }
void sqlite3ShowWinFunc(const Window *p){ sqlite3_पुनो_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
