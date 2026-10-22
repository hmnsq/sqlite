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
ساكن فراغ sqlite3_بشيا_View_تور(بشيا_View **pp, u8 moreToFollow){
  بشيا_View *p = *pp;
  إذا( p==0 ){
    *pp = p = sqlite3_malloc64( حجم(*p) );
    إذا( p==0 ) أرجع;
    memset(p, 0, حجم(*p));
  }وإلا{
    p->iLevel++;
  }
  أكد( moreToFollow==0 || moreToFollow==1 );
  إذا( p->iLevel<(صحيح)حجم(p->b_لي) ) p->b_لي[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
ساكن فراغ sqlite3_بشيا_View_چرو(بشيا_View **pp){
  بشيا_View *p = *pp;
  إذا( p==0 ) أرجع;
  p->iLevel--;
  إذا( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
فراغ sqlite3_بشيا_View_لي(بشيا_View *p, ثابت حرف *zFormat, ...){
  va_list ap;
  صحيح i;
  StrAccum acc;
  حرف zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
  إذا( p ){
    لكل(i=0; i<p->iLevel && i<(صحيح)حجم(p->b_لي)-1; i++){
      sqlite3_str_append(&acc, p->b_لي[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_لي[i] ? "|-- " : "'-- ", 4);
  }
  إذا( zFormat!=0 ){
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
ساكن فراغ sqlite3_بشيا_View_Item(بشيا_View *p, ثابت حرف *zLabel,u8 moreFollows){
  sqlite3_بشيا_View_تور(&p, moreFollows);
  sqlite3_بشيا_View_لي(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
فراغ sqlite3_بشيا_View_Column_List(
  بشيا_View *pView,
  ثابت Column *aCol,
  صحيح nCol,
  u8 moreToFollow
){
  صحيح i;
  sqlite3_بشيا_View_تور(&pView, moreToFollow);
  sqlite3_بشيا_View_لي(pView, "COLUMNS");
  لكل(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    صحيح colMoreToFollow = i<(nCol - 1);
    sqlite3_بشيا_View_تور(&pView, colMoreToFollow);
    sqlite3_بشيا_View_لي(pView, 0);
    printf(" %s", aCol[i].zCnName);
    بدل( aCol[i].eCType ){
      حالة COLTYPE_ANY:      printf(" ANY");        اكسر;
      حالة COLTYPE_BLOB:     printf(" BLOB");       اكسر;
      حالة COLTYPE_INT:      printf(" INT");        اكسر;
      حالة COLTYPE_INTEGER:  printf(" INTEGER");    اكسر;
      حالة COLTYPE_REAL:     printf(" REAL");       اكسر;
      حالة COLTYPE_TEXT:     printf(" TEXT");       اكسر;
      حالة COLTYPE_CUSTOM: {
        إذا( flg & COLFLAG_HASTYPE ){
          ثابت حرف *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          اكسر;
        }
      }
    }
    إذا( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    إذا( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    إذا( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    إذا( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_بشيا_View_چرو(&pView);
  }
  sqlite3_بشيا_View_چرو(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
فراغ sqlite3_بشيا_View_With(بشيا_View *pView, ثابت With *pWith, u8 moreToFollow){
  صحيح i;
  إذا( pWith==0 ) أرجع;
  إذا( pWith->nCte==0 ) أرجع;
  إذا( pWith->pOuter ){
    sqlite3_بشيا_View_لي(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }وإلا{
    sqlite3_بشيا_View_لي(pView, "WITH (0x%p)", pWith);
  }
  إذا( pWith->nCte>0 ){
    sqlite3_بشيا_View_تور(&pView, moreToFollow);
    لكل(i=0; i<pWith->nCte; i++){
      StrAccum x;
      حرف z_لي[1000];
      ثابت بنية Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_لي, حجم(z_لي), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      إذا( pCte->pCols && pCte->pCols->n_فرچ>0 ){
        حرف cSep = '(';
        صحيح j;
        لكل(j=0; j<pCte->pCols->n_فرچ; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      إذا( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      إذا( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_بشيا_View_Item(pView, z_لي, i<pWith->nCte-1);
      sqlite3_بشيا_View_Select(pView, pCte->pSelect, 0);
      sqlite3_بشيا_View_چرو(&pView);
    }
    sqlite3_بشيا_View_چرو(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
فراغ sqlite3_بشيا_View_Src_List(بشيا_View *pView, ثابت SrcList *pSrc){
  صحيح i;
  إذا( pSrc==0 ) أرجع;
  لكل(i=0; i<pSrc->nSrc; i++){
    ثابت SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    صحيح n = 0;
    حرف z_لي[1000];
    sqlite3StrAccumInit(&x, 0, z_لي, حجم(z_لي), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    إذا( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    إذا( (pItem->fg.jointype & (JT_هغو|JT_دام))==(JT_هغو|JT_دام) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }وإلا إذا( pItem->fg.jointype & JT_هغو ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }وإلا إذا( pItem->fg.jointype & JT_دام ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }وإلا إذا( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    إذا( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    إذا( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    إذا( pItem->fg.isCte ){
      ساكن ثابت حرف *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    إذا( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    إذا( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    إذا( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    إذا( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    إذا( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    إذا( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    إذا( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    إذا( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    إذا( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    إذا( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_بشيا_View_Item(pView, z_لي, i<pSrc->nSrc-1);
    n = 0;
    إذا( pItem->fg.isSubquery ) n++;
    إذا( pItem->fg.isTabFunc ) n++;
    إذا( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    إذا( pItem->fg.isUsing ){
      sqlite3_بشيا_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }وإلا إذا( pItem->u3.pOn!=0 ){
      sqlite3_بشيا_View_Item(pView, "ON", (--n)>0);
      sqlite3_بشيا_View_فرچ(pView, pItem->u3.pOn, 0);
      sqlite3_بشيا_View_چرو(&pView);
    }
    إذا( pItem->fg.isSubquery ){
      أكد( n==1 );
      إذا( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_بشيا_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      أكد( (صحيح)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_بشيا_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    إذا( pItem->fg.isTabFunc ){
      sqlite3_بشيا_View_فرچ_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_بشيا_View_چرو(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
فراغ sqlite3_بشيا_View_Select(بشيا_View *pView, ثابت Select *p, u8 moreToFollow){
  صحيح n = 0;
  صحيح cnt = 0;
  إذا( p==0 ){
    sqlite3_بشيا_View_لي(pView, "nil-SELECT");
    أرجع;
  } 
  sqlite3_بشيا_View_تور(&pView, moreToFollow);
  إذا( p->pWith ){
    sqlite3_بشيا_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_بشيا_View_تور(&pView, 1);
  }
  افعل{
    إذا( p->selFlags & SF_WhereBegin ){
      sqlite3_بشيا_View_لي(pView, "sqlite3WhereBegin()");
    }وإلا{
      sqlite3_بشيا_View_لي(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (صحيح)p->nSelectRow
      );
    }
    إذا( cnt++ ) sqlite3_بشيا_View_چرو(&pView);
    إذا( p->pPrior ){
      n = 1000;
    }وإلا{
      n = 0;
      إذا( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      إذا( p->pWhere ) n++;
      إذا( p->pGroupBy ) n++;
      إذا( p->pHaving ) n++;
      إذا( p->pOrderBy ) n++;
      إذا( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      إذا( p->pWin ) n++;
      إذا( p->pWinDefn ) n++;
#endif
    }
    إذا( p->pEList ){
      sqlite3_بشيا_View_فرچ_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    إذا( p->pWin ){
      Window *pX;
      sqlite3_بشيا_View_تور(&pView, (n--)>0);
      sqlite3_بشيا_View_لي(pView, "window-functions");
      لكل(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_بشيا_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_بشيا_View_چرو(&pView);
    }
#endif
    إذا( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_بشيا_View_تور(&pView, (n--)>0);
      sqlite3_بشيا_View_لي(pView, "FROM");
      sqlite3_بشيا_View_Src_List(pView, p->pSrc);
      sqlite3_بشيا_View_چرو(&pView);
    }
    إذا( p->pWhere ){
      sqlite3_بشيا_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_بشيا_View_فرچ(pView, p->pWhere, 0);
      sqlite3_بشيا_View_چرو(&pView);
    }
    إذا( p->pGroupBy ){
      sqlite3_بشيا_View_فرچ_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    إذا( p->pHaving ){
      sqlite3_بشيا_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_بشيا_View_فرچ(pView, p->pHaving, 0);
      sqlite3_بشيا_View_چرو(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    إذا( p->pWinDefn ){
      Window *pX;
      sqlite3_بشيا_View_Item(pView, "WINDOW", (n--)>0);
      لكل(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_بشيا_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_بشيا_View_چرو(&pView);
    }
#endif
    إذا( p->pOrderBy ){
      sqlite3_بشيا_View_فرچ_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    إذا( p->pLimit ){
      sqlite3_بشيا_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_بشيا_View_فرچ(pView, p->pLimit->p_هغو, p->pLimit->p_دام!=0);
      إذا( p->pLimit->p_دام ){
        sqlite3_بشيا_View_Item(pView, "OFFSET", 0);
        sqlite3_بشيا_View_فرچ(pView, p->pLimit->p_دام, 0);
        sqlite3_بشيا_View_چرو(&pView);
      }
      sqlite3_بشيا_View_چرو(&pView);
    }
    إذا( p->pPrior ){
      ثابت حرف *zOp = "UNION";
      بدل( p->op ){
        حالة TK_ALL:         zOp = "UNION ALL";  اكسر;
        حالة TK_INTERSECT:   zOp = "INTERSECT";  اكسر;
        حالة TK_EXCEPT:      zOp = "EXCEPT";     اكسر;
      }
      sqlite3_بشيا_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }طالما( p!=0 );
  sqlite3_بشيا_View_چرو(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
فراغ sqlite3_بشيا_View_Bound(
  بشيا_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  فرچ *p_فرچ,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  بدل( eBound ){
    حالة TK_UNBOUNDED: {
      sqlite3_بشيا_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_بشيا_View_چرو(&pView);
      اكسر;
    }
    حالة TK_CURRENT: {
      sqlite3_بشيا_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_بشيا_View_چرو(&pView);
      اكسر;
    }
    حالة TK_PRECEDING: {
      sqlite3_بشيا_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_بشيا_View_فرچ(pView, p_فرچ, 0);
      sqlite3_بشيا_View_چرو(&pView);
      اكسر;
    }
    حالة TK_FOLLOWING: {
      sqlite3_بشيا_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_بشيا_View_فرچ(pView, p_فرچ, 0);
      sqlite3_بشيا_View_چرو(&pView);
      اكسر;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
فراغ sqlite3_بشيا_View_Window(بشيا_View *pView, ثابت Window *pWin, u8 more){
  صحيح nElement = 0;
  إذا( pWin==0 ) أرجع;
  إذا( pWin->pFilter ){
    sqlite3_بشيا_View_Item(pView, "FILTER", 1);
    sqlite3_بشيا_View_فرچ(pView, pWin->pFilter, 0);
    sqlite3_بشيا_View_چرو(&pView);
    إذا( pWin->eFrmType==TK_FILTER ) أرجع;
  }
  sqlite3_بشيا_View_تور(&pView, more);
  إذا( pWin->zName ){
    sqlite3_بشيا_View_لي(pView, "OVER %s (%p)", pWin->zName, pWin);
  }وإلا{
    sqlite3_بشيا_View_لي(pView, "OVER (%p)", pWin);
  }
  إذا( pWin->zBase )    nElement++;
  إذا( pWin->pOrderBy ) nElement++;
  إذا( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  إذا( pWin->eExclude ) nElement++;
  إذا( pWin->zBase ){
    sqlite3_بشيا_View_تور(&pView, (--nElement)>0);
    sqlite3_بشيا_View_لي(pView, "window: %s", pWin->zBase);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pWin->pPartition ){
    sqlite3_بشيا_View_فرچ_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  إذا( pWin->pOrderBy ){
    sqlite3_بشيا_View_فرچ_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  إذا( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    حرف zBuf[30];
    ثابت حرف *zFrmType = "ROWS";
    إذا( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    إذا( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(حجم(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_بشيا_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_بشيا_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_بشيا_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pWin->eExclude ){
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
    sqlite3_بشيا_View_تور(&pView, 0);
    sqlite3_بشيا_View_لي(pView, "EXCLUDE %s", zExclude);
    sqlite3_بشيا_View_چرو(&pView);
  }
  sqlite3_بشيا_View_چرو(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
فراغ sqlite3_بشيا_View_Win_Func(بشيا_View *pView, ثابت Window *pWin, u8 more){
  إذا( pWin==0 ) أرجع;
  sqlite3_بشيا_View_تور(&pView, more);
  sqlite3_بشيا_View_لي(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_بشيا_View_Window(pView, pWin, 0);
  sqlite3_بشيا_View_چرو(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
فراغ sqlite3_بشيا_View_فرچ(بشيا_View *pView, ثابت فرچ *p_فرچ, u8 moreToFollow){
  ثابت حرف *zBinOp = 0;   /* Binary operator */
  ثابت حرف *zUniOp = 0;   /* Unary operator */
  حرف zFlgs[200];
  sqlite3_بشيا_View_تور(&pView, moreToFollow);
  إذا( p_فرچ==0 ){
    sqlite3_بشيا_View_لي(pView, "nil");
    sqlite3_بشيا_View_چرو(&pView);
    أرجع;
  }
  إذا( p_فرچ->flags || p_فرچ->aff_فرچ || p_فرچ->vvaFlags || p_فرچ->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, حجم(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_فرچ->flags, p_فرچ->aff_فرچ ? p_فرچ->aff_فرچ : 'n');
    إذا( فرچ_Has_Property(p_فرچ, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_فرچ->w.iJoin);
    }
    إذا( فرچ_Has_Property(p_فرچ, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_فرچ->w.iJoin);
    }
    إذا( فرچ_Has_Property(p_فرچ, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    إذا( فرچ_Has_VVAProperty(p_فرچ, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    إذا( p_فرچ->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_فرچ->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }وإلا{
    zFlgs[0] = 0;
  }
  بدل( p_فرچ->op ){
    حالة TK_AGG_COLUMN: {
      sqlite3_بشيا_View_لي(pView, "AGG{%d:%d}%s",
            p_فرچ->iTable, p_فرچ->iColumn, zFlgs);
      اكسر;
    }
    حالة TK_COLUMN: {
      إذا( p_فرچ->iTable<0 ){
        /* This only happens when coding check constraints */
        حرف zOp2[16];
        إذا( p_فرچ->op2 ){
          sqlite3_snprintf(حجم(zOp2),zOp2," op2=0x%02x",p_فرچ->op2);
        }وإلا{
          zOp2[0] = 0;
        }
        sqlite3_بشيا_View_لي(pView, "COLUMN(%d)%s%s",
                                    p_فرچ->iColumn, zFlgs, zOp2);
      }وإلا{
        أكد( فرچ_Use_YTab(p_فرچ) );
        sqlite3_بشيا_View_لي(pView, "{%d:%d} pTab=%p%s",
                        p_فرچ->iTable, p_فرچ->iColumn,
                        p_فرچ->y.pTab, zFlgs);
      }
      إذا( فرچ_Has_Property(p_فرچ, EP_FixedCol) ){
        sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 0);
      }
      اكسر;
    }
    حالة TK_INTEGER: {
      إذا( p_فرچ->flags & EP_Int_درج ){
        sqlite3_بشيا_View_لي(pView, "%d", p_فرچ->u.i_درج);
      }وإلا{
        sqlite3_بشيا_View_لي(pView, "%s", p_فرچ->u.zToken);
      }
      اكسر;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    حالة TK_FLOAT: {
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      sqlite3_بشيا_View_لي(pView,"%s", p_فرچ->u.zToken);
      اكسر;
    }
#endif
    حالة TK_STRING: {
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      sqlite3_بشيا_View_لي(pView,"%Q", p_فرچ->u.zToken);
      اكسر;
    }
    حالة TK_NULL: {
      sqlite3_بشيا_View_لي(pView,"NULL");
      اكسر;
    }
    حالة TK_TRUEFALSE: {
      sqlite3_بشيا_View_لي(pView,"%s%s",
         sqlite3_فرچ_Truth_درج(p_فرچ) ? "TRUE" : "FALSE", zFlgs);
      اكسر;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    حالة TK_BLOB: {
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      sqlite3_بشيا_View_لي(pView,"%s", p_فرچ->u.zToken);
      اكسر;
    }
#endif
    حالة TK_VARIABLE: {
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      sqlite3_بشيا_View_لي(pView,"VARIABLE(%s,%d)",
                          p_فرچ->u.zToken, p_فرچ->iColumn);
      اكسر;
    }
    حالة TK_REGISTER: {
      sqlite3_بشيا_View_لي(pView,"REGISTER(%d)", p_فرچ->iTable);
      اكسر;
    }
    حالة TK_ID: {
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      sqlite3_بشيا_View_لي(pView,"ID \"%w\"", p_فرچ->u.zToken);
      اكسر;
    }
    حالة TK_ASTERISK: {
      sqlite3_بشيا_View_لي(pView,"ASKTERISK");
      اكسر;
    }
#ifndef SQLITE_OMIT_CAST
    حالة TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      sqlite3_بشيا_View_لي(pView,"CAST %Q", p_فرچ->u.zToken);
      sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 0);
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
      أكد( p_فرچ->op2==TK_IS || p_فرچ->op2==TK_ISNOT );
      أكد( p_فرچ->p_دام );
      أكد( sqlite3_فرچ_Skip_Collate_And_Likely(p_فرچ->p_دام)->op
                  == TK_TRUEFALSE );
      x = (p_فرچ->op2==TK_ISNOT)*2 + sqlite3_فرچ_Truth_درج(p_فرچ->p_دام);
      zUniOp = azOp[x];
      اكسر;
    }

    حالة TK_SPAN: {
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      sqlite3_بشيا_View_لي(pView, "SPAN %Q", p_فرچ->u.zToken);
      sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 0);
      اكسر;
    }

    حالة TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      sqlite3_بشيا_View_لي(pView, "%sCOLLATE %Q%s",
        !فرچ_Has_Property(p_فرچ, EP_Collate) ? "SOFT-" : "",
        p_فرچ->u.zToken, zFlgs);
      sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 0);
      اكسر;
    }

    حالة TK_AGG_FUNCTION:
    حالة TK_FUNCTION: {
      فرچ_List *pFarg;       /* List of function arguments */
      Window *pWin;
      إذا( فرچ_Has_Property(p_فرچ, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }وإلا{
        أكد( فرچ_Use_XList(p_فرچ) );
        pFarg = p_فرچ->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_فرچ) ? p_فرچ->y.pWin : 0;
#وإلا
        pWin = 0;
#endif 
      }
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      إذا( p_فرچ->op==TK_AGG_FUNCTION ){
        sqlite3_بشيا_View_لي(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_فرچ->op2, p_فرچ->u.zToken, zFlgs,
                             p_فرچ->pAggInfo ? p_فرچ->pAggInfo->selId : 0,
                             p_فرچ->iAgg, p_فرچ->pAggInfo);
      }وإلا إذا( p_فرچ->op2!=0 ){
        ثابت حرف *zOp2;
        حرف zBuf[8];
        sqlite3_snprintf(حجم(zBuf),zBuf,"0x%02x",p_فرچ->op2);
        zOp2 = zBuf;
        إذا( p_فرچ->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        إذا( p_فرچ->op2==NC_Idx_فرچ ) zOp2 = "NC_IdxExpr";
        إذا( p_فرچ->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        إذا( p_فرچ->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_بشيا_View_لي(pView, "FUNCTION %Q%s op2=%s",
                            p_فرچ->u.zToken, zFlgs, zOp2);
      }وإلا{
        sqlite3_بشيا_View_لي(pView, "FUNCTION %Q%s", p_فرچ->u.zToken, zFlgs);
      }
      إذا( pFarg ){
        sqlite3_بشيا_View_فرچ_List(pView, pFarg, pWin!=0 || p_فرچ->p_هغو, 0);
        إذا( p_فرچ->p_هغو ){
          فرچ *pOB = p_فرچ->p_هغو;
          أكد( pOB->op==TK_ORDER );
          أكد( فرچ_Use_XList(pOB) );
          sqlite3_بشيا_View_فرچ_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      إذا( pWin ){
        sqlite3_بشيا_View_Window(pView, pWin, 0);
      }
#endif
      اكسر;
    }
    حالة TK_ORDER: {
      sqlite3_بشيا_View_فرچ_List(pView, p_فرچ->x.pList, 0, "ORDERBY");
      اكسر;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS: {
      أكد( فرچ_Use_XSelect(p_فرچ) );
      sqlite3_بشيا_View_لي(pView, "EXISTS-expr flags=0x%x", p_فرچ->flags);
      sqlite3_بشيا_View_Select(pView, p_فرچ->x.pSelect, 0);
      اكسر;
    }
    حالة TK_SELECT: {
      أكد( فرچ_Use_XSelect(p_فرچ) );
      sqlite3_بشيا_View_لي(pView, "subquery-expr flags=0x%x", p_فرچ->flags);
      sqlite3_بشيا_View_Select(pView, p_فرچ->x.pSelect, 0);
      اكسر;
    }
    حالة TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      حرف *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_فرچ->flags);
      إذا( p_فرچ->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_فرچ->iTable);
      إذا( فرچ_Has_Property(p_فرچ, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_فرچ->y.sub.regReturn, p_فرچ->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_بشيا_View_لي(pView, z);
      sqlite3_free(z);
      sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 1);
      إذا( فرچ_Use_XSelect(p_فرچ) ){
        sqlite3_بشيا_View_Select(pView, p_فرچ->x.pSelect, 0);
      }وإلا{
        sqlite3_بشيا_View_فرچ_List(pView, p_فرچ->x.pList, 0, 0);
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
      ثابت فرچ *pX, *pY, *pZ;
      pX = p_فرچ->p_هغو;
      أكد( فرچ_Use_XList(p_فرچ) );
      أكد( p_فرچ->x.pList->n_فرچ==2 );
      pY = p_فرچ->x.pList->a[0].p_فرچ;
      pZ = p_فرچ->x.pList->a[1].p_فرچ;
      sqlite3_بشيا_View_لي(pView, "BETWEEN%s", zFlgs);
      sqlite3_بشيا_View_فرچ(pView, pX, 1);
      sqlite3_بشيا_View_فرچ(pView, pY, 1);
      sqlite3_بشيا_View_فرچ(pView, pZ, 0);
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
      sqlite3_بشيا_View_لي(pView, "%s(%d)", 
          p_فرچ->iTable ? "NEW" : "OLD", p_فرچ->iColumn);
      اكسر;
    }
    حالة TK_CASE: {
      sqlite3_بشيا_View_لي(pView, "CASE");
      sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 1);
      أكد( فرچ_Use_XList(p_فرچ) );
      sqlite3_بشيا_View_فرچ_List(pView, p_فرچ->x.pList, 0, 0);
      اكسر;
    }
#ifndef SQLITE_OMIT_TRIGGER
    حالة TK_RAISE: {
      ثابت حرف *zType = "unk";
      بدل( p_فرچ->aff_فرچ ){
        حالة OE_Rollback:   zType = "rollback";  اكسر;
        حالة OE_Abort:      zType = "abort";     اكسر;
        حالة OE_Fail:       zType = "fail";      اكسر;
        حالة OE_Ignore:     zType = "ignore";    اكسر;
      }
      أكد( !فرچ_Has_Property(p_فرچ, EP_Int_درج) );
      sqlite3_بشيا_View_لي(pView, "RAISE %s", zType);
      sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 0);
      اكسر;
    }
#endif
    حالة TK_MATCH: {
      sqlite3_بشيا_View_لي(pView, "MATCH {%d:%d}%s",
                          p_فرچ->iTable, p_فرچ->iColumn, zFlgs);
      sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_دام, 0);
      اكسر;
    }
    حالة TK_VECTOR: {
      حرف *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      أكد( فرچ_Use_XList(p_فرچ) );
      sqlite3_بشيا_View_Bare_فرچ_List(pView, p_فرچ->x.pList, z);
      sqlite3_free(z);
      اكسر;
    }
    حالة TK_SELECT_COLUMN: {
      sqlite3_بشيا_View_لي(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_فرچ->iColumn, p_فرچ->iTable-1,
              p_فرچ->p_دام==p_فرچ->p_هغو ? " (SELECT-owner)" : "");
      أكد( فرچ_Use_XSelect(p_فرچ->p_هغو) );
      sqlite3_بشيا_View_Select(pView, p_فرچ->p_هغو->x.pSelect, 0);
      اكسر;
    }
    حالة TK_IF_NULL_ROW: {
      sqlite3_بشيا_View_لي(pView, "IF-NULL-ROW %d", p_فرچ->iTable);
      sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 0);
      اكسر;
    }
    حالة TK_ERROR: {
      فرچ tmp;
      sqlite3_بشيا_View_لي(pView, "ERROR");
      tmp = *p_فرچ;
      tmp.op = p_فرچ->op2;
      sqlite3_بشيا_View_فرچ(pView, &tmp, 0);
      اكسر;
    }
    حالة TK_ROW: {
      إذا( p_فرچ->iColumn<=0 ){
        sqlite3_بشيا_View_لي(pView, "First FROM table rowid");
      }وإلا{
        sqlite3_بشيا_View_لي(pView, "First FROM table column %d",
            p_فرچ->iColumn-1);
      }
      اكسر;
    }
    افتراضي: {
      sqlite3_بشيا_View_لي(pView, "op=%d", p_فرچ->op);
      اكسر;
    }
  }
  إذا( zBinOp ){
    sqlite3_بشيا_View_لي(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 1);
    sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_دام, 0);
  }وإلا إذا( zUniOp ){
    sqlite3_بشيا_View_لي(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_بشيا_View_فرچ(pView, p_فرچ->p_هغو, 0);
  }
  sqlite3_بشيا_View_چرو(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
فراغ sqlite3_بشيا_View_Bare_فرچ_List(
  بشيا_View *pView,
  ثابت فرچ_List *pList,
  ثابت حرف *zLabel
){
  إذا( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  إذا( pList==0 ){
    sqlite3_بشيا_View_لي(pView, "%s (empty)", zLabel);
  }وإلا{
    صحيح i;
    sqlite3_بشيا_View_لي(pView, "%s", zLabel);
    لكل(i=0; i<pList->n_فرچ; i++){
      صحيح j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      حرف *zName = pList->a[i].zEName;
      صحيح moreToFollow = i<pList->n_فرچ - 1;
      إذا( j || zName || sortFlags ){
        sqlite3_بشيا_View_تور(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_بشيا_View_لي(pView, 0);
        إذا( zName ){
          بدل( pList->a[i].fg.eEName ){
            افتراضي:
              fprintf(stdout, "AS %s ", zName);
              اكسر;
            حالة ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              إذا( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              إذا( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              إذا( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              اكسر;
            حالة ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              اكسر;
          }
        }
        إذا( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        إذا( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }وإلا إذا( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_بشيا_View_فرچ(pView, pList->a[i].p_فرچ, moreToFollow);
      إذا( j || zName || sortFlags ){
        sqlite3_بشيا_View_چرو(&pView);
      }
    }
  }
}
فراغ sqlite3_بشيا_View_فرچ_List(
  بشيا_View *pView,
  ثابت فرچ_List *pList,
  u8 moreToFollow,
  ثابت حرف *zLabel
){
  sqlite3_بشيا_View_تور(&pView, moreToFollow);
  sqlite3_بشيا_View_Bare_فرچ_List(pView, pList, zLabel);
  sqlite3_بشيا_View_چرو(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
فراغ sqlite3_بشيا_View_Bare_Id_List(
  بشيا_View *pView,
  ثابت IdList *pList,
  ثابت حرف *zLabel
){
  إذا( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  إذا( pList==0 ){
    sqlite3_بشيا_View_لي(pView, "%s (empty)", zLabel);
  }وإلا{
    صحيح i;
    sqlite3_بشيا_View_لي(pView, "%s", zLabel);
    لكل(i=0; i<pList->nId; i++){
      حرف *zName = pList->a[i].zName;
      صحيح moreToFollow = i<pList->nId - 1;
      إذا( zName==0 ) zName = "(null)";
      sqlite3_بشيا_View_تور(&pView, moreToFollow);
      sqlite3_بشيا_View_لي(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_بشيا_View_چرو(&pView);
    }
  }
}
فراغ sqlite3_بشيا_View_Id_List(
  بشيا_View *pView,
  ثابت IdList *pList,
  u8 moreToFollow,
  ثابت حرف *zLabel
){
  sqlite3_بشيا_View_تور(&pView, moreToFollow);
  sqlite3_بشيا_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_بشيا_View_چرو(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
فراغ sqlite3_بشيا_View_Upsert(
  بشيا_View *pView,
  ثابت Upsert *pUpsert,
  u8 moreToFollow
){
  إذا( pUpsert==0 ) أرجع;
  sqlite3_بشيا_View_تور(&pView, moreToFollow);
  طالما( pUpsert ){
    صحيح n;
    sqlite3_بشيا_View_تور(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_بشيا_View_لي(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_بشيا_View_فرچ_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_بشيا_View_فرچ_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    إذا( pUpsert->pUpsertWhere ){
      sqlite3_بشيا_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_بشيا_View_فرچ(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_بشيا_View_چرو(&pView);
    }
    sqlite3_بشيا_View_چرو(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_بشيا_View_چرو(&pView);
}

#إذا TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
فراغ sqlite3_بشيا_View_Delete(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت فرچ *pWhere,
  ثابت فرچ_List *pOrderBy,
  ثابت فرچ *pLimit,
  ثابت Trigger *pTrigger
){
  صحيح n = 0;
  بشيا_View *pView = 0;
  sqlite3_بشيا_View_تور(&pView, 0);
  sqlite3_بشيا_View_لي(pView, "DELETE");
  إذا( pWith ) n++;
  إذا( pTabList ) n++;
  إذا( pWhere ) n++;
  إذا( pOrderBy ) n++;
  إذا( pLimit ) n++;
  إذا( pTrigger ) n++;
  إذا( pWith ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_With(pView, pWith, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pTabList ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "FROM");
    sqlite3_بشيا_View_Src_List(pView, pTabList);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pWhere ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "WHERE");
    sqlite3_بشيا_View_فرچ(pView, pWhere, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pOrderBy ){
    sqlite3_بشيا_View_فرچ_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  إذا( pLimit ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "LIMIT");
    sqlite3_بشيا_View_فرچ(pView, pLimit, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pTrigger ){
    sqlite3_بشيا_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_بشيا_View_چرو(&pView);
}
#endif /* TREETRACE_ENABLED */

#إذا TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
فراغ sqlite3_بشيا_View_سك(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت IdList *pColumnList,
  ثابت Select *pSelect,
  ثابت فرچ_List *p_فرچ_List,
  صحيح onError,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  بشيا_View *pView = 0;
  صحيح n = 0;
  ثابت حرف *zLabel = "INSERT";
  بدل( onError ){
    حالة OE_Replace:  zLabel = "REPLACE";             اكسر;
    حالة OE_Ignore:   zLabel = "INSERT OR IGNORE";    اكسر;
    حالة OE_Rollback: zLabel = "INSERT OR ROLLBACK";  اكسر;
    حالة OE_Abort:    zLabel = "INSERT OR ABORT";     اكسر;
    حالة OE_Fail:     zLabel = "INSERT OR FAIL";      اكسر;
  }
  sqlite3_بشيا_View_تور(&pView, 0);
  sqlite3_بشيا_View_لي(pView, zLabel);
  إذا( pWith ) n++;
  إذا( pTabList ) n++;
  إذا( pColumnList ) n++;
  إذا( pSelect ) n++;
  إذا( p_فرچ_List ) n++;
  إذا( pUpsert ) n++;
  إذا( pTrigger ) n++;
  إذا( pWith ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_With(pView, pWith, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pTabList ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "INTO");
    sqlite3_بشيا_View_Src_List(pView, pTabList);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pColumnList ){
    sqlite3_بشيا_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  إذا( pSelect ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "DATA-SOURCE");
    sqlite3_بشيا_View_Select(pView, pSelect, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( p_فرچ_List ){
    sqlite3_بشيا_View_فرچ_List(pView, p_فرچ_List, (--n)>0, "VALUES");
  }
  إذا( pUpsert ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "UPSERT");
    sqlite3_بشيا_View_Upsert(pView, pUpsert, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pTrigger ){
    sqlite3_بشيا_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_بشيا_View_چرو(&pView);
}
#endif /* TREETRACE_ENABLED */

#إذا TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
فراغ sqlite3_بشيا_View_Update(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت فرچ_List *pChanges,
  ثابت فرچ *pWhere,
  صحيح onError,
  ثابت فرچ_List *pOrderBy,
  ثابت فرچ *pLimit,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  صحيح n = 0;
  بشيا_View *pView = 0;
  ثابت حرف *zLabel = "UPDATE";
  بدل( onError ){
    حالة OE_Replace:  zLabel = "UPDATE OR REPLACE";   اكسر;
    حالة OE_Ignore:   zLabel = "UPDATE OR IGNORE";    اكسر;
    حالة OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  اكسر;
    حالة OE_Abort:    zLabel = "UPDATE OR ABORT";     اكسر;
    حالة OE_Fail:     zLabel = "UPDATE OR FAIL";      اكسر;
  }
  sqlite3_بشيا_View_تور(&pView, 0);
  sqlite3_بشيا_View_لي(pView, zLabel);
  إذا( pWith ) n++;
  إذا( pTabList ) n++;
  إذا( pChanges ) n++;
  إذا( pWhere ) n++;
  إذا( pOrderBy ) n++;
  إذا( pLimit ) n++;
  إذا( pUpsert ) n++;
  إذا( pTrigger ) n++;
  إذا( pWith ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_With(pView, pWith, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pTabList ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "FROM");
    sqlite3_بشيا_View_Src_List(pView, pTabList);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pChanges ){
    sqlite3_بشيا_View_فرچ_List(pView, pChanges, (--n)>0, "SET");
  }
  إذا( pWhere ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "WHERE");
    sqlite3_بشيا_View_فرچ(pView, pWhere, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pOrderBy ){
    sqlite3_بشيا_View_فرچ_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  إذا( pLimit ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "LIMIT");
    sqlite3_بشيا_View_فرچ(pView, pLimit, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pUpsert ){
    sqlite3_بشيا_View_تور(&pView, (--n)>0);
    sqlite3_بشيا_View_لي(pView, "UPSERT");
    sqlite3_بشيا_View_Upsert(pView, pUpsert, 0);
    sqlite3_بشيا_View_چرو(&pView);
  }
  إذا( pTrigger ){
    sqlite3_بشيا_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_بشيا_View_چرو(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
فراغ sqlite3_بشيا_View_Trigger_Step(
  بشيا_View *pView,
  ثابت TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  صحيح cnt = 0;
  إذا( pStep==0 ) أرجع;
  sqlite3_بشيا_View_تور(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  افعل{
    إذا( cnt++ && pStep->pNext==0 ){
      sqlite3_بشيا_View_چرو(&pView);
      sqlite3_بشيا_View_تور(&pView, 0);
    }
    sqlite3_بشيا_View_لي(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }طالما( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_بشيا_View_چرو(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
فراغ sqlite3_بشيا_View_Trigger(
  بشيا_View *pView,
  ثابت Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  صحيح cnt = 0;
  إذا( pTrigger==0 ) أرجع;
  sqlite3_بشيا_View_تور(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  افعل{
    إذا( cnt++ && pTrigger->pNext==0 ){
      sqlite3_بشيا_View_چرو(&pView);
      sqlite3_بشيا_View_تور(&pView, 0);
    }
    sqlite3_بشيا_View_لي(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_بشيا_View_تور(&pView, 0);
    sqlite3_بشيا_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_بشيا_View_چرو(&pView);
  }طالما( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_بشيا_View_چرو(&pView);
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
فراغ sqlite3_Show_فرچ(ثابت فرچ *p){ sqlite3_بشيا_View_فرچ(0,p,0); }
فراغ sqlite3_Show_فرچ_List(ثابت فرچ_List *p){ sqlite3_بشيا_View_فرچ_List(0,p,0,0);}
فراغ sqlite3ShowIdList(ثابت IdList *p){ sqlite3_بشيا_View_Id_List(0,p,0,0); }
فراغ sqlite3ShowSrcList(ثابت SrcList *p){
  بشيا_View *pView = 0;
  sqlite3_بشيا_View_تور(&pView, 0);
  sqlite3_بشيا_View_لي(pView, "SRCLIST");
  sqlite3_بشيا_View_Src_List(pView,p);
  sqlite3_بشيا_View_چرو(&pView);
}
فراغ sqlite3ShowSelect(ثابت Select *p){ sqlite3_بشيا_View_Select(0,p,0); }
فراغ sqlite3ShowWith(ثابت With *p){ sqlite3_بشيا_View_With(0,p,0); }
فراغ sqlite3ShowUpsert(ثابت Upsert *p){ sqlite3_بشيا_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
فراغ sqlite3ShowTriggerStep(ثابت TriggerStep *p){
  sqlite3_بشيا_View_Trigger_Step(0,p,0,0);
}
فراغ sqlite3ShowTriggerStepList(ثابت TriggerStep *p){
  sqlite3_بشيا_View_Trigger_Step(0,p,0,1);
}
فراغ sqlite3ShowTrigger(ثابت Trigger *p){ sqlite3_بشيا_View_Trigger(0,p,0,0); }
فراغ sqlite3ShowTriggerList(ثابت Trigger *p){ sqlite3_بشيا_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
فراغ sqlite3ShowWindow(ثابت Window *p){ sqlite3_بشيا_View_Window(0,p,0); }
فراغ sqlite3ShowWinFunc(ثابت Window *p){ sqlite3_بشيا_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
