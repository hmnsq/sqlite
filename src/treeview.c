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
#که_تعریف_شوی SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
ساكن فراغ sqlite3_ونه_View_ټېل(ونه_View **pp, u8 نور_منزل_Follow){
  ونه_View *p = *pp;
  إذا( p==0 ){
    *pp = p = sqlite3_malloc64( حجم(*p) );
    إذا( p==0 ) أرجع;
    memset(p, 0, حجم(*p));
  }وإلا{
    p->iLevel++;
  }
  أكد( نور_منزل_Follow==0 || نور_منزل_Follow==1 );
  إذا( p->iLevel<(صحيح)حجم(p->b_کرښه) ) p->b_کرښه[p->iLevel] = نور_منزل_Follow;
}

/*
** Finished with one layer of the tree
*/
ساكن فراغ sqlite3_ونه_View_راوباسه(ونه_View **pp){
  ونه_View *p = *pp;
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
فراغ sqlite3_ونه_View_کرښه(ونه_View *p, ثابت حرف *zFormat, ...){
  va_list ap;
  صحيح i;
  StrAccum acc;
  حرف zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
  إذا( p ){
    لكل(i=0; i<p->iLevel && i<(صحيح)حجم(p->b_کرښه)-1; i++){
      sqlite3_str_append(&acc, p->b_کرښه[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_کرښه[i] ? "|-- " : "'-- ", 4);
  }
  إذا( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    أكد( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3_Str_Accum_پای(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
ساكن فراغ sqlite3_ونه_View_توکی(ونه_View *p, ثابت حرف *zLabel,u8 نور_Follows){
  sqlite3_ونه_View_ټېل(&p, نور_Follows);
  sqlite3_ونه_View_کرښه(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
فراغ sqlite3_ونه_View_ستنه_List(
  ونه_View *pView,
  ثابت ستنه *a_ستون,
  صحيح n_ستون,
  u8 نور_منزل_Follow
){
  صحيح i;
  sqlite3_ونه_View_ټېل(&pView, نور_منزل_Follow);
  sqlite3_ونه_View_کرښه(pView, "COLUMNS");
  لكل(i=0; i<n_ستون; i++){
    u16 flg = a_ستون[i].ستون_Flags;
    صحيح ستون_نور_منزل_Follow = i<(n_ستون - 1);
    sqlite3_ونه_View_ټېل(&pView, ستون_نور_منزل_Follow);
    sqlite3_ونه_View_کرښه(pView, 0);
    چاپ_کړه(" %s", a_ستون[i].zCnName);
    بدل( a_ستون[i].eCType ){
      حالة COLTYPE_ANY:      چاپ_کړه(" ANY");        اكسر;
      حالة COLTYPE_BLOB:     چاپ_کړه(" BLOB");       اكسر;
      حالة COLTYPE_INT:      چاپ_کړه(" INT");        اكسر;
      حالة COLTYPE_INTEGER:  چاپ_کړه(" INTEGER");    اكسر;
      حالة COLTYPE_REAL:     چاپ_کړه(" REAL");       اكسر;
      حالة COLTYPE_متن:     چاپ_کړه(" TEXT");       اكسر;
      حالة COLTYPE_CUSTOM: {
        إذا( flg & COLFLAG_HASTYPE ){
          ثابت حرف *z = a_ستون[i].zCnName;
          z += مزي_اوږدوالی(z)+1;
          چاپ_کړه(" X-%s", z);
          اكسر;
        }
      }
    }
    إذا( flg & COLFLAG_PRIMKEY ) چاپ_کړه(" PRIMARY KEY");
    إذا( flg & COLFLAG_HIDDEN ) چاپ_کړه(" HIDDEN");
#که_تعریف_شوی COLFLAG_NOEXPAND
    إذا( flg & COLFLAG_NOEXPAND ) چاپ_کړه(" NO-EXPAND");
#د_که_پای
    إذا( flg ) چاپ_کړه(" flags=%04x", flg);
    چاپ_کړه("\n");      
    fflush(stdout);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  sqlite3_ونه_View_راوباسه(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
فراغ sqlite3_ونه_View_With(ونه_View *pView, ثابت With *pWith, u8 نور_منزل_Follow){
  صحيح i;
  إذا( pWith==0 ) أرجع;
  إذا( pWith->nCte==0 ) أرجع;
  إذا( pWith->pOuter ){
    sqlite3_ونه_View_کرښه(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }وإلا{
    sqlite3_ونه_View_کرښه(pView, "WITH (0x%p)", pWith);
  }
  إذا( pWith->nCte>0 ){
    sqlite3_ونه_View_ټېل(&pView, نور_منزل_Follow);
    لكل(i=0; i<pWith->nCte; i++){
      StrAccum x;
      حرف z_کرښه[1000];
      ثابت بنية Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_کرښه, حجم(z_کرښه), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      إذا( pCte->p_ستون_شمېر && pCte->p_ستون_شمېر->n_بیان>0 ){
        حرف cSep = '(';
        صحيح j;
        لكل(j=0; j<pCte->p_ستون_شمېر->n_بیان; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->p_ستون_شمېر->a[j].zEName);
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
      sqlite3_Str_Accum_پای(&x);
      sqlite3_ونه_View_توکی(pView, z_کرښه, i<pWith->nCte-1);
      sqlite3_ونه_View_Select(pView, pCte->pSelect, 0);
      sqlite3_ونه_View_راوباسه(&pView);
    }
    sqlite3_ونه_View_راوباسه(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
فراغ sqlite3_ونه_View_Src_List(ونه_View *pView, ثابت SrcList *pSrc){
  صحيح i;
  إذا( pSrc==0 ) أرجع;
  لكل(i=0; i<pSrc->nSrc; i++){
    ثابت Src_توکی *p_توکی = &pSrc->a[i];
    StrAccum x;
    صحيح n = 0;
    حرف z_کرښه[1000];
    sqlite3StrAccumInit(&x, 0, z_کرښه, حجم(z_کرښه), 0);
    x.printfFlags |= SQLITE_PRINTF_دننی;
    sqlite3_str_appendf(&x, "{%d:*} %!S", p_توکی->iCursor, p_توکی);
    إذا( p_توکی->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           p_توکی->pSTab->zName, p_توکی->pSTab->n_ستون, p_توکی->pSTab, 
           p_توکی->ستون_کارول_شوی,
           p_توکی->fg.rowid_کارول_شوی ? "+rowid" : "");
    }
    إذا( (p_توکی->fg.jointype & (JT_کیڼ|JT_ښی))==(JT_کیڼ|JT_ښی) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }وإلا إذا( p_توکی->fg.jointype & JT_کیڼ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }وإلا إذا( p_توکی->fg.jointype & JT_ښی ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }وإلا إذا( p_توکی->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    إذا( p_توکی->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    إذا( p_توکی->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    إذا( p_توکی->fg.isCte ){
      ساكن ثابت حرف *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          p_توکی->u2.pCteUse->nUse,
                          aMat[p_توکی->u2.pCteUse->eM10d]);
    }
    إذا( p_توکی->fg.isOn || (p_توکی->fg.isUsing==0 && p_توکی->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    إذا( p_توکی->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    إذا( p_توکی->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    إذا( p_توکی->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    إذا( p_توکی->fg.له_لارې_Coroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    إذا( p_توکی->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    إذا( p_توکی->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    إذا( p_توکی->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    إذا( p_توکی->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    إذا( p_توکی->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3_Str_Accum_پای(&x);
    sqlite3_ونه_View_توکی(pView, z_کرښه, i<pSrc->nSrc-1);
    n = 0;
    إذا( p_توکی->fg.isSubquery ) n++;
    إذا( p_توکی->fg.isTabFunc ) n++;
    إذا( p_توکی->fg.isUsing || p_توکی->u3.pOn!=0 ) n++;
    إذا( p_توکی->fg.isUsing ){
      sqlite3_ونه_View_Id_List(pView, p_توکی->u3.pUsing, (--n)>0, "USING");
    }وإلا إذا( p_توکی->u3.pOn!=0 ){
      sqlite3_ونه_View_توکی(pView, "ON", (--n)>0);
      sqlite3_ونه_View_بیان(pView, p_توکی->u3.pOn, 0);
      sqlite3_ونه_View_راوباسه(&pView);
    }
    إذا( p_توکی->fg.isSubquery ){
      أكد( n==1 );
      إذا( p_توکی->pSTab ){
        جدول *pTab = p_توکی->pSTab;
        sqlite3_ونه_View_ستنه_List(pView, pTab->a_ستون, pTab->n_ستون, 1);
      }
      أكد( (صحيح)p_توکی->fg.isNestedFrom == IsNestedFrom(p_توکی) );
      sqlite3_ونه_View_Select(pView, p_توکی->u4.pSubq->pSelect, 0);
    }
    إذا( p_توکی->fg.isTabFunc ){
      sqlite3_ونه_View_بیان_List(pView, p_توکی->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_ونه_View_راوباسه(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
فراغ sqlite3_ونه_View_Select(ونه_View *pView, ثابت Select *p, u8 نور_منزل_Follow){
  صحيح n = 0;
  صحيح cnt = 0;
  إذا( p==0 ){
    sqlite3_ونه_View_کرښه(pView, "nil-SELECT");
    أرجع;
  } 
  sqlite3_ونه_View_ټېل(&pView, نور_منزل_Follow);
  إذا( p->pWith ){
    sqlite3_ونه_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_ونه_View_ټېل(&pView, 1);
  }
  افعل{
    إذا( p->selFlags & SF_WhereBegin ){
      sqlite3_ونه_View_کرښه(pView, "sqlite3WhereBegin()");
    }وإلا{
      sqlite3_ونه_View_کرښه(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (صحيح)p->n_Select_لیکه
      );
    }
    إذا( cnt++ ) sqlite3_ونه_View_راوباسه(&pView);
    إذا( p->p_مخکینی ){
      n = 1000;
    }وإلا{
      n = 0;
      إذا( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      إذا( p->pWhere ) n++;
      إذا( p->p_ډله_By ) n++;
      إذا( p->pHaving ) n++;
      إذا( p->p_ترتیب_By ) n++;
      إذا( p->p_حد ) n++;
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
      إذا( p->pWin ) n++;
      إذا( p->pWinDefn ) n++;
#د_که_پای
    }
    إذا( p->pEList ){
      sqlite3_ونه_View_بیان_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    إذا( p->pWin ){
      کړکۍ *pX;
      sqlite3_ونه_View_ټېل(&pView, (n--)>0);
      sqlite3_ونه_View_کرښه(pView, "window-functions");
      لكل(pX=p->pWin; pX; pX=pX->p_بل_Win){
        sqlite3_ونه_View_Win_Func(pView, pX, pX->p_بل_Win!=0);
      }
      sqlite3_ونه_View_راوباسه(&pView);
    }
#د_که_پای
    إذا( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_ونه_View_ټېل(&pView, (n--)>0);
      sqlite3_ونه_View_کرښه(pView, "FROM");
      sqlite3_ونه_View_Src_List(pView, p->pSrc);
      sqlite3_ونه_View_راوباسه(&pView);
    }
    إذا( p->pWhere ){
      sqlite3_ونه_View_توکی(pView, "WHERE", (n--)>0);
      sqlite3_ونه_View_بیان(pView, p->pWhere, 0);
      sqlite3_ونه_View_راوباسه(&pView);
    }
    إذا( p->p_ډله_By ){
      sqlite3_ونه_View_بیان_List(pView, p->p_ډله_By, (n--)>0, "GROUPBY");
    }
    إذا( p->pHaving ){
      sqlite3_ونه_View_توکی(pView, "HAVING", (n--)>0);
      sqlite3_ونه_View_بیان(pView, p->pHaving, 0);
      sqlite3_ونه_View_راوباسه(&pView);
    }
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
    إذا( p->pWinDefn ){
      کړکۍ *pX;
      sqlite3_ونه_View_توکی(pView, "WINDOW", (n--)>0);
      لكل(pX=p->pWinDefn; pX; pX=pX->p_بل_Win){
        sqlite3_ونه_View_کړکۍ(pView, pX, pX->p_بل_Win!=0);
      }
      sqlite3_ونه_View_راوباسه(&pView);
    }
#د_که_پای
    إذا( p->p_ترتیب_By ){
      sqlite3_ونه_View_بیان_List(pView, p->p_ترتیب_By, (n--)>0, "ORDERBY");
    }
    إذا( p->p_حد ){
      sqlite3_ونه_View_توکی(pView, "LIMIT", (n--)>0);
      sqlite3_ونه_View_بیان(pView, p->p_حد->p_کیڼ, p->p_حد->p_ښی!=0);
      إذا( p->p_حد->p_ښی ){
        sqlite3_ونه_View_توکی(pView, "OFFSET", 0);
        sqlite3_ونه_View_بیان(pView, p->p_حد->p_ښی, 0);
        sqlite3_ونه_View_راوباسه(&pView);
      }
      sqlite3_ونه_View_راوباسه(&pView);
    }
    إذا( p->p_مخکینی ){
      ثابت حرف *zOp = "UNION";
      بدل( p->op ){
        حالة TK_ALL:         zOp = "UNION ALL";  اكسر;
        حالة TK_INTERSECT:   zOp = "INTERSECT";  اكسر;
        حالة TK_EXCEPT:      zOp = "EXCEPT";     اكسر;
      }
      sqlite3_ونه_View_توکی(pView, zOp, 1);
    }
    p = p->p_مخکینی;
  }طالما( p!=0 );
  sqlite3_ونه_View_راوباسه(&pView);
}

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
فراغ sqlite3_ونه_View_Bound(
  ونه_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  بیان *p_بیان,            /* Value for PRECEDING or FOLLOWING */
  u8 نور_منزل_Follow         /* True if more to follow */
){
  بدل( eBound ){
    حالة TK_UNBOUNDED: {
      sqlite3_ونه_View_توکی(pView, "UNBOUNDED", نور_منزل_Follow);
      sqlite3_ونه_View_راوباسه(&pView);
      اكسر;
    }
    حالة TK_CURRENT: {
      sqlite3_ونه_View_توکی(pView, "CURRENT", نور_منزل_Follow);
      sqlite3_ونه_View_راوباسه(&pView);
      اكسر;
    }
    حالة TK_PRECEDING: {
      sqlite3_ونه_View_توکی(pView, "PRECEDING", نور_منزل_Follow);
      sqlite3_ونه_View_بیان(pView, p_بیان, 0);
      sqlite3_ونه_View_راوباسه(&pView);
      اكسر;
    }
    حالة TK_FOLLOWING: {
      sqlite3_ونه_View_توکی(pView, "FOLLOWING", نور_منزل_Follow);
      sqlite3_ونه_View_بیان(pView, p_بیان, 0);
      sqlite3_ونه_View_راوباسه(&pView);
      اكسر;
    }
  }
}
#د_که_پای /* SQLITE_OMIT_WINDOWFUNC */

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
فراغ sqlite3_ونه_View_کړکۍ(ونه_View *pView, ثابت کړکۍ *pWin, u8 نور){
  صحيح nElement = 0;
  إذا( pWin==0 ) أرجع;
  إذا( pWin->pFilter ){
    sqlite3_ونه_View_توکی(pView, "FILTER", 1);
    sqlite3_ونه_View_بیان(pView, pWin->pFilter, 0);
    sqlite3_ونه_View_راوباسه(&pView);
    إذا( pWin->eFrmType==TK_FILTER ) أرجع;
  }
  sqlite3_ونه_View_ټېل(&pView, نور);
  إذا( pWin->zName ){
    sqlite3_ونه_View_کرښه(pView, "OVER %s (%p)", pWin->zName, pWin);
  }وإلا{
    sqlite3_ونه_View_کرښه(pView, "OVER (%p)", pWin);
  }
  إذا( pWin->z_بنسټ )    nElement++;
  إذا( pWin->p_ترتیب_By ) nElement++;
  إذا( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  إذا( pWin->eExclude ) nElement++;
  إذا( pWin->z_بنسټ ){
    sqlite3_ونه_View_ټېل(&pView, (--nElement)>0);
    sqlite3_ونه_View_کرښه(pView, "window: %s", pWin->z_بنسټ);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pWin->p_برخه_کول ){
    sqlite3_ونه_View_بیان_List(pView, pWin->p_برخه_کول, nElement>0,"PARTITION-BY");
  }
  إذا( pWin->p_ترتیب_By ){
    sqlite3_ونه_View_بیان_List(pView, pWin->p_ترتیب_By, (--nElement)>0, "ORDER-BY");
  }
  إذا( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    حرف zBuf[30];
    ثابت حرف *zFrmType = "ROWS";
    إذا( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    إذا( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(حجم(zBuf),zBuf,"%s%s",zFrmType,
        pWin->b_Implicit_چوکاټ ? " (implied)" : "");
    sqlite3_ونه_View_توکی(pView, zBuf, (--nElement)>0);
    sqlite3_ونه_View_Bound(pView, pWin->e_پیل, pWin->p_پیل, 1);
    sqlite3_ونه_View_Bound(pView, pWin->e_پای_ټکی, pWin->p_پای_ټکی, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pWin->eExclude ){
    حرف zBuf[30];
    ثابت حرف *zExclude;
    بدل( pWin->eExclude ){
      حالة TK_NO:      zExclude = "NO OTHERS";   اكسر;
      حالة TK_CURRENT: zExclude = "CURRENT ROW"; اكسر;
      حالة TK_ډله:   zExclude = "GROUP";       اكسر;
      حالة TK_TIES:    zExclude = "TIES";        اكسر;
      افتراضي:
        sqlite3_snprintf(حجم(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        اكسر;
    }
    sqlite3_ونه_View_ټېل(&pView, 0);
    sqlite3_ونه_View_کرښه(pView, "EXCLUDE %s", zExclude);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  sqlite3_ونه_View_راوباسه(&pView);
}
#د_که_پای /* SQLITE_OMIT_WINDOWFUNC */

#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
فراغ sqlite3_ونه_View_Win_Func(ونه_View *pView, ثابت کړکۍ *pWin, u8 نور){
  إذا( pWin==0 ) أرجع;
  sqlite3_ونه_View_ټېل(&pView, نور);
  sqlite3_ونه_View_کرښه(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_ونه_View_کړکۍ(pView, pWin, 0);
  sqlite3_ونه_View_راوباسه(&pView);
}
#د_که_پای /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
فراغ sqlite3_ونه_View_بیان(ونه_View *pView, ثابت بیان *p_بیان, u8 نور_منزل_Follow){
  ثابت حرف *zBinOp = 0;   /* Binary operator */
  ثابت حرف *zUniOp = 0;   /* Unary operator */
  حرف zFlgs[200];
  sqlite3_ونه_View_ټېل(&pView, نور_منزل_Follow);
  إذا( p_بیان==0 ){
    sqlite3_ونه_View_کرښه(pView, "nil");
    sqlite3_ونه_View_راوباسه(&pView);
    أرجع;
  }
  إذا( p_بیان->flags || p_بیان->aff_بیان || p_بیان->vvaFlags || p_بیان->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, حجم(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_بیان->flags, p_بیان->aff_بیان ? p_بیان->aff_بیان : 'n');
    إذا( بیان_Has_Property(p_بیان, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_بیان->w.iJoin);
    }
    إذا( بیان_Has_Property(p_بیان, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_بیان->w.iJoin);
    }
    إذا( بیان_Has_Property(p_بیان, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    إذا( بیان_Has_VVAProperty(p_بیان, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    إذا( p_بیان->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_بیان->iAgg);
    }
    sqlite3_Str_Accum_پای(&x);
  }وإلا{
    zFlgs[0] = 0;
  }
  بدل( p_بیان->op ){
    حالة TK_AGG_ستنه: {
      sqlite3_ونه_View_کرښه(pView, "AGG{%d:%d}%s",
            p_بیان->i_جدول, p_بیان->i_ستنه, zFlgs);
      اكسر;
    }
    حالة TK_ستنه: {
      إذا( p_بیان->i_جدول<0 ){
        /* This only happens when coding check constraints */
        حرف zOp2[16];
        إذا( p_بیان->op2 ){
          sqlite3_snprintf(حجم(zOp2),zOp2," op2=0x%02x",p_بیان->op2);
        }وإلا{
          zOp2[0] = 0;
        }
        sqlite3_ونه_View_کرښه(pView, "COLUMN(%d)%s%s",
                                    p_بیان->i_ستنه, zFlgs, zOp2);
      }وإلا{
        أكد( بیان_Use_YTab(p_بیان) );
        sqlite3_ونه_View_کرښه(pView, "{%d:%d} pTab=%p%s",
                        p_بیان->i_جدول, p_بیان->i_ستنه,
                        p_بیان->y.pTab, zFlgs);
      }
      إذا( بیان_Has_Property(p_بیان, EP_Fixed_ستون) ){
        sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 0);
      }
      اكسر;
    }
    حالة TK_INTEGER: {
      إذا( p_بیان->flags & EP_Int_ارزښت ){
        sqlite3_ونه_View_کرښه(pView, "%d", p_بیان->u.i_ارزښت);
      }وإلا{
        sqlite3_ونه_View_کرښه(pView, "%s", p_بیان->u.zToken);
      }
      اكسر;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_FLOATING_ټکی
    حالة TK_FLOAT: {
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_ونه_View_کرښه(pView,"%s", p_بیان->u.zToken);
      اكسر;
    }
#د_که_پای
    حالة TK_STRING: {
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_ونه_View_کرښه(pView,"%Q", p_بیان->u.zToken);
      اكسر;
    }
    حالة TK_NULL: {
      sqlite3_ونه_View_کرښه(pView,"NULL");
      اكسر;
    }
    حالة TK_TRUEFALSE: {
      sqlite3_ونه_View_کرښه(pView,"%s%s",
         sqlite3_بیان_Truth_ارزښت(p_بیان) ? "TRUE" : "FALSE", zFlgs);
      اكسر;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_BLOB_LITERAL
    حالة TK_BLOB: {
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_ونه_View_کرښه(pView,"%s", p_بیان->u.zToken);
      اكسر;
    }
#د_که_پای
    حالة TK_VARIABLE: {
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_ونه_View_کرښه(pView,"VARIABLE(%s,%d)",
                          p_بیان->u.zToken, p_بیان->i_ستنه);
      اكسر;
    }
    حالة TK_ثبتوونکی: {
      sqlite3_ونه_View_کرښه(pView,"REGISTER(%d)", p_بیان->i_جدول);
      اكسر;
    }
    حالة TK_ID: {
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_ونه_View_کرښه(pView,"ID \"%w\"", p_بیان->u.zToken);
      اكسر;
    }
    حالة TK_ASTERISK: {
      sqlite3_ونه_View_کرښه(pView,"ASKTERISK");
      اكسر;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_CAST
    حالة TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_ونه_View_کرښه(pView,"CAST %Q", p_بیان->u.zToken);
      sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 0);
      اكسر;
    }
#د_که_پای /* SQLITE_OMIT_CAST */
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
    حالة TK_ستوری:    zBinOp = "MUL";    اكسر;
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
      أكد( p_بیان->op2==TK_IS || p_بیان->op2==TK_ISNOT );
      أكد( p_بیان->p_ښی );
      أكد( sqlite3_بیان_Skip_Collate_And_Likely(p_بیان->p_ښی)->op
                  == TK_TRUEFALSE );
      x = (p_بیان->op2==TK_ISNOT)*2 + sqlite3_بیان_Truth_ارزښت(p_بیان->p_ښی);
      zUniOp = azOp[x];
      اكسر;
    }

    حالة TK_پراختیا: {
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_ونه_View_کرښه(pView, "SPAN %Q", p_بیان->u.zToken);
      sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 0);
      اكسر;
    }

    حالة TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_ونه_View_کرښه(pView, "%sCOLLATE %Q%s",
        !بیان_Has_Property(p_بیان, EP_Collate) ? "SOFT-" : "",
        p_بیان->u.zToken, zFlgs);
      sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 0);
      اكسر;
    }

    حالة TK_AGG_FUNCTION:
    حالة TK_FUNCTION: {
      بیان_List *pFarg;       /* List of function arguments */
      کړکۍ *pWin;
      إذا( بیان_Has_Property(p_بیان, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }وإلا{
        أكد( بیان_Use_XList(p_بیان) );
        pFarg = p_بیان->x.pList;
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
        pWin = Is_کړکۍ_Func(p_بیان) ? p_بیان->y.pWin : 0;
#وإلا
        pWin = 0;
#د_که_پای 
      }
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      إذا( p_بیان->op==TK_AGG_FUNCTION ){
        sqlite3_ونه_View_کرښه(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_بیان->op2, p_بیان->u.zToken, zFlgs,
                             p_بیان->pAggInfo ? p_بیان->pAggInfo->selId : 0,
                             p_بیان->iAgg, p_بیان->pAggInfo);
      }وإلا إذا( p_بیان->op2!=0 ){
        ثابت حرف *zOp2;
        حرف zBuf[8];
        sqlite3_snprintf(حجم(zBuf),zBuf,"0x%02x",p_بیان->op2);
        zOp2 = zBuf;
        إذا( p_بیان->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        إذا( p_بیان->op2==NC_Idx_بیان ) zOp2 = "NC_IdxExpr";
        إذا( p_بیان->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        إذا( p_بیان->op2==NC_Gen_ستون ) zOp2 = "NC_GenCol";
        sqlite3_ونه_View_کرښه(pView, "FUNCTION %Q%s op2=%s",
                            p_بیان->u.zToken, zFlgs, zOp2);
      }وإلا{
        sqlite3_ونه_View_کرښه(pView, "FUNCTION %Q%s", p_بیان->u.zToken, zFlgs);
      }
      إذا( pFarg ){
        sqlite3_ونه_View_بیان_List(pView, pFarg, pWin!=0 || p_بیان->p_کیڼ, 0);
        إذا( p_بیان->p_کیڼ ){
          بیان *pOB = p_بیان->p_کیڼ;
          أكد( pOB->op==TK_ترتیب );
          أكد( بیان_Use_XList(pOB) );
          sqlite3_ونه_View_بیان_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
      إذا( pWin ){
        sqlite3_ونه_View_کړکۍ(pView, pWin, 0);
      }
#د_که_پای
      اكسر;
    }
    حالة TK_ترتیب: {
      sqlite3_ونه_View_بیان_List(pView, p_بیان->x.pList, 0, "ORDERBY");
      اكسر;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_SUBQUERY
    حالة TK_EXISTS: {
      أكد( بیان_Use_XSelect(p_بیان) );
      sqlite3_ونه_View_کرښه(pView, "EXISTS-expr flags=0x%x", p_بیان->flags);
      sqlite3_ونه_View_Select(pView, p_بیان->x.pSelect, 0);
      اكسر;
    }
    حالة TK_SELECT: {
      أكد( بیان_Use_XSelect(p_بیان) );
      sqlite3_ونه_View_کرښه(pView, "subquery-expr flags=0x%x", p_بیان->flags);
      sqlite3_ونه_View_Select(pView, p_بیان->x.pSelect, 0);
      اكسر;
    }
    حالة TK_دننه: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      حرف *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_بیان->flags);
      إذا( p_بیان->i_جدول ) sqlite3_str_appendf(pStr, " iTable=%d",p_بیان->i_جدول);
      إذا( بیان_Has_Property(p_بیان, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_بیان->y.sub.regReturn, p_بیان->y.sub.iAddr);
      }
      z = sqlite3_str_پای(pStr);
      sqlite3_ونه_View_کرښه(pView, z);
      sqlite3_free(z);
      sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 1);
      إذا( بیان_Use_XSelect(p_بیان) ){
        sqlite3_ونه_View_Select(pView, p_بیان->x.pSelect, 0);
      }وإلا{
        sqlite3_ونه_View_بیان_List(pView, p_بیان->x.pList, 0, 0);
      }
      اكسر;
    }
#د_که_پای /* SQLITE_OMIT_SUBQUERY */

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
      ثابت بیان *pX, *pY, *pZ;
      pX = p_بیان->p_کیڼ;
      أكد( بیان_Use_XList(p_بیان) );
      أكد( p_بیان->x.pList->n_بیان==2 );
      pY = p_بیان->x.pList->a[0].p_بیان;
      pZ = p_بیان->x.pList->a[1].p_بیان;
      sqlite3_ونه_View_کرښه(pView, "BETWEEN%s", zFlgs);
      sqlite3_ونه_View_بیان(pView, pX, 1);
      sqlite3_ونه_View_بیان(pView, pY, 1);
      sqlite3_ونه_View_بیان(pView, pZ, 0);
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
      sqlite3_ونه_View_کرښه(pView, "%s(%d)", 
          p_بیان->i_جدول ? "NEW" : "OLD", p_بیان->i_ستنه);
      اكسر;
    }
    حالة TK_CASE: {
      sqlite3_ونه_View_کرښه(pView, "CASE");
      sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 1);
      أكد( بیان_Use_XList(p_بیان) );
      sqlite3_ونه_View_بیان_List(pView, p_بیان->x.pList, 0, 0);
      اكسر;
    }
#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
    حالة TK_RAISE: {
      ثابت حرف *zType = "unk";
      بدل( p_بیان->aff_بیان ){
        حالة OE_Rollback:   zType = "rollback";  اكسر;
        حالة OE_Abort:      zType = "abort";     اكسر;
        حالة OE_Fail:       zType = "fail";      اكسر;
        حالة OE_Ignore:     zType = "ignore";    اكسر;
      }
      أكد( !بیان_Has_Property(p_بیان, EP_Int_ارزښت) );
      sqlite3_ونه_View_کرښه(pView, "RAISE %s", zType);
      sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 0);
      اكسر;
    }
#د_که_پای
    حالة TK_سمون: {
      sqlite3_ونه_View_کرښه(pView, "MATCH {%d:%d}%s",
                          p_بیان->i_جدول, p_بیان->i_ستنه, zFlgs);
      sqlite3_ونه_View_بیان(pView, p_بیان->p_ښی, 0);
      اكسر;
    }
    حالة TK_ویکټور: {
      حرف *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      أكد( بیان_Use_XList(p_بیان) );
      sqlite3_ونه_View_Bare_بیان_List(pView, p_بیان->x.pList, z);
      sqlite3_free(z);
      اكسر;
    }
    حالة TK_SELECT_ستنه: {
      sqlite3_ونه_View_کرښه(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_بیان->i_ستنه, p_بیان->i_جدول-1,
              p_بیان->p_ښی==p_بیان->p_کیڼ ? " (SELECT-owner)" : "");
      أكد( بیان_Use_XSelect(p_بیان->p_کیڼ) );
      sqlite3_ونه_View_Select(pView, p_بیان->p_کیڼ->x.pSelect, 0);
      اكسر;
    }
    حالة TK_IF_NULL_لیکه: {
      sqlite3_ونه_View_کرښه(pView, "IF-NULL-ROW %d", p_بیان->i_جدول);
      sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 0);
      اكسر;
    }
    حالة TK_ERROR: {
      بیان tmp;
      sqlite3_ونه_View_کرښه(pView, "ERROR");
      tmp = *p_بیان;
      tmp.op = p_بیان->op2;
      sqlite3_ونه_View_بیان(pView, &tmp, 0);
      اكسر;
    }
    حالة TK_لیکه: {
      إذا( p_بیان->i_ستنه<=0 ){
        sqlite3_ونه_View_کرښه(pView, "First FROM table rowid");
      }وإلا{
        sqlite3_ونه_View_کرښه(pView, "First FROM table column %d",
            p_بیان->i_ستنه-1);
      }
      اكسر;
    }
    افتراضي: {
      sqlite3_ونه_View_کرښه(pView, "op=%d", p_بیان->op);
      اكسر;
    }
  }
  إذا( zBinOp ){
    sqlite3_ونه_View_کرښه(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 1);
    sqlite3_ونه_View_بیان(pView, p_بیان->p_ښی, 0);
  }وإلا إذا( zUniOp ){
    sqlite3_ونه_View_کرښه(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_ونه_View_بیان(pView, p_بیان->p_کیڼ, 0);
  }
  sqlite3_ونه_View_راوباسه(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
فراغ sqlite3_ونه_View_Bare_بیان_List(
  ونه_View *pView,
  ثابت بیان_List *pList,
  ثابت حرف *zLabel
){
  إذا( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  إذا( pList==0 ){
    sqlite3_ونه_View_کرښه(pView, "%s (empty)", zLabel);
  }وإلا{
    صحيح i;
    sqlite3_ونه_View_کرښه(pView, "%s", zLabel);
    لكل(i=0; i<pList->n_بیان; i++){
      صحيح j = pList->a[i].u.x.i_ترتیب_By_ستون;
      u8 ترتیبول_Flags = pList->a[i].fg.ترتیبول_Flags;
      حرف *zName = pList->a[i].zEName;
      صحيح نور_منزل_Follow = i<pList->n_بیان - 1;
      إذا( j || zName || ترتیبول_Flags ){
        sqlite3_ونه_View_ټېل(&pView, نور_منزل_Follow);
        نور_منزل_Follow = 0;
        sqlite3_ونه_View_کرښه(pView, 0);
        إذا( zName ){
          بدل( pList->a[i].fg.eEName ){
            افتراضي:
              fprintf(stdout, "AS %s ", zName);
              اكسر;
            حالة ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              إذا( pList->a[i].fg.b_کارول_شوی ) fprintf(stdout, "(used) ");
              إذا( pList->a[i].fg.b_Using_غړی ) fprintf(stdout, "(USING-term) ");
              إذا( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              اكسر;
            حالة ENAME_پراختیا:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              اكسر;
          }
        }
        إذا( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        إذا( ترتیبول_Flags & KEYINFO_ترتیب_DESC ){
          fprintf(stdout, "DESC ");
        }وإلا إذا( ترتیبول_Flags & KEYINFO_ترتیب_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_ونه_View_بیان(pView, pList->a[i].p_بیان, نور_منزل_Follow);
      إذا( j || zName || ترتیبول_Flags ){
        sqlite3_ونه_View_راوباسه(&pView);
      }
    }
  }
}
فراغ sqlite3_ونه_View_بیان_List(
  ونه_View *pView,
  ثابت بیان_List *pList,
  u8 نور_منزل_Follow,
  ثابت حرف *zLabel
){
  sqlite3_ونه_View_ټېل(&pView, نور_منزل_Follow);
  sqlite3_ونه_View_Bare_بیان_List(pView, pList, zLabel);
  sqlite3_ونه_View_راوباسه(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
فراغ sqlite3_ونه_View_Bare_Id_List(
  ونه_View *pView,
  ثابت IdList *pList,
  ثابت حرف *zLabel
){
  إذا( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  إذا( pList==0 ){
    sqlite3_ونه_View_کرښه(pView, "%s (empty)", zLabel);
  }وإلا{
    صحيح i;
    sqlite3_ونه_View_کرښه(pView, "%s", zLabel);
    لكل(i=0; i<pList->nId; i++){
      حرف *zName = pList->a[i].zName;
      صحيح نور_منزل_Follow = i<pList->nId - 1;
      إذا( zName==0 ) zName = "(null)";
      sqlite3_ونه_View_ټېل(&pView, نور_منزل_Follow);
      sqlite3_ونه_View_کرښه(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_ونه_View_راوباسه(&pView);
    }
  }
}
فراغ sqlite3_ونه_View_Id_List(
  ونه_View *pView,
  ثابت IdList *pList,
  u8 نور_منزل_Follow,
  ثابت حرف *zLabel
){
  sqlite3_ونه_View_ټېل(&pView, نور_منزل_Follow);
  sqlite3_ونه_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_ونه_View_راوباسه(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
فراغ sqlite3_ونه_View_Upsert(
  ونه_View *pView,
  ثابت Upsert *pUpsert,
  u8 نور_منزل_Follow
){
  إذا( pUpsert==0 ) أرجع;
  sqlite3_ونه_View_ټېل(&pView, نور_منزل_Follow);
  طالما( pUpsert ){
    صحيح n;
    sqlite3_ونه_View_ټېل(&pView, pUpsert->p_بل_Upsert!=0 || نور_منزل_Follow);
    sqlite3_ونه_View_کرښه(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->p_Upsert_سېټ!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_ونه_View_بیان_List(pView, pUpsert->p_Upsert_موخه, (n--)>0, "TARGET");
    sqlite3_ونه_View_بیان_List(pView, pUpsert->p_Upsert_سېټ, (n--)>0, "SET");
    إذا( pUpsert->pUpsertWhere ){
      sqlite3_ونه_View_توکی(pView, "WHERE", (n--)>0);
      sqlite3_ونه_View_بیان(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_ونه_View_راوباسه(&pView);
    }
    sqlite3_ونه_View_راوباسه(&pView);
    pUpsert = pUpsert->p_بل_Upsert;
  }
  sqlite3_ونه_View_راوباسه(&pView);
}

#إذا TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
فراغ sqlite3_ونه_View_Delete(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت بیان *pWhere,
  ثابت بیان_List *p_ترتیب_By,
  ثابت بیان *p_حد,
  ثابت Trigger *pTrigger
){
  صحيح n = 0;
  ونه_View *pView = 0;
  sqlite3_ونه_View_ټېل(&pView, 0);
  sqlite3_ونه_View_کرښه(pView, "DELETE");
  إذا( pWith ) n++;
  إذا( pTabList ) n++;
  إذا( pWhere ) n++;
  إذا( p_ترتیب_By ) n++;
  إذا( p_حد ) n++;
  إذا( pTrigger ) n++;
  إذا( pWith ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_With(pView, pWith, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pTabList ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "FROM");
    sqlite3_ونه_View_Src_List(pView, pTabList);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pWhere ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "WHERE");
    sqlite3_ونه_View_بیان(pView, pWhere, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( p_ترتیب_By ){
    sqlite3_ونه_View_بیان_List(pView, p_ترتیب_By, (--n)>0, "ORDER-BY");
  }
  إذا( p_حد ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "LIMIT");
    sqlite3_ونه_View_بیان(pView, p_حد, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pTrigger ){
    sqlite3_ونه_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ونه_View_راوباسه(&pView);
}
#د_که_پای /* TREETRACE_ENABLED */

#إذا TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
فراغ sqlite3_ونه_View_ورننباسه(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت IdList *p_ستنه_List,
  ثابت Select *pSelect,
  ثابت بیان_List *p_بیان_List,
  صحيح onError,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  ونه_View *pView = 0;
  صحيح n = 0;
  ثابت حرف *zLabel = "INSERT";
  بدل( onError ){
    حالة OE_Replace:  zLabel = "REPLACE";             اكسر;
    حالة OE_Ignore:   zLabel = "INSERT OR IGNORE";    اكسر;
    حالة OE_Rollback: zLabel = "INSERT OR ROLLBACK";  اكسر;
    حالة OE_Abort:    zLabel = "INSERT OR ABORT";     اكسر;
    حالة OE_Fail:     zLabel = "INSERT OR FAIL";      اكسر;
  }
  sqlite3_ونه_View_ټېل(&pView, 0);
  sqlite3_ونه_View_کرښه(pView, zLabel);
  إذا( pWith ) n++;
  إذا( pTabList ) n++;
  إذا( p_ستنه_List ) n++;
  إذا( pSelect ) n++;
  إذا( p_بیان_List ) n++;
  إذا( pUpsert ) n++;
  إذا( pTrigger ) n++;
  إذا( pWith ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_With(pView, pWith, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pTabList ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "INTO");
    sqlite3_ونه_View_Src_List(pView, pTabList);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( p_ستنه_List ){
    sqlite3_ونه_View_Id_List(pView, p_ستنه_List, (--n)>0, "COLUMNS");
  }
  إذا( pSelect ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "DATA-SOURCE");
    sqlite3_ونه_View_Select(pView, pSelect, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( p_بیان_List ){
    sqlite3_ونه_View_بیان_List(pView, p_بیان_List, (--n)>0, "VALUES");
  }
  إذا( pUpsert ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "UPSERT");
    sqlite3_ونه_View_Upsert(pView, pUpsert, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pTrigger ){
    sqlite3_ونه_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ونه_View_راوباسه(&pView);
}
#د_که_پای /* TREETRACE_ENABLED */

#إذا TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
فراغ sqlite3_ونه_View_Update(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت بیان_List *pChanges,
  ثابت بیان *pWhere,
  صحيح onError,
  ثابت بیان_List *p_ترتیب_By,
  ثابت بیان *p_حد,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  صحيح n = 0;
  ونه_View *pView = 0;
  ثابت حرف *zLabel = "UPDATE";
  بدل( onError ){
    حالة OE_Replace:  zLabel = "UPDATE OR REPLACE";   اكسر;
    حالة OE_Ignore:   zLabel = "UPDATE OR IGNORE";    اكسر;
    حالة OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  اكسر;
    حالة OE_Abort:    zLabel = "UPDATE OR ABORT";     اكسر;
    حالة OE_Fail:     zLabel = "UPDATE OR FAIL";      اكسر;
  }
  sqlite3_ونه_View_ټېل(&pView, 0);
  sqlite3_ونه_View_کرښه(pView, zLabel);
  إذا( pWith ) n++;
  إذا( pTabList ) n++;
  إذا( pChanges ) n++;
  إذا( pWhere ) n++;
  إذا( p_ترتیب_By ) n++;
  إذا( p_حد ) n++;
  إذا( pUpsert ) n++;
  إذا( pTrigger ) n++;
  إذا( pWith ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_With(pView, pWith, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pTabList ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "FROM");
    sqlite3_ونه_View_Src_List(pView, pTabList);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pChanges ){
    sqlite3_ونه_View_بیان_List(pView, pChanges, (--n)>0, "SET");
  }
  إذا( pWhere ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "WHERE");
    sqlite3_ونه_View_بیان(pView, pWhere, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( p_ترتیب_By ){
    sqlite3_ونه_View_بیان_List(pView, p_ترتیب_By, (--n)>0, "ORDER-BY");
  }
  إذا( p_حد ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "LIMIT");
    sqlite3_ونه_View_بیان(pView, p_حد, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pUpsert ){
    sqlite3_ونه_View_ټېل(&pView, (--n)>0);
    sqlite3_ونه_View_کرښه(pView, "UPSERT");
    sqlite3_ونه_View_Upsert(pView, pUpsert, 0);
    sqlite3_ونه_View_راوباسه(&pView);
  }
  إذا( pTrigger ){
    sqlite3_ونه_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ونه_View_راوباسه(&pView);
}
#د_که_پای /* TREETRACE_ENABLED */

#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
فراغ sqlite3_ونه_View_Trigger_ګام(
  ونه_View *pView,
  ثابت Trigger_ګام *p_ګام,
  u8 نور_منزل_Follow,
  u8 show_ډک_List
){
  صحيح cnt = 0;
  إذا( p_ګام==0 ) أرجع;
  sqlite3_ونه_View_ټېل(&pView, 
      نور_منزل_Follow || (show_ډک_List && p_ګام->p_بل!=0));
  افعل{
    إذا( cnt++ && p_ګام->p_بل==0 ){
      sqlite3_ونه_View_راوباسه(&pView);
      sqlite3_ونه_View_ټېل(&pView, 0);
    }
    sqlite3_ونه_View_کرښه(pView, "%s", p_ګام->z_پراختیا ? p_ګام->z_پراختیا : "RETURNING");
  }طالما( show_ډک_List && (p_ګام = p_ګام->p_بل)!=0 );
  sqlite3_ونه_View_راوباسه(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
فراغ sqlite3_ونه_View_Trigger(
  ونه_View *pView,
  ثابت Trigger *pTrigger,
  u8 نور_منزل_Follow,
  u8 show_ډک_List
){
  صحيح cnt = 0;
  إذا( pTrigger==0 ) أرجع;
  sqlite3_ونه_View_ټېل(&pView,
     نور_منزل_Follow || (show_ډک_List && pTrigger->p_بل!=0));
  افعل{
    إذا( cnt++ && pTrigger->p_بل==0 ){
      sqlite3_ونه_View_راوباسه(&pView);
      sqlite3_ونه_View_ټېل(&pView, 0);
    }
    sqlite3_ونه_View_کرښه(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_ونه_View_ټېل(&pView, 0);
    sqlite3_ونه_View_Trigger_ګام(pView, pTrigger->ګام_list, 0, 1);
    sqlite3_ونه_View_راوباسه(&pView);
  }طالما( show_ډک_List && (pTrigger = pTrigger->p_بل)!=0 );
  sqlite3_ونه_View_راوباسه(&pView);
}
#د_که_پای /* SQLITE_OMIT_TRIGGER */
  

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
فراغ sqlite3_Show_بیان(ثابت بیان *p){ sqlite3_ونه_View_بیان(0,p,0); }
فراغ sqlite3_Show_بیان_List(ثابت بیان_List *p){ sqlite3_ونه_View_بیان_List(0,p,0,0);}
فراغ sqlite3ShowIdList(ثابت IdList *p){ sqlite3_ونه_View_Id_List(0,p,0,0); }
فراغ sqlite3ShowSrcList(ثابت SrcList *p){
  ونه_View *pView = 0;
  sqlite3_ونه_View_ټېل(&pView, 0);
  sqlite3_ونه_View_کرښه(pView, "SRCLIST");
  sqlite3_ونه_View_Src_List(pView,p);
  sqlite3_ونه_View_راوباسه(&pView);
}
فراغ sqlite3ShowSelect(ثابت Select *p){ sqlite3_ونه_View_Select(0,p,0); }
فراغ sqlite3ShowWith(ثابت With *p){ sqlite3_ونه_View_With(0,p,0); }
فراغ sqlite3ShowUpsert(ثابت Upsert *p){ sqlite3_ونه_View_Upsert(0,p,0); }
#که_نه_تعریف_شوی SQLITE_OMIT_TRIGGER
فراغ sqlite3_Show_Trigger_ګام(ثابت Trigger_ګام *p){
  sqlite3_ونه_View_Trigger_ګام(0,p,0,0);
}
فراغ sqlite3_Show_Trigger_ګام_List(ثابت Trigger_ګام *p){
  sqlite3_ونه_View_Trigger_ګام(0,p,0,1);
}
فراغ sqlite3ShowTrigger(ثابت Trigger *p){ sqlite3_ونه_View_Trigger(0,p,0,0); }
فراغ sqlite3ShowTriggerList(ثابت Trigger *p){ sqlite3_ونه_View_Trigger(0,p,0,1);}
#د_که_پای
#که_نه_تعریف_شوی SQLITE_OMIT_WINDOWFUNC
فراغ sqlite3_Show_کړکۍ(ثابت کړکۍ *p){ sqlite3_ونه_View_کړکۍ(0,p,0); }
فراغ sqlite3ShowWinFunc(ثابت کړکۍ *p){ sqlite3_ونه_View_Win_Func(0,p,0); }
#د_که_پای

#د_که_پای /* SQLITE_DEBUG */
