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
#اگر_تعریف_شده SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
ساكن فراغ sqlite3_درخت_View_افزودن(درخت_View **pp, u8 بیشتر_مقصد_Follow){
  درخت_View *p = *pp;
  إذا( p==0 ){
    *pp = p = sqlite3_malloc64( حجم(*p) );
    إذا( p==0 ) أرجع;
    memset(p, 0, حجم(*p));
  }وإلا{
    p->iLevel++;
  }
  أكد( بیشتر_مقصد_Follow==0 || بیشتر_مقصد_Follow==1 );
  إذا( p->iLevel<(صحيح)حجم(p->b_سطر) ) p->b_سطر[p->iLevel] = بیشتر_مقصد_Follow;
}

/*
** Finished with one layer of the tree
*/
ساكن فراغ sqlite3_درخت_View_برداشت(درخت_View **pp){
  درخت_View *p = *pp;
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
فراغ sqlite3_درخت_View_سطر(درخت_View *p, ثابت حرف *zFormat, ...){
  va_list ap;
  صحيح i;
  StrAccum acc;
  حرف zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
  إذا( p ){
    لكل(i=0; i<p->iLevel && i<(صحيح)حجم(p->b_سطر)-1; i++){
      sqlite3_str_append(&acc, p->b_سطر[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_سطر[i] ? "|-- " : "'-- ", 4);
  }
  إذا( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    أكد( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3_Str_Accum_پایان(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
ساكن فراغ sqlite3_درخت_View_قلم(درخت_View *p, ثابت حرف *zLabel,u8 بیشتر_Follows){
  sqlite3_درخت_View_افزودن(&p, بیشتر_Follows);
  sqlite3_درخت_View_سطر(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
فراغ sqlite3_درخت_View_ستونی_List(
  درخت_View *pView,
  ثابت ستونی *a_ستون,
  صحيح n_ستون,
  u8 بیشتر_مقصد_Follow
){
  صحيح i;
  sqlite3_درخت_View_افزودن(&pView, بیشتر_مقصد_Follow);
  sqlite3_درخت_View_سطر(pView, "COLUMNS");
  لكل(i=0; i<n_ستون; i++){
    u16 flg = a_ستون[i].ستون_Flags;
    صحيح ستون_بیشتر_مقصد_Follow = i<(n_ستون - 1);
    sqlite3_درخت_View_افزودن(&pView, ستون_بیشتر_مقصد_Follow);
    sqlite3_درخت_View_سطر(pView, 0);
    چاپ_کن(" %s", a_ستون[i].zCnName);
    بدل( a_ستون[i].eCType ){
      حالة COLTYPE_ANY:      چاپ_کن(" ANY");        اكسر;
      حالة COLTYPE_BLOB:     چاپ_کن(" BLOB");       اكسر;
      حالة COLTYPE_INT:      چاپ_کن(" INT");        اكسر;
      حالة COLTYPE_INTEGER:  چاپ_کن(" INTEGER");    اكسر;
      حالة COLTYPE_REAL:     چاپ_کن(" REAL");       اكسر;
      حالة COLTYPE_متن:     چاپ_کن(" TEXT");       اكسر;
      حالة COLTYPE_CUSTOM: {
        إذا( flg & COLFLAG_HASTYPE ){
          ثابت حرف *z = a_ستون[i].zCnName;
          z += طول_رشته(z)+1;
          چاپ_کن(" X-%s", z);
          اكسر;
        }
      }
    }
    إذا( flg & COLFLAG_PRIMKEY ) چاپ_کن(" PRIMARY KEY");
    إذا( flg & COLFLAG_HIDDEN ) چاپ_کن(" HIDDEN");
#اگر_تعریف_شده COLFLAG_NOEXPAND
    إذا( flg & COLFLAG_NOEXPAND ) چاپ_کن(" NO-EXPAND");
#پایان_اگر
    إذا( flg ) چاپ_کن(" flags=%04x", flg);
    چاپ_کن("\n");      
    fflush(stdout);
    sqlite3_درخت_View_برداشت(&pView);
  }
  sqlite3_درخت_View_برداشت(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
فراغ sqlite3_درخت_View_With(درخت_View *pView, ثابت With *pWith, u8 بیشتر_مقصد_Follow){
  صحيح i;
  إذا( pWith==0 ) أرجع;
  إذا( pWith->nCte==0 ) أرجع;
  إذا( pWith->pOuter ){
    sqlite3_درخت_View_سطر(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }وإلا{
    sqlite3_درخت_View_سطر(pView, "WITH (0x%p)", pWith);
  }
  إذا( pWith->nCte>0 ){
    sqlite3_درخت_View_افزودن(&pView, بیشتر_مقصد_Follow);
    لكل(i=0; i<pWith->nCte; i++){
      StrAccum x;
      حرف z_سطر[1000];
      ثابت بنية Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_سطر, حجم(z_سطر), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      إذا( pCte->p_تعداد_ستون && pCte->p_تعداد_ستون->n_عبارت>0 ){
        حرف cSep = '(';
        صحيح j;
        لكل(j=0; j<pCte->p_تعداد_ستون->n_عبارت; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->p_تعداد_ستون->a[j].zEName);
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
      sqlite3_Str_Accum_پایان(&x);
      sqlite3_درخت_View_قلم(pView, z_سطر, i<pWith->nCte-1);
      sqlite3_درخت_View_Select(pView, pCte->pSelect, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
    sqlite3_درخت_View_برداشت(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
فراغ sqlite3_درخت_View_Src_List(درخت_View *pView, ثابت SrcList *pSrc){
  صحيح i;
  إذا( pSrc==0 ) أرجع;
  لكل(i=0; i<pSrc->nSrc; i++){
    ثابت Src_قلم *p_قلم = &pSrc->a[i];
    StrAccum x;
    صحيح n = 0;
    حرف z_سطر[1000];
    sqlite3StrAccumInit(&x, 0, z_سطر, حجم(z_سطر), 0);
    x.printfFlags |= SQLITE_PRINTF_داخلی;
    sqlite3_str_appendf(&x, "{%d:*} %!S", p_قلم->iCursor, p_قلم);
    إذا( p_قلم->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           p_قلم->pSTab->zName, p_قلم->pSTab->n_ستون, p_قلم->pSTab, 
           p_قلم->ستون_استفاده_شده,
           p_قلم->fg.rowid_استفاده_شده ? "+rowid" : "");
    }
    إذا( (p_قلم->fg.jointype & (JT_چپ|JT_راست))==(JT_چپ|JT_راست) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }وإلا إذا( p_قلم->fg.jointype & JT_چپ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }وإلا إذا( p_قلم->fg.jointype & JT_راست ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }وإلا إذا( p_قلم->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    إذا( p_قلم->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    إذا( p_قلم->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    إذا( p_قلم->fg.isCte ){
      ساكن ثابت حرف *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          p_قلم->u2.pCteUse->nUse,
                          aMat[p_قلم->u2.pCteUse->eM10d]);
    }
    إذا( p_قلم->fg.isOn || (p_قلم->fg.isUsing==0 && p_قلم->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    إذا( p_قلم->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    إذا( p_قلم->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    إذا( p_قلم->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    إذا( p_قلم->fg.از_طریق_Coroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    إذا( p_قلم->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    إذا( p_قلم->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    إذا( p_قلم->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    إذا( p_قلم->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    إذا( p_قلم->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3_Str_Accum_پایان(&x);
    sqlite3_درخت_View_قلم(pView, z_سطر, i<pSrc->nSrc-1);
    n = 0;
    إذا( p_قلم->fg.isSubquery ) n++;
    إذا( p_قلم->fg.isTabFunc ) n++;
    إذا( p_قلم->fg.isUsing || p_قلم->u3.pOn!=0 ) n++;
    إذا( p_قلم->fg.isUsing ){
      sqlite3_درخت_View_Id_List(pView, p_قلم->u3.pUsing, (--n)>0, "USING");
    }وإلا إذا( p_قلم->u3.pOn!=0 ){
      sqlite3_درخت_View_قلم(pView, "ON", (--n)>0);
      sqlite3_درخت_View_عبارت(pView, p_قلم->u3.pOn, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
    إذا( p_قلم->fg.isSubquery ){
      أكد( n==1 );
      إذا( p_قلم->pSTab ){
        جدول *pTab = p_قلم->pSTab;
        sqlite3_درخت_View_ستونی_List(pView, pTab->a_ستون, pTab->n_ستون, 1);
      }
      أكد( (صحيح)p_قلم->fg.isNestedFrom == IsNestedFrom(p_قلم) );
      sqlite3_درخت_View_Select(pView, p_قلم->u4.pSubq->pSelect, 0);
    }
    إذا( p_قلم->fg.isTabFunc ){
      sqlite3_درخت_View_عبارت_List(pView, p_قلم->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_درخت_View_برداشت(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
فراغ sqlite3_درخت_View_Select(درخت_View *pView, ثابت Select *p, u8 بیشتر_مقصد_Follow){
  صحيح n = 0;
  صحيح cnt = 0;
  إذا( p==0 ){
    sqlite3_درخت_View_سطر(pView, "nil-SELECT");
    أرجع;
  } 
  sqlite3_درخت_View_افزودن(&pView, بیشتر_مقصد_Follow);
  إذا( p->pWith ){
    sqlite3_درخت_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_درخت_View_افزودن(&pView, 1);
  }
  افعل{
    إذا( p->selFlags & SF_WhereBegin ){
      sqlite3_درخت_View_سطر(pView, "sqlite3WhereBegin()");
    }وإلا{
      sqlite3_درخت_View_سطر(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (صحيح)p->n_Select_ردیف
      );
    }
    إذا( cnt++ ) sqlite3_درخت_View_برداشت(&pView);
    إذا( p->p_پیشین ){
      n = 1000;
    }وإلا{
      n = 0;
      إذا( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      إذا( p->pWhere ) n++;
      إذا( p->p_گروه_By ) n++;
      إذا( p->pHaving ) n++;
      إذا( p->p_ترتیب_By ) n++;
      إذا( p->p_حد ) n++;
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
      إذا( p->pWin ) n++;
      إذا( p->pWinDefn ) n++;
#پایان_اگر
    }
    إذا( p->pEList ){
      sqlite3_درخت_View_عبارت_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
    إذا( p->pWin ){
      پنجره *pX;
      sqlite3_درخت_View_افزودن(&pView, (n--)>0);
      sqlite3_درخت_View_سطر(pView, "window-functions");
      لكل(pX=p->pWin; pX; pX=pX->p_بعدی_Win){
        sqlite3_درخت_View_Win_Func(pView, pX, pX->p_بعدی_Win!=0);
      }
      sqlite3_درخت_View_برداشت(&pView);
    }
#پایان_اگر
    إذا( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_درخت_View_افزودن(&pView, (n--)>0);
      sqlite3_درخت_View_سطر(pView, "FROM");
      sqlite3_درخت_View_Src_List(pView, p->pSrc);
      sqlite3_درخت_View_برداشت(&pView);
    }
    إذا( p->pWhere ){
      sqlite3_درخت_View_قلم(pView, "WHERE", (n--)>0);
      sqlite3_درخت_View_عبارت(pView, p->pWhere, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
    إذا( p->p_گروه_By ){
      sqlite3_درخت_View_عبارت_List(pView, p->p_گروه_By, (n--)>0, "GROUPBY");
    }
    إذا( p->pHaving ){
      sqlite3_درخت_View_قلم(pView, "HAVING", (n--)>0);
      sqlite3_درخت_View_عبارت(pView, p->pHaving, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
    إذا( p->pWinDefn ){
      پنجره *pX;
      sqlite3_درخت_View_قلم(pView, "WINDOW", (n--)>0);
      لكل(pX=p->pWinDefn; pX; pX=pX->p_بعدی_Win){
        sqlite3_درخت_View_پنجره(pView, pX, pX->p_بعدی_Win!=0);
      }
      sqlite3_درخت_View_برداشت(&pView);
    }
#پایان_اگر
    إذا( p->p_ترتیب_By ){
      sqlite3_درخت_View_عبارت_List(pView, p->p_ترتیب_By, (n--)>0, "ORDERBY");
    }
    إذا( p->p_حد ){
      sqlite3_درخت_View_قلم(pView, "LIMIT", (n--)>0);
      sqlite3_درخت_View_عبارت(pView, p->p_حد->p_چپ, p->p_حد->p_راست!=0);
      إذا( p->p_حد->p_راست ){
        sqlite3_درخت_View_قلم(pView, "OFFSET", 0);
        sqlite3_درخت_View_عبارت(pView, p->p_حد->p_راست, 0);
        sqlite3_درخت_View_برداشت(&pView);
      }
      sqlite3_درخت_View_برداشت(&pView);
    }
    إذا( p->p_پیشین ){
      ثابت حرف *zOp = "UNION";
      بدل( p->op ){
        حالة TK_ALL:         zOp = "UNION ALL";  اكسر;
        حالة TK_INTERSECT:   zOp = "INTERSECT";  اكسر;
        حالة TK_EXCEPT:      zOp = "EXCEPT";     اكسر;
      }
      sqlite3_درخت_View_قلم(pView, zOp, 1);
    }
    p = p->p_پیشین;
  }طالما( p!=0 );
  sqlite3_درخت_View_برداشت(&pView);
}

#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
فراغ sqlite3_درخت_View_Bound(
  درخت_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  عبارت *p_عبارت,            /* Value for PRECEDING or FOLLOWING */
  u8 بیشتر_مقصد_Follow         /* True if more to follow */
){
  بدل( eBound ){
    حالة TK_UNBOUNDED: {
      sqlite3_درخت_View_قلم(pView, "UNBOUNDED", بیشتر_مقصد_Follow);
      sqlite3_درخت_View_برداشت(&pView);
      اكسر;
    }
    حالة TK_CURRENT: {
      sqlite3_درخت_View_قلم(pView, "CURRENT", بیشتر_مقصد_Follow);
      sqlite3_درخت_View_برداشت(&pView);
      اكسر;
    }
    حالة TK_PRECEDING: {
      sqlite3_درخت_View_قلم(pView, "PRECEDING", بیشتر_مقصد_Follow);
      sqlite3_درخت_View_عبارت(pView, p_عبارت, 0);
      sqlite3_درخت_View_برداشت(&pView);
      اكسر;
    }
    حالة TK_FOLLOWING: {
      sqlite3_درخت_View_قلم(pView, "FOLLOWING", بیشتر_مقصد_Follow);
      sqlite3_درخت_View_عبارت(pView, p_عبارت, 0);
      sqlite3_درخت_View_برداشت(&pView);
      اكسر;
    }
  }
}
#پایان_اگر /* SQLITE_OMIT_WINDOWFUNC */

#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
فراغ sqlite3_درخت_View_پنجره(درخت_View *pView, ثابت پنجره *pWin, u8 بیشتر){
  صحيح nElement = 0;
  إذا( pWin==0 ) أرجع;
  إذا( pWin->pFilter ){
    sqlite3_درخت_View_قلم(pView, "FILTER", 1);
    sqlite3_درخت_View_عبارت(pView, pWin->pFilter, 0);
    sqlite3_درخت_View_برداشت(&pView);
    إذا( pWin->eFrmType==TK_FILTER ) أرجع;
  }
  sqlite3_درخت_View_افزودن(&pView, بیشتر);
  إذا( pWin->zName ){
    sqlite3_درخت_View_سطر(pView, "OVER %s (%p)", pWin->zName, pWin);
  }وإلا{
    sqlite3_درخت_View_سطر(pView, "OVER (%p)", pWin);
  }
  إذا( pWin->z_پایه )    nElement++;
  إذا( pWin->p_ترتیب_By ) nElement++;
  إذا( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  إذا( pWin->eExclude ) nElement++;
  إذا( pWin->z_پایه ){
    sqlite3_درخت_View_افزودن(&pView, (--nElement)>0);
    sqlite3_درخت_View_سطر(pView, "window: %s", pWin->z_پایه);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pWin->p_افراز ){
    sqlite3_درخت_View_عبارت_List(pView, pWin->p_افراز, nElement>0,"PARTITION-BY");
  }
  إذا( pWin->p_ترتیب_By ){
    sqlite3_درخت_View_عبارت_List(pView, pWin->p_ترتیب_By, (--nElement)>0, "ORDER-BY");
  }
  إذا( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    حرف zBuf[30];
    ثابت حرف *zFrmType = "ROWS";
    إذا( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    إذا( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(حجم(zBuf),zBuf,"%s%s",zFrmType,
        pWin->b_Implicit_قاب ? " (implied)" : "");
    sqlite3_درخت_View_قلم(pView, zBuf, (--nElement)>0);
    sqlite3_درخت_View_Bound(pView, pWin->e_شروع, pWin->p_شروع, 1);
    sqlite3_درخت_View_Bound(pView, pWin->e_انتها, pWin->p_انتها, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pWin->eExclude ){
    حرف zBuf[30];
    ثابت حرف *zExclude;
    بدل( pWin->eExclude ){
      حالة TK_NO:      zExclude = "NO OTHERS";   اكسر;
      حالة TK_CURRENT: zExclude = "CURRENT ROW"; اكسر;
      حالة TK_گروه:   zExclude = "GROUP";       اكسر;
      حالة TK_TIES:    zExclude = "TIES";        اكسر;
      افتراضي:
        sqlite3_snprintf(حجم(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        اكسر;
    }
    sqlite3_درخت_View_افزودن(&pView, 0);
    sqlite3_درخت_View_سطر(pView, "EXCLUDE %s", zExclude);
    sqlite3_درخت_View_برداشت(&pView);
  }
  sqlite3_درخت_View_برداشت(&pView);
}
#پایان_اگر /* SQLITE_OMIT_WINDOWFUNC */

#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
فراغ sqlite3_درخت_View_Win_Func(درخت_View *pView, ثابت پنجره *pWin, u8 بیشتر){
  إذا( pWin==0 ) أرجع;
  sqlite3_درخت_View_افزودن(&pView, بیشتر);
  sqlite3_درخت_View_سطر(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_درخت_View_پنجره(pView, pWin, 0);
  sqlite3_درخت_View_برداشت(&pView);
}
#پایان_اگر /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
فراغ sqlite3_درخت_View_عبارت(درخت_View *pView, ثابت عبارت *p_عبارت, u8 بیشتر_مقصد_Follow){
  ثابت حرف *zBinOp = 0;   /* Binary operator */
  ثابت حرف *zUniOp = 0;   /* Unary operator */
  حرف zFlgs[200];
  sqlite3_درخت_View_افزودن(&pView, بیشتر_مقصد_Follow);
  إذا( p_عبارت==0 ){
    sqlite3_درخت_View_سطر(pView, "nil");
    sqlite3_درخت_View_برداشت(&pView);
    أرجع;
  }
  إذا( p_عبارت->flags || p_عبارت->aff_عبارت || p_عبارت->vvaFlags || p_عبارت->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, حجم(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_عبارت->flags, p_عبارت->aff_عبارت ? p_عبارت->aff_عبارت : 'n');
    إذا( عبارت_Has_Property(p_عبارت, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_عبارت->w.iJoin);
    }
    إذا( عبارت_Has_Property(p_عبارت, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_عبارت->w.iJoin);
    }
    إذا( عبارت_Has_Property(p_عبارت, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    إذا( عبارت_Has_VVAProperty(p_عبارت, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    إذا( p_عبارت->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_عبارت->iAgg);
    }
    sqlite3_Str_Accum_پایان(&x);
  }وإلا{
    zFlgs[0] = 0;
  }
  بدل( p_عبارت->op ){
    حالة TK_AGG_ستونی: {
      sqlite3_درخت_View_سطر(pView, "AGG{%d:%d}%s",
            p_عبارت->i_جدول, p_عبارت->i_ستونی, zFlgs);
      اكسر;
    }
    حالة TK_ستونی: {
      إذا( p_عبارت->i_جدول<0 ){
        /* This only happens when coding check constraints */
        حرف zOp2[16];
        إذا( p_عبارت->op2 ){
          sqlite3_snprintf(حجم(zOp2),zOp2," op2=0x%02x",p_عبارت->op2);
        }وإلا{
          zOp2[0] = 0;
        }
        sqlite3_درخت_View_سطر(pView, "COLUMN(%d)%s%s",
                                    p_عبارت->i_ستونی, zFlgs, zOp2);
      }وإلا{
        أكد( عبارت_Use_YTab(p_عبارت) );
        sqlite3_درخت_View_سطر(pView, "{%d:%d} pTab=%p%s",
                        p_عبارت->i_جدول, p_عبارت->i_ستونی,
                        p_عبارت->y.pTab, zFlgs);
      }
      إذا( عبارت_Has_Property(p_عبارت, EP_Fixed_ستون) ){
        sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      }
      اكسر;
    }
    حالة TK_INTEGER: {
      إذا( p_عبارت->flags & EP_Int_مقدار ){
        sqlite3_درخت_View_سطر(pView, "%d", p_عبارت->u.i_مقدار);
      }وإلا{
        sqlite3_درخت_View_سطر(pView, "%s", p_عبارت->u.zToken);
      }
      اكسر;
    }
#اگر_تعریف_نشده SQLITE_OMIT_FLOATING_نقطه
    حالة TK_FLOAT: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"%s", p_عبارت->u.zToken);
      اكسر;
    }
#پایان_اگر
    حالة TK_STRING: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"%Q", p_عبارت->u.zToken);
      اكسر;
    }
    حالة TK_NULL: {
      sqlite3_درخت_View_سطر(pView,"NULL");
      اكسر;
    }
    حالة TK_TRUEFALSE: {
      sqlite3_درخت_View_سطر(pView,"%s%s",
         sqlite3_عبارت_Truth_مقدار(p_عبارت) ? "TRUE" : "FALSE", zFlgs);
      اكسر;
    }
#اگر_تعریف_نشده SQLITE_OMIT_BLOB_LITERAL
    حالة TK_BLOB: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"%s", p_عبارت->u.zToken);
      اكسر;
    }
#پایان_اگر
    حالة TK_VARIABLE: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"VARIABLE(%s,%d)",
                          p_عبارت->u.zToken, p_عبارت->i_ستونی);
      اكسر;
    }
    حالة TK_ثبات: {
      sqlite3_درخت_View_سطر(pView,"REGISTER(%d)", p_عبارت->i_جدول);
      اكسر;
    }
    حالة TK_ID: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"ID \"%w\"", p_عبارت->u.zToken);
      اكسر;
    }
    حالة TK_ASTERISK: {
      sqlite3_درخت_View_سطر(pView,"ASKTERISK");
      اكسر;
    }
#اگر_تعریف_نشده SQLITE_OMIT_CAST
    حالة TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView,"CAST %Q", p_عبارت->u.zToken);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      اكسر;
    }
#پایان_اگر /* SQLITE_OMIT_CAST */
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
    حالة TK_ستاره:    zBinOp = "MUL";    اكسر;
    حالة TK_MINUS:   zBinOp = "SUB";    اكسر;
    حالة TK_REM:     zBinOp = "REM";    اكسر;
    حالة TK_BITAND:  zBinOp = "BITAND"; اكسر;
    حالة TK_BITOR:   zBinOp = "BITOR";  اكسر;
    حالة TK_SLASH:   zBinOp = "DIV";    اكسر;
    حالة TK_LSHIFT:  zBinOp = "LSHIFT"; اكسر;
    حالة TK_RSHIFT:  zBinOp = "RSHIFT"; اكسر;
    حالة TK_CONCAT:  zBinOp = "CONCAT"; اكسر;
    حالة TK_DOT:     zBinOp = "DOT";    اكسر;
    حالة TK_حد:   zBinOp = "LIMIT";  اكسر;

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
      أكد( p_عبارت->op2==TK_IS || p_عبارت->op2==TK_ISNOT );
      أكد( p_عبارت->p_راست );
      أكد( sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت->p_راست)->op
                  == TK_TRUEFALSE );
      x = (p_عبارت->op2==TK_ISNOT)*2 + sqlite3_عبارت_Truth_مقدار(p_عبارت->p_راست);
      zUniOp = azOp[x];
      اكسر;
    }

    حالة TK_بازه: {
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView, "SPAN %Q", p_عبارت->u.zToken);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      اكسر;
    }

    حالة TK_COLLATE: {
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
      اكسر;
    }

    حالة TK_AGG_FUNCTION:
    حالة TK_FUNCTION: {
      عبارت_List *pFarg;       /* List of function arguments */
      پنجره *pWin;
      إذا( عبارت_Has_Property(p_عبارت, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }وإلا{
        أكد( عبارت_Use_XList(p_عبارت) );
        pFarg = p_عبارت->x.pList;
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
        pWin = Is_پنجره_Func(p_عبارت) ? p_عبارت->y.pWin : 0;
#وإلا
        pWin = 0;
#پایان_اگر 
      }
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      إذا( p_عبارت->op==TK_AGG_FUNCTION ){
        sqlite3_درخت_View_سطر(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_عبارت->op2, p_عبارت->u.zToken, zFlgs,
                             p_عبارت->pAggInfo ? p_عبارت->pAggInfo->selId : 0,
                             p_عبارت->iAgg, p_عبارت->pAggInfo);
      }وإلا إذا( p_عبارت->op2!=0 ){
        ثابت حرف *zOp2;
        حرف zBuf[8];
        sqlite3_snprintf(حجم(zBuf),zBuf,"0x%02x",p_عبارت->op2);
        zOp2 = zBuf;
        إذا( p_عبارت->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        إذا( p_عبارت->op2==NC_Idx_عبارت ) zOp2 = "NC_IdxExpr";
        إذا( p_عبارت->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        إذا( p_عبارت->op2==NC_Gen_ستون ) zOp2 = "NC_GenCol";
        sqlite3_درخت_View_سطر(pView, "FUNCTION %Q%s op2=%s",
                            p_عبارت->u.zToken, zFlgs, zOp2);
      }وإلا{
        sqlite3_درخت_View_سطر(pView, "FUNCTION %Q%s", p_عبارت->u.zToken, zFlgs);
      }
      إذا( pFarg ){
        sqlite3_درخت_View_عبارت_List(pView, pFarg, pWin!=0 || p_عبارت->p_چپ, 0);
        إذا( p_عبارت->p_چپ ){
          عبارت *pOB = p_عبارت->p_چپ;
          أكد( pOB->op==TK_ترتیب );
          أكد( عبارت_Use_XList(pOB) );
          sqlite3_درخت_View_عبارت_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
      إذا( pWin ){
        sqlite3_درخت_View_پنجره(pView, pWin, 0);
      }
#پایان_اگر
      اكسر;
    }
    حالة TK_ترتیب: {
      sqlite3_درخت_View_عبارت_List(pView, p_عبارت->x.pList, 0, "ORDERBY");
      اكسر;
    }
#اگر_تعریف_نشده SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS: {
      أكد( عبارت_Use_XSelect(p_عبارت) );
      sqlite3_درخت_View_سطر(pView, "EXISTS-expr flags=0x%x", p_عبارت->flags);
      sqlite3_درخت_View_Select(pView, p_عبارت->x.pSelect, 0);
      اكسر;
    }
    حالة TK_SELECT: {
      أكد( عبارت_Use_XSelect(p_عبارت) );
      sqlite3_درخت_View_سطر(pView, "subquery-expr flags=0x%x", p_عبارت->flags);
      sqlite3_درخت_View_Select(pView, p_عبارت->x.pSelect, 0);
      اكسر;
    }
    حالة TK_درون: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      حرف *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_عبارت->flags);
      إذا( p_عبارت->i_جدول ) sqlite3_str_appendf(pStr, " iTable=%d",p_عبارت->i_جدول);
      إذا( عبارت_Has_Property(p_عبارت, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_عبارت->y.sub.regReturn, p_عبارت->y.sub.iAddr);
      }
      z = sqlite3_str_پایان(pStr);
      sqlite3_درخت_View_سطر(pView, z);
      sqlite3_free(z);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 1);
      إذا( عبارت_Use_XSelect(p_عبارت) ){
        sqlite3_درخت_View_Select(pView, p_عبارت->x.pSelect, 0);
      }وإلا{
        sqlite3_درخت_View_عبارت_List(pView, p_عبارت->x.pList, 0, 0);
      }
      اكسر;
    }
#پایان_اگر /* SQLITE_OMIT_SUBQUERY */

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
      sqlite3_درخت_View_سطر(pView, "%s(%d)", 
          p_عبارت->i_جدول ? "NEW" : "OLD", p_عبارت->i_ستونی);
      اكسر;
    }
    حالة TK_CASE: {
      sqlite3_درخت_View_سطر(pView, "CASE");
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 1);
      أكد( عبارت_Use_XList(p_عبارت) );
      sqlite3_درخت_View_عبارت_List(pView, p_عبارت->x.pList, 0, 0);
      اكسر;
    }
#اگر_تعریف_نشده SQLITE_OMIT_TRIGGER
    حالة TK_RAISE: {
      ثابت حرف *zType = "unk";
      بدل( p_عبارت->aff_عبارت ){
        حالة OE_Rollback:   zType = "rollback";  اكسر;
        حالة OE_Abort:      zType = "abort";     اكسر;
        حالة OE_Fail:       zType = "fail";      اكسر;
        حالة OE_Ignore:     zType = "ignore";    اكسر;
      }
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      sqlite3_درخت_View_سطر(pView, "RAISE %s", zType);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      اكسر;
    }
#پایان_اگر
    حالة TK_تطابق: {
      sqlite3_درخت_View_سطر(pView, "MATCH {%d:%d}%s",
                          p_عبارت->i_جدول, p_عبارت->i_ستونی, zFlgs);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_راست, 0);
      اكسر;
    }
    حالة TK_وکتور: {
      حرف *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      أكد( عبارت_Use_XList(p_عبارت) );
      sqlite3_درخت_View_Bare_عبارت_List(pView, p_عبارت->x.pList, z);
      sqlite3_free(z);
      اكسر;
    }
    حالة TK_SELECT_ستونی: {
      sqlite3_درخت_View_سطر(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_عبارت->i_ستونی, p_عبارت->i_جدول-1,
              p_عبارت->p_راست==p_عبارت->p_چپ ? " (SELECT-owner)" : "");
      أكد( عبارت_Use_XSelect(p_عبارت->p_چپ) );
      sqlite3_درخت_View_Select(pView, p_عبارت->p_چپ->x.pSelect, 0);
      اكسر;
    }
    حالة TK_IF_NULL_ردیف: {
      sqlite3_درخت_View_سطر(pView, "IF-NULL-ROW %d", p_عبارت->i_جدول);
      sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
      اكسر;
    }
    حالة TK_ERROR: {
      عبارت tmp;
      sqlite3_درخت_View_سطر(pView, "ERROR");
      tmp = *p_عبارت;
      tmp.op = p_عبارت->op2;
      sqlite3_درخت_View_عبارت(pView, &tmp, 0);
      اكسر;
    }
    حالة TK_ردیف: {
      إذا( p_عبارت->i_ستونی<=0 ){
        sqlite3_درخت_View_سطر(pView, "First FROM table rowid");
      }وإلا{
        sqlite3_درخت_View_سطر(pView, "First FROM table column %d",
            p_عبارت->i_ستونی-1);
      }
      اكسر;
    }
    افتراضي: {
      sqlite3_درخت_View_سطر(pView, "op=%d", p_عبارت->op);
      اكسر;
    }
  }
  إذا( zBinOp ){
    sqlite3_درخت_View_سطر(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 1);
    sqlite3_درخت_View_عبارت(pView, p_عبارت->p_راست, 0);
  }وإلا إذا( zUniOp ){
    sqlite3_درخت_View_سطر(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_درخت_View_عبارت(pView, p_عبارت->p_چپ, 0);
  }
  sqlite3_درخت_View_برداشت(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
فراغ sqlite3_درخت_View_Bare_عبارت_List(
  درخت_View *pView,
  ثابت عبارت_List *pList,
  ثابت حرف *zLabel
){
  إذا( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  إذا( pList==0 ){
    sqlite3_درخت_View_سطر(pView, "%s (empty)", zLabel);
  }وإلا{
    صحيح i;
    sqlite3_درخت_View_سطر(pView, "%s", zLabel);
    لكل(i=0; i<pList->n_عبارت; i++){
      صحيح j = pList->a[i].u.x.i_ترتیب_By_ستون;
      u8 مرتب_سازی_Flags = pList->a[i].fg.مرتب_سازی_Flags;
      حرف *zName = pList->a[i].zEName;
      صحيح بیشتر_مقصد_Follow = i<pList->n_عبارت - 1;
      إذا( j || zName || مرتب_سازی_Flags ){
        sqlite3_درخت_View_افزودن(&pView, بیشتر_مقصد_Follow);
        بیشتر_مقصد_Follow = 0;
        sqlite3_درخت_View_سطر(pView, 0);
        إذا( zName ){
          بدل( pList->a[i].fg.eEName ){
            افتراضي:
              fprintf(stdout, "AS %s ", zName);
              اكسر;
            حالة ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              إذا( pList->a[i].fg.b_استفاده_شده ) fprintf(stdout, "(used) ");
              إذا( pList->a[i].fg.b_Using_جمله ) fprintf(stdout, "(USING-term) ");
              إذا( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              اكسر;
            حالة ENAME_بازه:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              اكسر;
          }
        }
        إذا( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        إذا( مرتب_سازی_Flags & KEYINFO_ترتیب_DESC ){
          fprintf(stdout, "DESC ");
        }وإلا إذا( مرتب_سازی_Flags & KEYINFO_ترتیب_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_درخت_View_عبارت(pView, pList->a[i].p_عبارت, بیشتر_مقصد_Follow);
      إذا( j || zName || مرتب_سازی_Flags ){
        sqlite3_درخت_View_برداشت(&pView);
      }
    }
  }
}
فراغ sqlite3_درخت_View_عبارت_List(
  درخت_View *pView,
  ثابت عبارت_List *pList,
  u8 بیشتر_مقصد_Follow,
  ثابت حرف *zLabel
){
  sqlite3_درخت_View_افزودن(&pView, بیشتر_مقصد_Follow);
  sqlite3_درخت_View_Bare_عبارت_List(pView, pList, zLabel);
  sqlite3_درخت_View_برداشت(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
فراغ sqlite3_درخت_View_Bare_Id_List(
  درخت_View *pView,
  ثابت IdList *pList,
  ثابت حرف *zLabel
){
  إذا( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  إذا( pList==0 ){
    sqlite3_درخت_View_سطر(pView, "%s (empty)", zLabel);
  }وإلا{
    صحيح i;
    sqlite3_درخت_View_سطر(pView, "%s", zLabel);
    لكل(i=0; i<pList->nId; i++){
      حرف *zName = pList->a[i].zName;
      صحيح بیشتر_مقصد_Follow = i<pList->nId - 1;
      إذا( zName==0 ) zName = "(null)";
      sqlite3_درخت_View_افزودن(&pView, بیشتر_مقصد_Follow);
      sqlite3_درخت_View_سطر(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_درخت_View_برداشت(&pView);
    }
  }
}
فراغ sqlite3_درخت_View_Id_List(
  درخت_View *pView,
  ثابت IdList *pList,
  u8 بیشتر_مقصد_Follow,
  ثابت حرف *zLabel
){
  sqlite3_درخت_View_افزودن(&pView, بیشتر_مقصد_Follow);
  sqlite3_درخت_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_درخت_View_برداشت(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
فراغ sqlite3_درخت_View_Upsert(
  درخت_View *pView,
  ثابت Upsert *pUpsert,
  u8 بیشتر_مقصد_Follow
){
  إذا( pUpsert==0 ) أرجع;
  sqlite3_درخت_View_افزودن(&pView, بیشتر_مقصد_Follow);
  طالما( pUpsert ){
    صحيح n;
    sqlite3_درخت_View_افزودن(&pView, pUpsert->p_بعدی_Upsert!=0 || بیشتر_مقصد_Follow);
    sqlite3_درخت_View_سطر(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->p_Upsert_مجموعه!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_درخت_View_عبارت_List(pView, pUpsert->p_Upsert_هدف, (n--)>0, "TARGET");
    sqlite3_درخت_View_عبارت_List(pView, pUpsert->p_Upsert_مجموعه, (n--)>0, "SET");
    إذا( pUpsert->pUpsertWhere ){
      sqlite3_درخت_View_قلم(pView, "WHERE", (n--)>0);
      sqlite3_درخت_View_عبارت(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_درخت_View_برداشت(&pView);
    }
    sqlite3_درخت_View_برداشت(&pView);
    pUpsert = pUpsert->p_بعدی_Upsert;
  }
  sqlite3_درخت_View_برداشت(&pView);
}

#إذا TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
فراغ sqlite3_درخت_View_Delete(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت عبارت *pWhere,
  ثابت عبارت_List *p_ترتیب_By,
  ثابت عبارت *p_حد,
  ثابت Trigger *pTrigger
){
  صحيح n = 0;
  درخت_View *pView = 0;
  sqlite3_درخت_View_افزودن(&pView, 0);
  sqlite3_درخت_View_سطر(pView, "DELETE");
  إذا( pWith ) n++;
  إذا( pTabList ) n++;
  إذا( pWhere ) n++;
  إذا( p_ترتیب_By ) n++;
  إذا( p_حد ) n++;
  إذا( pTrigger ) n++;
  إذا( pWith ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_With(pView, pWith, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pTabList ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "FROM");
    sqlite3_درخت_View_Src_List(pView, pTabList);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pWhere ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "WHERE");
    sqlite3_درخت_View_عبارت(pView, pWhere, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( p_ترتیب_By ){
    sqlite3_درخت_View_عبارت_List(pView, p_ترتیب_By, (--n)>0, "ORDER-BY");
  }
  إذا( p_حد ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "LIMIT");
    sqlite3_درخت_View_عبارت(pView, p_حد, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pTrigger ){
    sqlite3_درخت_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_درخت_View_برداشت(&pView);
}
#پایان_اگر /* TREETRACE_ENABLED */

#إذا TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
فراغ sqlite3_درخت_View_درج(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت IdList *p_ستونی_List,
  ثابت Select *pSelect,
  ثابت عبارت_List *p_عبارت_List,
  صحيح onError,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  درخت_View *pView = 0;
  صحيح n = 0;
  ثابت حرف *zLabel = "INSERT";
  بدل( onError ){
    حالة OE_Replace:  zLabel = "REPLACE";             اكسر;
    حالة OE_Ignore:   zLabel = "INSERT OR IGNORE";    اكسر;
    حالة OE_Rollback: zLabel = "INSERT OR ROLLBACK";  اكسر;
    حالة OE_Abort:    zLabel = "INSERT OR ABORT";     اكسر;
    حالة OE_Fail:     zLabel = "INSERT OR FAIL";      اكسر;
  }
  sqlite3_درخت_View_افزودن(&pView, 0);
  sqlite3_درخت_View_سطر(pView, zLabel);
  إذا( pWith ) n++;
  إذا( pTabList ) n++;
  إذا( p_ستونی_List ) n++;
  إذا( pSelect ) n++;
  إذا( p_عبارت_List ) n++;
  إذا( pUpsert ) n++;
  إذا( pTrigger ) n++;
  إذا( pWith ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_With(pView, pWith, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pTabList ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "INTO");
    sqlite3_درخت_View_Src_List(pView, pTabList);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( p_ستونی_List ){
    sqlite3_درخت_View_Id_List(pView, p_ستونی_List, (--n)>0, "COLUMNS");
  }
  إذا( pSelect ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "DATA-SOURCE");
    sqlite3_درخت_View_Select(pView, pSelect, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( p_عبارت_List ){
    sqlite3_درخت_View_عبارت_List(pView, p_عبارت_List, (--n)>0, "VALUES");
  }
  إذا( pUpsert ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "UPSERT");
    sqlite3_درخت_View_Upsert(pView, pUpsert, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pTrigger ){
    sqlite3_درخت_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_درخت_View_برداشت(&pView);
}
#پایان_اگر /* TREETRACE_ENABLED */

#إذا TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
فراغ sqlite3_درخت_View_Update(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت عبارت_List *pChanges,
  ثابت عبارت *pWhere,
  صحيح onError,
  ثابت عبارت_List *p_ترتیب_By,
  ثابت عبارت *p_حد,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  صحيح n = 0;
  درخت_View *pView = 0;
  ثابت حرف *zLabel = "UPDATE";
  بدل( onError ){
    حالة OE_Replace:  zLabel = "UPDATE OR REPLACE";   اكسر;
    حالة OE_Ignore:   zLabel = "UPDATE OR IGNORE";    اكسر;
    حالة OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  اكسر;
    حالة OE_Abort:    zLabel = "UPDATE OR ABORT";     اكسر;
    حالة OE_Fail:     zLabel = "UPDATE OR FAIL";      اكسر;
  }
  sqlite3_درخت_View_افزودن(&pView, 0);
  sqlite3_درخت_View_سطر(pView, zLabel);
  إذا( pWith ) n++;
  إذا( pTabList ) n++;
  إذا( pChanges ) n++;
  إذا( pWhere ) n++;
  إذا( p_ترتیب_By ) n++;
  إذا( p_حد ) n++;
  إذا( pUpsert ) n++;
  إذا( pTrigger ) n++;
  إذا( pWith ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_With(pView, pWith, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pTabList ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "FROM");
    sqlite3_درخت_View_Src_List(pView, pTabList);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pChanges ){
    sqlite3_درخت_View_عبارت_List(pView, pChanges, (--n)>0, "SET");
  }
  إذا( pWhere ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "WHERE");
    sqlite3_درخت_View_عبارت(pView, pWhere, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( p_ترتیب_By ){
    sqlite3_درخت_View_عبارت_List(pView, p_ترتیب_By, (--n)>0, "ORDER-BY");
  }
  إذا( p_حد ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "LIMIT");
    sqlite3_درخت_View_عبارت(pView, p_حد, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pUpsert ){
    sqlite3_درخت_View_افزودن(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "UPSERT");
    sqlite3_درخت_View_Upsert(pView, pUpsert, 0);
    sqlite3_درخت_View_برداشت(&pView);
  }
  إذا( pTrigger ){
    sqlite3_درخت_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_درخت_View_برداشت(&pView);
}
#پایان_اگر /* TREETRACE_ENABLED */

#اگر_تعریف_نشده SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
فراغ sqlite3_درخت_View_Trigger_گام(
  درخت_View *pView,
  ثابت Trigger_گام *p_گام,
  u8 بیشتر_مقصد_Follow,
  u8 show_پر_List
){
  صحيح cnt = 0;
  إذا( p_گام==0 ) أرجع;
  sqlite3_درخت_View_افزودن(&pView, 
      بیشتر_مقصد_Follow || (show_پر_List && p_گام->p_بعدی!=0));
  افعل{
    إذا( cnt++ && p_گام->p_بعدی==0 ){
      sqlite3_درخت_View_برداشت(&pView);
      sqlite3_درخت_View_افزودن(&pView, 0);
    }
    sqlite3_درخت_View_سطر(pView, "%s", p_گام->z_بازه ? p_گام->z_بازه : "RETURNING");
  }طالما( show_پر_List && (p_گام = p_گام->p_بعدی)!=0 );
  sqlite3_درخت_View_برداشت(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
فراغ sqlite3_درخت_View_Trigger(
  درخت_View *pView,
  ثابت Trigger *pTrigger,
  u8 بیشتر_مقصد_Follow,
  u8 show_پر_List
){
  صحيح cnt = 0;
  إذا( pTrigger==0 ) أرجع;
  sqlite3_درخت_View_افزودن(&pView,
     بیشتر_مقصد_Follow || (show_پر_List && pTrigger->p_بعدی!=0));
  افعل{
    إذا( cnt++ && pTrigger->p_بعدی==0 ){
      sqlite3_درخت_View_برداشت(&pView);
      sqlite3_درخت_View_افزودن(&pView, 0);
    }
    sqlite3_درخت_View_سطر(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_درخت_View_افزودن(&pView, 0);
    sqlite3_درخت_View_Trigger_گام(pView, pTrigger->گام_list, 0, 1);
    sqlite3_درخت_View_برداشت(&pView);
  }طالما( show_پر_List && (pTrigger = pTrigger->p_بعدی)!=0 );
  sqlite3_درخت_View_برداشت(&pView);
}
#پایان_اگر /* SQLITE_OMIT_TRIGGER */
  

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
فراغ sqlite3_Show_عبارت(ثابت عبارت *p){ sqlite3_درخت_View_عبارت(0,p,0); }
فراغ sqlite3_Show_عبارت_List(ثابت عبارت_List *p){ sqlite3_درخت_View_عبارت_List(0,p,0,0);}
فراغ sqlite3ShowIdList(ثابت IdList *p){ sqlite3_درخت_View_Id_List(0,p,0,0); }
فراغ sqlite3ShowSrcList(ثابت SrcList *p){
  درخت_View *pView = 0;
  sqlite3_درخت_View_افزودن(&pView, 0);
  sqlite3_درخت_View_سطر(pView, "SRCLIST");
  sqlite3_درخت_View_Src_List(pView,p);
  sqlite3_درخت_View_برداشت(&pView);
}
فراغ sqlite3ShowSelect(ثابت Select *p){ sqlite3_درخت_View_Select(0,p,0); }
فراغ sqlite3ShowWith(ثابت With *p){ sqlite3_درخت_View_With(0,p,0); }
فراغ sqlite3ShowUpsert(ثابت Upsert *p){ sqlite3_درخت_View_Upsert(0,p,0); }
#اگر_تعریف_نشده SQLITE_OMIT_TRIGGER
فراغ sqlite3_Show_Trigger_گام(ثابت Trigger_گام *p){
  sqlite3_درخت_View_Trigger_گام(0,p,0,0);
}
فراغ sqlite3_Show_Trigger_گام_List(ثابت Trigger_گام *p){
  sqlite3_درخت_View_Trigger_گام(0,p,0,1);
}
فراغ sqlite3ShowTrigger(ثابت Trigger *p){ sqlite3_درخت_View_Trigger(0,p,0,0); }
فراغ sqlite3ShowTriggerList(ثابت Trigger *p){ sqlite3_درخت_View_Trigger(0,p,0,1);}
#پایان_اگر
#اگر_تعریف_نشده SQLITE_OMIT_WINDOWFUNC
فراغ sqlite3_Show_پنجره(ثابت پنجره *p){ sqlite3_درخت_View_پنجره(0,p,0); }
فراغ sqlite3ShowWinFunc(ثابت پنجره *p){ sqlite3_درخت_View_Win_Func(0,p,0); }
#پایان_اگر

#پایان_اگر /* SQLITE_DEBUG */
