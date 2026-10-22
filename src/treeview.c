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
#ਅਨ੍ਤਰ੍ਭਾਵ "sqliteInt.h"
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
ਸਥਿਰ ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(ਰੁੱਖ_View **pp, u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow){
  ਰੁੱਖ_View *p = *pp;
  ਜੇ( p==0 ){
    *pp = p = sqlite3_malloc64( ਆਕਾਰ(*p) );
    ਜੇ( p==0 ) ਵਾਪਸ;
    memset(p, 0, ਆਕਾਰ(*p));
  }ਨਹੀਂਤਾਂ{
    p->iLevel++;
  }
  ਪ੍ਰਤਿਜ੍ਞਾ( ਹੋਰ_ਮੰਜ਼ਿਲ_Follow==0 || ਹੋਰ_ਮੰਜ਼ਿਲ_Follow==1 );
  ਜੇ( p->iLevel<(ਪੂਰਨ)ਆਕਾਰ(p->b_ਸਤਰ) ) p->b_ਸਤਰ[p->iLevel] = ਹੋਰ_ਮੰਜ਼ਿਲ_Follow;
}

/*
** Finished with one layer of the tree
*/
ਸਥਿਰ ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(ਰੁੱਖ_View **pp){
  ਰੁੱਖ_View *p = *pp;
  ਜੇ( p==0 ) ਵਾਪਸ;
  p->iLevel--;
  ਜੇ( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਸਤਰ(ਰੁੱਖ_View *p, ਅਚਲ ਅੱਖਰ *zFormat, ...){
  va_list ap;
  ਪੂਰਨ i;
  StrAccum acc;
  ਅੱਖਰ zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, ਆਕਾਰ(zBuf), 0);
  ਜੇ( p ){
    ਲਈ(i=0; i<p->iLevel && i<(ਪੂਰਨ)ਆਕਾਰ(p->b_ਸਤਰ)-1; i++){
      sqlite3_str_append(&acc, p->b_ਸਤਰ[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_ਸਤਰ[i] ? "|-- " : "'-- ", 4);
  }
  ਜੇ( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    ਪ੍ਰਤਿਜ੍ਞਾ( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3_Str_Accum_ਅੰਤ(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
ਸਥਿਰ ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(ਰੁੱਖ_View *p, ਅਚਲ ਅੱਖਰ *zLabel,u8 ਹੋਰ_Follows){
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&p, ਹੋਰ_Follows);
  sqlite3_ਰੁੱਖ_View_ਸਤਰ(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਕਾਲਮ_List(
  ਰੁੱਖ_View *pView,
  ਅਚਲ ਕਾਲਮ *a_ਖਾਨਾ,
  ਪੂਰਨ n_ਖਾਨਾ,
  u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow
){
  ਪੂਰਨ i;
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
  sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "COLUMNS");
  ਲਈ(i=0; i<n_ਖਾਨਾ; i++){
    u16 flg = a_ਖਾਨਾ[i].ਖਾਨਾ_Flags;
    ਪੂਰਨ ਖਾਨਾ_ਹੋਰ_ਮੰਜ਼ਿਲ_Follow = i<(n_ਖਾਨਾ - 1);
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਖਾਨਾ_ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, 0);
    ਲਿਖੋ(" %s", a_ਖਾਨਾ[i].zCnName);
    ਚੋਣ( a_ਖਾਨਾ[i].eCType ){
      ਹਾਲਤ COLTYPE_ANY:      ਲਿਖੋ(" ANY");        ਰੋਕੋ;
      ਹਾਲਤ COLTYPE_BLOB:     ਲਿਖੋ(" BLOB");       ਰੋਕੋ;
      ਹਾਲਤ COLTYPE_INT:      ਲਿਖੋ(" INT");        ਰੋਕੋ;
      ਹਾਲਤ COLTYPE_INTEGER:  ਲਿਖੋ(" INTEGER");    ਰੋਕੋ;
      ਹਾਲਤ COLTYPE_REAL:     ਲਿਖੋ(" REAL");       ਰੋਕੋ;
      ਹਾਲਤ COLTYPE_ਪਾਠ:     ਲਿਖੋ(" TEXT");       ਰੋਕੋ;
      ਹਾਲਤ COLTYPE_CUSTOM: {
        ਜੇ( flg & COLFLAG_HASTYPE ){
          ਅਚਲ ਅੱਖਰ *z = a_ਖਾਨਾ[i].zCnName;
          z += ਲੜੀ_ਲੰਬਾਈ(z)+1;
          ਲਿਖੋ(" X-%s", z);
          ਰੋਕੋ;
        }
      }
    }
    ਜੇ( flg & COLFLAG_PRIMKEY ) ਲਿਖੋ(" PRIMARY KEY");
    ਜੇ( flg & COLFLAG_HIDDEN ) ਲਿਖੋ(" HIDDEN");
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ COLFLAG_NOEXPAND
    ਜੇ( flg & COLFLAG_NOEXPAND ) ਲਿਖੋ(" NO-EXPAND");
#ਜੇ_ਅੰਤ
    ਜੇ( flg ) ਲਿਖੋ(" flags=%04x", flg);
    ਲਿਖੋ("\n");      
    fflush(stdout);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_With(ਰੁੱਖ_View *pView, ਅਚਲ With *pWith, u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow){
  ਪੂਰਨ i;
  ਜੇ( pWith==0 ) ਵਾਪਸ;
  ਜੇ( pWith->nCte==0 ) ਵਾਪਸ;
  ਜੇ( pWith->pOuter ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "WITH (0x%p)", pWith);
  }
  ਜੇ( pWith->nCte>0 ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
    ਲਈ(i=0; i<pWith->nCte; i++){
      StrAccum x;
      ਅੱਖਰ z_ਸਤਰ[1000];
      ਅਚਲ ਬਣਤਰ Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_ਸਤਰ, ਆਕਾਰ(z_ਸਤਰ), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      ਜੇ( pCte->p_ਖਾਨੇ_ਗਿਣਤੀ && pCte->p_ਖਾਨੇ_ਗਿਣਤੀ->n_ਵਿਅੰਜਕ>0 ){
        ਅੱਖਰ cSep = '(';
        ਪੂਰਨ j;
        ਲਈ(j=0; j<pCte->p_ਖਾਨੇ_ਗਿਣਤੀ->n_ਵਿਅੰਜਕ; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->p_ਖਾਨੇ_ਗਿਣਤੀ->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      ਜੇ( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      ਜੇ( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3_Str_Accum_ਅੰਤ(&x);
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, z_ਸਤਰ, i<pWith->nCte-1);
      sqlite3_ਰੁੱਖ_View_Select(pView, pCte->pSelect, 0);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Src_List(ਰੁੱਖ_View *pView, ਅਚਲ SrcList *pSrc){
  ਪੂਰਨ i;
  ਜੇ( pSrc==0 ) ਵਾਪਸ;
  ਲਈ(i=0; i<pSrc->nSrc; i++){
    ਅਚਲ Src_ਵਸਤੂ *p_ਵਸਤੂ = &pSrc->a[i];
    StrAccum x;
    ਪੂਰਨ n = 0;
    ਅੱਖਰ z_ਸਤਰ[1000];
    sqlite3StrAccumInit(&x, 0, z_ਸਤਰ, ਆਕਾਰ(z_ਸਤਰ), 0);
    x.printfFlags |= SQLITE_PRINTF_ਅੰਦਰੂਨੀ;
    sqlite3_str_appendf(&x, "{%d:*} %!S", p_ਵਸਤੂ->iCursor, p_ਵਸਤੂ);
    ਜੇ( p_ਵਸਤੂ->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           p_ਵਸਤੂ->pSTab->zName, p_ਵਸਤੂ->pSTab->n_ਖਾਨਾ, p_ਵਸਤੂ->pSTab, 
           p_ਵਸਤੂ->ਖਾਨਾ_ਵਰਤਿਆ,
           p_ਵਸਤੂ->fg.rowid_ਵਰਤਿਆ ? "+rowid" : "");
    }
    ਜੇ( (p_ਵਸਤੂ->fg.jointype & (JT_ਖੱਬਾ|JT_ਸੱਜਾ))==(JT_ਖੱਬਾ|JT_ਸੱਜਾ) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਸਤੂ->fg.jointype & JT_ਖੱਬਾ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਸਤੂ->fg.jointype & JT_ਸੱਜਾ ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਸਤੂ->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    ਜੇ( p_ਵਸਤੂ->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    ਜੇ( p_ਵਸਤੂ->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ਜੇ( p_ਵਸਤੂ->fg.isCte ){
      ਸਥਿਰ ਅਚਲ ਅੱਖਰ *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          p_ਵਸਤੂ->u2.pCteUse->nUse,
                          aMat[p_ਵਸਤੂ->u2.pCteUse->eM10d]);
    }
    ਜੇ( p_ਵਸਤੂ->fg.isOn || (p_ਵਸਤੂ->fg.isUsing==0 && p_ਵਸਤੂ->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    ਜੇ( p_ਵਸਤੂ->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    ਜੇ( p_ਵਸਤੂ->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    ਜੇ( p_ਵਸਤੂ->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    ਜੇ( p_ਵਸਤੂ->fg.ਰਾਹੀਂ_Coroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    ਜੇ( p_ਵਸਤੂ->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    ਜੇ( p_ਵਸਤੂ->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    ਜੇ( p_ਵਸਤੂ->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    ਜੇ( p_ਵਸਤੂ->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    ਜੇ( p_ਵਸਤੂ->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3_Str_Accum_ਅੰਤ(&x);
    sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, z_ਸਤਰ, i<pSrc->nSrc-1);
    n = 0;
    ਜੇ( p_ਵਸਤੂ->fg.isSubquery ) n++;
    ਜੇ( p_ਵਸਤੂ->fg.isTabFunc ) n++;
    ਜੇ( p_ਵਸਤੂ->fg.isUsing || p_ਵਸਤੂ->u3.pOn!=0 ) n++;
    ਜੇ( p_ਵਸਤੂ->fg.isUsing ){
      sqlite3_ਰੁੱਖ_View_Id_List(pView, p_ਵਸਤੂ->u3.pUsing, (--n)>0, "USING");
    }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਸਤੂ->u3.pOn!=0 ){
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "ON", (--n)>0);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਸਤੂ->u3.pOn, 0);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
    ਜੇ( p_ਵਸਤੂ->fg.isSubquery ){
      ਪ੍ਰਤਿਜ੍ਞਾ( n==1 );
      ਜੇ( p_ਵਸਤੂ->pSTab ){
        ਸਾਰਣੀ *pTab = p_ਵਸਤੂ->pSTab;
        sqlite3_ਰੁੱਖ_View_ਕਾਲਮ_List(pView, pTab->a_ਖਾਨਾ, pTab->n_ਖਾਨਾ, 1);
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( (ਪੂਰਨ)p_ਵਸਤੂ->fg.isNestedFrom == IsNestedFrom(p_ਵਸਤੂ) );
      sqlite3_ਰੁੱਖ_View_Select(pView, p_ਵਸਤੂ->u4.pSubq->pSelect, 0);
    }
    ਜੇ( p_ਵਸਤੂ->fg.isTabFunc ){
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p_ਵਸਤੂ->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Select(ਰੁੱਖ_View *pView, ਅਚਲ Select *p, u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow){
  ਪੂਰਨ n = 0;
  ਪੂਰਨ cnt = 0;
  ਜੇ( p==0 ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "nil-SELECT");
    ਵਾਪਸ;
  } 
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
  ਜੇ( p->pWith ){
    sqlite3_ਰੁੱਖ_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 1);
  }
  ਕਰੋ{
    ਜੇ( p->selFlags & SF_WhereBegin ){
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "sqlite3WhereBegin()");
    }ਨਹੀਂਤਾਂ{
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (ਪੂਰਨ)p->n_Select_ਪੰਕਤੀ
      );
    }
    ਜੇ( cnt++ ) sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    ਜੇ( p->p_ਪੂਰਵ ){
      n = 1000;
    }ਨਹੀਂਤਾਂ{
      n = 0;
      ਜੇ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      ਜੇ( p->pWhere ) n++;
      ਜੇ( p->p_ਸਮੂਹ_By ) n++;
      ਜੇ( p->pHaving ) n++;
      ਜੇ( p->p_ਕ੍ਰਮ_By ) n++;
      ਜੇ( p->p_ਹੱਦਬੰਦੀ ) n++;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਜੇ( p->pWin ) n++;
      ਜੇ( p->pWinDefn ) n++;
#ਜੇ_ਅੰਤ
    }
    ਜੇ( p->pEList ){
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( p->pWin ){
      ਖਿੜਕੀ *pX;
      sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (n--)>0);
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "window-functions");
      ਲਈ(pX=p->pWin; pX; pX=pX->p_ਅਗਲਾ_Win){
        sqlite3_ਰੁੱਖ_View_Win_Func(pView, pX, pX->p_ਅਗਲਾ_Win!=0);
      }
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (n--)>0);
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "FROM");
      sqlite3_ਰੁੱਖ_View_Src_List(pView, p->pSrc);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
    ਜੇ( p->pWhere ){
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "WHERE", (n--)>0);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p->pWhere, 0);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
    ਜੇ( p->p_ਸਮੂਹ_By ){
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p->p_ਸਮੂਹ_By, (n--)>0, "GROUPBY");
    }
    ਜੇ( p->pHaving ){
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "HAVING", (n--)>0);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p->pHaving, 0);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
    ਜੇ( p->pWinDefn ){
      ਖਿੜਕੀ *pX;
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "WINDOW", (n--)>0);
      ਲਈ(pX=p->pWinDefn; pX; pX=pX->p_ਅਗਲਾ_Win){
        sqlite3_ਰੁੱਖ_View_ਖਿੜਕੀ(pView, pX, pX->p_ਅਗਲਾ_Win!=0);
      }
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
#ਜੇ_ਅੰਤ
    ਜੇ( p->p_ਕ੍ਰਮ_By ){
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p->p_ਕ੍ਰਮ_By, (n--)>0, "ORDERBY");
    }
    ਜੇ( p->p_ਹੱਦਬੰਦੀ ){
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "LIMIT", (n--)>0);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p->p_ਹੱਦਬੰਦੀ->p_ਖੱਬਾ, p->p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ!=0);
      ਜੇ( p->p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ ){
        sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "OFFSET", 0);
        sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p->p_ਹੱਦਬੰਦੀ->p_ਸੱਜਾ, 0);
        sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
      }
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
    ਜੇ( p->p_ਪੂਰਵ ){
      ਅਚਲ ਅੱਖਰ *zOp = "UNION";
      ਚੋਣ( p->op ){
        ਹਾਲਤ TK_ALL:         zOp = "UNION ALL";  ਰੋਕੋ;
        ਹਾਲਤ TK_INTERSECT:   zOp = "INTERSECT";  ਰੋਕੋ;
        ਹਾਲਤ TK_EXCEPT:      zOp = "EXCEPT";     ਰੋਕੋ;
      }
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, zOp, 1);
    }
    p = p->p_ਪੂਰਵ;
  }ਜਦੋਂ( p!=0 );
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Bound(
  ਰੁੱਖ_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ,            /* Value for PRECEDING or FOLLOWING */
  u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow         /* True if more to follow */
){
  ਚੋਣ( eBound ){
    ਹਾਲਤ TK_UNBOUNDED: {
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "UNBOUNDED", ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_CURRENT: {
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "CURRENT", ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_PRECEDING: {
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "PRECEDING", ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ, 0);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_FOLLOWING: {
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "FOLLOWING", ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ, 0);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
      ਰੋਕੋ;
    }
  }
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਖਿੜਕੀ(ਰੁੱਖ_View *pView, ਅਚਲ ਖਿੜਕੀ *pWin, u8 ਹੋਰ){
  ਪੂਰਨ nElement = 0;
  ਜੇ( pWin==0 ) ਵਾਪਸ;
  ਜੇ( pWin->pFilter ){
    sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "FILTER", 1);
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, pWin->pFilter, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    ਜੇ( pWin->eFrmType==TK_FILTER ) ਵਾਪਸ;
  }
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ);
  ਜੇ( pWin->zName ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "OVER %s (%p)", pWin->zName, pWin);
  }ਨਹੀਂਤਾਂ{
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "OVER (%p)", pWin);
  }
  ਜੇ( pWin->z_ਅਧਾਰ )    nElement++;
  ਜੇ( pWin->p_ਕ੍ਰਮ_By ) nElement++;
  ਜੇ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  ਜੇ( pWin->eExclude ) nElement++;
  ਜੇ( pWin->z_ਅਧਾਰ ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--nElement)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "window: %s", pWin->z_ਅਧਾਰ);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pWin->p_ਹਿੱਸੇ_ਕਰੋ ){
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, pWin->p_ਹਿੱਸੇ_ਕਰੋ, nElement>0,"PARTITION-BY");
  }
  ਜੇ( pWin->p_ਕ੍ਰਮ_By ){
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, pWin->p_ਕ੍ਰਮ_By, (--nElement)>0, "ORDER-BY");
  }
  ਜੇ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    ਅੱਖਰ zBuf[30];
    ਅਚਲ ਅੱਖਰ *zFrmType = "ROWS";
    ਜੇ( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    ਜੇ( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(ਆਕਾਰ(zBuf),zBuf,"%s%s",zFrmType,
        pWin->b_Implicit_ਚੌਖਟਾ ? " (implied)" : "");
    sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, zBuf, (--nElement)>0);
    sqlite3_ਰੁੱਖ_View_Bound(pView, pWin->e_ਸ਼ੁਰੂ, pWin->p_ਸ਼ੁਰੂ, 1);
    sqlite3_ਰੁੱਖ_View_Bound(pView, pWin->e_ਸਿਰਾ, pWin->p_ਸਿਰਾ, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pWin->eExclude ){
    ਅੱਖਰ zBuf[30];
    ਅਚਲ ਅੱਖਰ *zExclude;
    ਚੋਣ( pWin->eExclude ){
      ਹਾਲਤ TK_NO:      zExclude = "NO OTHERS";   ਰੋਕੋ;
      ਹਾਲਤ TK_CURRENT: zExclude = "CURRENT ROW"; ਰੋਕੋ;
      ਹਾਲਤ TK_ਸਮੂਹ:   zExclude = "GROUP";       ਰੋਕੋ;
      ਹਾਲਤ TK_TIES:    zExclude = "TIES";        ਰੋਕੋ;
      ਮੂਲ:
        sqlite3_snprintf(ਆਕਾਰ(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        ਰੋਕੋ;
    }
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "EXCLUDE %s", zExclude);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Win_Func(ਰੁੱਖ_View *pView, ਅਚਲ ਖਿੜਕੀ *pWin, u8 ਹੋਰ){
  ਜੇ( pWin==0 ) ਵਾਪਸ;
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ);
  sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_ਰੁੱਖ_View_ਖਿੜਕੀ(pView, pWin, 0);
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(ਰੁੱਖ_View *pView, ਅਚਲ ਵਿਅੰਜਕ *p_ਵਿਅੰਜਕ, u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow){
  ਅਚਲ ਅੱਖਰ *zBinOp = 0;   /* Binary operator */
  ਅਚਲ ਅੱਖਰ *zUniOp = 0;   /* Unary operator */
  ਅੱਖਰ zFlgs[200];
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
  ਜੇ( p_ਵਿਅੰਜਕ==0 ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "nil");
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    ਵਾਪਸ;
  }
  ਜੇ( p_ਵਿਅੰਜਕ->flags || p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ || p_ਵਿਅੰਜਕ->vvaFlags || p_ਵਿਅੰਜਕ->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, ਆਕਾਰ(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_ਵਿਅੰਜਕ->flags, p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ ? p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ : 'n');
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_ਵਿਅੰਜਕ->w.iJoin);
    }
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_ਵਿਅੰਜਕ->w.iJoin);
    }
    ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ਜੇ( ਵਿਅੰਜਕ_Has_VVAProperty(p_ਵਿਅੰਜਕ, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    ਜੇ( p_ਵਿਅੰਜਕ->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_ਵਿਅੰਜਕ->iAgg);
    }
    sqlite3_Str_Accum_ਅੰਤ(&x);
  }ਨਹੀਂਤਾਂ{
    zFlgs[0] = 0;
  }
  ਚੋਣ( p_ਵਿਅੰਜਕ->op ){
    ਹਾਲਤ TK_AGG_ਕਾਲਮ: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "AGG{%d:%d}%s",
            p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ, zFlgs);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ਕਾਲਮ: {
      ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ<0 ){
        /* This only happens when coding check constraints */
        ਅੱਖਰ zOp2[16];
        ਜੇ( p_ਵਿਅੰਜਕ->op2 ){
          sqlite3_snprintf(ਆਕਾਰ(zOp2),zOp2," op2=0x%02x",p_ਵਿਅੰਜਕ->op2);
        }ਨਹੀਂਤਾਂ{
          zOp2[0] = 0;
        }
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "COLUMN(%d)%s%s",
                                    p_ਵਿਅੰਜਕ->i_ਕਾਲਮ, zFlgs, zOp2);
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_YTab(p_ਵਿਅੰਜਕ) );
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "{%d:%d} pTab=%p%s",
                        p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ,
                        p_ਵਿਅੰਜਕ->y.pTab, zFlgs);
      }
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Fixed_ਖਾਨਾ) ){
        sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
      }
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_INTEGER: {
      ਜੇ( p_ਵਿਅੰਜਕ->flags & EP_Int_ਮੁੱਲ ){
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%d", p_ਵਿਅੰਜਕ->u.i_ਮੁੱਲ);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%s", p_ਵਿਅੰਜਕ->u.zToken);
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_FLOATING_ਬਿੰਦੂ
    ਹਾਲਤ TK_FLOAT: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"%s", p_ਵਿਅੰਜਕ->u.zToken);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ TK_STRING: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"%Q", p_ਵਿਅੰਜਕ->u.zToken);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_NULL: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"NULL");
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_TRUEFALSE: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"%s%s",
         sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(p_ਵਿਅੰਜਕ) ? "TRUE" : "FALSE", zFlgs);
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_BLOB_LITERAL
    ਹਾਲਤ TK_BLOB: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"%s", p_ਵਿਅੰਜਕ->u.zToken);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ TK_VARIABLE: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"VARIABLE(%s,%d)",
                          p_ਵਿਅੰਜਕ->u.zToken, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ਰਜਿਸਟਰ: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"REGISTER(%d)", p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ID: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"ID \"%w\"", p_ਵਿਅੰਜਕ->u.zToken);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ASTERISK: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"ASKTERISK");
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_CAST
    ਹਾਲਤ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView,"CAST %Q", p_ਵਿਅੰਜਕ->u.zToken);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_CAST */
    ਹਾਲਤ TK_LT:      zBinOp = "LT";     ਰੋਕੋ;
    ਹਾਲਤ TK_LE:      zBinOp = "LE";     ਰੋਕੋ;
    ਹਾਲਤ TK_GT:      zBinOp = "GT";     ਰੋਕੋ;
    ਹਾਲਤ TK_GE:      zBinOp = "GE";     ਰੋਕੋ;
    ਹਾਲਤ TK_NE:      zBinOp = "NE";     ਰੋਕੋ;
    ਹਾਲਤ TK_EQ:      zBinOp = "EQ";     ਰੋਕੋ;
    ਹਾਲਤ TK_IS:      zBinOp = "IS";     ਰੋਕੋ;
    ਹਾਲਤ TK_ISNOT:   zBinOp = "ISNOT";  ਰੋਕੋ;
    ਹਾਲਤ TK_AND:     zBinOp = "AND";    ਰੋਕੋ;
    ਹਾਲਤ TK_OR:      zBinOp = "OR";     ਰੋਕੋ;
    ਹਾਲਤ TK_PLUS:    zBinOp = "ADD";    ਰੋਕੋ;
    ਹਾਲਤ TK_ਤਾਰਾ:    zBinOp = "MUL";    ਰੋਕੋ;
    ਹਾਲਤ TK_MINUS:   zBinOp = "SUB";    ਰੋਕੋ;
    ਹਾਲਤ TK_REM:     zBinOp = "REM";    ਰੋਕੋ;
    ਹਾਲਤ TK_BITAND:  zBinOp = "BITAND"; ਰੋਕੋ;
    ਹਾਲਤ TK_BITOR:   zBinOp = "BITOR";  ਰੋਕੋ;
    ਹਾਲਤ TK_SLASH:   zBinOp = "DIV";    ਰੋਕੋ;
    ਹਾਲਤ TK_LSHIFT:  zBinOp = "LSHIFT"; ਰੋਕੋ;
    ਹਾਲਤ TK_RSHIFT:  zBinOp = "RSHIFT"; ਰੋਕੋ;
    ਹਾਲਤ TK_CONCAT:  zBinOp = "CONCAT"; ਰੋਕੋ;
    ਹਾਲਤ TK_DOT:     zBinOp = "DOT";    ਰੋਕੋ;
    ਹਾਲਤ TK_ਹੱਦਬੰਦੀ:   zBinOp = "LIMIT";  ਰੋਕੋ;

    ਹਾਲਤ TK_UMINUS:  zUniOp = "UMINUS"; ਰੋਕੋ;
    ਹਾਲਤ TK_UPLUS:   zUniOp = "UPLUS";  ਰੋਕੋ;
    ਹਾਲਤ TK_BITNOT:  zUniOp = "BITNOT"; ਰੋਕੋ;
    ਹਾਲਤ TK_NOT:     zUniOp = "NOT";    ਰੋਕੋ;
    ਹਾਲਤ TK_ISNULL:  zUniOp = "ISNULL"; ਰੋਕੋ;
    ਹਾਲਤ TK_NOTNULL: zUniOp = "NOTNULL"; ਰੋਕੋ;

    ਹਾਲਤ TK_TRUTH: {
      ਪੂਰਨ x;
      ਅਚਲ ਅੱਖਰ *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->op2==TK_IS || p_ਵਿਅੰਜਕ->op2==TK_ISNOT );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->p_ਸੱਜਾ );
      ਪ੍ਰਤਿਜ੍ਞਾ( sqlite3_ਵਿਅੰਜਕ_Skip_Collate_And_Likely(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ)->op
                  == TK_TRUEFALSE );
      x = (p_ਵਿਅੰਜਕ->op2==TK_ISNOT)*2 + sqlite3_ਵਿਅੰਜਕ_Truth_ਮੁੱਲ(p_ਵਿਅੰਜਕ->p_ਸੱਜਾ);
      zUniOp = azOp[x];
      ਰੋਕੋ;
    }

    ਹਾਲਤ TK_ਵਿਸਤਾਰ: {
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "SPAN %Q", p_ਵਿਅੰਜਕ->u.zToken);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
      ਰੋਕੋ;
    }

    ਹਾਲਤ TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%sCOLLATE %Q%s",
        !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Collate) ? "SOFT-" : "",
        p_ਵਿਅੰਜਕ->u.zToken, zFlgs);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
      ਰੋਕੋ;
    }

    ਹਾਲਤ TK_AGG_FUNCTION:
    ਹਾਲਤ TK_FUNCTION: {
      ਵਿਅੰਜਕ_List *pFarg;       /* List of function arguments */
      ਖਿੜਕੀ *pWin;
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }ਨਹੀਂਤਾਂ{
        ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
        pFarg = p_ਵਿਅੰਜਕ->x.pList;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
        pWin = Is_ਖਿੜਕੀ_Func(p_ਵਿਅੰਜਕ) ? p_ਵਿਅੰਜਕ->y.pWin : 0;
#ਨਹੀਂਤਾਂ
        pWin = 0;
#ਜੇ_ਅੰਤ 
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      ਜੇ( p_ਵਿਅੰਜਕ->op==TK_AGG_FUNCTION ){
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_ਵਿਅੰਜਕ->op2, p_ਵਿਅੰਜਕ->u.zToken, zFlgs,
                             p_ਵਿਅੰਜਕ->pAggInfo ? p_ਵਿਅੰਜਕ->pAggInfo->selId : 0,
                             p_ਵਿਅੰਜਕ->iAgg, p_ਵਿਅੰਜਕ->pAggInfo);
      }ਨਹੀਂਤਾਂ ਜੇ( p_ਵਿਅੰਜਕ->op2!=0 ){
        ਅਚਲ ਅੱਖਰ *zOp2;
        ਅੱਖਰ zBuf[8];
        sqlite3_snprintf(ਆਕਾਰ(zBuf),zBuf,"0x%02x",p_ਵਿਅੰਜਕ->op2);
        zOp2 = zBuf;
        ਜੇ( p_ਵਿਅੰਜਕ->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        ਜੇ( p_ਵਿਅੰਜਕ->op2==NC_Idx_ਵਿਅੰਜਕ ) zOp2 = "NC_IdxExpr";
        ਜੇ( p_ਵਿਅੰਜਕ->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        ਜੇ( p_ਵਿਅੰਜਕ->op2==NC_Gen_ਖਾਨਾ ) zOp2 = "NC_GenCol";
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "FUNCTION %Q%s op2=%s",
                            p_ਵਿਅੰਜਕ->u.zToken, zFlgs, zOp2);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "FUNCTION %Q%s", p_ਵਿਅੰਜਕ->u.zToken, zFlgs);
      }
      ਜੇ( pFarg ){
        sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, pFarg, pWin!=0 || p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
        ਜੇ( p_ਵਿਅੰਜਕ->p_ਖੱਬਾ ){
          ਵਿਅੰਜਕ *pOB = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
          ਪ੍ਰਤਿਜ੍ਞਾ( pOB->op==TK_ਕ੍ਰਮ );
          ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(pOB) );
          sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
      ਜੇ( pWin ){
        sqlite3_ਰੁੱਖ_View_ਖਿੜਕੀ(pView, pWin, 0);
      }
#ਜੇ_ਅੰਤ
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ਕ੍ਰਮ: {
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p_ਵਿਅੰਜਕ->x.pList, 0, "ORDERBY");
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_SUBQUERY
    ਹਾਲਤ TK_EXISTS: {
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "EXISTS-expr flags=0x%x", p_ਵਿਅੰਜਕ->flags);
      sqlite3_ਰੁੱਖ_View_Select(pView, p_ਵਿਅੰਜਕ->x.pSelect, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_SELECT: {
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "subquery-expr flags=0x%x", p_ਵਿਅੰਜਕ->flags);
      sqlite3_ਰੁੱਖ_View_Select(pView, p_ਵਿਅੰਜਕ->x.pSelect, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ਵਿੱਚ: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      ਅੱਖਰ *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_ਵਿਅੰਜਕ->flags);
      ਜੇ( p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ ) sqlite3_str_appendf(pStr, " iTable=%d",p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ);
      ਜੇ( ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_ਵਿਅੰਜਕ->y.sub.regReturn, p_ਵਿਅੰਜਕ->y.sub.iAddr);
      }
      z = sqlite3_str_ਅੰਤ(pStr);
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, z);
      sqlite3_free(z);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 1);
      ਜੇ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ) ){
        sqlite3_ਰੁੱਖ_View_Select(pView, p_ਵਿਅੰਜਕ->x.pSelect, 0);
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p_ਵਿਅੰਜਕ->x.pList, 0, 0);
      }
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_SUBQUERY */

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
    ਹਾਲਤ TK_BETWEEN: {
      ਅਚਲ ਵਿਅੰਜਕ *pX, *pY, *pZ;
      pX = p_ਵਿਅੰਜਕ->p_ਖੱਬਾ;
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
      ਪ੍ਰਤਿਜ੍ਞਾ( p_ਵਿਅੰਜਕ->x.pList->n_ਵਿਅੰਜਕ==2 );
      pY = p_ਵਿਅੰਜਕ->x.pList->a[0].p_ਵਿਅੰਜਕ;
      pZ = p_ਵਿਅੰਜਕ->x.pList->a[1].p_ਵਿਅੰਜਕ;
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "BETWEEN%s", zFlgs);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, pX, 1);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, pY, 1);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, pZ, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%s(%d)", 
          p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ ? "NEW" : "OLD", p_ਵਿਅੰਜਕ->i_ਕਾਲਮ);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_CASE: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "CASE");
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 1);
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p_ਵਿਅੰਜਕ->x.pList, 0, 0);
      ਰੋਕੋ;
    }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
    ਹਾਲਤ TK_RAISE: {
      ਅਚਲ ਅੱਖਰ *zType = "unk";
      ਚੋਣ( p_ਵਿਅੰਜਕ->aff_ਵਿਅੰਜਕ ){
        ਹਾਲਤ OE_Rollback:   zType = "rollback";  ਰੋਕੋ;
        ਹਾਲਤ OE_Abort:      zType = "abort";     ਰੋਕੋ;
        ਹਾਲਤ OE_Fail:       zType = "fail";      ਰੋਕੋ;
        ਹਾਲਤ OE_Ignore:     zType = "ignore";    ਰੋਕੋ;
      }
      ਪ੍ਰਤਿਜ੍ਞਾ( !ਵਿਅੰਜਕ_Has_Property(p_ਵਿਅੰਜਕ, EP_Int_ਮੁੱਲ) );
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "RAISE %s", zType);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
      ਰੋਕੋ;
    }
