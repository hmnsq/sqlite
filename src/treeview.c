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
#অন্তর্ভাৱ "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
স্থির শূন্য sqlite3_বৃক্ষ_View_ঠেলো(বৃক্ষ_View **pp, u8 moreToFollow){
  বৃক্ষ_View *p = *pp;
  যদি( p==0 ){
    *pp = p = sqlite3_malloc64( পরিমাণ(*p) );
    যদি( p==0 ) ঘূৰাই;
    memset(p, 0, পরিমাণ(*p));
  }অন্যথা{
    p->iLevel++;
  }
  প্রতিজ্ঞা( moreToFollow==0 || moreToFollow==1 );
  যদি( p->iLevel<(পূর্ণাঙ্ক)পরিমাণ(p->b_সাৰি) ) p->b_সাৰি[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
স্থির শূন্য sqlite3_বৃক্ষ_View_বেৰ_কৰো(বৃক্ষ_View **pp){
  বৃক্ষ_View *p = *pp;
  যদি( p==0 ) ঘূৰাই;
  p->iLevel--;
  যদি( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
শূন্য sqlite3_বৃক্ষ_View_সাৰি(বৃক্ষ_View *p, ধ্রুৱ ৱর্ণ *zFormat, ...){
  va_list ap;
  পূর্ণাঙ্ক i;
  StrAccum acc;
  ৱর্ণ zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, পরিমাণ(zBuf), 0);
  যদি( p ){
    বাবে(i=0; i<p->iLevel && i<(পূর্ণাঙ্ক)পরিমাণ(p->b_সাৰি)-1; i++){
      sqlite3_str_append(&acc, p->b_সাৰি[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_সাৰি[i] ? "|-- " : "'-- ", 4);
  }
  যদি( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    প্রতিজ্ঞা( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
স্থির শূন্য sqlite3_বৃক্ষ_View_Item(বৃক্ষ_View *p, ধ্রুৱ ৱর্ণ *zLabel,u8 moreFollows){
  sqlite3_বৃক্ষ_View_ঠেলো(&p, moreFollows);
  sqlite3_বৃক্ষ_View_সাৰি(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
শূন্য sqlite3_বৃক্ষ_View_Column_List(
  বৃক্ষ_View *pView,
  ধ্রুৱ Column *aCol,
  পূর্ণাঙ্ক nCol,
  u8 moreToFollow
){
  পূর্ণাঙ্ক i;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  sqlite3_বৃক্ষ_View_সাৰি(pView, "COLUMNS");
  বাবে(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    পূর্ণাঙ্ক colMoreToFollow = i<(nCol - 1);
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, colMoreToFollow);
    sqlite3_বৃক্ষ_View_সাৰি(pView, 0);
    printf(" %s", aCol[i].zCnName);
    ৱিকল্প( aCol[i].eCType ){
      পক্ষ COLTYPE_ANY:      printf(" ANY");        ভঙ্গ;
      পক্ষ COLTYPE_BLOB:     printf(" BLOB");       ভঙ্গ;
      পক্ষ COLTYPE_INT:      printf(" INT");        ভঙ্গ;
      পক্ষ COLTYPE_INTEGER:  printf(" INTEGER");    ভঙ্গ;
      পক্ষ COLTYPE_REAL:     printf(" REAL");       ভঙ্গ;
      পক্ষ COLTYPE_TEXT:     printf(" TEXT");       ভঙ্গ;
      পক্ষ COLTYPE_CUSTOM: {
        যদি( flg & COLFLAG_HASTYPE ){
          ধ্রুৱ ৱর্ণ *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          ভঙ্গ;
        }
      }
    }
    যদি( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    যদি( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    যদি( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    যদি( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
শূন্য sqlite3_বৃক্ষ_View_With(বৃক্ষ_View *pView, ধ্রুৱ With *pWith, u8 moreToFollow){
  পূর্ণাঙ্ক i;
  যদি( pWith==0 ) ঘূৰাই;
  যদি( pWith->nCte==0 ) ঘূৰাই;
  যদি( pWith->pOuter ){
    sqlite3_বৃক্ষ_View_সাৰি(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }অন্যথা{
    sqlite3_বৃক্ষ_View_সাৰি(pView, "WITH (0x%p)", pWith);
  }
  যদি( pWith->nCte>0 ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
    বাবে(i=0; i<pWith->nCte; i++){
      StrAccum x;
      ৱর্ণ z_সাৰি[1000];
      ধ্রুৱ সংরচনা Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_সাৰি, পরিমাণ(z_সাৰি), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      যদি( pCte->pCols && pCte->pCols->n_ৰাশিমালা>0 ){
        ৱর্ণ cSep = '(';
        পূর্ণাঙ্ক j;
        বাবে(j=0; j<pCte->pCols->n_ৰাশিমালা; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      যদি( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      যদি( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_বৃক্ষ_View_Item(pView, z_সাৰি, i<pWith->nCte-1);
      sqlite3_বৃক্ষ_View_Select(pView, pCte->pSelect, 0);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
শূন্য sqlite3_বৃক্ষ_View_Src_List(বৃক্ষ_View *pView, ধ্রুৱ SrcList *pSrc){
  পূর্ণাঙ্ক i;
  যদি( pSrc==0 ) ঘূৰাই;
  বাবে(i=0; i<pSrc->nSrc; i++){
    ধ্রুৱ SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    পূর্ণাঙ্ক n = 0;
    ৱর্ণ z_সাৰি[1000];
    sqlite3StrAccumInit(&x, 0, z_সাৰি, পরিমাণ(z_সাৰি), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    যদি( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    যদি( (pItem->fg.jointype & (JT_বাম|JT_ডান))==(JT_বাম|JT_ডান) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }অন্যথা যদি( pItem->fg.jointype & JT_বাম ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }অন্যথা যদি( pItem->fg.jointype & JT_ডান ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }অন্যথা যদি( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    যদি( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    যদি( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    যদি( pItem->fg.isCte ){
      স্থির ধ্রুৱ ৱর্ণ *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    যদি( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    যদি( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    যদি( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    যদি( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    যদি( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    যদি( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    যদি( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    যদি( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    যদি( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    যদি( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_বৃক্ষ_View_Item(pView, z_সাৰি, i<pSrc->nSrc-1);
    n = 0;
    যদি( pItem->fg.isSubquery ) n++;
    যদি( pItem->fg.isTabFunc ) n++;
    যদি( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    যদি( pItem->fg.isUsing ){
      sqlite3_বৃক্ষ_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }অন্যথা যদি( pItem->u3.pOn!=0 ){
      sqlite3_বৃক্ষ_View_Item(pView, "ON", (--n)>0);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pItem->u3.pOn, 0);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
    যদি( pItem->fg.isSubquery ){
      প্রতিজ্ঞা( n==1 );
      যদি( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_বৃক্ষ_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      প্রতিজ্ঞা( (পূর্ণাঙ্ক)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_বৃক্ষ_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    যদি( pItem->fg.isTabFunc ){
      sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
শূন্য sqlite3_বৃক্ষ_View_Select(বৃক্ষ_View *pView, ধ্রুৱ Select *p, u8 moreToFollow){
  পূর্ণাঙ্ক n = 0;
  পূর্ণাঙ্ক cnt = 0;
  যদি( p==0 ){
    sqlite3_বৃক্ষ_View_সাৰি(pView, "nil-SELECT");
    ঘূৰাই;
  } 
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  যদি( p->pWith ){
    sqlite3_বৃক্ষ_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, 1);
  }
  কৰা{
    যদি( p->selFlags & SF_WhereBegin ){
      sqlite3_বৃক্ষ_View_সাৰি(pView, "sqlite3WhereBegin()");
    }অন্যথা{
      sqlite3_বৃক্ষ_View_সাৰি(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (পূর্ণাঙ্ক)p->nSelectRow
      );
    }
    যদি( cnt++ ) sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    যদি( p->pPrior ){
      n = 1000;
    }অন্যথা{
      n = 0;
      যদি( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      যদি( p->pWhere ) n++;
      যদি( p->pGroupBy ) n++;
      যদি( p->pHaving ) n++;
      যদি( p->pOrderBy ) n++;
      যদি( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      যদি( p->pWin ) n++;
      যদি( p->pWinDefn ) n++;
#endif
    }
    যদি( p->pEList ){
      sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    যদি( p->pWin ){
      Window *pX;
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, (n--)>0);
      sqlite3_বৃক্ষ_View_সাৰি(pView, "window-functions");
      বাবে(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_বৃক্ষ_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
#endif
    যদি( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, (n--)>0);
      sqlite3_বৃক্ষ_View_সাৰি(pView, "FROM");
      sqlite3_বৃক্ষ_View_Src_List(pView, p->pSrc);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
    যদি( p->pWhere ){
      sqlite3_বৃক্ষ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p->pWhere, 0);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
    যদি( p->pGroupBy ){
      sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    যদি( p->pHaving ){
      sqlite3_বৃক্ষ_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p->pHaving, 0);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    যদি( p->pWinDefn ){
      Window *pX;
      sqlite3_বৃক্ষ_View_Item(pView, "WINDOW", (n--)>0);
      বাবে(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_বৃক্ষ_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
#endif
    যদি( p->pOrderBy ){
      sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    যদি( p->pLimit ){
      sqlite3_বৃক্ষ_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p->pLimit->p_বাম, p->pLimit->p_ডান!=0);
      যদি( p->pLimit->p_ডান ){
        sqlite3_বৃক্ষ_View_Item(pView, "OFFSET", 0);
        sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p->pLimit->p_ডান, 0);
        sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
      }
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
    যদি( p->pPrior ){
      ধ্রুৱ ৱর্ণ *zOp = "UNION";
      ৱিকল্প( p->op ){
        পক্ষ TK_ALL:         zOp = "UNION ALL";  ভঙ্গ;
        পক্ষ TK_INTERSECT:   zOp = "INTERSECT";  ভঙ্গ;
        পক্ষ TK_EXCEPT:      zOp = "EXCEPT";     ভঙ্গ;
      }
      sqlite3_বৃক্ষ_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }যেতিয়ালৈ( p!=0 );
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
শূন্য sqlite3_বৃক্ষ_View_Bound(
  বৃক্ষ_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  ৰাশিমালা *p_ৰাশিমালা,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  ৱিকল্প( eBound ){
    পক্ষ TK_UNBOUNDED: {
      sqlite3_বৃক্ষ_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
      ভঙ্গ;
    }
    পক্ষ TK_CURRENT: {
      sqlite3_বৃক্ষ_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
      ভঙ্গ;
    }
    পক্ষ TK_PRECEDING: {
      sqlite3_বৃক্ষ_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা, 0);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
      ভঙ্গ;
    }
    পক্ষ TK_FOLLOWING: {
      sqlite3_বৃক্ষ_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা, 0);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
      ভঙ্গ;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
শূন্য sqlite3_বৃক্ষ_View_Window(বৃক্ষ_View *pView, ধ্রুৱ Window *pWin, u8 more){
  পূর্ণাঙ্ক nElement = 0;
  যদি( pWin==0 ) ঘূৰাই;
  যদি( pWin->pFilter ){
    sqlite3_বৃক্ষ_View_Item(pView, "FILTER", 1);
    sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pWin->pFilter, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    যদি( pWin->eFrmType==TK_FILTER ) ঘূৰাই;
  }
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, more);
  যদি( pWin->zName ){
    sqlite3_বৃক্ষ_View_সাৰি(pView, "OVER %s (%p)", pWin->zName, pWin);
  }অন্যথা{
    sqlite3_বৃক্ষ_View_সাৰি(pView, "OVER (%p)", pWin);
  }
  যদি( pWin->zBase )    nElement++;
  যদি( pWin->pOrderBy ) nElement++;
  যদি( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  যদি( pWin->eExclude ) nElement++;
  যদি( pWin->zBase ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--nElement)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "window: %s", pWin->zBase);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pWin->pPartition ){
    sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  যদি( pWin->pOrderBy ){
    sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  যদি( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    ৱর্ণ zBuf[30];
    ধ্রুৱ ৱর্ণ *zFrmType = "ROWS";
    যদি( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    যদি( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(পরিমাণ(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_বৃক্ষ_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_বৃক্ষ_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_বৃক্ষ_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pWin->eExclude ){
    ৱর্ণ zBuf[30];
    ধ্রুৱ ৱর্ণ *zExclude;
    ৱিকল্প( pWin->eExclude ){
      পক্ষ TK_NO:      zExclude = "NO OTHERS";   ভঙ্গ;
      পক্ষ TK_CURRENT: zExclude = "CURRENT ROW"; ভঙ্গ;
      পক্ষ TK_GROUP:   zExclude = "GROUP";       ভঙ্গ;
      পক্ষ TK_TIES:    zExclude = "TIES";        ভঙ্গ;
      স্ৱভাৱ:
        sqlite3_snprintf(পরিমাণ(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        ভঙ্গ;
    }
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "EXCLUDE %s", zExclude);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
শূন্য sqlite3_বৃক্ষ_View_Win_Func(বৃক্ষ_View *pView, ধ্রুৱ Window *pWin, u8 more){
  যদি( pWin==0 ) ঘূৰাই;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, more);
  sqlite3_বৃক্ষ_View_সাৰি(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_বৃক্ষ_View_Window(pView, pWin, 0);
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
শূন্য sqlite3_বৃক্ষ_View_ৰাশিমালা(বৃক্ষ_View *pView, ধ্রুৱ ৰাশিমালা *p_ৰাশিমালা, u8 moreToFollow){
  ধ্রুৱ ৱর্ণ *zBinOp = 0;   /* Binary operator */
  ধ্রুৱ ৱর্ণ *zUniOp = 0;   /* Unary operator */
  ৱর্ণ zFlgs[200];
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  যদি( p_ৰাশিমালা==0 ){
    sqlite3_বৃক্ষ_View_সাৰি(pView, "nil");
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    ঘূৰাই;
  }
  যদি( p_ৰাশিমালা->flags || p_ৰাশিমালা->aff_ৰাশিমালা || p_ৰাশিমালা->vvaFlags || p_ৰাশিমালা->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, পরিমাণ(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_ৰাশিমালা->flags, p_ৰাশিমালা->aff_ৰাশিমালা ? p_ৰাশিমালা->aff_ৰাশিমালা : 'n');
    যদি( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_ৰাশিমালা->w.iJoin);
    }
    যদি( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_ৰাশিমালা->w.iJoin);
    }
    যদি( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    যদি( ৰাশিমালা_Has_VVAProperty(p_ৰাশিমালা, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    যদি( p_ৰাশিমালা->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_ৰাশিমালা->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }অন্যথা{
    zFlgs[0] = 0;
  }
  ৱিকল্প( p_ৰাশিমালা->op ){
    পক্ষ TK_AGG_COLUMN: {
      sqlite3_বৃক্ষ_View_সাৰি(pView, "AGG{%d:%d}%s",
            p_ৰাশিমালা->iTable, p_ৰাশিমালা->iColumn, zFlgs);
      ভঙ্গ;
    }
    পক্ষ TK_COLUMN: {
      যদি( p_ৰাশিমালা->iTable<0 ){
        /* This only happens when coding check constraints */
        ৱর্ণ zOp2[16];
        যদি( p_ৰাশিমালা->op2 ){
          sqlite3_snprintf(পরিমাণ(zOp2),zOp2," op2=0x%02x",p_ৰাশিমালা->op2);
        }অন্যথা{
          zOp2[0] = 0;
        }
        sqlite3_বৃক্ষ_View_সাৰি(pView, "COLUMN(%d)%s%s",
                                    p_ৰাশিমালা->iColumn, zFlgs, zOp2);
      }অন্যথা{
        প্রতিজ্ঞা( ৰাশিমালা_Use_YTab(p_ৰাশিমালা) );
        sqlite3_বৃক্ষ_View_সাৰি(pView, "{%d:%d} pTab=%p%s",
                        p_ৰাশিমালা->iTable, p_ৰাশিমালা->iColumn,
                        p_ৰাশিমালা->y.pTab, zFlgs);
      }
      যদি( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_FixedCol) ){
        sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 0);
      }
      ভঙ্গ;
    }
    পক্ষ TK_INTEGER: {
      যদি( p_ৰাশিমালা->flags & EP_Int_মান ){
        sqlite3_বৃক্ষ_View_সাৰি(pView, "%d", p_ৰাশিমালা->u.i_মান);
      }অন্যথা{
        sqlite3_বৃক্ষ_View_সাৰি(pView, "%s", p_ৰাশিমালা->u.zToken);
      }
      ভঙ্গ;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    পক্ষ TK_FLOAT: {
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সাৰি(pView,"%s", p_ৰাশিমালা->u.zToken);
      ভঙ্গ;
    }
#endif
    পক্ষ TK_STRING: {
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সাৰি(pView,"%Q", p_ৰাশিমালা->u.zToken);
      ভঙ্গ;
    }
    পক্ষ TK_NULL: {
      sqlite3_বৃক্ষ_View_সাৰি(pView,"NULL");
      ভঙ্গ;
    }
    পক্ষ TK_TRUEFALSE: {
      sqlite3_বৃক্ষ_View_সাৰি(pView,"%s%s",
         sqlite3_ৰাশিমালা_Truth_মান(p_ৰাশিমালা) ? "TRUE" : "FALSE", zFlgs);
      ভঙ্গ;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    পক্ষ TK_BLOB: {
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সাৰি(pView,"%s", p_ৰাশিমালা->u.zToken);
      ভঙ্গ;
    }
#endif
    পক্ষ TK_VARIABLE: {
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সাৰি(pView,"VARIABLE(%s,%d)",
                          p_ৰাশিমালা->u.zToken, p_ৰাশিমালা->iColumn);
      ভঙ্গ;
    }
    পক্ষ TK_REGISTER: {
      sqlite3_বৃক্ষ_View_সাৰি(pView,"REGISTER(%d)", p_ৰাশিমালা->iTable);
      ভঙ্গ;
    }
    পক্ষ TK_ID: {
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সাৰি(pView,"ID \"%w\"", p_ৰাশিমালা->u.zToken);
      ভঙ্গ;
    }
    পক্ষ TK_ASTERISK: {
      sqlite3_বৃক্ষ_View_সাৰি(pView,"ASKTERISK");
      ভঙ্গ;
    }
#ifndef SQLITE_OMIT_CAST
    পক্ষ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সাৰি(pView,"CAST %Q", p_ৰাশিমালা->u.zToken);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 0);
      ভঙ্গ;
    }
#endif /* SQLITE_OMIT_CAST */
    পক্ষ TK_LT:      zBinOp = "LT";     ভঙ্গ;
    পক্ষ TK_LE:      zBinOp = "LE";     ভঙ্গ;
    পক্ষ TK_GT:      zBinOp = "GT";     ভঙ্গ;
    পক্ষ TK_GE:      zBinOp = "GE";     ভঙ্গ;
    পক্ষ TK_NE:      zBinOp = "NE";     ভঙ্গ;
    পক্ষ TK_EQ:      zBinOp = "EQ";     ভঙ্গ;
    পক্ষ TK_IS:      zBinOp = "IS";     ভঙ্গ;
    পক্ষ TK_ISNOT:   zBinOp = "ISNOT";  ভঙ্গ;
    পক্ষ TK_AND:     zBinOp = "AND";    ভঙ্গ;
    পক্ষ TK_OR:      zBinOp = "OR";     ভঙ্গ;
    পক্ষ TK_PLUS:    zBinOp = "ADD";    ভঙ্গ;
    পক্ষ TK_STAR:    zBinOp = "MUL";    ভঙ্গ;
    পক্ষ TK_MINUS:   zBinOp = "SUB";    ভঙ্গ;
    পক্ষ TK_REM:     zBinOp = "REM";    ভঙ্গ;
    পক্ষ TK_BITAND:  zBinOp = "BITAND"; ভঙ্গ;
    পক্ষ TK_BITOR:   zBinOp = "BITOR";  ভঙ্গ;
    পক্ষ TK_SLASH:   zBinOp = "DIV";    ভঙ্গ;
    পক্ষ TK_LSHIFT:  zBinOp = "LSHIFT"; ভঙ্গ;
    পক্ষ TK_RSHIFT:  zBinOp = "RSHIFT"; ভঙ্গ;
    পক্ষ TK_CONCAT:  zBinOp = "CONCAT"; ভঙ্গ;
    পক্ষ TK_DOT:     zBinOp = "DOT";    ভঙ্গ;
    পক্ষ TK_LIMIT:   zBinOp = "LIMIT";  ভঙ্গ;

    পক্ষ TK_UMINUS:  zUniOp = "UMINUS"; ভঙ্গ;
    পক্ষ TK_UPLUS:   zUniOp = "UPLUS";  ভঙ্গ;
    পক্ষ TK_BITNOT:  zUniOp = "BITNOT"; ভঙ্গ;
    পক্ষ TK_NOT:     zUniOp = "NOT";    ভঙ্গ;
    পক্ষ TK_ISNULL:  zUniOp = "ISNULL"; ভঙ্গ;
    পক্ষ TK_NOTNULL: zUniOp = "NOTNULL"; ভঙ্গ;

    পক্ষ TK_TRUTH: {
      পূর্ণাঙ্ক x;
      ধ্রুৱ ৱর্ণ *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      প্রতিজ্ঞা( p_ৰাশিমালা->op2==TK_IS || p_ৰাশিমালা->op2==TK_ISNOT );
      প্রতিজ্ঞা( p_ৰাশিমালা->p_ডান );
      প্রতিজ্ঞা( sqlite3_ৰাশিমালা_Skip_Collate_And_Likely(p_ৰাশিমালা->p_ডান)->op
                  == TK_TRUEFALSE );
      x = (p_ৰাশিমালা->op2==TK_ISNOT)*2 + sqlite3_ৰাশিমালা_Truth_মান(p_ৰাশিমালা->p_ডান);
      zUniOp = azOp[x];
      ভঙ্গ;
    }

    পক্ষ TK_SPAN: {
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সাৰি(pView, "SPAN %Q", p_ৰাশিমালা->u.zToken);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 0);
      ভঙ্গ;
    }

    পক্ষ TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সাৰি(pView, "%sCOLLATE %Q%s",
        !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Collate) ? "SOFT-" : "",
        p_ৰাশিমালা->u.zToken, zFlgs);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 0);
      ভঙ্গ;
    }

    পক্ষ TK_AGG_FUNCTION:
    পক্ষ TK_FUNCTION: {
      ৰাশিমালা_List *pFarg;       /* List of function arguments */
      Window *pWin;
      যদি( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }অন্যথা{
        প্রতিজ্ঞা( ৰাশিমালা_Use_XList(p_ৰাশিমালা) );
        pFarg = p_ৰাশিমালা->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_ৰাশিমালা) ? p_ৰাশিমালা->y.pWin : 0;
#অন্যথা
        pWin = 0;
#endif 
      }
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      যদি( p_ৰাশিমালা->op==TK_AGG_FUNCTION ){
        sqlite3_বৃক্ষ_View_সাৰি(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_ৰাশিমালা->op2, p_ৰাশিমালা->u.zToken, zFlgs,
                             p_ৰাশিমালা->pAggInfo ? p_ৰাশিমালা->pAggInfo->selId : 0,
                             p_ৰাশিমালা->iAgg, p_ৰাশিমালা->pAggInfo);
      }অন্যথা যদি( p_ৰাশিমালা->op2!=0 ){
        ধ্রুৱ ৱর্ণ *zOp2;
        ৱর্ণ zBuf[8];
        sqlite3_snprintf(পরিমাণ(zBuf),zBuf,"0x%02x",p_ৰাশিমালা->op2);
        zOp2 = zBuf;
        যদি( p_ৰাশিমালা->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        যদি( p_ৰাশিমালা->op2==NC_Idx_ৰাশিমালা ) zOp2 = "NC_IdxExpr";
        যদি( p_ৰাশিমালা->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        যদি( p_ৰাশিমালা->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_বৃক্ষ_View_সাৰি(pView, "FUNCTION %Q%s op2=%s",
                            p_ৰাশিমালা->u.zToken, zFlgs, zOp2);
      }অন্যথা{
        sqlite3_বৃক্ষ_View_সাৰি(pView, "FUNCTION %Q%s", p_ৰাশিমালা->u.zToken, zFlgs);
      }
      যদি( pFarg ){
        sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pFarg, pWin!=0 || p_ৰাশিমালা->p_বাম, 0);
        যদি( p_ৰাশিমালা->p_বাম ){
          ৰাশিমালা *pOB = p_ৰাশিমালা->p_বাম;
          প্রতিজ্ঞা( pOB->op==TK_ORDER );
          প্রতিজ্ঞা( ৰাশিমালা_Use_XList(pOB) );
          sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      যদি( pWin ){
        sqlite3_বৃক্ষ_View_Window(pView, pWin, 0);
      }
#endif
      ভঙ্গ;
    }
    পক্ষ TK_ORDER: {
      sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, p_ৰাশিমালা->x.pList, 0, "ORDERBY");
      ভঙ্গ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    পক্ষ TK_EXISTS: {
      প্রতিজ্ঞা( ৰাশিমালা_Use_XSelect(p_ৰাশিমালা) );
      sqlite3_বৃক্ষ_View_সাৰি(pView, "EXISTS-expr flags=0x%x", p_ৰাশিমালা->flags);
      sqlite3_বৃক্ষ_View_Select(pView, p_ৰাশিমালা->x.pSelect, 0);
      ভঙ্গ;
    }
    পক্ষ TK_SELECT: {
      প্রতিজ্ঞা( ৰাশিমালা_Use_XSelect(p_ৰাশিমালা) );
      sqlite3_বৃক্ষ_View_সাৰি(pView, "subquery-expr flags=0x%x", p_ৰাশিমালা->flags);
      sqlite3_বৃক্ষ_View_Select(pView, p_ৰাশিমালা->x.pSelect, 0);
      ভঙ্গ;
    }
    পক্ষ TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      ৱর্ণ *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_ৰাশিমালা->flags);
      যদি( p_ৰাশিমালা->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_ৰাশিমালা->iTable);
      যদি( ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_ৰাশিমালা->y.sub.regReturn, p_ৰাশিমালা->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_বৃক্ষ_View_সাৰি(pView, z);
      sqlite3_free(z);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 1);
      যদি( ৰাশিমালা_Use_XSelect(p_ৰাশিমালা) ){
        sqlite3_বৃক্ষ_View_Select(pView, p_ৰাশিমালা->x.pSelect, 0);
      }অন্যথা{
        sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, p_ৰাশিমালা->x.pList, 0, 0);
      }
      ভঙ্গ;
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
    পক্ষ TK_BETWEEN: {
      ধ্রুৱ ৰাশিমালা *pX, *pY, *pZ;
      pX = p_ৰাশিমালা->p_বাম;
      প্রতিজ্ঞা( ৰাশিমালা_Use_XList(p_ৰাশিমালা) );
      প্রতিজ্ঞা( p_ৰাশিমালা->x.pList->n_ৰাশিমালা==2 );
      pY = p_ৰাশিমালা->x.pList->a[0].p_ৰাশিমালা;
      pZ = p_ৰাশিমালা->x.pList->a[1].p_ৰাশিমালা;
      sqlite3_বৃক্ষ_View_সাৰি(pView, "BETWEEN%s", zFlgs);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pX, 1);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pY, 1);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pZ, 0);
      ভঙ্গ;
    }
    পক্ষ TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_বৃক্ষ_View_সাৰি(pView, "%s(%d)", 
          p_ৰাশিমালা->iTable ? "NEW" : "OLD", p_ৰাশিমালা->iColumn);
      ভঙ্গ;
    }
    পক্ষ TK_CASE: {
      sqlite3_বৃক্ষ_View_সাৰি(pView, "CASE");
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 1);
      প্রতিজ্ঞা( ৰাশিমালা_Use_XList(p_ৰাশিমালা) );
      sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, p_ৰাশিমালা->x.pList, 0, 0);
      ভঙ্গ;
    }
#ifndef SQLITE_OMIT_TRIGGER
    পক্ষ TK_RAISE: {
      ধ্রুৱ ৱর্ণ *zType = "unk";
      ৱিকল্প( p_ৰাশিমালা->aff_ৰাশিমালা ){
        পক্ষ OE_Rollback:   zType = "rollback";  ভঙ্গ;
        পক্ষ OE_Abort:      zType = "abort";     ভঙ্গ;
        পক্ষ OE_Fail:       zType = "fail";      ভঙ্গ;
        পক্ষ OE_Ignore:     zType = "ignore";    ভঙ্গ;
      }
      প্রতিজ্ঞা( !ৰাশিমালা_Has_Property(p_ৰাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সাৰি(pView, "RAISE %s", zType);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 0);
      ভঙ্গ;
    }
#endif
    পক্ষ TK_MATCH: {
      sqlite3_বৃক্ষ_View_সাৰি(pView, "MATCH {%d:%d}%s",
                          p_ৰাশিমালা->iTable, p_ৰাশিমালা->iColumn, zFlgs);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_ডান, 0);
      ভঙ্গ;
    }
    পক্ষ TK_VECTOR: {
      ৱর্ণ *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      প্রতিজ্ঞা( ৰাশিমালা_Use_XList(p_ৰাশিমালা) );
      sqlite3_বৃক্ষ_View_Bare_ৰাশিমালা_List(pView, p_ৰাশিমালা->x.pList, z);
      sqlite3_free(z);
      ভঙ্গ;
    }
    পক্ষ TK_SELECT_COLUMN: {
      sqlite3_বৃক্ষ_View_সাৰি(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_ৰাশিমালা->iColumn, p_ৰাশিমালা->iTable-1,
              p_ৰাশিমালা->p_ডান==p_ৰাশিমালা->p_বাম ? " (SELECT-owner)" : "");
      প্রতিজ্ঞা( ৰাশিমালা_Use_XSelect(p_ৰাশিমালা->p_বাম) );
      sqlite3_বৃক্ষ_View_Select(pView, p_ৰাশিমালা->p_বাম->x.pSelect, 0);
      ভঙ্গ;
    }
    পক্ষ TK_IF_NULL_ROW: {
      sqlite3_বৃক্ষ_View_সাৰি(pView, "IF-NULL-ROW %d", p_ৰাশিমালা->iTable);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 0);
      ভঙ্গ;
    }
    পক্ষ TK_ERROR: {
      ৰাশিমালা tmp;
      sqlite3_বৃক্ষ_View_সাৰি(pView, "ERROR");
      tmp = *p_ৰাশিমালা;
      tmp.op = p_ৰাশিমালা->op2;
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, &tmp, 0);
      ভঙ্গ;
    }
    পক্ষ TK_ROW: {
      যদি( p_ৰাশিমালা->iColumn<=0 ){
        sqlite3_বৃক্ষ_View_সাৰি(pView, "First FROM table rowid");
      }অন্যথা{
        sqlite3_বৃক্ষ_View_সাৰি(pView, "First FROM table column %d",
            p_ৰাশিমালা->iColumn-1);
      }
      ভঙ্গ;
    }
    স্ৱভাৱ: {
      sqlite3_বৃক্ষ_View_সাৰি(pView, "op=%d", p_ৰাশিমালা->op);
      ভঙ্গ;
    }
  }
  যদি( zBinOp ){
    sqlite3_বৃক্ষ_View_সাৰি(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 1);
    sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_ডান, 0);
  }অন্যথা যদি( zUniOp ){
    sqlite3_বৃক্ষ_View_সাৰি(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, p_ৰাশিমালা->p_বাম, 0);
  }
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
শূন্য sqlite3_বৃক্ষ_View_Bare_ৰাশিমালা_List(
  বৃক্ষ_View *pView,
  ধ্রুৱ ৰাশিমালা_List *pList,
  ধ্রুৱ ৱর্ণ *zLabel
){
  যদি( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  যদি( pList==0 ){
    sqlite3_বৃক্ষ_View_সাৰি(pView, "%s (empty)", zLabel);
  }অন্যথা{
    পূর্ণাঙ্ক i;
    sqlite3_বৃক্ষ_View_সাৰি(pView, "%s", zLabel);
    বাবে(i=0; i<pList->n_ৰাশিমালা; i++){
      পূর্ণাঙ্ক j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      ৱর্ণ *zName = pList->a[i].zEName;
      পূর্ণাঙ্ক moreToFollow = i<pList->n_ৰাশিমালা - 1;
      যদি( j || zName || sortFlags ){
        sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_বৃক্ষ_View_সাৰি(pView, 0);
        যদি( zName ){
          ৱিকল্প( pList->a[i].fg.eEName ){
            স্ৱভাৱ:
              fprintf(stdout, "AS %s ", zName);
              ভঙ্গ;
            পক্ষ ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              যদি( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              যদি( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              যদি( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              ভঙ্গ;
            পক্ষ ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              ভঙ্গ;
          }
        }
        যদি( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        যদি( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }অন্যথা যদি( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pList->a[i].p_ৰাশিমালা, moreToFollow);
      যদি( j || zName || sortFlags ){
        sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
      }
    }
  }
}
শূন্য sqlite3_বৃক্ষ_View_ৰাশিমালা_List(
  বৃক্ষ_View *pView,
  ধ্রুৱ ৰাশিমালা_List *pList,
  u8 moreToFollow,
  ধ্রুৱ ৱর্ণ *zLabel
){
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  sqlite3_বৃক্ষ_View_Bare_ৰাশিমালা_List(pView, pList, zLabel);
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
শূন্য sqlite3_বৃক্ষ_View_Bare_Id_List(
  বৃক্ষ_View *pView,
  ধ্রুৱ IdList *pList,
  ধ্রুৱ ৱর্ণ *zLabel
){
  যদি( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  যদি( pList==0 ){
    sqlite3_বৃক্ষ_View_সাৰি(pView, "%s (empty)", zLabel);
  }অন্যথা{
    পূর্ণাঙ্ক i;
    sqlite3_বৃক্ষ_View_সাৰি(pView, "%s", zLabel);
    বাবে(i=0; i<pList->nId; i++){
      ৱর্ণ *zName = pList->a[i].zName;
      পূর্ণাঙ্ক moreToFollow = i<pList->nId - 1;
      যদি( zName==0 ) zName = "(null)";
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
      sqlite3_বৃক্ষ_View_সাৰি(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
  }
}
শূন্য sqlite3_বৃক্ষ_View_Id_List(
  বৃক্ষ_View *pView,
  ধ্রুৱ IdList *pList,
  u8 moreToFollow,
  ধ্রুৱ ৱর্ণ *zLabel
){
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  sqlite3_বৃক্ষ_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
শূন্য sqlite3_বৃক্ষ_View_Upsert(
  বৃক্ষ_View *pView,
  ধ্রুৱ Upsert *pUpsert,
  u8 moreToFollow
){
  যদি( pUpsert==0 ) ঘূৰাই;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  যেতিয়ালৈ( pUpsert ){
    পূর্ণাঙ্ক n;
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    যদি( pUpsert->pUpsertWhere ){
      sqlite3_বৃক্ষ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    }
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}

#যদি TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
শূন্য sqlite3_বৃক্ষ_View_Delete(
  ধ্রুৱ With *pWith,
  ধ্রুৱ SrcList *pTabList,
  ধ্রুৱ ৰাশিমালা *pWhere,
  ধ্রুৱ ৰাশিমালা_List *pOrderBy,
  ধ্রুৱ ৰাশিমালা *pLimit,
  ধ্রুৱ Trigger *pTrigger
){
  পূর্ণাঙ্ক n = 0;
  বৃক্ষ_View *pView = 0;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
  sqlite3_বৃক্ষ_View_সাৰি(pView, "DELETE");
  যদি( pWith ) n++;
  যদি( pTabList ) n++;
  যদি( pWhere ) n++;
  যদি( pOrderBy ) n++;
  যদি( pLimit ) n++;
  যদি( pTrigger ) n++;
  যদি( pWith ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_With(pView, pWith, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pTabList ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "FROM");
    sqlite3_বৃক্ষ_View_Src_List(pView, pTabList);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pWhere ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "WHERE");
    sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pWhere, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pOrderBy ){
    sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  যদি( pLimit ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "LIMIT");
    sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pLimit, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pTrigger ){
    sqlite3_বৃক্ষ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}
#endif /* TREETRACE_ENABLED */

#যদি TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
শূন্য sqlite3_বৃক্ষ_View_ঢোকাও(
  ধ্রুৱ With *pWith,
  ধ্রুৱ SrcList *pTabList,
  ধ্রুৱ IdList *pColumnList,
  ধ্রুৱ Select *pSelect,
  ধ্রুৱ ৰাশিমালা_List *p_ৰাশিমালা_List,
  পূর্ণাঙ্ক onError,
  ধ্রুৱ Upsert *pUpsert,
  ধ্রুৱ Trigger *pTrigger
){
  বৃক্ষ_View *pView = 0;
  পূর্ণাঙ্ক n = 0;
  ধ্রুৱ ৱর্ণ *zLabel = "INSERT";
  ৱিকল্প( onError ){
    পক্ষ OE_Replace:  zLabel = "REPLACE";             ভঙ্গ;
    পক্ষ OE_Ignore:   zLabel = "INSERT OR IGNORE";    ভঙ্গ;
    পক্ষ OE_Rollback: zLabel = "INSERT OR ROLLBACK";  ভঙ্গ;
    পক্ষ OE_Abort:    zLabel = "INSERT OR ABORT";     ভঙ্গ;
    পক্ষ OE_Fail:     zLabel = "INSERT OR FAIL";      ভঙ্গ;
  }
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
  sqlite3_বৃক্ষ_View_সাৰি(pView, zLabel);
  যদি( pWith ) n++;
  যদি( pTabList ) n++;
  যদি( pColumnList ) n++;
  যদি( pSelect ) n++;
  যদি( p_ৰাশিমালা_List ) n++;
  যদি( pUpsert ) n++;
  যদি( pTrigger ) n++;
  যদি( pWith ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_With(pView, pWith, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pTabList ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "INTO");
    sqlite3_বৃক্ষ_View_Src_List(pView, pTabList);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pColumnList ){
    sqlite3_বৃক্ষ_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  যদি( pSelect ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "DATA-SOURCE");
    sqlite3_বৃক্ষ_View_Select(pView, pSelect, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( p_ৰাশিমালা_List ){
    sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, p_ৰাশিমালা_List, (--n)>0, "VALUES");
  }
  যদি( pUpsert ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "UPSERT");
    sqlite3_বৃক্ষ_View_Upsert(pView, pUpsert, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pTrigger ){
    sqlite3_বৃক্ষ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}
#endif /* TREETRACE_ENABLED */

#যদি TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
শূন্য sqlite3_বৃক্ষ_View_Update(
  ধ্রুৱ With *pWith,
  ধ্রুৱ SrcList *pTabList,
  ধ্রুৱ ৰাশিমালা_List *pChanges,
  ধ্রুৱ ৰাশিমালা *pWhere,
  পূর্ণাঙ্ক onError,
  ধ্রুৱ ৰাশিমালা_List *pOrderBy,
  ধ্রুৱ ৰাশিমালা *pLimit,
  ধ্রুৱ Upsert *pUpsert,
  ধ্রুৱ Trigger *pTrigger
){
  পূর্ণাঙ্ক n = 0;
  বৃক্ষ_View *pView = 0;
  ধ্রুৱ ৱর্ণ *zLabel = "UPDATE";
  ৱিকল্প( onError ){
    পক্ষ OE_Replace:  zLabel = "UPDATE OR REPLACE";   ভঙ্গ;
    পক্ষ OE_Ignore:   zLabel = "UPDATE OR IGNORE";    ভঙ্গ;
    পক্ষ OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  ভঙ্গ;
    পক্ষ OE_Abort:    zLabel = "UPDATE OR ABORT";     ভঙ্গ;
    পক্ষ OE_Fail:     zLabel = "UPDATE OR FAIL";      ভঙ্গ;
  }
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
  sqlite3_বৃক্ষ_View_সাৰি(pView, zLabel);
  যদি( pWith ) n++;
  যদি( pTabList ) n++;
  যদি( pChanges ) n++;
  যদি( pWhere ) n++;
  যদি( pOrderBy ) n++;
  যদি( pLimit ) n++;
  যদি( pUpsert ) n++;
  যদি( pTrigger ) n++;
  যদি( pWith ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_With(pView, pWith, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pTabList ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "FROM");
    sqlite3_বৃক্ষ_View_Src_List(pView, pTabList);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pChanges ){
    sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pChanges, (--n)>0, "SET");
  }
  যদি( pWhere ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "WHERE");
    sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pWhere, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pOrderBy ){
    sqlite3_বৃক্ষ_View_ৰাশিমালা_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  যদি( pLimit ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "LIMIT");
    sqlite3_বৃক্ষ_View_ৰাশিমালা(pView, pLimit, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pUpsert ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সাৰি(pView, "UPSERT");
    sqlite3_বৃক্ষ_View_Upsert(pView, pUpsert, 0);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }
  যদি( pTrigger ){
    sqlite3_বৃক্ষ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
শূন্য sqlite3_বৃক্ষ_View_Trigger_Step(
  বৃক্ষ_View *pView,
  ধ্রুৱ TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  পূর্ণাঙ্ক cnt = 0;
  যদি( pStep==0 ) ঘূৰাই;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  কৰা{
    যদি( cnt++ && pStep->pNext==0 ){
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
    }
    sqlite3_বৃক্ষ_View_সাৰি(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }যেতিয়ালৈ( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
শূন্য sqlite3_বৃক্ষ_View_Trigger(
  বৃক্ষ_View *pView,
  ধ্রুৱ Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  পূর্ণাঙ্ক cnt = 0;
  যদি( pTrigger==0 ) ঘূৰাই;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  কৰা{
    যদি( cnt++ && pTrigger->pNext==0 ){
      sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
    }
    sqlite3_বৃক্ষ_View_সাৰি(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
    sqlite3_বৃক্ষ_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
  }যেতিয়ালৈ( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
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
শূন্য sqlite3_Show_ৰাশিমালা(ধ্রুৱ ৰাশিমালা *p){ sqlite3_বৃক্ষ_View_ৰাশিমালা(0,p,0); }
শূন্য sqlite3_Show_ৰাশিমালা_List(ধ্রুৱ ৰাশিমালা_List *p){ sqlite3_বৃক্ষ_View_ৰাশিমালা_List(0,p,0,0);}
শূন্য sqlite3ShowIdList(ধ্রুৱ IdList *p){ sqlite3_বৃক্ষ_View_Id_List(0,p,0,0); }
শূন্য sqlite3ShowSrcList(ধ্রুৱ SrcList *p){
  বৃক্ষ_View *pView = 0;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
  sqlite3_বৃক্ষ_View_সাৰি(pView, "SRCLIST");
  sqlite3_বৃক্ষ_View_Src_List(pView,p);
  sqlite3_বৃক্ষ_View_বেৰ_কৰো(&pView);
}
শূন্য sqlite3ShowSelect(ধ্রুৱ Select *p){ sqlite3_বৃক্ষ_View_Select(0,p,0); }
শূন্য sqlite3ShowWith(ধ্রুৱ With *p){ sqlite3_বৃক্ষ_View_With(0,p,0); }
শূন্য sqlite3ShowUpsert(ধ্রুৱ Upsert *p){ sqlite3_বৃক্ষ_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
শূন্য sqlite3ShowTriggerStep(ধ্রুৱ TriggerStep *p){
  sqlite3_বৃক্ষ_View_Trigger_Step(0,p,0,0);
}
শূন্য sqlite3ShowTriggerStepList(ধ্রুৱ TriggerStep *p){
  sqlite3_বৃক্ষ_View_Trigger_Step(0,p,0,1);
}
শূন্য sqlite3ShowTrigger(ধ্রুৱ Trigger *p){ sqlite3_বৃক্ষ_View_Trigger(0,p,0,0); }
শূন্য sqlite3ShowTriggerList(ধ্রুৱ Trigger *p){ sqlite3_বৃক্ষ_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
শূন্য sqlite3ShowWindow(ধ্রুৱ Window *p){ sqlite3_বৃক্ষ_View_Window(0,p,0); }
শূন্য sqlite3ShowWinFunc(ধ্রুৱ Window *p){ sqlite3_বৃক্ষ_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
