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
#اگر_تعریف_شدہ SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
ساکن خالی sqlite3_درخت_View_ڈالو(درخت_View **pp, u8 مزید_مقصد_Follow){
  درخت_View *p = *pp;
  اگر( p==0 ){
    *pp = p = sqlite3_malloc64( حجم(*p) );
    اگر( p==0 ) واپس;
    memset(p, 0, حجم(*p));
  }ورنہ{
    p->iLevel++;
  }
  أكد( مزید_مقصد_Follow==0 || مزید_مقصد_Follow==1 );
  اگر( p->iLevel<(صحیح)حجم(p->b_سطر) ) p->b_سطر[p->iLevel] = مزید_مقصد_Follow;
}

/*
** Finished with one layer of the tree
*/
ساکن خالی sqlite3_درخت_View_نکالو(درخت_View **pp){
  درخت_View *p = *pp;
  اگر( p==0 ) واپس;
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
خالی sqlite3_درخت_View_سطر(درخت_View *p, ثابت حرف *zFormat, ...){
  va_list ap;
  صحیح i;
  StrAccum acc;
  حرف zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, حجم(zBuf), 0);
  اگر( p ){
    برائے(i=0; i<p->iLevel && i<(صحیح)حجم(p->b_سطر)-1; i++){
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
  sqlite3_Str_Accum_اختتام(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
ساکن خالی sqlite3_درخت_View_شے(درخت_View *p, ثابت حرف *zLabel,u8 مزید_Follows){
  sqlite3_درخت_View_ڈالو(&p, مزید_Follows);
  sqlite3_درخت_View_سطر(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
خالی sqlite3_درخت_View_عمود_List(
  درخت_View *pView,
  ثابت عمود *a_ستون,
  صحیح n_ستون,
  u8 مزید_مقصد_Follow
){
  صحیح i;
  sqlite3_درخت_View_ڈالو(&pView, مزید_مقصد_Follow);
  sqlite3_درخت_View_سطر(pView, "COLUMNS");
  برائے(i=0; i<n_ستون; i++){
    u16 flg = a_ستون[i].ستون_Flags;
    صحیح ستون_مزید_مقصد_Follow = i<(n_ستون - 1);
    sqlite3_درخت_View_ڈالو(&pView, ستون_مزید_مقصد_Follow);
    sqlite3_درخت_View_سطر(pView, 0);
    لکھو(" %s", a_ستون[i].zCnName);
    انتخاب( a_ستون[i].eCType ){
      حالت COLTYPE_ANY:      لکھو(" ANY");        توڑو;
      حالت COLTYPE_BLOB:     لکھو(" BLOB");       توڑو;
      حالت COLTYPE_INT:      لکھو(" INT");        توڑو;
      حالت COLTYPE_INTEGER:  لکھو(" INTEGER");    توڑو;
      حالت COLTYPE_REAL:     لکھو(" REAL");       توڑو;
      حالت COLTYPE_متن:     لکھو(" TEXT");       توڑو;
      حالت COLTYPE_CUSTOM: {
        اگر( flg & COLFLAG_HASTYPE ){
          ثابت حرف *z = a_ستون[i].zCnName;
          z += لڑی_لمبائی(z)+1;
          لکھو(" X-%s", z);
          توڑو;
        }
      }
    }
    اگر( flg & COLFLAG_PRIMKEY ) لکھو(" PRIMARY KEY");
    اگر( flg & COLFLAG_HIDDEN ) لکھو(" HIDDEN");
#اگر_تعریف_شدہ COLFLAG_NOEXPAND
    اگر( flg & COLFLAG_NOEXPAND ) لکھو(" NO-EXPAND");
#اگر_ختم
    اگر( flg ) لکھو(" flags=%04x", flg);
    لکھو("\n");      
    fflush(stdout);
    sqlite3_درخت_View_نکالو(&pView);
  }
  sqlite3_درخت_View_نکالو(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
خالی sqlite3_درخت_View_With(درخت_View *pView, ثابت With *pWith, u8 مزید_مقصد_Follow){
  صحیح i;
  اگر( pWith==0 ) واپس;
  اگر( pWith->nCte==0 ) واپس;
  اگر( pWith->pOuter ){
    sqlite3_درخت_View_سطر(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }ورنہ{
    sqlite3_درخت_View_سطر(pView, "WITH (0x%p)", pWith);
  }
  اگر( pWith->nCte>0 ){
    sqlite3_درخت_View_ڈالو(&pView, مزید_مقصد_Follow);
    برائے(i=0; i<pWith->nCte; i++){
      StrAccum x;
      حرف z_سطر[1000];
      ثابت ساخت Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_سطر, حجم(z_سطر), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      اگر( pCte->p_ستون_تعداد && pCte->p_ستون_تعداد->n_اظہاریہ>0 ){
        حرف cSep = '(';
        صحیح j;
        برائے(j=0; j<pCte->p_ستون_تعداد->n_اظہاریہ; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->p_ستون_تعداد->a[j].zEName);
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
      sqlite3_Str_Accum_اختتام(&x);
      sqlite3_درخت_View_شے(pView, z_سطر, i<pWith->nCte-1);
      sqlite3_درخت_View_Select(pView, pCte->pSelect, 0);
      sqlite3_درخت_View_نکالو(&pView);
    }
    sqlite3_درخت_View_نکالو(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
خالی sqlite3_درخت_View_Src_List(درخت_View *pView, ثابت SrcList *pSrc){
  صحیح i;
  اگر( pSrc==0 ) واپس;
  برائے(i=0; i<pSrc->nSrc; i++){
    ثابت Src_شے *p_شے = &pSrc->a[i];
    StrAccum x;
    صحیح n = 0;
    حرف z_سطر[1000];
    sqlite3StrAccumInit(&x, 0, z_سطر, حجم(z_سطر), 0);
    x.printfFlags |= SQLITE_PRINTF_اندرونی;
    sqlite3_str_appendf(&x, "{%d:*} %!S", p_شے->iCursor, p_شے);
    اگر( p_شے->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           p_شے->pSTab->zName, p_شے->pSTab->n_ستون, p_شے->pSTab, 
           p_شے->ستون_مستعمل,
           p_شے->fg.rowid_مستعمل ? "+rowid" : "");
    }
    اگر( (p_شے->fg.jointype & (JT_بایاں|JT_دایاں))==(JT_بایاں|JT_دایاں) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }ورنہ اگر( p_شے->fg.jointype & JT_بایاں ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }ورنہ اگر( p_شے->fg.jointype & JT_دایاں ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }ورنہ اگر( p_شے->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    اگر( p_شے->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    اگر( p_شے->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    اگر( p_شے->fg.isCte ){
      ساکن ثابت حرف *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          p_شے->u2.pCteUse->nUse,
                          aMat[p_شے->u2.pCteUse->eM10d]);
    }
    اگر( p_شے->fg.isOn || (p_شے->fg.isUsing==0 && p_شے->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    اگر( p_شے->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    اگر( p_شے->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    اگر( p_شے->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    اگر( p_شے->fg.بذریعہ_Coroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    اگر( p_شے->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    اگر( p_شے->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    اگر( p_شے->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    اگر( p_شے->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    اگر( p_شے->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3_Str_Accum_اختتام(&x);
    sqlite3_درخت_View_شے(pView, z_سطر, i<pSrc->nSrc-1);
    n = 0;
    اگر( p_شے->fg.isSubquery ) n++;
    اگر( p_شے->fg.isTabFunc ) n++;
    اگر( p_شے->fg.isUsing || p_شے->u3.pOn!=0 ) n++;
    اگر( p_شے->fg.isUsing ){
      sqlite3_درخت_View_Id_List(pView, p_شے->u3.pUsing, (--n)>0, "USING");
    }ورنہ اگر( p_شے->u3.pOn!=0 ){
      sqlite3_درخت_View_شے(pView, "ON", (--n)>0);
      sqlite3_درخت_View_اظہاریہ(pView, p_شے->u3.pOn, 0);
      sqlite3_درخت_View_نکالو(&pView);
    }
    اگر( p_شے->fg.isSubquery ){
      أكد( n==1 );
      اگر( p_شے->pSTab ){
        جدول *pTab = p_شے->pSTab;
        sqlite3_درخت_View_عمود_List(pView, pTab->a_ستون, pTab->n_ستون, 1);
      }
      أكد( (صحیح)p_شے->fg.isNestedFrom == IsNestedFrom(p_شے) );
      sqlite3_درخت_View_Select(pView, p_شے->u4.pSubq->pSelect, 0);
    }
    اگر( p_شے->fg.isTabFunc ){
      sqlite3_درخت_View_اظہاریہ_List(pView, p_شے->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_درخت_View_نکالو(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
خالی sqlite3_درخت_View_Select(درخت_View *pView, ثابت Select *p, u8 مزید_مقصد_Follow){
  صحیح n = 0;
  صحیح cnt = 0;
  اگر( p==0 ){
    sqlite3_درخت_View_سطر(pView, "nil-SELECT");
    واپس;
  } 
  sqlite3_درخت_View_ڈالو(&pView, مزید_مقصد_Follow);
  اگر( p->pWith ){
    sqlite3_درخت_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_درخت_View_ڈالو(&pView, 1);
  }
  کرو{
    اگر( p->selFlags & SF_WhereBegin ){
      sqlite3_درخت_View_سطر(pView, "sqlite3WhereBegin()");
    }ورنہ{
      sqlite3_درخت_View_سطر(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (صحیح)p->n_Select_صف
      );
    }
    اگر( cnt++ ) sqlite3_درخت_View_نکالو(&pView);
    اگر( p->p_پیشگی ){
      n = 1000;
    }ورنہ{
      n = 0;
      اگر( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      اگر( p->pWhere ) n++;
      اگر( p->p_گروہ_By ) n++;
      اگر( p->pHaving ) n++;
      اگر( p->p_ترتیب_By ) n++;
      اگر( p->p_حد_بندی ) n++;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      اگر( p->pWin ) n++;
      اگر( p->pWinDefn ) n++;
#اگر_ختم
    }
    اگر( p->pEList ){
      sqlite3_درخت_View_اظہاریہ_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( p->pWin ){
      کھڑکی *pX;
      sqlite3_درخت_View_ڈالو(&pView, (n--)>0);
      sqlite3_درخت_View_سطر(pView, "window-functions");
      برائے(pX=p->pWin; pX; pX=pX->p_اگلا_Win){
        sqlite3_درخت_View_Win_Func(pView, pX, pX->p_اگلا_Win!=0);
      }
      sqlite3_درخت_View_نکالو(&pView);
    }
#اگر_ختم
    اگر( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_درخت_View_ڈالو(&pView, (n--)>0);
      sqlite3_درخت_View_سطر(pView, "FROM");
      sqlite3_درخت_View_Src_List(pView, p->pSrc);
      sqlite3_درخت_View_نکالو(&pView);
    }
    اگر( p->pWhere ){
      sqlite3_درخت_View_شے(pView, "WHERE", (n--)>0);
      sqlite3_درخت_View_اظہاریہ(pView, p->pWhere, 0);
      sqlite3_درخت_View_نکالو(&pView);
    }
    اگر( p->p_گروہ_By ){
      sqlite3_درخت_View_اظہاریہ_List(pView, p->p_گروہ_By, (n--)>0, "GROUPBY");
    }
    اگر( p->pHaving ){
      sqlite3_درخت_View_شے(pView, "HAVING", (n--)>0);
      sqlite3_درخت_View_اظہاریہ(pView, p->pHaving, 0);
      sqlite3_درخت_View_نکالو(&pView);
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
    اگر( p->pWinDefn ){
      کھڑکی *pX;
      sqlite3_درخت_View_شے(pView, "WINDOW", (n--)>0);
      برائے(pX=p->pWinDefn; pX; pX=pX->p_اگلا_Win){
        sqlite3_درخت_View_کھڑکی(pView, pX, pX->p_اگلا_Win!=0);
      }
      sqlite3_درخت_View_نکالو(&pView);
    }
#اگر_ختم
    اگر( p->p_ترتیب_By ){
      sqlite3_درخت_View_اظہاریہ_List(pView, p->p_ترتیب_By, (n--)>0, "ORDERBY");
    }
    اگر( p->p_حد_بندی ){
      sqlite3_درخت_View_شے(pView, "LIMIT", (n--)>0);
      sqlite3_درخت_View_اظہاریہ(pView, p->p_حد_بندی->p_بایاں, p->p_حد_بندی->p_دایاں!=0);
      اگر( p->p_حد_بندی->p_دایاں ){
        sqlite3_درخت_View_شے(pView, "OFFSET", 0);
        sqlite3_درخت_View_اظہاریہ(pView, p->p_حد_بندی->p_دایاں, 0);
        sqlite3_درخت_View_نکالو(&pView);
      }
      sqlite3_درخت_View_نکالو(&pView);
    }
    اگر( p->p_پیشگی ){
      ثابت حرف *zOp = "UNION";
      انتخاب( p->op ){
        حالت TK_ALL:         zOp = "UNION ALL";  توڑو;
        حالت TK_INTERSECT:   zOp = "INTERSECT";  توڑو;
        حالت TK_EXCEPT:      zOp = "EXCEPT";     توڑو;
      }
      sqlite3_درخت_View_شے(pView, zOp, 1);
    }
    p = p->p_پیشگی;
  }جب( p!=0 );
  sqlite3_درخت_View_نکالو(&pView);
}

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
خالی sqlite3_درخت_View_Bound(
  درخت_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  اظہاریہ *p_اظہاریہ,            /* Value for PRECEDING or FOLLOWING */
  u8 مزید_مقصد_Follow         /* True if more to follow */
){
  انتخاب( eBound ){
    حالت TK_UNBOUNDED: {
      sqlite3_درخت_View_شے(pView, "UNBOUNDED", مزید_مقصد_Follow);
      sqlite3_درخت_View_نکالو(&pView);
      توڑو;
    }
    حالت TK_CURRENT: {
      sqlite3_درخت_View_شے(pView, "CURRENT", مزید_مقصد_Follow);
      sqlite3_درخت_View_نکالو(&pView);
      توڑو;
    }
    حالت TK_PRECEDING: {
      sqlite3_درخت_View_شے(pView, "PRECEDING", مزید_مقصد_Follow);
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ, 0);
      sqlite3_درخت_View_نکالو(&pView);
      توڑو;
    }
    حالت TK_FOLLOWING: {
      sqlite3_درخت_View_شے(pView, "FOLLOWING", مزید_مقصد_Follow);
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ, 0);
      sqlite3_درخت_View_نکالو(&pView);
      توڑو;
    }
  }
}
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
خالی sqlite3_درخت_View_کھڑکی(درخت_View *pView, ثابت کھڑکی *pWin, u8 مزید){
  صحیح nElement = 0;
  اگر( pWin==0 ) واپس;
  اگر( pWin->pFilter ){
    sqlite3_درخت_View_شے(pView, "FILTER", 1);
    sqlite3_درخت_View_اظہاریہ(pView, pWin->pFilter, 0);
    sqlite3_درخت_View_نکالو(&pView);
    اگر( pWin->eFrmType==TK_FILTER ) واپس;
  }
  sqlite3_درخت_View_ڈالو(&pView, مزید);
  اگر( pWin->zName ){
    sqlite3_درخت_View_سطر(pView, "OVER %s (%p)", pWin->zName, pWin);
  }ورنہ{
    sqlite3_درخت_View_سطر(pView, "OVER (%p)", pWin);
  }
  اگر( pWin->z_بنیاد )    nElement++;
  اگر( pWin->p_ترتیب_By ) nElement++;
  اگر( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  اگر( pWin->eExclude ) nElement++;
  اگر( pWin->z_بنیاد ){
    sqlite3_درخت_View_ڈالو(&pView, (--nElement)>0);
    sqlite3_درخت_View_سطر(pView, "window: %s", pWin->z_بنیاد);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pWin->p_تقسیم ){
    sqlite3_درخت_View_اظہاریہ_List(pView, pWin->p_تقسیم, nElement>0,"PARTITION-BY");
  }
  اگر( pWin->p_ترتیب_By ){
    sqlite3_درخت_View_اظہاریہ_List(pView, pWin->p_ترتیب_By, (--nElement)>0, "ORDER-BY");
  }
  اگر( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    حرف zBuf[30];
    ثابت حرف *zFrmType = "ROWS";
    اگر( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    اگر( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(حجم(zBuf),zBuf,"%s%s",zFrmType,
        pWin->b_Implicit_چوکھٹا ? " (implied)" : "");
    sqlite3_درخت_View_شے(pView, zBuf, (--nElement)>0);
    sqlite3_درخت_View_Bound(pView, pWin->e_آغاز, pWin->p_آغاز, 1);
    sqlite3_درخت_View_Bound(pView, pWin->e_انتہا, pWin->p_انتہا, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pWin->eExclude ){
    حرف zBuf[30];
    ثابت حرف *zExclude;
    انتخاب( pWin->eExclude ){
      حالت TK_NO:      zExclude = "NO OTHERS";   توڑو;
      حالت TK_CURRENT: zExclude = "CURRENT ROW"; توڑو;
      حالت TK_گروہ:   zExclude = "GROUP";       توڑو;
      حالت TK_TIES:    zExclude = "TIES";        توڑو;
      طےشدہ:
        sqlite3_snprintf(حجم(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        توڑو;
    }
    sqlite3_درخت_View_ڈالو(&pView, 0);
    sqlite3_درخت_View_سطر(pView, "EXCLUDE %s", zExclude);
    sqlite3_درخت_View_نکالو(&pView);
  }
  sqlite3_درخت_View_نکالو(&pView);
}
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
خالی sqlite3_درخت_View_Win_Func(درخت_View *pView, ثابت کھڑکی *pWin, u8 مزید){
  اگر( pWin==0 ) واپس;
  sqlite3_درخت_View_ڈالو(&pView, مزید);
  sqlite3_درخت_View_سطر(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_درخت_View_کھڑکی(pView, pWin, 0);
  sqlite3_درخت_View_نکالو(&pView);
}
#اگر_ختم /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
خالی sqlite3_درخت_View_اظہاریہ(درخت_View *pView, ثابت اظہاریہ *p_اظہاریہ, u8 مزید_مقصد_Follow){
  ثابت حرف *zBinOp = 0;   /* Binary operator */
  ثابت حرف *zUniOp = 0;   /* Unary operator */
  حرف zFlgs[200];
  sqlite3_درخت_View_ڈالو(&pView, مزید_مقصد_Follow);
  اگر( p_اظہاریہ==0 ){
    sqlite3_درخت_View_سطر(pView, "nil");
    sqlite3_درخت_View_نکالو(&pView);
    واپس;
  }
  اگر( p_اظہاریہ->flags || p_اظہاریہ->aff_اظہاریہ || p_اظہاریہ->vvaFlags || p_اظہاریہ->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, حجم(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_اظہاریہ->flags, p_اظہاریہ->aff_اظہاریہ ? p_اظہاریہ->aff_اظہاریہ : 'n');
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_اظہاریہ->w.iJoin);
    }
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_اظہاریہ->w.iJoin);
    }
    اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    اگر( اظہاریہ_Has_VVAProperty(p_اظہاریہ, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    اگر( p_اظہاریہ->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_اظہاریہ->iAgg);
    }
    sqlite3_Str_Accum_اختتام(&x);
  }ورنہ{
    zFlgs[0] = 0;
  }
  انتخاب( p_اظہاریہ->op ){
    حالت TK_AGG_عمود: {
      sqlite3_درخت_View_سطر(pView, "AGG{%d:%d}%s",
            p_اظہاریہ->i_جدول, p_اظہاریہ->i_عمود, zFlgs);
      توڑو;
    }
    حالت TK_عمود: {
      اگر( p_اظہاریہ->i_جدول<0 ){
        /* This only happens when coding check constraints */
        حرف zOp2[16];
        اگر( p_اظہاریہ->op2 ){
          sqlite3_snprintf(حجم(zOp2),zOp2," op2=0x%02x",p_اظہاریہ->op2);
        }ورنہ{
          zOp2[0] = 0;
        }
        sqlite3_درخت_View_سطر(pView, "COLUMN(%d)%s%s",
                                    p_اظہاریہ->i_عمود, zFlgs, zOp2);
      }ورنہ{
        أكد( اظہاریہ_Use_YTab(p_اظہاریہ) );
        sqlite3_درخت_View_سطر(pView, "{%d:%d} pTab=%p%s",
                        p_اظہاریہ->i_جدول, p_اظہاریہ->i_عمود,
                        p_اظہاریہ->y.pTab, zFlgs);
      }
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Fixed_ستون) ){
        sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 0);
      }
      توڑو;
    }
    حالت TK_INTEGER: {
      اگر( p_اظہاریہ->flags & EP_Int_قدر ){
        sqlite3_درخت_View_سطر(pView, "%d", p_اظہاریہ->u.i_قدر);
      }ورنہ{
        sqlite3_درخت_View_سطر(pView, "%s", p_اظہاریہ->u.zToken);
      }
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_FLOATING_نقطہ
    حالت TK_FLOAT: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_درخت_View_سطر(pView,"%s", p_اظہاریہ->u.zToken);
      توڑو;
    }
#اگر_ختم
    حالت TK_STRING: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_درخت_View_سطر(pView,"%Q", p_اظہاریہ->u.zToken);
      توڑو;
    }
    حالت TK_NULL: {
      sqlite3_درخت_View_سطر(pView,"NULL");
      توڑو;
    }
    حالت TK_TRUEFALSE: {
      sqlite3_درخت_View_سطر(pView,"%s%s",
         sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ) ? "TRUE" : "FALSE", zFlgs);
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_BLOB_LITERAL
    حالت TK_BLOB: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_درخت_View_سطر(pView,"%s", p_اظہاریہ->u.zToken);
      توڑو;
    }
#اگر_ختم
    حالت TK_VARIABLE: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_درخت_View_سطر(pView,"VARIABLE(%s,%d)",
                          p_اظہاریہ->u.zToken, p_اظہاریہ->i_عمود);
      توڑو;
    }
    حالت TK_رجسٹر: {
      sqlite3_درخت_View_سطر(pView,"REGISTER(%d)", p_اظہاریہ->i_جدول);
      توڑو;
    }
    حالت TK_ID: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_درخت_View_سطر(pView,"ID \"%w\"", p_اظہاریہ->u.zToken);
      توڑو;
    }
    حالت TK_ASTERISK: {
      sqlite3_درخت_View_سطر(pView,"ASKTERISK");
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_CAST
    حالت TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_درخت_View_سطر(pView,"CAST %Q", p_اظہاریہ->u.zToken);
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 0);
      توڑو;
    }
#اگر_ختم /* SQLITE_OMIT_CAST */
    حالت TK_LT:      zBinOp = "LT";     توڑو;
    حالت TK_LE:      zBinOp = "LE";     توڑو;
    حالت TK_GT:      zBinOp = "GT";     توڑو;
    حالت TK_GE:      zBinOp = "GE";     توڑو;
    حالت TK_NE:      zBinOp = "NE";     توڑو;
    حالت TK_EQ:      zBinOp = "EQ";     توڑو;
    حالت TK_IS:      zBinOp = "IS";     توڑو;
    حالت TK_ISNOT:   zBinOp = "ISNOT";  توڑو;
    حالت TK_AND:     zBinOp = "AND";    توڑو;
    حالت TK_OR:      zBinOp = "OR";     توڑو;
    حالت TK_PLUS:    zBinOp = "ADD";    توڑو;
    حالت TK_ستارہ:    zBinOp = "MUL";    توڑو;
    حالت TK_MINUS:   zBinOp = "SUB";    توڑو;
    حالت TK_REM:     zBinOp = "REM";    توڑو;
    حالت TK_BITAND:  zBinOp = "BITAND"; توڑو;
    حالت TK_BITOR:   zBinOp = "BITOR";  توڑو;
    حالت TK_SLASH:   zBinOp = "DIV";    توڑو;
    حالت TK_LSHIFT:  zBinOp = "LSHIFT"; توڑو;
    حالت TK_RSHIFT:  zBinOp = "RSHIFT"; توڑو;
    حالت TK_CONCAT:  zBinOp = "CONCAT"; توڑو;
    حالت TK_DOT:     zBinOp = "DOT";    توڑو;
    حالت TK_حد_بندی:   zBinOp = "LIMIT";  توڑو;

    حالت TK_UMINUS:  zUniOp = "UMINUS"; توڑو;
    حالت TK_UPLUS:   zUniOp = "UPLUS";  توڑو;
    حالت TK_BITNOT:  zUniOp = "BITNOT"; توڑو;
    حالت TK_NOT:     zUniOp = "NOT";    توڑو;
    حالت TK_ISNULL:  zUniOp = "ISNULL"; توڑو;
    حالت TK_NOTNULL: zUniOp = "NOTNULL"; توڑو;

    حالت TK_TRUTH: {
      صحیح x;
      ثابت حرف *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      أكد( p_اظہاریہ->op2==TK_IS || p_اظہاریہ->op2==TK_ISNOT );
      أكد( p_اظہاریہ->p_دایاں );
      أكد( sqlite3_اظہاریہ_Skip_Collate_And_Likely(p_اظہاریہ->p_دایاں)->op
                  == TK_TRUEFALSE );
      x = (p_اظہاریہ->op2==TK_ISNOT)*2 + sqlite3_اظہاریہ_Truth_قدر(p_اظہاریہ->p_دایاں);
      zUniOp = azOp[x];
      توڑو;
    }

    حالت TK_وسعت: {
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_درخت_View_سطر(pView, "SPAN %Q", p_اظہاریہ->u.zToken);
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 0);
      توڑو;
    }

    حالت TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_درخت_View_سطر(pView, "%sCOLLATE %Q%s",
        !اظہاریہ_Has_Property(p_اظہاریہ, EP_Collate) ? "SOFT-" : "",
        p_اظہاریہ->u.zToken, zFlgs);
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 0);
      توڑو;
    }

    حالت TK_AGG_FUNCTION:
    حالت TK_FUNCTION: {
      اظہاریہ_List *pFarg;       /* List of function arguments */
      کھڑکی *pWin;
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }ورنہ{
        أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
        pFarg = p_اظہاریہ->x.pList;
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
        pWin = Is_کھڑکی_Func(p_اظہاریہ) ? p_اظہاریہ->y.pWin : 0;
#ورنہ
        pWin = 0;
#اگر_ختم 
      }
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      اگر( p_اظہاریہ->op==TK_AGG_FUNCTION ){
        sqlite3_درخت_View_سطر(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_اظہاریہ->op2, p_اظہاریہ->u.zToken, zFlgs,
                             p_اظہاریہ->pAggInfo ? p_اظہاریہ->pAggInfo->selId : 0,
                             p_اظہاریہ->iAgg, p_اظہاریہ->pAggInfo);
      }ورنہ اگر( p_اظہاریہ->op2!=0 ){
        ثابت حرف *zOp2;
        حرف zBuf[8];
        sqlite3_snprintf(حجم(zBuf),zBuf,"0x%02x",p_اظہاریہ->op2);
        zOp2 = zBuf;
        اگر( p_اظہاریہ->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        اگر( p_اظہاریہ->op2==NC_Idx_اظہاریہ ) zOp2 = "NC_IdxExpr";
        اگر( p_اظہاریہ->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        اگر( p_اظہاریہ->op2==NC_Gen_ستون ) zOp2 = "NC_GenCol";
        sqlite3_درخت_View_سطر(pView, "FUNCTION %Q%s op2=%s",
                            p_اظہاریہ->u.zToken, zFlgs, zOp2);
      }ورنہ{
        sqlite3_درخت_View_سطر(pView, "FUNCTION %Q%s", p_اظہاریہ->u.zToken, zFlgs);
      }
      اگر( pFarg ){
        sqlite3_درخت_View_اظہاریہ_List(pView, pFarg, pWin!=0 || p_اظہاریہ->p_بایاں, 0);
        اگر( p_اظہاریہ->p_بایاں ){
          اظہاریہ *pOB = p_اظہاریہ->p_بایاں;
          أكد( pOB->op==TK_ترتیب );
          أكد( اظہاریہ_Use_XList(pOB) );
          sqlite3_درخت_View_اظہاریہ_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
      اگر( pWin ){
        sqlite3_درخت_View_کھڑکی(pView, pWin, 0);
      }
#اگر_ختم
      توڑو;
    }
    حالت TK_ترتیب: {
      sqlite3_درخت_View_اظہاریہ_List(pView, p_اظہاریہ->x.pList, 0, "ORDERBY");
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_SUBQUERY
    حالت TK_EXISTS: {
      أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
      sqlite3_درخت_View_سطر(pView, "EXISTS-expr flags=0x%x", p_اظہاریہ->flags);
      sqlite3_درخت_View_Select(pView, p_اظہاریہ->x.pSelect, 0);
      توڑو;
    }
    حالت TK_SELECT: {
      أكد( اظہاریہ_Use_XSelect(p_اظہاریہ) );
      sqlite3_درخت_View_سطر(pView, "subquery-expr flags=0x%x", p_اظہاریہ->flags);
      sqlite3_درخت_View_Select(pView, p_اظہاریہ->x.pSelect, 0);
      توڑو;
    }
    حالت TK_اندر: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      حرف *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_اظہاریہ->flags);
      اگر( p_اظہاریہ->i_جدول ) sqlite3_str_appendf(pStr, " iTable=%d",p_اظہاریہ->i_جدول);
      اگر( اظہاریہ_Has_Property(p_اظہاریہ, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_اظہاریہ->y.sub.regReturn, p_اظہاریہ->y.sub.iAddr);
      }
      z = sqlite3_str_اختتام(pStr);
      sqlite3_درخت_View_سطر(pView, z);
      sqlite3_free(z);
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 1);
      اگر( اظہاریہ_Use_XSelect(p_اظہاریہ) ){
        sqlite3_درخت_View_Select(pView, p_اظہاریہ->x.pSelect, 0);
      }ورنہ{
        sqlite3_درخت_View_اظہاریہ_List(pView, p_اظہاریہ->x.pList, 0, 0);
      }
      توڑو;
    }
#اگر_ختم /* SQLITE_OMIT_SUBQUERY */

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
      ثابت اظہاریہ *pX, *pY, *pZ;
      pX = p_اظہاریہ->p_بایاں;
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      أكد( p_اظہاریہ->x.pList->n_اظہاریہ==2 );
      pY = p_اظہاریہ->x.pList->a[0].p_اظہاریہ;
      pZ = p_اظہاریہ->x.pList->a[1].p_اظہاریہ;
      sqlite3_درخت_View_سطر(pView, "BETWEEN%s", zFlgs);
      sqlite3_درخت_View_اظہاریہ(pView, pX, 1);
      sqlite3_درخت_View_اظہاریہ(pView, pY, 1);
      sqlite3_درخت_View_اظہاریہ(pView, pZ, 0);
      توڑو;
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
          p_اظہاریہ->i_جدول ? "NEW" : "OLD", p_اظہاریہ->i_عمود);
      توڑو;
    }
    حالت TK_CASE: {
      sqlite3_درخت_View_سطر(pView, "CASE");
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 1);
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      sqlite3_درخت_View_اظہاریہ_List(pView, p_اظہاریہ->x.pList, 0, 0);
      توڑو;
    }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
    حالت TK_RAISE: {
      ثابت حرف *zType = "unk";
      انتخاب( p_اظہاریہ->aff_اظہاریہ ){
        حالت OE_Rollback:   zType = "rollback";  توڑو;
        حالت OE_Abort:      zType = "abort";     توڑو;
        حالت OE_Fail:       zType = "fail";      توڑو;
        حالت OE_Ignore:     zType = "ignore";    توڑو;
      }
      أكد( !اظہاریہ_Has_Property(p_اظہاریہ, EP_Int_قدر) );
      sqlite3_درخت_View_سطر(pView, "RAISE %s", zType);
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 0);
      توڑو;
    }
#اگر_ختم
    حالت TK_مطابقت: {
      sqlite3_درخت_View_سطر(pView, "MATCH {%d:%d}%s",
                          p_اظہاریہ->i_جدول, p_اظہاریہ->i_عمود, zFlgs);
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_دایاں, 0);
      توڑو;
    }
    حالت TK_سمتیہ: {
      حرف *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      أكد( اظہاریہ_Use_XList(p_اظہاریہ) );
      sqlite3_درخت_View_Bare_اظہاریہ_List(pView, p_اظہاریہ->x.pList, z);
      sqlite3_free(z);
      توڑو;
    }
    حالت TK_SELECT_عمود: {
      sqlite3_درخت_View_سطر(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_اظہاریہ->i_عمود, p_اظہاریہ->i_جدول-1,
              p_اظہاریہ->p_دایاں==p_اظہاریہ->p_بایاں ? " (SELECT-owner)" : "");
      أكد( اظہاریہ_Use_XSelect(p_اظہاریہ->p_بایاں) );
      sqlite3_درخت_View_Select(pView, p_اظہاریہ->p_بایاں->x.pSelect, 0);
      توڑو;
    }
    حالت TK_IF_NULL_صف: {
      sqlite3_درخت_View_سطر(pView, "IF-NULL-ROW %d", p_اظہاریہ->i_جدول);
      sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 0);
      توڑو;
    }
    حالت TK_ERROR: {
      اظہاریہ tmp;
      sqlite3_درخت_View_سطر(pView, "ERROR");
      tmp = *p_اظہاریہ;
      tmp.op = p_اظہاریہ->op2;
      sqlite3_درخت_View_اظہاریہ(pView, &tmp, 0);
      توڑو;
    }
    حالت TK_صف: {
      اگر( p_اظہاریہ->i_عمود<=0 ){
        sqlite3_درخت_View_سطر(pView, "First FROM table rowid");
      }ورنہ{
        sqlite3_درخت_View_سطر(pView, "First FROM table column %d",
            p_اظہاریہ->i_عمود-1);
      }
      توڑو;
    }
    طےشدہ: {
      sqlite3_درخت_View_سطر(pView, "op=%d", p_اظہاریہ->op);
      توڑو;
    }
  }
  اگر( zBinOp ){
    sqlite3_درخت_View_سطر(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 1);
    sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_دایاں, 0);
  }ورنہ اگر( zUniOp ){
    sqlite3_درخت_View_سطر(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_درخت_View_اظہاریہ(pView, p_اظہاریہ->p_بایاں, 0);
  }
  sqlite3_درخت_View_نکالو(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
خالی sqlite3_درخت_View_Bare_اظہاریہ_List(
  درخت_View *pView,
  ثابت اظہاریہ_List *pList,
  ثابت حرف *zLabel
){
  اگر( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  اگر( pList==0 ){
    sqlite3_درخت_View_سطر(pView, "%s (empty)", zLabel);
  }ورنہ{
    صحیح i;
    sqlite3_درخت_View_سطر(pView, "%s", zLabel);
    برائے(i=0; i<pList->n_اظہاریہ; i++){
      صحیح j = pList->a[i].u.x.i_ترتیب_By_ستون;
      u8 ترتیب_دو_Flags = pList->a[i].fg.ترتیب_دو_Flags;
      حرف *zName = pList->a[i].zEName;
      صحیح مزید_مقصد_Follow = i<pList->n_اظہاریہ - 1;
      اگر( j || zName || ترتیب_دو_Flags ){
        sqlite3_درخت_View_ڈالو(&pView, مزید_مقصد_Follow);
        مزید_مقصد_Follow = 0;
        sqlite3_درخت_View_سطر(pView, 0);
        اگر( zName ){
          انتخاب( pList->a[i].fg.eEName ){
            طےشدہ:
              fprintf(stdout, "AS %s ", zName);
              توڑو;
            حالت ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              اگر( pList->a[i].fg.b_مستعمل ) fprintf(stdout, "(used) ");
              اگر( pList->a[i].fg.b_Using_رکن ) fprintf(stdout, "(USING-term) ");
              اگر( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              توڑو;
            حالت ENAME_وسعت:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              توڑو;
          }
        }
        اگر( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        اگر( ترتیب_دو_Flags & KEYINFO_ترتیب_DESC ){
          fprintf(stdout, "DESC ");
        }ورنہ اگر( ترتیب_دو_Flags & KEYINFO_ترتیب_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_درخت_View_اظہاریہ(pView, pList->a[i].p_اظہاریہ, مزید_مقصد_Follow);
      اگر( j || zName || ترتیب_دو_Flags ){
        sqlite3_درخت_View_نکالو(&pView);
      }
    }
  }
}
خالی sqlite3_درخت_View_اظہاریہ_List(
  درخت_View *pView,
  ثابت اظہاریہ_List *pList,
  u8 مزید_مقصد_Follow,
  ثابت حرف *zLabel
){
  sqlite3_درخت_View_ڈالو(&pView, مزید_مقصد_Follow);
  sqlite3_درخت_View_Bare_اظہاریہ_List(pView, pList, zLabel);
  sqlite3_درخت_View_نکالو(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
خالی sqlite3_درخت_View_Bare_Id_List(
  درخت_View *pView,
  ثابت IdList *pList,
  ثابت حرف *zLabel
){
  اگر( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  اگر( pList==0 ){
    sqlite3_درخت_View_سطر(pView, "%s (empty)", zLabel);
  }ورنہ{
    صحیح i;
    sqlite3_درخت_View_سطر(pView, "%s", zLabel);
    برائے(i=0; i<pList->nId; i++){
      حرف *zName = pList->a[i].zName;
      صحیح مزید_مقصد_Follow = i<pList->nId - 1;
      اگر( zName==0 ) zName = "(null)";
      sqlite3_درخت_View_ڈالو(&pView, مزید_مقصد_Follow);
      sqlite3_درخت_View_سطر(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_درخت_View_نکالو(&pView);
    }
  }
}
خالی sqlite3_درخت_View_Id_List(
  درخت_View *pView,
  ثابت IdList *pList,
  u8 مزید_مقصد_Follow,
  ثابت حرف *zLabel
){
  sqlite3_درخت_View_ڈالو(&pView, مزید_مقصد_Follow);
  sqlite3_درخت_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_درخت_View_نکالو(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
خالی sqlite3_درخت_View_Upsert(
  درخت_View *pView,
  ثابت Upsert *pUpsert,
  u8 مزید_مقصد_Follow
){
  اگر( pUpsert==0 ) واپس;
  sqlite3_درخت_View_ڈالو(&pView, مزید_مقصد_Follow);
  جب( pUpsert ){
    صحیح n;
    sqlite3_درخت_View_ڈالو(&pView, pUpsert->p_اگلا_Upsert!=0 || مزید_مقصد_Follow);
    sqlite3_درخت_View_سطر(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->p_Upsert_سیٹ!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_درخت_View_اظہاریہ_List(pView, pUpsert->p_Upsert_ہدف, (n--)>0, "TARGET");
    sqlite3_درخت_View_اظہاریہ_List(pView, pUpsert->p_Upsert_سیٹ, (n--)>0, "SET");
    اگر( pUpsert->pUpsertWhere ){
      sqlite3_درخت_View_شے(pView, "WHERE", (n--)>0);
      sqlite3_درخت_View_اظہاریہ(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_درخت_View_نکالو(&pView);
    }
    sqlite3_درخت_View_نکالو(&pView);
    pUpsert = pUpsert->p_اگلا_Upsert;
  }
  sqlite3_درخت_View_نکالو(&pView);
}

#اگر TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
خالی sqlite3_درخت_View_Delete(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت اظہاریہ *pWhere,
  ثابت اظہاریہ_List *p_ترتیب_By,
  ثابت اظہاریہ *p_حد_بندی,
  ثابت Trigger *pTrigger
){
  صحیح n = 0;
  درخت_View *pView = 0;
  sqlite3_درخت_View_ڈالو(&pView, 0);
  sqlite3_درخت_View_سطر(pView, "DELETE");
  اگر( pWith ) n++;
  اگر( pTabList ) n++;
  اگر( pWhere ) n++;
  اگر( p_ترتیب_By ) n++;
  اگر( p_حد_بندی ) n++;
  اگر( pTrigger ) n++;
  اگر( pWith ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_With(pView, pWith, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pTabList ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "FROM");
    sqlite3_درخت_View_Src_List(pView, pTabList);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pWhere ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "WHERE");
    sqlite3_درخت_View_اظہاریہ(pView, pWhere, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( p_ترتیب_By ){
    sqlite3_درخت_View_اظہاریہ_List(pView, p_ترتیب_By, (--n)>0, "ORDER-BY");
  }
  اگر( p_حد_بندی ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "LIMIT");
    sqlite3_درخت_View_اظہاریہ(pView, p_حد_بندی, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pTrigger ){
    sqlite3_درخت_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_درخت_View_نکالو(&pView);
}
#اگر_ختم /* TREETRACE_ENABLED */

#اگر TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
خالی sqlite3_درخت_View_داخل_کرو(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت IdList *p_عمود_List,
  ثابت Select *pSelect,
  ثابت اظہاریہ_List *p_اظہاریہ_List,
  صحیح onError,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  درخت_View *pView = 0;
  صحیح n = 0;
  ثابت حرف *zLabel = "INSERT";
  انتخاب( onError ){
    حالت OE_Replace:  zLabel = "REPLACE";             توڑو;
    حالت OE_Ignore:   zLabel = "INSERT OR IGNORE";    توڑو;
    حالت OE_Rollback: zLabel = "INSERT OR ROLLBACK";  توڑو;
    حالت OE_Abort:    zLabel = "INSERT OR ABORT";     توڑو;
    حالت OE_Fail:     zLabel = "INSERT OR FAIL";      توڑو;
  }
  sqlite3_درخت_View_ڈالو(&pView, 0);
  sqlite3_درخت_View_سطر(pView, zLabel);
  اگر( pWith ) n++;
  اگر( pTabList ) n++;
  اگر( p_عمود_List ) n++;
  اگر( pSelect ) n++;
  اگر( p_اظہاریہ_List ) n++;
  اگر( pUpsert ) n++;
  اگر( pTrigger ) n++;
  اگر( pWith ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_With(pView, pWith, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pTabList ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "INTO");
    sqlite3_درخت_View_Src_List(pView, pTabList);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( p_عمود_List ){
    sqlite3_درخت_View_Id_List(pView, p_عمود_List, (--n)>0, "COLUMNS");
  }
  اگر( pSelect ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "DATA-SOURCE");
    sqlite3_درخت_View_Select(pView, pSelect, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( p_اظہاریہ_List ){
    sqlite3_درخت_View_اظہاریہ_List(pView, p_اظہاریہ_List, (--n)>0, "VALUES");
  }
  اگر( pUpsert ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "UPSERT");
    sqlite3_درخت_View_Upsert(pView, pUpsert, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pTrigger ){
    sqlite3_درخت_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_درخت_View_نکالو(&pView);
}
#اگر_ختم /* TREETRACE_ENABLED */

#اگر TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
خالی sqlite3_درخت_View_Update(
  ثابت With *pWith,
  ثابت SrcList *pTabList,
  ثابت اظہاریہ_List *pChanges,
  ثابت اظہاریہ *pWhere,
  صحیح onError,
  ثابت اظہاریہ_List *p_ترتیب_By,
  ثابت اظہاریہ *p_حد_بندی,
  ثابت Upsert *pUpsert,
  ثابت Trigger *pTrigger
){
  صحیح n = 0;
  درخت_View *pView = 0;
  ثابت حرف *zLabel = "UPDATE";
  انتخاب( onError ){
    حالت OE_Replace:  zLabel = "UPDATE OR REPLACE";   توڑو;
    حالت OE_Ignore:   zLabel = "UPDATE OR IGNORE";    توڑو;
    حالت OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  توڑو;
    حالت OE_Abort:    zLabel = "UPDATE OR ABORT";     توڑو;
    حالت OE_Fail:     zLabel = "UPDATE OR FAIL";      توڑو;
  }
  sqlite3_درخت_View_ڈالو(&pView, 0);
  sqlite3_درخت_View_سطر(pView, zLabel);
  اگر( pWith ) n++;
  اگر( pTabList ) n++;
  اگر( pChanges ) n++;
  اگر( pWhere ) n++;
  اگر( p_ترتیب_By ) n++;
  اگر( p_حد_بندی ) n++;
  اگر( pUpsert ) n++;
  اگر( pTrigger ) n++;
  اگر( pWith ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_With(pView, pWith, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pTabList ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "FROM");
    sqlite3_درخت_View_Src_List(pView, pTabList);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pChanges ){
    sqlite3_درخت_View_اظہاریہ_List(pView, pChanges, (--n)>0, "SET");
  }
  اگر( pWhere ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "WHERE");
    sqlite3_درخت_View_اظہاریہ(pView, pWhere, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( p_ترتیب_By ){
    sqlite3_درخت_View_اظہاریہ_List(pView, p_ترتیب_By, (--n)>0, "ORDER-BY");
  }
  اگر( p_حد_بندی ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "LIMIT");
    sqlite3_درخت_View_اظہاریہ(pView, p_حد_بندی, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pUpsert ){
    sqlite3_درخت_View_ڈالو(&pView, (--n)>0);
    sqlite3_درخت_View_سطر(pView, "UPSERT");
    sqlite3_درخت_View_Upsert(pView, pUpsert, 0);
    sqlite3_درخت_View_نکالو(&pView);
  }
  اگر( pTrigger ){
    sqlite3_درخت_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_درخت_View_نکالو(&pView);
}
#اگر_ختم /* TREETRACE_ENABLED */

#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
خالی sqlite3_درخت_View_Trigger_قدم(
  درخت_View *pView,
  ثابت Trigger_قدم *p_قدم,
  u8 مزید_مقصد_Follow,
  u8 show_بھرا_List
){
  صحیح cnt = 0;
  اگر( p_قدم==0 ) واپس;
  sqlite3_درخت_View_ڈالو(&pView, 
      مزید_مقصد_Follow || (show_بھرا_List && p_قدم->p_اگلا!=0));
  کرو{
    اگر( cnt++ && p_قدم->p_اگلا==0 ){
      sqlite3_درخت_View_نکالو(&pView);
      sqlite3_درخت_View_ڈالو(&pView, 0);
    }
    sqlite3_درخت_View_سطر(pView, "%s", p_قدم->z_وسعت ? p_قدم->z_وسعت : "RETURNING");
  }جب( show_بھرا_List && (p_قدم = p_قدم->p_اگلا)!=0 );
  sqlite3_درخت_View_نکالو(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
خالی sqlite3_درخت_View_Trigger(
  درخت_View *pView,
  ثابت Trigger *pTrigger,
  u8 مزید_مقصد_Follow,
  u8 show_بھرا_List
){
  صحیح cnt = 0;
  اگر( pTrigger==0 ) واپس;
  sqlite3_درخت_View_ڈالو(&pView,
     مزید_مقصد_Follow || (show_بھرا_List && pTrigger->p_اگلا!=0));
  کرو{
    اگر( cnt++ && pTrigger->p_اگلا==0 ){
      sqlite3_درخت_View_نکالو(&pView);
      sqlite3_درخت_View_ڈالو(&pView, 0);
    }
    sqlite3_درخت_View_سطر(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_درخت_View_ڈالو(&pView, 0);
    sqlite3_درخت_View_Trigger_قدم(pView, pTrigger->قدم_list, 0, 1);
    sqlite3_درخت_View_نکالو(&pView);
  }جب( show_بھرا_List && (pTrigger = pTrigger->p_اگلا)!=0 );
  sqlite3_درخت_View_نکالو(&pView);
}
#اگر_ختم /* SQLITE_OMIT_TRIGGER */
  

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
خالی sqlite3_Show_اظہاریہ(ثابت اظہاریہ *p){ sqlite3_درخت_View_اظہاریہ(0,p,0); }
خالی sqlite3_Show_اظہاریہ_List(ثابت اظہاریہ_List *p){ sqlite3_درخت_View_اظہاریہ_List(0,p,0,0);}
خالی sqlite3ShowIdList(ثابت IdList *p){ sqlite3_درخت_View_Id_List(0,p,0,0); }
خالی sqlite3ShowSrcList(ثابت SrcList *p){
  درخت_View *pView = 0;
  sqlite3_درخت_View_ڈالو(&pView, 0);
  sqlite3_درخت_View_سطر(pView, "SRCLIST");
  sqlite3_درخت_View_Src_List(pView,p);
  sqlite3_درخت_View_نکالو(&pView);
}
خالی sqlite3ShowSelect(ثابت Select *p){ sqlite3_درخت_View_Select(0,p,0); }
خالی sqlite3ShowWith(ثابت With *p){ sqlite3_درخت_View_With(0,p,0); }
خالی sqlite3ShowUpsert(ثابت Upsert *p){ sqlite3_درخت_View_Upsert(0,p,0); }
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_TRIGGER
خالی sqlite3_Show_Trigger_قدم(ثابت Trigger_قدم *p){
  sqlite3_درخت_View_Trigger_قدم(0,p,0,0);
}
خالی sqlite3_Show_Trigger_قدم_List(ثابت Trigger_قدم *p){
  sqlite3_درخت_View_Trigger_قدم(0,p,0,1);
}
خالی sqlite3ShowTrigger(ثابت Trigger *p){ sqlite3_درخت_View_Trigger(0,p,0,0); }
خالی sqlite3ShowTriggerList(ثابت Trigger *p){ sqlite3_درخت_View_Trigger(0,p,0,1);}
#اگر_ختم
#اگر_غیر_تعریف_شدہ SQLITE_OMIT_WINDOWFUNC
خالی sqlite3_Show_کھڑکی(ثابت کھڑکی *p){ sqlite3_درخت_View_کھڑکی(0,p,0); }
خالی sqlite3ShowWinFunc(ثابت کھڑکی *p){ sqlite3_درخت_View_Win_Func(0,p,0); }
#اگر_ختم

#اگر_ختم /* SQLITE_DEBUG */