#ਜੇ_ਅੰਤ
    ਹਾਲਤ TK_ਮੇਲ: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "MATCH {%d:%d}%s",
                          p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ, p_ਵਿਅੰਜਕ->i_ਕਾਲਮ, zFlgs);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ਸਦਿਸ਼: {
      ਅੱਖਰ *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XList(p_ਵਿਅੰਜਕ) );
      sqlite3_ਰੁੱਖ_View_Bare_ਵਿਅੰਜਕ_List(pView, p_ਵਿਅੰਜਕ->x.pList, z);
      sqlite3_free(z);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_SELECT_ਕਾਲਮ: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_ਵਿਅੰਜਕ->i_ਕਾਲਮ, p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ-1,
              p_ਵਿਅੰਜਕ->p_ਸੱਜਾ==p_ਵਿਅੰਜਕ->p_ਖੱਬਾ ? " (SELECT-owner)" : "");
      ਪ੍ਰਤਿਜ੍ਞਾ( ਵਿਅੰਜਕ_Use_XSelect(p_ਵਿਅੰਜਕ->p_ਖੱਬਾ) );
      sqlite3_ਰੁੱਖ_View_Select(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ->x.pSelect, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_IF_NULL_ਪੰਕਤੀ: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "IF-NULL-ROW %d", p_ਵਿਅੰਜਕ->i_ਸਾਰਣੀ);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ERROR: {
      ਵਿਅੰਜਕ tmp;
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "ERROR");
      tmp = *p_ਵਿਅੰਜਕ;
      tmp.op = p_ਵਿਅੰਜਕ->op2;
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, &tmp, 0);
      ਰੋਕੋ;
    }
    ਹਾਲਤ TK_ਪੰਕਤੀ: {
      ਜੇ( p_ਵਿਅੰਜਕ->i_ਕਾਲਮ<=0 ){
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "First FROM table rowid");
      }ਨਹੀਂਤਾਂ{
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "First FROM table column %d",
            p_ਵਿਅੰਜਕ->i_ਕਾਲਮ-1);
      }
      ਰੋਕੋ;
    }
    ਮੂਲ: {
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "op=%d", p_ਵਿਅੰਜਕ->op);
      ਰੋਕੋ;
    }
  }
  ਜੇ( zBinOp ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 1);
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਸੱਜਾ, 0);
  }ਨਹੀਂਤਾਂ ਜੇ( zUniOp ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਵਿਅੰਜਕ->p_ਖੱਬਾ, 0);
  }
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Bare_ਵਿਅੰਜਕ_List(
  ਰੁੱਖ_View *pView,
  ਅਚਲ ਵਿਅੰਜਕ_List *pList,
  ਅਚਲ ਅੱਖਰ *zLabel
){
  ਜੇ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ਜੇ( pList==0 ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%s (empty)", zLabel);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ i;
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%s", zLabel);
    ਲਈ(i=0; i<pList->n_ਵਿਅੰਜਕ; i++){
      ਪੂਰਨ j = pList->a[i].u.x.i_ਕ੍ਰਮ_By_ਖਾਨਾ;
      u8 ਛਾਂਟੋ_Flags = pList->a[i].fg.ਛਾਂਟੋ_Flags;
      ਅੱਖਰ *zName = pList->a[i].zEName;
      ਪੂਰਨ ਹੋਰ_ਮੰਜ਼ਿਲ_Follow = i<pList->n_ਵਿਅੰਜਕ - 1;
      ਜੇ( j || zName || ਛਾਂਟੋ_Flags ){
        sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
        ਹੋਰ_ਮੰਜ਼ਿਲ_Follow = 0;
        sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, 0);
        ਜੇ( zName ){
          ਚੋਣ( pList->a[i].fg.eEName ){
            ਮੂਲ:
              fprintf(stdout, "AS %s ", zName);
              ਰੋਕੋ;
            ਹਾਲਤ ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              ਜੇ( pList->a[i].fg.b_ਵਰਤਿਆ ) fprintf(stdout, "(used) ");
              ਜੇ( pList->a[i].fg.b_Using_ਪਦ ) fprintf(stdout, "(USING-term) ");
              ਜੇ( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              ਰੋਕੋ;
            ਹਾਲਤ ENAME_ਵਿਸਤਾਰ:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              ਰੋਕੋ;
          }
        }
        ਜੇ( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        ਜੇ( ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_DESC ){
          fprintf(stdout, "DESC ");
        }ਨਹੀਂਤਾਂ ਜੇ( ਛਾਂਟੋ_Flags & KEYINFO_ਕ੍ਰਮ_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, pList->a[i].p_ਵਿਅੰਜਕ, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
      ਜੇ( j || zName || ਛਾਂਟੋ_Flags ){
        sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
      }
    }
  }
}
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(
  ਰੁੱਖ_View *pView,
  ਅਚਲ ਵਿਅੰਜਕ_List *pList,
  u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow,
  ਅਚਲ ਅੱਖਰ *zLabel
){
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
  sqlite3_ਰੁੱਖ_View_Bare_ਵਿਅੰਜਕ_List(pView, pList, zLabel);
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Bare_Id_List(
  ਰੁੱਖ_View *pView,
  ਅਚਲ IdList *pList,
  ਅਚਲ ਅੱਖਰ *zLabel
){
  ਜੇ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ਜੇ( pList==0 ){
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%s (empty)", zLabel);
  }ਨਹੀਂਤਾਂ{
    ਪੂਰਨ i;
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%s", zLabel);
    ਲਈ(i=0; i<pList->nId; i++){
      ਅੱਖਰ *zName = pList->a[i].zName;
      ਪੂਰਨ ਹੋਰ_ਮੰਜ਼ਿਲ_Follow = i<pList->nId - 1;
      ਜੇ( zName==0 ) zName = "(null)";
      sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
      sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
  }
}
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Id_List(
  ਰੁੱਖ_View *pView,
  ਅਚਲ IdList *pList,
  u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow,
  ਅਚਲ ਅੱਖਰ *zLabel
){
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
  sqlite3_ਰੁੱਖ_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Upsert(
  ਰੁੱਖ_View *pView,
  ਅਚਲ Upsert *pUpsert,
  u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow
){
  ਜੇ( pUpsert==0 ) ਵਾਪਸ;
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
  ਜਦੋਂ( pUpsert ){
    ਪੂਰਨ n;
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, pUpsert->p_ਅਗਲਾ_Upsert!=0 || ਹੋਰ_ਮੰਜ਼ਿਲ_Follow);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->p_Upsert_ਸੈੱਟ!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, pUpsert->p_Upsert_ਨਿਸ਼ਾਨਾ, (n--)>0, "TARGET");
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, pUpsert->p_Upsert_ਸੈੱਟ, (n--)>0, "SET");
    ਜੇ( pUpsert->pUpsertWhere ){
      sqlite3_ਰੁੱਖ_View_ਵਸਤੂ(pView, "WHERE", (n--)>0);
      sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    }
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
    pUpsert = pUpsert->p_ਅਗਲਾ_Upsert;
  }
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}

#ਜੇ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Delete(
  ਅਚਲ With *pWith,
  ਅਚਲ SrcList *pTabList,
  ਅਚਲ ਵਿਅੰਜਕ *pWhere,
  ਅਚਲ ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,
  ਅਚਲ ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ,
  ਅਚਲ Trigger *pTrigger
){
  ਪੂਰਨ n = 0;
  ਰੁੱਖ_View *pView = 0;
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 0);
  sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "DELETE");
  ਜੇ( pWith ) n++;
  ਜੇ( pTabList ) n++;
  ਜੇ( pWhere ) n++;
  ਜੇ( p_ਕ੍ਰਮ_By ) n++;
  ਜੇ( p_ਹੱਦਬੰਦੀ ) n++;
  ਜੇ( pTrigger ) n++;
  ਜੇ( pWith ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_With(pView, pWith, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pTabList ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "FROM");
    sqlite3_ਰੁੱਖ_View_Src_List(pView, pTabList);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pWhere ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "WHERE");
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, pWhere, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( p_ਕ੍ਰਮ_By ){
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p_ਕ੍ਰਮ_By, (--n)>0, "ORDER-BY");
  }
  ਜੇ( p_ਹੱਦਬੰਦੀ ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "LIMIT");
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਹੱਦਬੰਦੀ, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pTrigger ){
    sqlite3_ਰੁੱਖ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}
