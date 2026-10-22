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
#شامل "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
ایستا تهی sqlite3_درخت_View_افزودن(درخت_View **pp, u8 moreToFollow){
  درخت_View *p = *pp;
  اگر( p==0 ){
    *pp = p = sqlite3_malloc64( اندازه(*p) );
    اگر( p==0 ) بازگشت;
    memset(p, 0, اندازه(*p));
  }وگرنه{
    p->iLevel++;
  }
  أكد( moreToFollow==0 || moreToFollow==1 );
  اگر( p->iLevel<(صحیح)اندازه(p->b_سطر) ) p->b_سطر[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
ایستا تهی sqlite3_درخت_View_برداشت(درخت_View **pp){
  درخت_View *p = *pp;
  اگر( p==0 ) بازگشت;
  p->iLevel--;
  اگر( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
تهی sqlite3_درخت_View_سطر(درخت_View *p, ثابت نویسه *zFormat, ...){
  va_list ap;
  صحیح i;
  StrAccum acc;
  نویسه zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, اندازه(zBuf), 0);
  اگر( p ){
    برای(i=0; i<p->iLevel && i<(صحیح)اندازه(p->b_سطر)-1; i++){
      sqlite3_str_append(&acc, p->b_سطر[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_سطر[i] ? "|-- " : "'-- ", 4);
  }
  اگر( zFormat!=0 ){
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
ایستا تهی sqlite3_درخت_View_Item(درخت_View *p, ثابت نویسه *zLabel,u8 moreFollows){
  sqlite3_درخت_View_افزودن(&p, moreFollows);
  sqlite3_درخت_View_سطر(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
تهی sqlite3_درخت_View_Column_List(
  درخت_View *pView,
  ثابت Column *aCol,
  صحیح nCol,
  u8 moreToFollow
){
  صحیح i;
  sqlite3_درخت_View_افزودن(&pView, moreToFollow);
  sqlite3_درخت_View_سطر(pView, "COLUMNS");
  برای(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    صحیح colMoreToFollow = i<(nCol - 1);
    sqlite3_درخت_View_افزودن(&pView, colMoreToFollow);
    sqlite3_درخت_View_سطر(pView, 0);
    printf(" %s", aCol[i].zCnName);
    انتخاب( aCol[i].eCType ){
      حالت COLTYPE_ANY:      printf(" ANY");        بشکن;
      حالت COLTYPE_BLOB:     printf(" BLOB");       بشکن;
      حالت COLTYPE_INT:      printf(" INT");        بشکن;
      حالت COLTYPE_INTEGER:  printf(" INTEGER");    بشکن;
      حالت COLTYPE_REAL:     printf(" REAL");       بشکن;
      حالت COLTYPE_TEXT:     printf(" TEXT");       بشکن;
      حالت COLTYPE_CUSTOM: {
        اگر( flg & COLFLAG_HASTYPE ){
          ثابت نویسه *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          بشکن;
        }
      }
    }
    اگر( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    اگر( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    اگر( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    اگر( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_درخت_View_برداشت(&pView);
  }
  sqlite3_درخت_View_برداشت(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
تهی sqlite3_درخت_View_With(درخت_View *pView, ثابت With *pWith, u8 moreToFollow){
  صحیح i;
  اگر( pWith==0 ) بازگشت;
  اگر( pWith->nCte==0 ) بازگشت;
  اگر( pWith->pOuter ){
    sqlite3_درخت_View_سطر(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }وگرنه{
    sqlite3_درخت_View_سطر(pView, "WITH (0x%p)", pWith);
  }
  اگر( pWith->nCte>0 ){
    sqlite3_درخت_View_افزودن(&pView, moreToFollow);
    برای(i=0; i<pWith->nCte; i++){
      StrAccum x;
      نویسه z_سطر[1000];
      ثابت ساختار Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_سطر, اندازه(z_سطر), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      اگر( pCte->pCols && pCte->pCols->n_عبارت>0 ){
        نویسه cSep = '(';
        صحیح j;
        برای(j=0; j<pCte->pCols->n_عبارت; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      اگر( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      اگر( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_درخت_View_Item(pView, z_سطر, i<pWith->nCte-1);
      sqlite3_درخت_View_Select(pView, pCte->pSelect, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
    sqlite3_درخت_View_برداشت(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
تهی sqlite3_درخت_View_Src_List(درخت_View *pView, ثابت SrcList *pSrc){
  صحیح i;
  اگر( pSrc==0 ) بازگشت;
  برای(i=0; i<pSrc->nSrc; i++){
    ثابت SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    صحیح n = 0;
    نویسه z_سطر[1000];
    sqlite3StrAccumInit(&x, 0, z_سطر, اندازه(z_سطر), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    اگر( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    اگر( (pItem->fg.jointype & (JT_چپ|JT_راست))==(JT_چپ|JT_راست) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }وگرنه اگر( pItem->fg.jointype & JT_چپ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }وگرنه اگر( pItem->fg.jointype & JT_راست ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }وگرنه اگر( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    اگر( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    اگر( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    اگر( pItem->fg.isCte ){
      ایستا ثابت نویسه *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    اگر( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    اگر( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    اگر( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    اگر( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    اگر( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    اگر( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    اگر( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    اگر( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    اگر( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    اگر( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_درخت_View_Item(pView, z_سطر, i<pSrc->nSrc-1);
    n = 0;
    اگر( pItem->fg.isSubquery ) n++;
    اگر( pItem->fg.isTabFunc ) n++;
    اگر( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    اگر( pItem->fg.isUsing ){
      sqlite3_درخت_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }وگرنه اگر( pItem->u3.pOn!=0 ){
      sqlite3_درخت_View_Item(pView, "ON", (--n)>0);
      sqlite3_درخت_View_عبارت(pView, pItem->u3.pOn, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
    اگر( pItem->fg.isSubquery ){
      أكد( n==1 );
      اگر( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_درخت_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      أكد( (صحیح)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_درخت_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    اگر( pItem->fg.isTabFunc ){
      sqlite3_درخت_View_عبارت_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_درخت_View_برداشت(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
تهی sqlite3_درخت_View_Select(درخت_View *pView, ثابت Select *p, u8 moreToFollow){
  صحیح n = 0;
  صحیح cnt = 0;
  اگر( p==0 ){
    sqlite3_درخت_View_سطر(pView, "nil-SELECT");
    بازگشت;
  } 
  sqlite3_درخت_View_افزودن(&pView, moreToFollow);
  اگر( p->pWith ){
    sqlite3_درخت_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_درخت_View_افزودن(&pView, 1);
  }
  انجامبده{
    اگر( p->selFlags & SF_WhereBegin ){
      sqlite3_درخت_View_سطر(pView, "sqlite3WhereBegin()");
    }وگرنه{
      sqlite3_درخت_View_سطر(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (صحیح)p->nSelectRow
      );
    }
    اگر( cnt++ ) sqlite3_درخت_View_برداشت(&pView);
    اگر( p->pPrior ){
      n = 1000;
    }وگرنه{
      n = 0;
      اگر( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      اگر( p->pWhere ) n++;
      اگر( p->pGroupBy ) n++;
      اگر( p->pHaving ) n++;
      اگر( p->pOrderBy ) n++;
      اگر( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      اگر( p->pWin ) n++;
      اگر( p->pWinDefn ) n++;
#endif
    }
    اگر( p->pEList ){
      sqlite3_درخت_View_عبارت_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    اگر( p->pWin ){
      Window *pX;
      sqlite3_درخت_View_افزودن(&pView, (n--)>0);
      sqlite3_درخت_View_سطر(pView, "window-functions");
      برای(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_درخت_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_درخت_View_برداشت(&pView);
    }
#endif
    اگر( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_درخت_View_افزودن(&pView, (n--)>0);
      sqlite3_درخت_View_سطر(pView, "FROM");
      sqlite3_درخت_View_Src_List(pView, p->pSrc);
      sqlite3_درخت_View_برداشت(&pView);
    }
    اگر( p->pWhere ){
      sqlite3_درخت_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_درخت_View_عبارت(pView, p->pWhere, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
    اگر( p->pGroupBy ){
      sqlite3_درخت_View_عبارت_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    اگر( p->pHaving ){
      sqlite3_درخت_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_درخت_View_عبارت(pView, p->pHaving, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    اگر( p->pWinDefn ){
      Window *pX;
      sqlite3_درخت_View_Item(pView, "WINDOW", (n--)>0);
      برای(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_درخت_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_درخت_View_برداشت(&pView);
    }
#endif
    اگر( p->pOrderBy ){
      sqlite3_درخت_View_عبارت_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    اگر( p->pLimit ){
      sqlite3_درخت_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_درخت_View_عبارت(pView, p->pLimit->p_چپ, p->pLimit->p_راست!=0);
      اگر( p->pLimit->p_راست ){
        sqlite3_درخت_View_Item(pView, "OFFSET", 0);
        sqlite3_درخت_View_عبارت(pView, p->pLimit->p_راست, 0);
        sqlite3_درخت_View_برداشت(&pView);
      }
      sqlite3_درخت_View_برداشت(&pView);
    }
    اگر( p->pPrior ){
      ثابت نویسه *zOp = "UNION";
      انتخاب( p->op ){
        حالت TK_ALL:         zOp = "UNION ALL";  بشکن;
        حالت TK_INTERSECT:   zOp = "INTERSECT";  بشکن;
        حالت TK_EXCEPT:      zOp = "EXCEPT";     بشکن;
      }
      sqlite3_درخت_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }وقتی( p!=0 );
  sqlite3_درخت_View_برداشت(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
تهی sqlite3_درخت_View_Bound(
  درخت_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  عبارت *p_عبارت,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  انتخاب( eBound ){
    حالت TK_UNBOUNDED: {
      sqlite3_درخت_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_درخت_View_برداشت(&pView);
      بشکن;
    }
    حالت TK_CURRENT: {
      sqlite3_درخت_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_درخت_View_برداشت(&pView);
      بشکن;
    }
    حالت TK_PRECEDING: {
      sqlite3_درخت_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_درخت_View_عبارت(pView, p_عبارت, 0);
      sqlite3_درخت_View_برداشت(&pView);
      بشکن;
    }
    حالت TK_FOLLOWING: {
      sqlite3_درخت_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_درخت_View_عبارت(pView, p_عبارت, 0);
      sqlite3_درخت_View_برداشت(&pView);
      بشکن;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
تهی sqlite3_درخت_View_Window(درخت_View *pView, ثابت Window *pWin, u8 more){
  صحیح nElement = 0;
  اگر( pWin==0 ) بازگشت;
  اگر( pWin->pFilter ){
    sqlite3_درخت_View_Item(pView, "FILTER", 1);
    sqlite3_درخت_View_عبارت(pView, pWin->pFilter, 0);
    sqlite3_درخت_View_برداشت(&pView);
    اگر( pWin->eFrmType==TK_FILTER ) بازگشت;
  }
  sqlite3_درخت_View_افزودن(&pView, more);
  اگر( pWin->zName ){
    sqlite3_درخت_View_سطر(pView, "OVER %s (%p)", pWin->zName, pWin);
  }وگرنه{
    sqlite3_درخت_View_سطر(pView, "OVER (%p)", pWin);
  }
  اگر( pWin->zBase )    nElement++;
  اگر( pWin->pOrderBy ) nElement++;
  اگر( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  اگر( pWin->eExclude ) nElement++;
  اگر( pWin->zBase ){
    sqlite3_درخت_View_افزودن(&pView, (--nElement)>0);
    sqlite3_درخت_View_سطر(pView, "window: %s", pWin->zBase);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pWin->pPartition ){
    sqlite3_درخت_View_عبارت_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  اگر( pWin->pOrderBy ){
    sqlite3_درخت_View_عبارت_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  اگر( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    نویسه zBuf[30];
    ثابت نویسه *zFrmType = "ROWS";
    اگر( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    اگر( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(اندازه(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_درخت_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_درخت_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_درخت_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pWin->eExclude ){
    نویسه zBuf[30];
    ثابت نویسه *zExclude;
    انتخاب( pWin->eExclude ){
      حالت TK_NO:      zExclude = "NO OTHERS";   بشکن;
      حالت TK_CURRENT: zExclude = "CURRENT ROW"; بشکن;
      حالت TK_GROUP:   zExclude = "GROUP";       بشکن;
      حالت TK_TIES:    zExclude = "TIES";        بشکن;
      پیشفرض:
        sqlite3_snprintf(اندازه(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        بشکن;
    }
    sqlite3_درخت_View_افزودن(&pView, 0);
    sqlite3_درخت_View_سطر(pView, "EXCLUDE %s", zExclude);
    sqlite3_درخت_View_برداشت(&pView);
  }
  sqlite3_درخت_View_برداشت(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
تهی sqlite3_درخت_View_Win_Func(درخت_View *pView, ثابت Window *pWin, u8 more){
  اگر( pWin==0 ) بازگشت;
  sqlite3_درخت_View_افزودن(&pView, more);
  sqlite3_درخت_View_سطر(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_درخت_View_Window(pView, pWin, 0);
  sqlite3_درخت_View_برداشت(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
تهی sqlite3_درخت_View_عبارت(درخت_View *pView, ثابت عبارت *p_عبارت, u8 moreToFollow){
  ثابت نویسه *zBinOp = 0;   /* Binary operator */
  ثابت نویسه *zUniOp = 0;   /* Unary operator */
  نویسه zFlgs[200];
  sqlite3_درخت_View_افزودن(&pView, moreToFollow);
  اگر( p_عبارت==0 ){
    sqlite3_درخت_View_سطر(pView, "nil");
    sqlite3_درخت_View_برداشت(&pView);
    بازگشت;
  }
  اگر( p_عبارت->flags || p_عبارت->aff_عبارت || p_عبارت->vvaFlags || p_عبارت->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, اندازه(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_عبارت->flags, p_عبارت->aff_عبارت ? p_عبارت->aff_عبارت : 'n');
    اگر( عبارت_Has_Property(p_عبارت, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_عبارت->w.iJoin);
    }
    اگر( عبارت_Has_Property(p_عبارت, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_عبارت->w.iJoin);
    }
    اگر( عبارت_Has_Property(p_عبارت, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    اگر( عبارت_Has_VVAProperty(p_عبارت, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    اگر( p_عبارت->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_عبارت->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }وگرنه{
    zFlgs[0] = 0;
  }
  انتخاب( p_عبارت->op ){
    حالت TK_AGG_COLUMN: {
      sqlite3_درخت_View_سطر(pView, "AGG{%d:%d}%s",
            p_عبارت->iTable, p_عبارت->iColumn, zFlgs);
      بشکن;
    }
    حالت TK_COLUMN: {
      اگر( p_عبارت->iTable<0 ){
        /* This only happens when coding check constraints */
        نویسه zOp2[16];
        اگر( p_عبارت->op2 ){
          sqlite3_snprintf(اندازه(zOp2),zOp2," op2=0x%02x",p_عبارت->op2);
        }وگرنه{
          zOp2[0] = 0;
        }
        sqlite3_درخت_View_سطر(pView, "COLUMN(%d)%s%s",
                                    p_عبارت->iColumn, zFlgs, zOp2);
      }وگرنه{
        أكد( عبارت_Use_YTab(p_عبارت) );
        sqlite3_درخت_View_سطر(pView, "{%d:%d} pTab=%p%s",
                        p_عبارت->iTable, p_عبارت->iColumn,
                        p_عبارت->y.pTab, zFlgs);
      }
      اگر( عبارت_Has_Property(p_عبارت, EP_FixedCol) ){
        sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      }
      بشکن;
    }
    حالت TK_INTEGER: {
      اگر( p_عبارت->flags & EP_Int_مقدار ){
        sqlite3_درخت_View_سطر(pView, "%d", p_عبارت->u.i_مقدار);
      }وگرنه{
        sqlite3_درخت_View_سطر(pView, "%s", p_عبارت->u.zToken);
      }
      بشکن;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    حالت TK_FLOAT: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"%s", p_عبارت->u.zToken);
      بشکن;
    }
#endif
    حالت TK_STRING: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"%Q", p_عبارت->u.zToken);
      بشکن;
    }
    حالت TK_NULL: {
      sqlite3_درخت_View_سطر(pView,"NULL");
      بشکن;
    }
    حالت TK_TRUEFALSE: {
      sqlite3_درخت_View_سطر(pView,"%s%s",
         sqlite3_عبارت_Truth_مقدار(p_عبارت) ? "TRUE" : "FALSE", zFlgs);
      بشکن;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    حالت TK_BLOB: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"%s", p_عبارت->u.zToken);
      بشکن;
    }
#endif
    حالت TK_VARIABLE: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"VARIABLE(%s,%d)",
                          p_عبارت->u.zToken, p_عبارت->iColumn);
      بشکن;
    }
    حالت TK_REGISTER: {
      sqlite3_درخت_View_سطر(pView,"REGISTER(%d)", p_عبارت->iTable);
      بشکن;
    }
    حالت TK_ID: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"ID \"%w\"", p_عبارت->u.zToken);
      بشکن;
    }
    حالت TK_ASTERISK: {
      sqlite3_درخت_View_سطر(pView,"ASKTERISK");
      بشکن;
    }
#ifndef SQLITE_OMIT_CAST
    حالت TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"CAST %Q", p_عبارت->u.zToken);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      بشکن;
    }
#endif /* SQLITE_OMIT_CAST */
    حالت TK_LT:      zBinOp = "LT";     بشکن;
    حالت TK_LE:      zBinOp = "LE";     بشکن;
    حالت TK_GT:      zBinOp = "GT";     بشکن;
    حالت TK_GE:      zBinOp = "GE";     بشکن;
    حالت TK_NE:      zBinOp = "NE";     بشکن;
    حالت TK_EQ:      zBinOp = "EQ";     بشکن;
    حالت TK_IS:      zBinOp = "IS";     بشکن;
    حالت TK_ISNOT:   zBinOp = "ISNOT";  بشکن;
    حالت TK_AND:     zBinOp = "AND";    بشکن;
    حالت TK_OR:      zBinOp = "OR";     بشکن;
    حالت TK_PLUS:    zBinOp = "ADD";    بشکن;
    حالت TK_STAR:    zBinOp = "MUL";    بشکن;
    حالت TK_MINUS:   zBinOp = "SUB";    بشکن;
    حالت TK_REM:     zBinOp = "REM";    بشکن;
    حالت TK_BITAND:  zBinOp = "BITAND"; بشکن;
    حالت TK_BITOR:   zBinOp = "BITOR";  بشکن;
    حالت TK_SLASH:   zBinOp = "DIV";    بشکن;
    حالت TK_LSHIFT:  zBinOp = "LSHIFT"; بشکن;
    حالت TK_RSHIFT:  zBinOp = "RSHIFT"; بشکن;
    حالت TK_CONCAT:  zBinOp = "CONCAT"; بشکن;
    حالت TK_DOT:     zBinOp = "DOT";    بشکن;
    حالت TK_LIMIT:   zBinOp = "LIMIT";  بشکن;

    حالت TK_UMINUS:  zUniOp = "UMINUS"; بشکن;
    حالت TK_UPLUS:   zUniOp = "UPLUS";  بشکن;
    حالت TK_BITNOT:  zUniOp = "BITNOT"; بشکن;
    حالت TK_NOT:     zUniOp = "NOT";    بشکن;
    حالت TK_ISNULL:  zUniOp = "ISNULL"; بشکن;
    حالت TK_NOTNULL: zUniOp = "NOTNULL"; بشکن;

    حالت TK_TRUTH: {
      صحیح x;
      ثابت نویسه *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      أكد( p_عبارت->op2==TK_IS || p_عبارت->op2==TK_ISNOT );
      أكد( p_عبارت->p_راست );
      أكد( sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت->p_راست)->op
                  == TK_TRUEFALSE );
      x = (p_عبارت->op2==TK_ISNOT)*2 + sqlite3_عبارت_Truth_مقدار(p_عبارت->p_راست);
      zUniOp = azOp[x];
      بشکن;
    }

    حالت TK_SPAN: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView, "SPAN %Q", p_عبارت->u.zToken);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      بشکن;
    }

    حالت TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView, "%sCOLLATE %Q%s",
        !عبارت_Has_Property(p_عبارت, EP_Collate) ? "SOFT-" : "",
        p_عبارت->u.zToken, zFlgs);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      بشکن;
    }

    حالت TK_AGG_FUNCTION:
    حالت TK_FUNCTION: {
      عبارت_List *pFarg;       /* List of function arguments */
      Window *pWin;
      اگر( عبارت_Has_Property(p_عبارت, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }وگرنه{
        أكد( عبارت_Use_XList(p_عبارت) );
        pFarg = p_عبارت->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_عبارت) ? p_عبارت->y.pWin : 0;
#وگرنه
        pWin = 0;
#endif 
      }
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      اگر( p_عبارت->op==TK_AGG_FUNCTION ){
        sqlite3_درخت_View_سطر(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_عبارت->op2, p_عبارت->u.zToken, zFlgs,
                             p_عبارت->pAggInfo ? p_عبارت->pAggInfo->selId : 0,
                             p_عبارت->iAgg, p_عبارت->pAggInfo);
      }وگرنه اگر( p_عبارت->op2!=0 ){
        ثابت نویسه *zOp2;
        نویسه zBuf[8];
        sqlite3_snprintf(اندازه(zBuf),zBuf,"0x%02x",p_عبارت->op2);
        zOp2 = zBuf;
        اگر( p_عبارت->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        اگر( p_عبارت->op2==NC_Idx_عبارت ) zOp2 = "NC_IdxExpr";
        اگر( p_عبارت->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        اگر( p_عبارت->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_درخت_View_سطر(pView, "FUNCTION %Q%s op2=%s",
                            p_عبارت->u.zToken, zFlgs, zOp2);
      }وگرنه{
        sqlite3_درخت_View_سطر(pView, "FUNCTION %Q%s", p_عبارت->u.zToken, zFlgs);
      }
      اگر( pFarg ){
        sqlite3_درخت_View_عبارت_List(pView, pFarg, pWin!=0 || p_عبارت->p_چپ, 0);
        اگر( p_عبارت->p_چپ ){
          عبارت *pOB = p_عبارت->p_چپ;
          أكد( pOB->op==TK_ORDER );
          أكد( عبارت_Use_XList(pOB) );
          sqlite3_درخت_View_عبارت_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      اگر( pWin ){
        sqlite3_درخت_View_Window(pView, pWin, 0);
      }
#endif
      بشکن;
    }
    حالت TK_ORDER: {
      sqlite3_درخت_View_عبارت_List(pView, p_عبارت->x.pList, 0, "ORDERBY");
      بشکن;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    حالت TK_EXISTS: {
      أكد( عبارت_Use_XSelect(p_عبارت) );
      sqlite3_درخت_View_سطر(pView, "EXISTS-expr flags=0x%x", p_عبارت->flags);
      sqlite3_درخت_View_Select(pView, p_عبارت->x.pSelect, 0);
      بشکن;
    }
    حالت TK_SELECT: {
      أكد( عبارت_Use_XSelect(p_عبارت) );
      sqlite3_درخت_View_سطر(pView, "subquery-expr flags=0x%x", p_عبارت->flags);
      sqlite3_درخت_View_Select(pView, p_عبارت->x.pSelect, 0);
      بشکن;
    }
    حالت TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      نویسه *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_عبارت->flags);
      اگر( p_عبارت->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_عبارت->iTable);
      اگر( عبارت_Has_Property(p_عبارت, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_عبارت->y.sub.regReturn, p_عبارت->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_درخت_View_سطر(pView, z);
      sqlite3_free(z);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 1);
      اگر( عبارت_Use_XSelect(p_عبارت) ){
        sqlite3_درخت_View_Select(pView, p_عبارت->x.pSelect, 0);
      }وگرنه{
        sqlite3_درخت_View_عبارت_List(pView, p_عبارت->x.pList, 0, 0);
      }
      بشکن;
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
    حالت TK_BETWEEN: {
      ثابت عبارت *pX, *pY, *pZ;
      pX = p_عبارت->p_چپ;
      أكد( عبارت_Use_XList(p_عبارت) );
      أكد( p_عبارت->x.pList->n_عبارت==2 );
      pY = p_عبارت->x.pList->a[0].p_عبارت;
      pZ = p_عبارت->x.pList->a[1].p_عبارت;
      sqlite3_درخت_View_سطر(pView, "BETWEEN%s", zFlgs);
      sqlite3_درخت_View_عبارت(pView, pX, 1);
      sqlite3_درخت_View_عبارت(pView, pY, 1);
      sqlite3_درخت_View_عبارت(pView, pZ, 0);
      بشکن;
    }
    حالت TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_درخت_View_سطر(pView, "%s(%d)", 
          p_عبارت->iTable ? "NEW" : "OLD", p_عبارت->iColumn);
      بشکن;
    }
    حالت TK_CASE: {
      sqlite3_درخت_View_سطر(pView, "CASE");
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 1);
      أكد( عبارت_Use_XList(p_عبارت) );
      sqlite3_درخت_View_عبارت_List(pView, p_عبارت->x.pList, 0, 0);
      بشکن;
    }
#ifndef SQLITE_OMIT_TRIGGER
    حالت TK_RAISE: {
      ثابت نویسه *zType = "unk";
      انتخاب( p_عبارت->aff_عبارت ){
        حالت OE_Rollback:   zType = "rollback";  بشکن;
        حالت OE_Abort:      zType = "abort";     بشکن;
        حالت OE_Fail:       zType = "fail";      بشکن;
        حالت OE_Ignore:     zType = "ignore";    بشکن;
      }
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView, "RAISE %s", zType);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      بشکن;
    }
#endif
    حالت TK_MATCH: {
      sqlite3_درخت_View_سطر(pView, "MATCH {%d:%d}%s",
                          p_عبارت->iTable, p_عبارت->iColumn, zFlgs);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_راست, 0);
      بشکن;
    }
    حالت TK_VECTOR: {
      نویسه *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      أكد( عبارت_Use_XList(p_عبارت) );
      sqlite3_درخت_View_Bare_عبارت_List(pView, p_عبارت->x.pList, z);
      sqlite3_free(z);
      بشکن;
    }
    حالت TK_SELECT_COLUMN: {
      sqlite3_درخت_View_سطر(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_عبارت->iColumn, p_عبارت->iTable-1,
              p_عبارت->p_راست==p_عبارت->p_چپ ? " (SELECT-owner)" : "");
      أكد( عبارت_Use_XSelect(p_عبارت->p_چپ) );
      sqlite3_درخت_View_Select(pView, p_عبارت->p_چپ->x.pSelect, 0);
      بشکن;
    }
    حالت TK_IF_NULL_ROW: {
      sqlite3_درخت_View_سطر(pView, "IF-NULL-ROW %d", p_عبارت->iTable);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      بشکن;
    }
    حالت TK_ERROR: {
      عبارت tmp;
      sqlite3_درخت_View_سطر(pView, "ERROR");
      tmp = *p_عبارت;
      tmp.op = p_عبارت->op2;
      sqlite3_درخت_View_عبارت(pView, &tmp, 0);
      بشکن;
    }
    حالت TK_ROW: {
      اگر( p_عبارت->iColumn<=0 ){
        sqlite3_درخت_View_سطر(pView, "First FROM table rowid");
      }وگرنه{
        sqlite3_درخت_View_سطر(pView, "First FROM table column %d",
            p_عبارت->iColumn-1);
      }
      بشکن;
    }
    پیشفرض: {
      sqlite3_درخت_View_سطر(pView, "op=%d", p_عبارت->op);
      بشکن;
    }
  }
  اگر( zBinOp ){
    sqlite3_درخت_View_سطر(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 1);
    sqlite3_درخت_View_عبارت(pView, p_عبارت->p_راست, 0);
  }وگرنه اگر( zUniOp ){
    sqlite3_درخت_View_سطر(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
  }
  sqlite3_درخت_View_برداشت(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
تهی sqlite3_درخت_View_Bare_عبارت_List(
  درخت_View *pView,
  ثابت عبارت_List *pList,
  ثابت نویسه *zLabel
){
  اگر( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  اگر( pList==0 ){
    sqlite3_درخت_View_سطر(pView, "%s (empty)", zLabel);
  }وگرنه{
    صحیح i;
    sqlite3_درخت_View_سطر(pView, "%s", zLabel);
    برای(i=0; i<pList->n_عبارت; i++){
      صحیح j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      نویسه *zName = pList->a[i].zEName;
      صحیح moreToFollow = i<pList->n_عبارت - 1;
      اگر( j || zName || sortFlags ){
        sqlite3_درخت_View_افزودن(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_درخت_View_سطر(pView, 0);
        اگر( zName ){
          انتخاب( pList->a[i].fg.eEName ){
            پیشفرض:
              fprintf(stdout, "AS %s ", zName);
              بشکن;
            حالت ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              اگر( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              اگر( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              اگر( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              بشکن;
            حالت ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              بشکن;
          }
        }
        اگر( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        اگر( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }وگرنه اگر( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_درخت_View_عبارت(pView, pList->a[i].p_عبارت, moreToFollow);
      اگر( j || zName || sortFlags ){
        sqlite3_درخت_View_برداشت(&pView);
      }
    }
  }
}
تهی sqlite3_درخت_View_عبارت_List(
  درخت_View *pView,
  ثابت عبارت_List *pList,
  u8 moreToFollow,
  ثابت نویسه *zLabel
){
  sqlite3_درخت_View_افزودن(&pView, moreToFollow);
  sqlite3_درخت_View_Bare_عبارت_List(pView, pList, zLabel);
  sqlite3_درخت_View_برداشت(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
تهی sqlite3_درخت_View_Bare_Id_List(
  درخت_View *pView,
  ثابت IdList *pList,
  ثابت نویسه *zLabel
){
  اگر( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  اگر( pList==0 ){
    sqlite3_درخت_View_سطر(pView, "%s (empty)", zLabel);
  }وگرنه{
    صحیح i;
    sqlite3_درخت_View_سطر(pView, "%s", zLabel);
    برای(i=0; i<pList->nId; i++){
      نویسه *zName = pList->a[i].zName;
      صحیح moreToFollow = i<pList->nId - 1;
      اگر( zName==0 ) zName = "(null)";
      sqlite3_درخت_View_افزودن(&pView, moreToFollow);
      sqlite3_درخت_View_سطر(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_درخت_View_برداشت(&pView);
    }
  }
}
تهی sqlite3_درخت_View_Id_List(
  درخت_View *pView,
  ثابت IdList *pList,
  u8 moreToFollow,
  ثابت نویسه *zLabel
){
  sqlite3_درخت_View_افزودن(&pView, moreToFollow);
  sqlite3_درخت_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_درخت_View_برداشت(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
تهی sqlite3_درخت_View_Upsert(
  درخت_View *pView,
  ثابت Upsert *pUpsert,
  u8 moreToFollow
){
  اگر( pUpsert==0 ) بازگشت;
  sqlite3_درخت_View_افزودن(&pView, moreToFollow);
  وقتی( pUpsert ){
    صحیح n;
    sqlite3_درخت_View_افزودن(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_درخت_View_سطر(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_درخت_View_عبارت_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_درخت_View_عبارت_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    اگر( pUpsert->pUpsertWhere ){
      sqlite3_درخت_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_درخت_View_عبارت(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
    sqlite3_درخت_View_برداشت(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_درخت_View_برداشت(&pView);
}

#اگر TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
تهی sqlite3_درخت_View_Delete(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت عبارت *pWhere,
  ثابت عبارت_List *pOrderBy,
  ثابت عبارت *pLimit,
  ثابت Trigger *pTrigger
){
  صحیح n = 0;
  درخت_View *pView = 0;
  sqlite3_درخت_View_افزودن(&pView, 0);
  sqlite3_درخت_View_سطر(pView, "DELETE");
  اگر( pWith ) n++;
  اگر( pTabList ) n++;
  اگر( pWhere ) n++;
  اگر( pOrderBy ) n++;
  اگر( pLimit ) n++;
  اگر( pTrigger ) n++;
  اگر( pWith ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_With(pView, pWith, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pTabList ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "FROM");
    sqlite3_درخت_View_Src_List(pView, pTabList);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pWhere ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "WHERE");
    sqlite3_درخت_View_عبارت(pView, pWhere, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pOrderBy ){
    sqlite3_درخت_View_عبارت_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  اگر( pLimit ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "LIMIT");
    sqlite3_درخت_View_عبارت(pView, pLimit, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pTrigger ){
    sqlite3_درخت_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_درخت_View_برداشت(&pView);
}
#endif /* TREETRACE_ENABLED */

#اگر TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
تهی sqlite3_درخت_View_درج(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت IdList *pColumnList,
  ثابت Select *pSelect,
  ثابت عبارت_List *p_عبارت_List,
  صحیح onError,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  درخت_View *pView = 0;
  صحیح n = 0;
  ثابت نویسه *zLabel = "INSERT";
  انتخاب( onError ){
    حالت OE_Replace:  zLabel = "REPLACE";             بشکن;
    حالت OE_Ignore:   zLabel = "INSERT OR IGNORE";    بشکن;
    حالت OE_Rollback: zLabel = "INSERT OR ROLLBACK";  بشکن;
    حالت OE_Abort:    zLabel = "INSERT OR ABORT";     بشکن;
    حالت OE_Fail:     zLabel = "INSERT OR FAIL";      بشکن;
  }
  sqlite3_درخت_View_افزودن(&pView, 0);
  sqlite3_درخت_View_سطر(pView, zLabel);
  اگر( pWith ) n++;
  اگر( pTabList ) n++;
  اگر( pColumnList ) n++;
  اگر( pSelect ) n++;
  اگر( p_عبارت_List ) n++;
  اگر( pUpsert ) n++;
  اگر( pTrigger ) n++;
  اگر( pWith ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_With(pView, pWith, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pTabList ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "INTO");
    sqlite3_درخت_View_Src_List(pView, pTabList);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pColumnList ){
    sqlite3_درخت_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  اگر( pSelect ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "DATA-SOURCE");
    sqlite3_درخت_View_Select(pView, pSelect, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( p_عبارت_List ){
    sqlite3_درخت_View_عبارت_List(pView, p_عبارت_List, (--n)>0, "VALUES");
  }
  اگر( pUpsert ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "UPSERT");
    sqlite3_درخت_View_Upsert(pView, pUpsert, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pTrigger ){
    sqlite3_درخت_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_درخت_View_برداشت(&pView);
}
#endif /* TREETRACE_ENABLED */

#اگر TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
تهی sqlite3_درخت_View_Update(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت عبارت_List *pChanges,
  ثابت عبارت *pWhere,
  صحیح onError,
  ثابت عبارت_List *pOrderBy,
  ثابت عبارت *pLimit,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  صحیح n = 0;
  درخت_View *pView = 0;
  ثابت نویسه *zLabel = "UPDATE";
  انتخاب( onError ){
    حالت OE_Replace:  zLabel = "UPDATE OR REPLACE";   بشکن;
    حالت OE_Ignore:   zLabel = "UPDATE OR IGNORE";    بشکن;
    حالت OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  بشکن;
    حالت OE_Abort:    zLabel = "UPDATE OR ABORT";     بشکن;
    حالت OE_Fail:     zLabel = "UPDATE OR FAIL";      بشکن;
  }
  sqlite3_درخت_View_افزودن(&pView, 0);
  sqlite3_درخت_View_سطر(pView, zLabel);
  اگر( pWith ) n++;
  اگر( pTabList ) n++;
  اگر( pChanges ) n++;
  اگر( pWhere ) n++;
  اگر( pOrderBy ) n++;
  اگر( pLimit ) n++;
  اگر( pUpsert ) n++;
  اگر( pTrigger ) n++;
  اگر( pWith ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_With(pView, pWith, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pTabList ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "FROM");
    sqlite3_درخت_View_Src_List(pView, pTabList);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pChanges ){
    sqlite3_درخت_View_عبارت_List(pView, pChanges, (--n)>0, "SET");
  }
  اگر( pWhere ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "WHERE");
    sqlite3_درخت_View_عبارت(pView, pWhere, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pOrderBy ){
    sqlite3_درخت_View_عبارت_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  اگر( pLimit ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "LIMIT");
    sqlite3_درخت_View_عبارت(pView, pLimit, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pUpsert ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "UPSERT");
    sqlite3_درخت_View_Upsert(pView, pUpsert, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  اگر( pTrigger ){
    sqlite3_درخت_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_درخت_View_برداشت(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
تهی sqlite3_درخت_View_Trigger_Step(
  درخت_View *pView,
  ثابت TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  صحیح cnt = 0;
  اگر( pStep==0 ) بازگشت;
  sqlite3_درخت_View_افزودن(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  انجامبده{
    اگر( cnt++ && pStep->pNext==0 ){
      sqlite3_درخت_View_برداشت(&pView);
      sqlite3_درخت_View_افزودن(&pView, 0);
    }
    sqlite3_درخت_View_سطر(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }وقتی( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_درخت_View_برداشت(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
تهی sqlite3_درخت_View_Trigger(
  درخت_View *pView,
  ثابت Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  صحیح cnt = 0;
  اگر( pTrigger==0 ) بازگشت;
  sqlite3_درخت_View_افزودن(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  انجامبده{
    اگر( cnt++ && pTrigger->pNext==0 ){
      sqlite3_درخت_View_برداشت(&pView);
      sqlite3_درخت_View_افزودن(&pView, 0);
    }
    sqlite3_درخت_View_سطر(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_درخت_View_افزودن(&pView, 0);
    sqlite3_درخت_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_درخت_View_برداشت(&pView);
  }وقتی( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_درخت_View_برداشت(&pView);
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
تهی sqlite3_Show_عبارت(ثابت عبارت *p){ sqlite3_درخت_View_عبارت(0,p,0); }
تهی sqlite3_Show_عبارت_List(ثابت عبارت_List *p){ sqlite3_درخت_View_عبارت_List(0,p,0,0);}
تهی sqlite3ShowIdList(ثابت IdList *p){ sqlite3_درخت_View_Id_List(0,p,0,0); }
تهی sqlite3ShowSrcList(ثابت SrcList *p){
  درخت_View *pView = 0;
  sqlite3_درخت_View_افزودن(&pView, 0);
  sqlite3_درخت_View_سطر(pView, "SRCLIST");
  sqlite3_درخت_View_Src_List(pView,p);
  sqlite3_درخت_View_برداشت(&pView);
}
تهی sqlite3ShowSelect(ثابت Select *p){ sqlite3_درخت_View_Select(0,p,0); }
تهی sqlite3ShowWith(ثابت With *p){ sqlite3_درخت_View_With(0,p,0); }
تهی sqlite3ShowUpsert(ثابت Upsert *p){ sqlite3_درخت_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
تهی sqlite3ShowTriggerStep(ثابت TriggerStep *p){
  sqlite3_درخت_View_Trigger_Step(0,p,0,0);
}
تهی sqlite3ShowTriggerStepList(ثابت TriggerStep *p){
  sqlite3_درخت_View_Trigger_Step(0,p,0,1);
}
تهی sqlite3ShowTrigger(ثابت Trigger *p){ sqlite3_درخت_View_Trigger(0,p,0,0); }
تهی sqlite3ShowTriggerList(ثابت Trigger *p){ sqlite3_درخت_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
تهی sqlite3ShowWindow(ثابت Window *p){ sqlite3_درخت_View_Window(0,p,0); }
تهی sqlite3ShowWinFunc(ثابت Window *p){ sqlite3_درخت_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
