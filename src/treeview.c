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
#അന്തര്ഭാവ "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
സ്ഥിതം ശൂന്യം sqlite3_വൃക്ഷം_View_തള്ളുക(വൃക്ഷം_View **pp, u8 moreToFollow){
  വൃക്ഷം_View *p = *pp;
  എങ്കിൽ( p==0 ){
    *pp = p = sqlite3_malloc64( വലുപ്പം(*p) );
    എങ്കിൽ( p==0 ) തിരികെ;
    memset(p, 0, വലുപ്പം(*p));
  }അല്ലെങ്കിൽ{
    p->iLevel++;
  }
  പ്രതിജ്ഞാ( moreToFollow==0 || moreToFollow==1 );
  എങ്കിൽ( p->iLevel<(പൂർണ്ണസംഖ്യ)വലുപ്പം(p->b_വരി) ) p->b_വരി[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
സ്ഥിതം ശൂന്യം sqlite3_വൃക്ഷം_View_എടുക്കുക(വൃക്ഷം_View **pp){
  വൃക്ഷം_View *p = *pp;
  എങ്കിൽ( p==0 ) തിരികെ;
  p->iLevel--;
  എങ്കിൽ( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_വരി(വൃക്ഷം_View *p, സ്ഥിരം അക്ഷരം *zFormat, ...){
  va_list ap;
  പൂർണ്ണസംഖ്യ i;
  StrAccum acc;
  അക്ഷരം zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, വലുപ്പം(zBuf), 0);
  എങ്കിൽ( p ){
    വേണ്ടി(i=0; i<p->iLevel && i<(പൂർണ്ണസംഖ്യ)വലുപ്പം(p->b_വരി)-1; i++){
      sqlite3_str_append(&acc, p->b_വരി[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_വരി[i] ? "|-- " : "'-- ", 4);
  }
  എങ്കിൽ( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    പ്രതിജ്ഞാ( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
സ്ഥിതം ശൂന്യം sqlite3_വൃക്ഷം_View_Item(വൃക്ഷം_View *p, സ്ഥിരം അക്ഷരം *zLabel,u8 moreFollows){
  sqlite3_വൃക്ഷം_View_തള്ളുക(&p, moreFollows);
  sqlite3_വൃക്ഷം_View_വരി(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Column_List(
  വൃക്ഷം_View *pView,
  സ്ഥിരം Column *aCol,
  പൂർണ്ണസംഖ്യ nCol,
  u8 moreToFollow
){
  പൂർണ്ണസംഖ്യ i;
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, moreToFollow);
  sqlite3_വൃക്ഷം_View_വരി(pView, "COLUMNS");
  വേണ്ടി(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    പൂർണ്ണസംഖ്യ colMoreToFollow = i<(nCol - 1);
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, colMoreToFollow);
    sqlite3_വൃക്ഷം_View_വരി(pView, 0);
    printf(" %s", aCol[i].zCnName);
    തിരഞ്ഞെടുപ്പ്( aCol[i].eCType ){
      അവസ്ഥ COLTYPE_ANY:      printf(" ANY");        നിർത്തുക;
      അവസ്ഥ COLTYPE_BLOB:     printf(" BLOB");       നിർത്തുക;
      അവസ്ഥ COLTYPE_INT:      printf(" INT");        നിർത്തുക;
      അവസ്ഥ COLTYPE_INTEGER:  printf(" INTEGER");    നിർത്തുക;
      അവസ്ഥ COLTYPE_REAL:     printf(" REAL");       നിർത്തുക;
      അവസ്ഥ COLTYPE_TEXT:     printf(" TEXT");       നിർത്തുക;
      അവസ്ഥ COLTYPE_CUSTOM: {
        എങ്കിൽ( flg & COLFLAG_HASTYPE ){
          സ്ഥിരം അക്ഷരം *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          നിർത്തുക;
        }
      }
    }
    എങ്കിൽ( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    എങ്കിൽ( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    എങ്കിൽ( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    എങ്കിൽ( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_With(വൃക്ഷം_View *pView, സ്ഥിരം With *pWith, u8 moreToFollow){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( pWith==0 ) തിരികെ;
  എങ്കിൽ( pWith->nCte==0 ) തിരികെ;
  എങ്കിൽ( pWith->pOuter ){
    sqlite3_വൃക്ഷം_View_വരി(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }അല്ലെങ്കിൽ{
    sqlite3_വൃക്ഷം_View_വരി(pView, "WITH (0x%p)", pWith);
  }
  എങ്കിൽ( pWith->nCte>0 ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, moreToFollow);
    വേണ്ടി(i=0; i<pWith->nCte; i++){
      StrAccum x;
      അക്ഷരം z_വരി[1000];
      സ്ഥിരം ഘടന Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_വരി, വലുപ്പം(z_വരി), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      എങ്കിൽ( pCte->pCols && pCte->pCols->n_വ്യഞ്ജകം>0 ){
        അക്ഷരം cSep = '(';
        പൂർണ്ണസംഖ്യ j;
        വേണ്ടി(j=0; j<pCte->pCols->n_വ്യഞ്ജകം; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      എങ്കിൽ( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      എങ്കിൽ( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_വൃക്ഷം_View_Item(pView, z_വരി, i<pWith->nCte-1);
      sqlite3_വൃക്ഷം_View_Select(pView, pCte->pSelect, 0);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Src_List(വൃക്ഷം_View *pView, സ്ഥിരം SrcList *pSrc){
  പൂർണ്ണസംഖ്യ i;
  എങ്കിൽ( pSrc==0 ) തിരികെ;
  വേണ്ടി(i=0; i<pSrc->nSrc; i++){
    സ്ഥിരം SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    പൂർണ്ണസംഖ്യ n = 0;
    അക്ഷരം z_വരി[1000];
    sqlite3StrAccumInit(&x, 0, z_വരി, വലുപ്പം(z_വരി), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    എങ്കിൽ( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    എങ്കിൽ( (pItem->fg.jointype & (JT_ഇടത്|JT_വലത്))==(JT_ഇടത്|JT_വലത്) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }അല്ലെങ്കിൽ എങ്കിൽ( pItem->fg.jointype & JT_ഇടത് ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }അല്ലെങ്കിൽ എങ്കിൽ( pItem->fg.jointype & JT_വലത് ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }അല്ലെങ്കിൽ എങ്കിൽ( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    എങ്കിൽ( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    എങ്കിൽ( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    എങ്കിൽ( pItem->fg.isCte ){
      സ്ഥിതം സ്ഥിരം അക്ഷരം *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    എങ്കിൽ( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    എങ്കിൽ( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    എങ്കിൽ( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    എങ്കിൽ( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    എങ്കിൽ( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    എങ്കിൽ( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    എങ്കിൽ( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    എങ്കിൽ( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    എങ്കിൽ( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    എങ്കിൽ( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_വൃക്ഷം_View_Item(pView, z_വരി, i<pSrc->nSrc-1);
    n = 0;
    എങ്കിൽ( pItem->fg.isSubquery ) n++;
    എങ്കിൽ( pItem->fg.isTabFunc ) n++;
    എങ്കിൽ( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    എങ്കിൽ( pItem->fg.isUsing ){
      sqlite3_വൃക്ഷം_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }അല്ലെങ്കിൽ എങ്കിൽ( pItem->u3.pOn!=0 ){
      sqlite3_വൃക്ഷം_View_Item(pView, "ON", (--n)>0);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pItem->u3.pOn, 0);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
    എങ്കിൽ( pItem->fg.isSubquery ){
      പ്രതിജ്ഞാ( n==1 );
      എങ്കിൽ( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_വൃക്ഷം_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      പ്രതിജ്ഞാ( (പൂർണ്ണസംഖ്യ)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_വൃക്ഷം_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    എങ്കിൽ( pItem->fg.isTabFunc ){
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Select(വൃക്ഷം_View *pView, സ്ഥിരം Select *p, u8 moreToFollow){
  പൂർണ്ണസംഖ്യ n = 0;
  പൂർണ്ണസംഖ്യ cnt = 0;
  എങ്കിൽ( p==0 ){
    sqlite3_വൃക്ഷം_View_വരി(pView, "nil-SELECT");
    തിരികെ;
  } 
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, moreToFollow);
  എങ്കിൽ( p->pWith ){
    sqlite3_വൃക്ഷം_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 1);
  }
  ചെയ്യുക{
    എങ്കിൽ( p->selFlags & SF_WhereBegin ){
      sqlite3_വൃക്ഷം_View_വരി(pView, "sqlite3WhereBegin()");
    }അല്ലെങ്കിൽ{
      sqlite3_വൃക്ഷം_View_വരി(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (പൂർണ്ണസംഖ്യ)p->nSelectRow
      );
    }
    എങ്കിൽ( cnt++ ) sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    എങ്കിൽ( p->pPrior ){
      n = 1000;
    }അല്ലെങ്കിൽ{
      n = 0;
      എങ്കിൽ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      എങ്കിൽ( p->pWhere ) n++;
      എങ്കിൽ( p->pGroupBy ) n++;
      എങ്കിൽ( p->pHaving ) n++;
      എങ്കിൽ( p->pOrderBy ) n++;
      എങ്കിൽ( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      എങ്കിൽ( p->pWin ) n++;
      എങ്കിൽ( p->pWinDefn ) n++;
#endif
    }
    എങ്കിൽ( p->pEList ){
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    എങ്കിൽ( p->pWin ){
      Window *pX;
      sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (n--)>0);
      sqlite3_വൃക്ഷം_View_വരി(pView, "window-functions");
      വേണ്ടി(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_വൃക്ഷം_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
#endif
    എങ്കിൽ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (n--)>0);
      sqlite3_വൃക്ഷം_View_വരി(pView, "FROM");
      sqlite3_വൃക്ഷം_View_Src_List(pView, p->pSrc);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
    എങ്കിൽ( p->pWhere ){
      sqlite3_വൃക്ഷം_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p->pWhere, 0);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
    എങ്കിൽ( p->pGroupBy ){
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    എങ്കിൽ( p->pHaving ){
      sqlite3_വൃക്ഷം_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p->pHaving, 0);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    എങ്കിൽ( p->pWinDefn ){
      Window *pX;
      sqlite3_വൃക്ഷം_View_Item(pView, "WINDOW", (n--)>0);
      വേണ്ടി(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_വൃക്ഷം_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
#endif
    എങ്കിൽ( p->pOrderBy ){
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    എങ്കിൽ( p->pLimit ){
      sqlite3_വൃക്ഷം_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p->pLimit->p_ഇടത്, p->pLimit->p_വലത്!=0);
      എങ്കിൽ( p->pLimit->p_വലത് ){
        sqlite3_വൃക്ഷം_View_Item(pView, "OFFSET", 0);
        sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p->pLimit->p_വലത്, 0);
        sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
      }
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
    എങ്കിൽ( p->pPrior ){
      സ്ഥിരം അക്ഷരം *zOp = "UNION";
      തിരഞ്ഞെടുപ്പ്( p->op ){
        അവസ്ഥ TK_ALL:         zOp = "UNION ALL";  നിർത്തുക;
        അവസ്ഥ TK_INTERSECT:   zOp = "INTERSECT";  നിർത്തുക;
        അവസ്ഥ TK_EXCEPT:      zOp = "EXCEPT";     നിർത്തുക;
      }
      sqlite3_വൃക്ഷം_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }ആകുമ്പോൾ( p!=0 );
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Bound(
  വൃക്ഷം_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  തിരഞ്ഞെടുപ്പ്( eBound ){
    അവസ്ഥ TK_UNBOUNDED: {
      sqlite3_വൃക്ഷം_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
      നിർത്തുക;
    }
    അവസ്ഥ TK_CURRENT: {
      sqlite3_വൃക്ഷം_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
      നിർത്തുക;
    }
    അവസ്ഥ TK_PRECEDING: {
      sqlite3_വൃക്ഷം_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം, 0);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
      നിർത്തുക;
    }
    അവസ്ഥ TK_FOLLOWING: {
      sqlite3_വൃക്ഷം_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം, 0);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
      നിർത്തുക;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Window(വൃക്ഷം_View *pView, സ്ഥിരം Window *pWin, u8 more){
  പൂർണ്ണസംഖ്യ nElement = 0;
  എങ്കിൽ( pWin==0 ) തിരികെ;
  എങ്കിൽ( pWin->pFilter ){
    sqlite3_വൃക്ഷം_View_Item(pView, "FILTER", 1);
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pWin->pFilter, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    എങ്കിൽ( pWin->eFrmType==TK_FILTER ) തിരികെ;
  }
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, more);
  എങ്കിൽ( pWin->zName ){
    sqlite3_വൃക്ഷം_View_വരി(pView, "OVER %s (%p)", pWin->zName, pWin);
  }അല്ലെങ്കിൽ{
    sqlite3_വൃക്ഷം_View_വരി(pView, "OVER (%p)", pWin);
  }
  എങ്കിൽ( pWin->zBase )    nElement++;
  എങ്കിൽ( pWin->pOrderBy ) nElement++;
  എങ്കിൽ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  എങ്കിൽ( pWin->eExclude ) nElement++;
  എങ്കിൽ( pWin->zBase ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--nElement)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "window: %s", pWin->zBase);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pWin->pPartition ){
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  എങ്കിൽ( pWin->pOrderBy ){
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  എങ്കിൽ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    അക്ഷരം zBuf[30];
    സ്ഥിരം അക്ഷരം *zFrmType = "ROWS";
    എങ്കിൽ( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    എങ്കിൽ( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(വലുപ്പം(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_വൃക്ഷം_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_വൃക്ഷം_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_വൃക്ഷം_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pWin->eExclude ){
    അക്ഷരം zBuf[30];
    സ്ഥിരം അക്ഷരം *zExclude;
    തിരഞ്ഞെടുപ്പ്( pWin->eExclude ){
      അവസ്ഥ TK_NO:      zExclude = "NO OTHERS";   നിർത്തുക;
      അവസ്ഥ TK_CURRENT: zExclude = "CURRENT ROW"; നിർത്തുക;
      അവസ്ഥ TK_GROUP:   zExclude = "GROUP";       നിർത്തുക;
      അവസ്ഥ TK_TIES:    zExclude = "TIES";        നിർത്തുക;
      സ്ഥിരസ്ഥിതി:
        sqlite3_snprintf(വലുപ്പം(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        നിർത്തുക;
    }
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "EXCLUDE %s", zExclude);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Win_Func(വൃക്ഷം_View *pView, സ്ഥിരം Window *pWin, u8 more){
  എങ്കിൽ( pWin==0 ) തിരികെ;
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, more);
  sqlite3_വൃക്ഷം_View_വരി(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_വൃക്ഷം_View_Window(pView, pWin, 0);
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(വൃക്ഷം_View *pView, സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം, u8 moreToFollow){
  സ്ഥിരം അക്ഷരം *zBinOp = 0;   /* Binary operator */
  സ്ഥിരം അക്ഷരം *zUniOp = 0;   /* Unary operator */
  അക്ഷരം zFlgs[200];
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, moreToFollow);
  എങ്കിൽ( p_വ്യഞ്ജകം==0 ){
    sqlite3_വൃക്ഷം_View_വരി(pView, "nil");
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    തിരികെ;
  }
  എങ്കിൽ( p_വ്യഞ്ജകം->flags || p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം || p_വ്യഞ്ജകം->vvaFlags || p_വ്യഞ്ജകം->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, വലുപ്പം(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_വ്യഞ്ജകം->flags, p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം ? p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം : 'n');
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_വ്യഞ്ജകം->w.iJoin);
    }
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_വ്യഞ്ജകം->w.iJoin);
    }
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    എങ്കിൽ( വ്യഞ്ജകം_Has_VVAProperty(p_വ്യഞ്ജകം, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    എങ്കിൽ( p_വ്യഞ്ജകം->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_വ്യഞ്ജകം->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }അല്ലെങ്കിൽ{
    zFlgs[0] = 0;
  }
  തിരഞ്ഞെടുപ്പ്( p_വ്യഞ്ജകം->op ){
    അവസ്ഥ TK_AGG_COLUMN: {
      sqlite3_വൃക്ഷം_View_വരി(pView, "AGG{%d:%d}%s",
            p_വ്യഞ്ജകം->iTable, p_വ്യഞ്ജകം->iColumn, zFlgs);
      നിർത്തുക;
    }
    അവസ്ഥ TK_COLUMN: {
      എങ്കിൽ( p_വ്യഞ്ജകം->iTable<0 ){
        /* This only happens when coding check constraints */
        അക്ഷരം zOp2[16];
        എങ്കിൽ( p_വ്യഞ്ജകം->op2 ){
          sqlite3_snprintf(വലുപ്പം(zOp2),zOp2," op2=0x%02x",p_വ്യഞ്ജകം->op2);
        }അല്ലെങ്കിൽ{
          zOp2[0] = 0;
        }
        sqlite3_വൃക്ഷം_View_വരി(pView, "COLUMN(%d)%s%s",
                                    p_വ്യഞ്ജകം->iColumn, zFlgs, zOp2);
      }അല്ലെങ്കിൽ{
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(p_വ്യഞ്ജകം) );
        sqlite3_വൃക്ഷം_View_വരി(pView, "{%d:%d} pTab=%p%s",
                        p_വ്യഞ്ജകം->iTable, p_വ്യഞ്ജകം->iColumn,
                        p_വ്യഞ്ജകം->y.pTab, zFlgs);
      }
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_FixedCol) ){
        sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 0);
      }
      നിർത്തുക;
    }
    അവസ്ഥ TK_INTEGER: {
      എങ്കിൽ( p_വ്യഞ്ജകം->flags & EP_Int_മൂല്യം ){
        sqlite3_വൃക്ഷം_View_വരി(pView, "%d", p_വ്യഞ്ജകം->u.i_മൂല്യം);
      }അല്ലെങ്കിൽ{
        sqlite3_വൃക്ഷം_View_വരി(pView, "%s", p_വ്യഞ്ജകം->u.zToken);
      }
      നിർത്തുക;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    അവസ്ഥ TK_FLOAT: {
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3_വൃക്ഷം_View_വരി(pView,"%s", p_വ്യഞ്ജകം->u.zToken);
      നിർത്തുക;
    }
#endif
    അവസ്ഥ TK_STRING: {
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3_വൃക്ഷം_View_വരി(pView,"%Q", p_വ്യഞ്ജകം->u.zToken);
      നിർത്തുക;
    }
    അവസ്ഥ TK_NULL: {
      sqlite3_വൃക്ഷം_View_വരി(pView,"NULL");
      നിർത്തുക;
    }
    അവസ്ഥ TK_TRUEFALSE: {
      sqlite3_വൃക്ഷം_View_വരി(pView,"%s%s",
         sqlite3_വ്യഞ്ജകം_Truth_മൂല്യം(p_വ്യഞ്ജകം) ? "TRUE" : "FALSE", zFlgs);
      നിർത്തുക;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    അവസ്ഥ TK_BLOB: {
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3_വൃക്ഷം_View_വരി(pView,"%s", p_വ്യഞ്ജകം->u.zToken);
      നിർത്തുക;
    }
#endif
    അവസ്ഥ TK_VARIABLE: {
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3_വൃക്ഷം_View_വരി(pView,"VARIABLE(%s,%d)",
                          p_വ്യഞ്ജകം->u.zToken, p_വ്യഞ്ജകം->iColumn);
      നിർത്തുക;
    }
    അവസ്ഥ TK_REGISTER: {
      sqlite3_വൃക്ഷം_View_വരി(pView,"REGISTER(%d)", p_വ്യഞ്ജകം->iTable);
      നിർത്തുക;
    }
    അവസ്ഥ TK_ID: {
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3_വൃക്ഷം_View_വരി(pView,"ID \"%w\"", p_വ്യഞ്ജകം->u.zToken);
      നിർത്തുക;
    }
    അവസ്ഥ TK_ASTERISK: {
      sqlite3_വൃക്ഷം_View_വരി(pView,"ASKTERISK");
      നിർത്തുക;
    }
#ifndef SQLITE_OMIT_CAST
    അവസ്ഥ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3_വൃക്ഷം_View_വരി(pView,"CAST %Q", p_വ്യഞ്ജകം->u.zToken);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 0);
      നിർത്തുക;
    }
#endif /* SQLITE_OMIT_CAST */
    അവസ്ഥ TK_LT:      zBinOp = "LT";     നിർത്തുക;
    അവസ്ഥ TK_LE:      zBinOp = "LE";     നിർത്തുക;
    അവസ്ഥ TK_GT:      zBinOp = "GT";     നിർത്തുക;
    അവസ്ഥ TK_GE:      zBinOp = "GE";     നിർത്തുക;
    അവസ്ഥ TK_NE:      zBinOp = "NE";     നിർത്തുക;
    അവസ്ഥ TK_EQ:      zBinOp = "EQ";     നിർത്തുക;
    അവസ്ഥ TK_IS:      zBinOp = "IS";     നിർത്തുക;
    അവസ്ഥ TK_ISNOT:   zBinOp = "ISNOT";  നിർത്തുക;
    അവസ്ഥ TK_AND:     zBinOp = "AND";    നിർത്തുക;
    അവസ്ഥ TK_OR:      zBinOp = "OR";     നിർത്തുക;
    അവസ്ഥ TK_PLUS:    zBinOp = "ADD";    നിർത്തുക;
    അവസ്ഥ TK_STAR:    zBinOp = "MUL";    നിർത്തുക;
    അവസ്ഥ TK_MINUS:   zBinOp = "SUB";    നിർത്തുക;
    അവസ്ഥ TK_REM:     zBinOp = "REM";    നിർത്തുക;
    അവസ്ഥ TK_BITAND:  zBinOp = "BITAND"; നിർത്തുക;
    അവസ്ഥ TK_BITOR:   zBinOp = "BITOR";  നിർത്തുക;
    അവസ്ഥ TK_SLASH:   zBinOp = "DIV";    നിർത്തുക;
    അവസ്ഥ TK_LSHIFT:  zBinOp = "LSHIFT"; നിർത്തുക;
    അവസ്ഥ TK_RSHIFT:  zBinOp = "RSHIFT"; നിർത്തുക;
    അവസ്ഥ TK_CONCAT:  zBinOp = "CONCAT"; നിർത്തുക;
    അവസ്ഥ TK_DOT:     zBinOp = "DOT";    നിർത്തുക;
    അവസ്ഥ TK_LIMIT:   zBinOp = "LIMIT";  നിർത്തുക;

    അവസ്ഥ TK_UMINUS:  zUniOp = "UMINUS"; നിർത്തുക;
    അവസ്ഥ TK_UPLUS:   zUniOp = "UPLUS";  നിർത്തുക;
    അവസ്ഥ TK_BITNOT:  zUniOp = "BITNOT"; നിർത്തുക;
    അവസ്ഥ TK_NOT:     zUniOp = "NOT";    നിർത്തുക;
    അവസ്ഥ TK_ISNULL:  zUniOp = "ISNULL"; നിർത്തുക;
    അവസ്ഥ TK_NOTNULL: zUniOp = "NOTNULL"; നിർത്തുക;

    അവസ്ഥ TK_TRUTH: {
      പൂർണ്ണസംഖ്യ x;
      സ്ഥിരം അക്ഷരം *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op2==TK_IS || p_വ്യഞ്ജകം->op2==TK_ISNOT );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_വലത് );
      പ്രതിജ്ഞാ( sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(p_വ്യഞ്ജകം->p_വലത്)->op
                  == TK_TRUEFALSE );
      x = (p_വ്യഞ്ജകം->op2==TK_ISNOT)*2 + sqlite3_വ്യഞ്ജകം_Truth_മൂല്യം(p_വ്യഞ്ജകം->p_വലത്);
      zUniOp = azOp[x];
      നിർത്തുക;
    }

    അവസ്ഥ TK_SPAN: {
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3_വൃക്ഷം_View_വരി(pView, "SPAN %Q", p_വ്യഞ്ജകം->u.zToken);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 0);
      നിർത്തുക;
    }

    അവസ്ഥ TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3_വൃക്ഷം_View_വരി(pView, "%sCOLLATE %Q%s",
        !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Collate) ? "SOFT-" : "",
        p_വ്യഞ്ജകം->u.zToken, zFlgs);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 0);
      നിർത്തുക;
    }

    അവസ്ഥ TK_AGG_FUNCTION:
    അവസ്ഥ TK_FUNCTION: {
      വ്യഞ്ജകം_List *pFarg;       /* List of function arguments */
      Window *pWin;
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }അല്ലെങ്കിൽ{
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
        pFarg = p_വ്യഞ്ജകം->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_വ്യഞ്ജകം) ? p_വ്യഞ്ജകം->y.pWin : 0;
#അല്ലെങ്കിൽ
        pWin = 0;
#endif 
      }
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_AGG_FUNCTION ){
        sqlite3_വൃക്ഷം_View_വരി(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_വ്യഞ്ജകം->op2, p_വ്യഞ്ജകം->u.zToken, zFlgs,
                             p_വ്യഞ്ജകം->pAggInfo ? p_വ്യഞ്ജകം->pAggInfo->selId : 0,
                             p_വ്യഞ്ജകം->iAgg, p_വ്യഞ്ജകം->pAggInfo);
      }അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op2!=0 ){
        സ്ഥിരം അക്ഷരം *zOp2;
        അക്ഷരം zBuf[8];
        sqlite3_snprintf(വലുപ്പം(zBuf),zBuf,"0x%02x",p_വ്യഞ്ജകം->op2);
        zOp2 = zBuf;
        എങ്കിൽ( p_വ്യഞ്ജകം->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        എങ്കിൽ( p_വ്യഞ്ജകം->op2==NC_Idx_വ്യഞ്ജകം ) zOp2 = "NC_IdxExpr";
        എങ്കിൽ( p_വ്യഞ്ജകം->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        എങ്കിൽ( p_വ്യഞ്ജകം->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_വൃക്ഷം_View_വരി(pView, "FUNCTION %Q%s op2=%s",
                            p_വ്യഞ്ജകം->u.zToken, zFlgs, zOp2);
      }അല്ലെങ്കിൽ{
        sqlite3_വൃക്ഷം_View_വരി(pView, "FUNCTION %Q%s", p_വ്യഞ്ജകം->u.zToken, zFlgs);
      }
      എങ്കിൽ( pFarg ){
        sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pFarg, pWin!=0 || p_വ്യഞ്ജകം->p_ഇടത്, 0);
        എങ്കിൽ( p_വ്യഞ്ജകം->p_ഇടത് ){
          വ്യഞ്ജകം *pOB = p_വ്യഞ്ജകം->p_ഇടത്;
          പ്രതിജ്ഞാ( pOB->op==TK_ORDER );
          പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(pOB) );
          sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      എങ്കിൽ( pWin ){
        sqlite3_വൃക്ഷം_View_Window(pView, pWin, 0);
      }
#endif
      നിർത്തുക;
    }
    അവസ്ഥ TK_ORDER: {
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, p_വ്യഞ്ജകം->x.pList, 0, "ORDERBY");
      നിർത്തുക;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    അവസ്ഥ TK_EXISTS: {
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) );
      sqlite3_വൃക്ഷം_View_വരി(pView, "EXISTS-expr flags=0x%x", p_വ്യഞ്ജകം->flags);
      sqlite3_വൃക്ഷം_View_Select(pView, p_വ്യഞ്ജകം->x.pSelect, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_SELECT: {
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) );
      sqlite3_വൃക്ഷം_View_വരി(pView, "subquery-expr flags=0x%x", p_വ്യഞ്ജകം->flags);
      sqlite3_വൃക്ഷം_View_Select(pView, p_വ്യഞ്ജകം->x.pSelect, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      അക്ഷരം *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_വ്യഞ്ജകം->flags);
      എങ്കിൽ( p_വ്യഞ്ജകം->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_വ്യഞ്ജകം->iTable);
      എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_വ്യഞ്ജകം->y.sub.regReturn, p_വ്യഞ്ജകം->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_വൃക്ഷം_View_വരി(pView, z);
      sqlite3_free(z);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 1);
      എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
        sqlite3_വൃക്ഷം_View_Select(pView, p_വ്യഞ്ജകം->x.pSelect, 0);
      }അല്ലെങ്കിൽ{
        sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, p_വ്യഞ്ജകം->x.pList, 0, 0);
      }
      നിർത്തുക;
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
    അവസ്ഥ TK_BETWEEN: {
      സ്ഥിരം വ്യഞ്ജകം *pX, *pY, *pZ;
      pX = p_വ്യഞ്ജകം->p_ഇടത്;
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
      പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->x.pList->n_വ്യഞ്ജകം==2 );
      pY = p_വ്യഞ്ജകം->x.pList->a[0].p_വ്യഞ്ജകം;
      pZ = p_വ്യഞ്ജകം->x.pList->a[1].p_വ്യഞ്ജകം;
      sqlite3_വൃക്ഷം_View_വരി(pView, "BETWEEN%s", zFlgs);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pX, 1);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pY, 1);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pZ, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_വൃക്ഷം_View_വരി(pView, "%s(%d)", 
          p_വ്യഞ്ജകം->iTable ? "NEW" : "OLD", p_വ്യഞ്ജകം->iColumn);
      നിർത്തുക;
    }
    അവസ്ഥ TK_CASE: {
      sqlite3_വൃക്ഷം_View_വരി(pView, "CASE");
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 1);
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, p_വ്യഞ്ജകം->x.pList, 0, 0);
      നിർത്തുക;
    }
#ifndef SQLITE_OMIT_TRIGGER
    അവസ്ഥ TK_RAISE: {
      സ്ഥിരം അക്ഷരം *zType = "unk";
      തിരഞ്ഞെടുപ്പ്( p_വ്യഞ്ജകം->aff_വ്യഞ്ജകം ){
        അവസ്ഥ OE_Rollback:   zType = "rollback";  നിർത്തുക;
        അവസ്ഥ OE_Abort:      zType = "abort";     നിർത്തുക;
        അവസ്ഥ OE_Fail:       zType = "fail";      നിർത്തുക;
        അവസ്ഥ OE_Ignore:     zType = "ignore";    നിർത്തുക;
      }
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      sqlite3_വൃക്ഷം_View_വരി(pView, "RAISE %s", zType);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 0);
      നിർത്തുക;
    }
#endif
    അവസ്ഥ TK_MATCH: {
      sqlite3_വൃക്ഷം_View_വരി(pView, "MATCH {%d:%d}%s",
                          p_വ്യഞ്ജകം->iTable, p_വ്യഞ്ജകം->iColumn, zFlgs);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_വലത്, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_VECTOR: {
      അക്ഷരം *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
      sqlite3_വൃക്ഷം_View_Bare_വ്യഞ്ജകം_List(pView, p_വ്യഞ്ജകം->x.pList, z);
      sqlite3_free(z);
      നിർത്തുക;
    }
    അവസ്ഥ TK_SELECT_COLUMN: {
      sqlite3_വൃക്ഷം_View_വരി(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_വ്യഞ്ജകം->iColumn, p_വ്യഞ്ജകം->iTable-1,
              p_വ്യഞ്ജകം->p_വലത്==p_വ്യഞ്ജകം->p_ഇടത് ? " (SELECT-owner)" : "");
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം->p_ഇടത്) );
      sqlite3_വൃക്ഷം_View_Select(pView, p_വ്യഞ്ജകം->p_ഇടത്->x.pSelect, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_IF_NULL_ROW: {
      sqlite3_വൃക്ഷം_View_വരി(pView, "IF-NULL-ROW %d", p_വ്യഞ്ജകം->iTable);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_ERROR: {
      വ്യഞ്ജകം tmp;
      sqlite3_വൃക്ഷം_View_വരി(pView, "ERROR");
      tmp = *p_വ്യഞ്ജകം;
      tmp.op = p_വ്യഞ്ജകം->op2;
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, &tmp, 0);
      നിർത്തുക;
    }
    അവസ്ഥ TK_ROW: {
      എങ്കിൽ( p_വ്യഞ്ജകം->iColumn<=0 ){
        sqlite3_വൃക്ഷം_View_വരി(pView, "First FROM table rowid");
      }അല്ലെങ്കിൽ{
        sqlite3_വൃക്ഷം_View_വരി(pView, "First FROM table column %d",
            p_വ്യഞ്ജകം->iColumn-1);
      }
      നിർത്തുക;
    }
    സ്ഥിരസ്ഥിതി: {
      sqlite3_വൃക്ഷം_View_വരി(pView, "op=%d", p_വ്യഞ്ജകം->op);
      നിർത്തുക;
    }
  }
  എങ്കിൽ( zBinOp ){
    sqlite3_വൃക്ഷം_View_വരി(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 1);
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_വലത്, 0);
  }അല്ലെങ്കിൽ എങ്കിൽ( zUniOp ){
    sqlite3_വൃക്ഷം_View_വരി(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, p_വ്യഞ്ജകം->p_ഇടത്, 0);
  }
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Bare_വ്യഞ്ജകം_List(
  വൃക്ഷം_View *pView,
  സ്ഥിരം വ്യഞ്ജകം_List *pList,
  സ്ഥിരം അക്ഷരം *zLabel
){
  എങ്കിൽ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  എങ്കിൽ( pList==0 ){
    sqlite3_വൃക്ഷം_View_വരി(pView, "%s (empty)", zLabel);
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ i;
    sqlite3_വൃക്ഷം_View_വരി(pView, "%s", zLabel);
    വേണ്ടി(i=0; i<pList->n_വ്യഞ്ജകം; i++){
      പൂർണ്ണസംഖ്യ j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      അക്ഷരം *zName = pList->a[i].zEName;
      പൂർണ്ണസംഖ്യ moreToFollow = i<pList->n_വ്യഞ്ജകം - 1;
      എങ്കിൽ( j || zName || sortFlags ){
        sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_വൃക്ഷം_View_വരി(pView, 0);
        എങ്കിൽ( zName ){
          തിരഞ്ഞെടുപ്പ്( pList->a[i].fg.eEName ){
            സ്ഥിരസ്ഥിതി:
              fprintf(stdout, "AS %s ", zName);
              നിർത്തുക;
            അവസ്ഥ ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              എങ്കിൽ( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              എങ്കിൽ( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              എങ്കിൽ( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              നിർത്തുക;
            അവസ്ഥ ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              നിർത്തുക;
          }
        }
        എങ്കിൽ( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        എങ്കിൽ( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }അല്ലെങ്കിൽ എങ്കിൽ( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pList->a[i].p_വ്യഞ്ജകം, moreToFollow);
      എങ്കിൽ( j || zName || sortFlags ){
        sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
      }
    }
  }
}
ശൂന്യം sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(
  വൃക്ഷം_View *pView,
  സ്ഥിരം വ്യഞ്ജകം_List *pList,
  u8 moreToFollow,
  സ്ഥിരം അക്ഷരം *zLabel
){
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, moreToFollow);
  sqlite3_വൃക്ഷം_View_Bare_വ്യഞ്ജകം_List(pView, pList, zLabel);
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Bare_Id_List(
  വൃക്ഷം_View *pView,
  സ്ഥിരം IdList *pList,
  സ്ഥിരം അക്ഷരം *zLabel
){
  എങ്കിൽ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  എങ്കിൽ( pList==0 ){
    sqlite3_വൃക്ഷം_View_വരി(pView, "%s (empty)", zLabel);
  }അല്ലെങ്കിൽ{
    പൂർണ്ണസംഖ്യ i;
    sqlite3_വൃക്ഷം_View_വരി(pView, "%s", zLabel);
    വേണ്ടി(i=0; i<pList->nId; i++){
      അക്ഷരം *zName = pList->a[i].zName;
      പൂർണ്ണസംഖ്യ moreToFollow = i<pList->nId - 1;
      എങ്കിൽ( zName==0 ) zName = "(null)";
      sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, moreToFollow);
      sqlite3_വൃക്ഷം_View_വരി(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
  }
}
ശൂന്യം sqlite3_വൃക്ഷം_View_Id_List(
  വൃക്ഷം_View *pView,
  സ്ഥിരം IdList *pList,
  u8 moreToFollow,
  സ്ഥിരം അക്ഷരം *zLabel
){
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, moreToFollow);
  sqlite3_വൃക്ഷം_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Upsert(
  വൃക്ഷം_View *pView,
  സ്ഥിരം Upsert *pUpsert,
  u8 moreToFollow
){
  എങ്കിൽ( pUpsert==0 ) തിരികെ;
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, moreToFollow);
  ആകുമ്പോൾ( pUpsert ){
    പൂർണ്ണസംഖ്യ n;
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_വൃക്ഷം_View_വരി(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    എങ്കിൽ( pUpsert->pUpsertWhere ){
      sqlite3_വൃക്ഷം_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    }
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}

#എങ്കിൽ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Delete(
  സ്ഥിരം With *pWith,
  സ്ഥിരം SrcList *pTabList,
  സ്ഥിരം വ്യഞ്ജകം *pWhere,
  സ്ഥിരം വ്യഞ്ജകം_List *pOrderBy,
  സ്ഥിരം വ്യഞ്ജകം *pLimit,
  സ്ഥിരം Trigger *pTrigger
){
  പൂർണ്ണസംഖ്യ n = 0;
  വൃക്ഷം_View *pView = 0;
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 0);
  sqlite3_വൃക്ഷം_View_വരി(pView, "DELETE");
  എങ്കിൽ( pWith ) n++;
  എങ്കിൽ( pTabList ) n++;
  എങ്കിൽ( pWhere ) n++;
  എങ്കിൽ( pOrderBy ) n++;
  എങ്കിൽ( pLimit ) n++;
  എങ്കിൽ( pTrigger ) n++;
  എങ്കിൽ( pWith ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_With(pView, pWith, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pTabList ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "FROM");
    sqlite3_വൃക്ഷം_View_Src_List(pView, pTabList);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pWhere ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "WHERE");
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pWhere, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pOrderBy ){
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  എങ്കിൽ( pLimit ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "LIMIT");
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pLimit, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pTrigger ){
    sqlite3_വൃക്ഷം_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}
#endif /* TREETRACE_ENABLED */

#എങ്കിൽ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_തിരുകുക(
  സ്ഥിരം With *pWith,
  സ്ഥിരം SrcList *pTabList,
  സ്ഥിരം IdList *pColumnList,
  സ്ഥിരം Select *pSelect,
  സ്ഥിരം വ്യഞ്ജകം_List *p_വ്യഞ്ജകം_List,
  പൂർണ്ണസംഖ്യ onError,
  സ്ഥിരം Upsert *pUpsert,
  സ്ഥിരം Trigger *pTrigger
){
  വൃക്ഷം_View *pView = 0;
  പൂർണ്ണസംഖ്യ n = 0;
  സ്ഥിരം അക്ഷരം *zLabel = "INSERT";
  തിരഞ്ഞെടുപ്പ്( onError ){
    അവസ്ഥ OE_Replace:  zLabel = "REPLACE";             നിർത്തുക;
    അവസ്ഥ OE_Ignore:   zLabel = "INSERT OR IGNORE";    നിർത്തുക;
    അവസ്ഥ OE_Rollback: zLabel = "INSERT OR ROLLBACK";  നിർത്തുക;
    അവസ്ഥ OE_Abort:    zLabel = "INSERT OR ABORT";     നിർത്തുക;
    അവസ്ഥ OE_Fail:     zLabel = "INSERT OR FAIL";      നിർത്തുക;
  }
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 0);
  sqlite3_വൃക്ഷം_View_വരി(pView, zLabel);
  എങ്കിൽ( pWith ) n++;
  എങ്കിൽ( pTabList ) n++;
  എങ്കിൽ( pColumnList ) n++;
  എങ്കിൽ( pSelect ) n++;
  എങ്കിൽ( p_വ്യഞ്ജകം_List ) n++;
  എങ്കിൽ( pUpsert ) n++;
  എങ്കിൽ( pTrigger ) n++;
  എങ്കിൽ( pWith ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_With(pView, pWith, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pTabList ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "INTO");
    sqlite3_വൃക്ഷം_View_Src_List(pView, pTabList);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pColumnList ){
    sqlite3_വൃക്ഷം_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  എങ്കിൽ( pSelect ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "DATA-SOURCE");
    sqlite3_വൃക്ഷം_View_Select(pView, pSelect, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( p_വ്യഞ്ജകം_List ){
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, p_വ്യഞ്ജകം_List, (--n)>0, "VALUES");
  }
  എങ്കിൽ( pUpsert ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "UPSERT");
    sqlite3_വൃക്ഷം_View_Upsert(pView, pUpsert, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pTrigger ){
    sqlite3_വൃക്ഷം_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}
#endif /* TREETRACE_ENABLED */

#എങ്കിൽ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Update(
  സ്ഥിരം With *pWith,
  സ്ഥിരം SrcList *pTabList,
  സ്ഥിരം വ്യഞ്ജകം_List *pChanges,
  സ്ഥിരം വ്യഞ്ജകം *pWhere,
  പൂർണ്ണസംഖ്യ onError,
  സ്ഥിരം വ്യഞ്ജകം_List *pOrderBy,
  സ്ഥിരം വ്യഞ്ജകം *pLimit,
  സ്ഥിരം Upsert *pUpsert,
  സ്ഥിരം Trigger *pTrigger
){
  പൂർണ്ണസംഖ്യ n = 0;
  വൃക്ഷം_View *pView = 0;
  സ്ഥിരം അക്ഷരം *zLabel = "UPDATE";
  തിരഞ്ഞെടുപ്പ്( onError ){
    അവസ്ഥ OE_Replace:  zLabel = "UPDATE OR REPLACE";   നിർത്തുക;
    അവസ്ഥ OE_Ignore:   zLabel = "UPDATE OR IGNORE";    നിർത്തുക;
    അവസ്ഥ OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  നിർത്തുക;
    അവസ്ഥ OE_Abort:    zLabel = "UPDATE OR ABORT";     നിർത്തുക;
    അവസ്ഥ OE_Fail:     zLabel = "UPDATE OR FAIL";      നിർത്തുക;
  }
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 0);
  sqlite3_വൃക്ഷം_View_വരി(pView, zLabel);
  എങ്കിൽ( pWith ) n++;
  എങ്കിൽ( pTabList ) n++;
  എങ്കിൽ( pChanges ) n++;
  എങ്കിൽ( pWhere ) n++;
  എങ്കിൽ( pOrderBy ) n++;
  എങ്കിൽ( pLimit ) n++;
  എങ്കിൽ( pUpsert ) n++;
  എങ്കിൽ( pTrigger ) n++;
  എങ്കിൽ( pWith ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_With(pView, pWith, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pTabList ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "FROM");
    sqlite3_വൃക്ഷം_View_Src_List(pView, pTabList);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pChanges ){
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pChanges, (--n)>0, "SET");
  }
  എങ്കിൽ( pWhere ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "WHERE");
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pWhere, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pOrderBy ){
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  എങ്കിൽ( pLimit ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "LIMIT");
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(pView, pLimit, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pUpsert ){
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, (--n)>0);
    sqlite3_വൃക്ഷം_View_വരി(pView, "UPSERT");
    sqlite3_വൃക്ഷം_View_Upsert(pView, pUpsert, 0);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }
  എങ്കിൽ( pTrigger ){
    sqlite3_വൃക്ഷം_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Trigger_Step(
  വൃക്ഷം_View *pView,
  സ്ഥിരം TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  പൂർണ്ണസംഖ്യ cnt = 0;
  എങ്കിൽ( pStep==0 ) തിരികെ;
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  ചെയ്യുക{
    എങ്കിൽ( cnt++ && pStep->pNext==0 ){
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
      sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 0);
    }
    sqlite3_വൃക്ഷം_View_വരി(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }ആകുമ്പോൾ( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
ശൂന്യം sqlite3_വൃക്ഷം_View_Trigger(
  വൃക്ഷം_View *pView,
  സ്ഥിരം Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  പൂർണ്ണസംഖ്യ cnt = 0;
  എങ്കിൽ( pTrigger==0 ) തിരികെ;
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  ചെയ്യുക{
    എങ്കിൽ( cnt++ && pTrigger->pNext==0 ){
      sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
      sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 0);
    }
    sqlite3_വൃക്ഷം_View_വരി(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 0);
    sqlite3_വൃക്ഷം_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
  }ആകുമ്പോൾ( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
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
ശൂന്യം sqlite3_Show_വ്യഞ്ജകം(സ്ഥിരം വ്യഞ്ജകം *p){ sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(0,p,0); }
ശൂന്യം sqlite3_Show_വ്യഞ്ജകം_List(സ്ഥിരം വ്യഞ്ജകം_List *p){ sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം_List(0,p,0,0);}
ശൂന്യം sqlite3ShowIdList(സ്ഥിരം IdList *p){ sqlite3_വൃക്ഷം_View_Id_List(0,p,0,0); }
ശൂന്യം sqlite3ShowSrcList(സ്ഥിരം SrcList *p){
  വൃക്ഷം_View *pView = 0;
  sqlite3_വൃക്ഷം_View_തള്ളുക(&pView, 0);
  sqlite3_വൃക്ഷം_View_വരി(pView, "SRCLIST");
  sqlite3_വൃക്ഷം_View_Src_List(pView,p);
  sqlite3_വൃക്ഷം_View_എടുക്കുക(&pView);
}
ശൂന്യം sqlite3ShowSelect(സ്ഥിരം Select *p){ sqlite3_വൃക്ഷം_View_Select(0,p,0); }
ശൂന്യം sqlite3ShowWith(സ്ഥിരം With *p){ sqlite3_വൃക്ഷം_View_With(0,p,0); }
ശൂന്യം sqlite3ShowUpsert(സ്ഥിരം Upsert *p){ sqlite3_വൃക്ഷം_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
ശൂന്യം sqlite3ShowTriggerStep(സ്ഥിരം TriggerStep *p){
  sqlite3_വൃക്ഷം_View_Trigger_Step(0,p,0,0);
}
ശൂന്യം sqlite3ShowTriggerStepList(സ്ഥിരം TriggerStep *p){
  sqlite3_വൃക്ഷം_View_Trigger_Step(0,p,0,1);
}
ശൂന്യം sqlite3ShowTrigger(സ്ഥിരം Trigger *p){ sqlite3_വൃക്ഷം_View_Trigger(0,p,0,0); }
ശൂന്യം sqlite3ShowTriggerList(സ്ഥിരം Trigger *p){ sqlite3_വൃക്ഷം_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
ശൂന്യം sqlite3ShowWindow(സ്ഥിരം Window *p){ sqlite3_വൃക്ഷം_View_Window(0,p,0); }
ശൂന്യം sqlite3ShowWinFunc(സ്ഥിരം Window *p){ sqlite3_വൃക്ഷം_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