#ਜੇ_ਅੰਤ /* TREETRACE_ENABLED */

#ਜੇ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_ਪਾਓ(
  ਅਚਲ With *pWith,
  ਅਚਲ SrcList *pTabList,
  ਅਚਲ IdList *p_ਕਾਲਮ_List,
  ਅਚਲ Select *pSelect,
  ਅਚਲ ਵਿਅੰਜਕ_List *p_ਵਿਅੰਜਕ_List,
  ਪੂਰਨ onError,
  ਅਚਲ Upsert *pUpsert,
  ਅਚਲ Trigger *pTrigger
){
  ਰੁੱਖ_View *pView = 0;
  ਪੂਰਨ n = 0;
  ਅਚਲ ਅੱਖਰ *zLabel = "INSERT";
  ਚੋਣ( onError ){
    ਹਾਲਤ OE_Replace:  zLabel = "REPLACE";             ਰੋਕੋ;
    ਹਾਲਤ OE_Ignore:   zLabel = "INSERT OR IGNORE";    ਰੋਕੋ;
    ਹਾਲਤ OE_Rollback: zLabel = "INSERT OR ROLLBACK";  ਰੋਕੋ;
    ਹਾਲਤ OE_Abort:    zLabel = "INSERT OR ABORT";     ਰੋਕੋ;
    ਹਾਲਤ OE_Fail:     zLabel = "INSERT OR FAIL";      ਰੋਕੋ;
  }
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 0);
  sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, zLabel);
  ਜੇ( pWith ) n++;
  ਜੇ( pTabList ) n++;
  ਜੇ( p_ਕਾਲਮ_List ) n++;
  ਜੇ( pSelect ) n++;
  ਜੇ( p_ਵਿਅੰਜਕ_List ) n++;
  ਜੇ( pUpsert ) n++;
  ਜੇ( pTrigger ) n++;
  ਜੇ( pWith ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_With(pView, pWith, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pTabList ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "INTO");
    sqlite3_ਰੁੱਖ_View_Src_List(pView, pTabList);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( p_ਕਾਲਮ_List ){
    sqlite3_ਰੁੱਖ_View_Id_List(pView, p_ਕਾਲਮ_List, (--n)>0, "COLUMNS");
  }
  ਜੇ( pSelect ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "DATA-SOURCE");
    sqlite3_ਰੁੱਖ_View_Select(pView, pSelect, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( p_ਵਿਅੰਜਕ_List ){
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p_ਵਿਅੰਜਕ_List, (--n)>0, "VALUES");
  }
  ਜੇ( pUpsert ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "UPSERT");
    sqlite3_ਰੁੱਖ_View_Upsert(pView, pUpsert, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pTrigger ){
    sqlite3_ਰੁੱਖ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}
#ਜੇ_ਅੰਤ /* TREETRACE_ENABLED */

#ਜੇ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Update(
  ਅਚਲ With *pWith,
  ਅਚਲ SrcList *pTabList,
  ਅਚਲ ਵਿਅੰਜਕ_List *pChanges,
  ਅਚਲ ਵਿਅੰਜਕ *pWhere,
  ਪੂਰਨ onError,
  ਅਚਲ ਵਿਅੰਜਕ_List *p_ਕ੍ਰਮ_By,
  ਅਚਲ ਵਿਅੰਜਕ *p_ਹੱਦਬੰਦੀ,
  ਅਚਲ Upsert *pUpsert,
  ਅਚਲ Trigger *pTrigger
){
  ਪੂਰਨ n = 0;
  ਰੁੱਖ_View *pView = 0;
  ਅਚਲ ਅੱਖਰ *zLabel = "UPDATE";
  ਚੋਣ( onError ){
    ਹਾਲਤ OE_Replace:  zLabel = "UPDATE OR REPLACE";   ਰੋਕੋ;
    ਹਾਲਤ OE_Ignore:   zLabel = "UPDATE OR IGNORE";    ਰੋਕੋ;
    ਹਾਲਤ OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  ਰੋਕੋ;
    ਹਾਲਤ OE_Abort:    zLabel = "UPDATE OR ABORT";     ਰੋਕੋ;
    ਹਾਲਤ OE_Fail:     zLabel = "UPDATE OR FAIL";      ਰੋਕੋ;
  }
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 0);
  sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, zLabel);
  ਜੇ( pWith ) n++;
  ਜੇ( pTabList ) n++;
  ਜੇ( pChanges ) n++;
  ਜੇ( pWhere ) n++;
  ਜੇ( p_ਕ੍ਰਮ_By ) n++;
  ਜੇ( p_ਹੱਦਬੰਦੀ ) n++;
  ਜੇ( pUpsert ) n++;
  ਜੇ( pTrigger ) n++;
  ਜੇ( pWith ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_With(pView, pWith, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pTabList ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "FROM");
    sqlite3_ਰੁੱਖ_View_Src_List(pView, pTabList);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pChanges ){
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, pChanges, (--n)>0, "SET");
  }
  ਜੇ( pWhere ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "WHERE");
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, pWhere, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( p_ਕ੍ਰਮ_By ){
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(pView, p_ਕ੍ਰਮ_By, (--n)>0, "ORDER-BY");
  }
  ਜੇ( p_ਹੱਦਬੰਦੀ ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "LIMIT");
    sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(pView, p_ਹੱਦਬੰਦੀ, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pUpsert ){
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, (--n)>0);
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "UPSERT");
    sqlite3_ਰੁੱਖ_View_Upsert(pView, pUpsert, 0);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }
  ਜੇ( pTrigger ){
    sqlite3_ਰੁੱਖ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}
#ਜੇ_ਅੰਤ /* TREETRACE_ENABLED */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Trigger_ਕਦਮ(
  ਰੁੱਖ_View *pView,
  ਅਚਲ Trigger_ਕਦਮ *p_ਕਦਮ,
  u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow,
  u8 show_ਭਰਿਆ_List
){
  ਪੂਰਨ cnt = 0;
  ਜੇ( p_ਕਦਮ==0 ) ਵਾਪਸ;
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 
      ਹੋਰ_ਮੰਜ਼ਿਲ_Follow || (show_ਭਰਿਆ_List && p_ਕਦਮ->p_ਅਗਲਾ!=0));
  ਕਰੋ{
    ਜੇ( cnt++ && p_ਕਦਮ->p_ਅਗਲਾ==0 ){
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
      sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 0);
    }
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "%s", p_ਕਦਮ->z_ਵਿਸਤਾਰ ? p_ਕਦਮ->z_ਵਿਸਤਾਰ : "RETURNING");
  }ਜਦੋਂ( show_ਭਰਿਆ_List && (p_ਕਦਮ = p_ਕਦਮ->p_ਅਗਲਾ)!=0 );
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
ਖਾਲੀ sqlite3_ਰੁੱਖ_View_Trigger(
  ਰੁੱਖ_View *pView,
  ਅਚਲ Trigger *pTrigger,
  u8 ਹੋਰ_ਮੰਜ਼ਿਲ_Follow,
  u8 show_ਭਰਿਆ_List
){
  ਪੂਰਨ cnt = 0;
  ਜੇ( pTrigger==0 ) ਵਾਪਸ;
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView,
     ਹੋਰ_ਮੰਜ਼ਿਲ_Follow || (show_ਭਰਿਆ_List && pTrigger->p_ਅਗਲਾ!=0));
  ਕਰੋ{
    ਜੇ( cnt++ && pTrigger->p_ਅਗਲਾ==0 ){
      sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
      sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 0);
    }
    sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 0);
    sqlite3_ਰੁੱਖ_View_Trigger_ਕਦਮ(pView, pTrigger->ਕਦਮ_list, 0, 1);
    sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
  }ਜਦੋਂ( show_ਭਰਿਆ_List && (pTrigger = pTrigger->p_ਅਗਲਾ)!=0 );
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}
#ਜੇ_ਅੰਤ /* SQLITE_OMIT_TRIGGER */
  

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
ਖਾਲੀ sqlite3_Show_ਵਿਅੰਜਕ(ਅਚਲ ਵਿਅੰਜਕ *p){ sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ(0,p,0); }
ਖਾਲੀ sqlite3_Show_ਵਿਅੰਜਕ_List(ਅਚਲ ਵਿਅੰਜਕ_List *p){ sqlite3_ਰੁੱਖ_View_ਵਿਅੰਜਕ_List(0,p,0,0);}
ਖਾਲੀ sqlite3ShowIdList(ਅਚਲ IdList *p){ sqlite3_ਰੁੱਖ_View_Id_List(0,p,0,0); }
ਖਾਲੀ sqlite3ShowSrcList(ਅਚਲ SrcList *p){
  ਰੁੱਖ_View *pView = 0;
  sqlite3_ਰੁੱਖ_View_ਧੱਕੋ(&pView, 0);
  sqlite3_ਰੁੱਖ_View_ਸਤਰ(pView, "SRCLIST");
  sqlite3_ਰੁੱਖ_View_Src_List(pView,p);
  sqlite3_ਰੁੱਖ_View_ਕੱਢੋ(&pView);
}
ਖਾਲੀ sqlite3ShowSelect(ਅਚਲ Select *p){ sqlite3_ਰੁੱਖ_View_Select(0,p,0); }
ਖਾਲੀ sqlite3ShowWith(ਅਚਲ With *p){ sqlite3_ਰੁੱਖ_View_With(0,p,0); }
ਖਾਲੀ sqlite3ShowUpsert(ਅਚਲ Upsert *p){ sqlite3_ਰੁੱਖ_View_Upsert(0,p,0); }
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_TRIGGER
ਖਾਲੀ sqlite3_Show_Trigger_ਕਦਮ(ਅਚਲ Trigger_ਕਦਮ *p){
  sqlite3_ਰੁੱਖ_View_Trigger_ਕਦਮ(0,p,0,0);
}
ਖਾਲੀ sqlite3_Show_Trigger_ਕਦਮ_List(ਅਚਲ Trigger_ਕਦਮ *p){
  sqlite3_ਰੁੱਖ_View_Trigger_ਕਦਮ(0,p,0,1);
}
ਖਾਲੀ sqlite3ShowTrigger(ਅਚਲ Trigger *p){ sqlite3_ਰੁੱਖ_View_Trigger(0,p,0,0); }
ਖਾਲੀ sqlite3ShowTriggerList(ਅਚਲ Trigger *p){ sqlite3_ਰੁੱਖ_View_Trigger(0,p,0,1);}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ SQLITE_OMIT_WINDOWFUNC
ਖਾਲੀ sqlite3_Show_ਖਿੜਕੀ(ਅਚਲ ਖਿੜਕੀ *p){ sqlite3_ਰੁੱਖ_View_ਖਿੜਕੀ(0,p,0); }
ਖਾਲੀ sqlite3ShowWinFunc(ਅਚਲ ਖਿੜਕੀ *p){ sqlite3_ਰੁੱਖ_View_Win_Func(0,p,0); }
#ਜੇ_ਅੰਤ

#ਜੇ_ਅੰਤ /* SQLITE_DEBUG */
