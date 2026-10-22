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
#অন্তর্ভাব "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
স্থির শূন্য sqlite3_বৃক্ষ_View_ঠেলো(বৃক্ষ_View **pp, u8 moreToFollow){
  বৃক্ষ_View *p = *pp;
  যদি( p==0 ){
    *pp = p = sqlite3_malloc64( আকার(*p) );
    যদি( p==0 ) ফেরত;
    memset(p, 0, আকার(*p));
  }অন্যথা{
    p->iLevel++;
  }
  প্রতিজ্ঞা( moreToFollow==0 || moreToFollow==1 );
  যদি( p->iLevel<(পূর্ণসংখ্যা)আকার(p->b_সারি) ) p->b_সারি[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
স্থির শূন্য sqlite3_বৃক্ষ_View_বের_করো(বৃক্ষ_View **pp){
  বৃক্ষ_View *p = *pp;
  যদি( p==0 ) ফেরত;
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
শূন্য sqlite3_বৃক্ষ_View_সারি(বৃক্ষ_View *p, ধ্রুবক অক্ষর *zFormat, ...){
  va_list ap;
  পূর্ণসংখ্যা i;
  StrAccum acc;
  অক্ষর zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, আকার(zBuf), 0);
  যদি( p ){
    জন্য(i=0; i<p->iLevel && i<(পূর্ণসংখ্যা)আকার(p->b_সারি)-1; i++){
      sqlite3_str_append(&acc, p->b_সারি[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_সারি[i] ? "|-- " : "'-- ", 4);
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
স্থির শূন্য sqlite3_বৃক্ষ_View_Item(বৃক্ষ_View *p, ধ্রুবক অক্ষর *zLabel,u8 moreFollows){
  sqlite3_বৃক্ষ_View_ঠেলো(&p, moreFollows);
  sqlite3_বৃক্ষ_View_সারি(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
শূন্য sqlite3_বৃক্ষ_View_Column_List(
  বৃক্ষ_View *pView,
  ধ্রুবক Column *aCol,
  পূর্ণসংখ্যা nCol,
  u8 moreToFollow
){
  পূর্ণসংখ্যা i;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  sqlite3_বৃক্ষ_View_সারি(pView, "COLUMNS");
  জন্য(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    পূর্ণসংখ্যা colMoreToFollow = i<(nCol - 1);
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, colMoreToFollow);
    sqlite3_বৃক্ষ_View_সারি(pView, 0);
    printf(" %s", aCol[i].zCnName);
    নির্বাচন( aCol[i].eCType ){
      ক্ষেত্র COLTYPE_ANY:      printf(" ANY");        বিরতি;
      ক্ষেত্র COLTYPE_BLOB:     printf(" BLOB");       বিরতি;
      ক্ষেত্র COLTYPE_INT:      printf(" INT");        বিরতি;
      ক্ষেত্র COLTYPE_INTEGER:  printf(" INTEGER");    বিরতি;
      ক্ষেত্র COLTYPE_REAL:     printf(" REAL");       বিরতি;
      ক্ষেত্র COLTYPE_TEXT:     printf(" TEXT");       বিরতি;
      ক্ষেত্র COLTYPE_CUSTOM: {
        যদি( flg & COLFLAG_HASTYPE ){
          ধ্রুবক অক্ষর *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          বিরতি;
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
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
শূন্য sqlite3_বৃক্ষ_View_With(বৃক্ষ_View *pView, ধ্রুবক With *pWith, u8 moreToFollow){
  পূর্ণসংখ্যা i;
  যদি( pWith==0 ) ফেরত;
  যদি( pWith->nCte==0 ) ফেরত;
  যদি( pWith->pOuter ){
    sqlite3_বৃক্ষ_View_সারি(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }অন্যথা{
    sqlite3_বৃক্ষ_View_সারি(pView, "WITH (0x%p)", pWith);
  }
  যদি( pWith->nCte>0 ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
    জন্য(i=0; i<pWith->nCte; i++){
      StrAccum x;
      অক্ষর z_সারি[1000];
      ধ্রুবক কাঠামো Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_সারি, আকার(z_সারি), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      যদি( pCte->pCols && pCte->pCols->n_রাশিমালা>0 ){
        অক্ষর cSep = '(';
        পূর্ণসংখ্যা j;
        জন্য(j=0; j<pCte->pCols->n_রাশিমালা; j++){
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
      sqlite3_বৃক্ষ_View_Item(pView, z_সারি, i<pWith->nCte-1);
      sqlite3_বৃক্ষ_View_Select(pView, pCte->pSelect, 0);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
শূন্য sqlite3_বৃক্ষ_View_Src_List(বৃক্ষ_View *pView, ধ্রুবক SrcList *pSrc){
  পূর্ণসংখ্যা i;
  যদি( pSrc==0 ) ফেরত;
  জন্য(i=0; i<pSrc->nSrc; i++){
    ধ্রুবক SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    পূর্ণসংখ্যা n = 0;
    অক্ষর z_সারি[1000];
    sqlite3StrAccumInit(&x, 0, z_সারি, আকার(z_সারি), 0);
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
      স্থির ধ্রুবক অক্ষর *aMat[] = {",MAT", "", ",NO-MAT"};
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
    sqlite3_বৃক্ষ_View_Item(pView, z_সারি, i<pSrc->nSrc-1);
    n = 0;
    যদি( pItem->fg.isSubquery ) n++;
    যদি( pItem->fg.isTabFunc ) n++;
    যদি( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    যদি( pItem->fg.isUsing ){
      sqlite3_বৃক্ষ_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }অন্যথা যদি( pItem->u3.pOn!=0 ){
      sqlite3_বৃক্ষ_View_Item(pView, "ON", (--n)>0);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, pItem->u3.pOn, 0);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
    যদি( pItem->fg.isSubquery ){
      প্রতিজ্ঞা( n==1 );
      যদি( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_বৃক্ষ_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      প্রতিজ্ঞা( (পূর্ণসংখ্যা)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_বৃক্ষ_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    যদি( pItem->fg.isTabFunc ){
      sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
শূন্য sqlite3_বৃক্ষ_View_Select(বৃক্ষ_View *pView, ধ্রুবক Select *p, u8 moreToFollow){
  পূর্ণসংখ্যা n = 0;
  পূর্ণসংখ্যা cnt = 0;
  যদি( p==0 ){
    sqlite3_বৃক্ষ_View_সারি(pView, "nil-SELECT");
    ফেরত;
  } 
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  যদি( p->pWith ){
    sqlite3_বৃক্ষ_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, 1);
  }
  করো{
    যদি( p->selFlags & SF_WhereBegin ){
      sqlite3_বৃক্ষ_View_সারি(pView, "sqlite3WhereBegin()");
    }অন্যথা{
      sqlite3_বৃক্ষ_View_সারি(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (পূর্ণসংখ্যা)p->nSelectRow
      );
    }
    যদি( cnt++ ) sqlite3_বৃক্ষ_View_বের_করো(&pView);
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
      sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    যদি( p->pWin ){
      Window *pX;
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, (n--)>0);
      sqlite3_বৃক্ষ_View_সারি(pView, "window-functions");
      জন্য(pX=p->pWin; pX; pX=pX->p_পরবর্তী_Win){
        sqlite3_বৃক্ষ_View_Win_Func(pView, pX, pX->p_পরবর্তী_Win!=0);
      }
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
#endif
    যদি( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, (n--)>0);
      sqlite3_বৃক্ষ_View_সারি(pView, "FROM");
      sqlite3_বৃক্ষ_View_Src_List(pView, p->pSrc);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
    যদি( p->pWhere ){
      sqlite3_বৃক্ষ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p->pWhere, 0);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
    যদি( p->pGroupBy ){
      sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    যদি( p->pHaving ){
      sqlite3_বৃক্ষ_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p->pHaving, 0);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    যদি( p->pWinDefn ){
      Window *pX;
      sqlite3_বৃক্ষ_View_Item(pView, "WINDOW", (n--)>0);
      জন্য(pX=p->pWinDefn; pX; pX=pX->p_পরবর্তী_Win){
        sqlite3_বৃক্ষ_View_Window(pView, pX, pX->p_পরবর্তী_Win!=0);
      }
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
#endif
    যদি( p->pOrderBy ){
      sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    যদি( p->pLimit ){
      sqlite3_বৃক্ষ_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p->pLimit->p_বাম, p->pLimit->p_ডান!=0);
      যদি( p->pLimit->p_ডান ){
        sqlite3_বৃক্ষ_View_Item(pView, "OFFSET", 0);
        sqlite3_বৃক্ষ_View_রাশিমালা(pView, p->pLimit->p_ডান, 0);
        sqlite3_বৃক্ষ_View_বের_করো(&pView);
      }
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
    যদি( p->pPrior ){
      ধ্রুবক অক্ষর *zOp = "UNION";
      নির্বাচন( p->op ){
        ক্ষেত্র TK_ALL:         zOp = "UNION ALL";  বিরতি;
        ক্ষেত্র TK_INTERSECT:   zOp = "INTERSECT";  বিরতি;
        ক্ষেত্র TK_EXCEPT:      zOp = "EXCEPT";     বিরতি;
      }
      sqlite3_বৃক্ষ_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }যখন( p!=0 );
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
শূন্য sqlite3_বৃক্ষ_View_Bound(
  বৃক্ষ_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  রাশিমালা *p_রাশিমালা,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  নির্বাচন( eBound ){
    ক্ষেত্র TK_UNBOUNDED: {
      sqlite3_বৃক্ষ_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
      বিরতি;
    }
    ক্ষেত্র TK_CURRENT: {
      sqlite3_বৃক্ষ_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
      বিরতি;
    }
    ক্ষেত্র TK_PRECEDING: {
      sqlite3_বৃক্ষ_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা, 0);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
      বিরতি;
    }
    ক্ষেত্র TK_FOLLOWING: {
      sqlite3_বৃক্ষ_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা, 0);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
      বিরতি;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
শূন্য sqlite3_বৃক্ষ_View_Window(বৃক্ষ_View *pView, ধ্রুবক Window *pWin, u8 more){
  পূর্ণসংখ্যা nElement = 0;
  যদি( pWin==0 ) ফেরত;
  যদি( pWin->pFilter ){
    sqlite3_বৃক্ষ_View_Item(pView, "FILTER", 1);
    sqlite3_বৃক্ষ_View_রাশিমালা(pView, pWin->pFilter, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
    যদি( pWin->eFrmType==TK_FILTER ) ফেরত;
  }
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, more);
  যদি( pWin->zName ){
    sqlite3_বৃক্ষ_View_সারি(pView, "OVER %s (%p)", pWin->zName, pWin);
  }অন্যথা{
    sqlite3_বৃক্ষ_View_সারি(pView, "OVER (%p)", pWin);
  }
  যদি( pWin->zBase )    nElement++;
  যদি( pWin->pOrderBy ) nElement++;
  যদি( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  যদি( pWin->eExclude ) nElement++;
  যদি( pWin->zBase ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--nElement)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "window: %s", pWin->zBase);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pWin->pPartition ){
    sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  যদি( pWin->pOrderBy ){
    sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  যদি( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    অক্ষর zBuf[30];
    ধ্রুবক অক্ষর *zFrmType = "ROWS";
    যদি( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    যদি( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(আকার(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_বৃক্ষ_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_বৃক্ষ_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_বৃক্ষ_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pWin->eExclude ){
    অক্ষর zBuf[30];
    ধ্রুবক অক্ষর *zExclude;
    নির্বাচন( pWin->eExclude ){
      ক্ষেত্র TK_NO:      zExclude = "NO OTHERS";   বিরতি;
      ক্ষেত্র TK_CURRENT: zExclude = "CURRENT ROW"; বিরতি;
      ক্ষেত্র TK_GROUP:   zExclude = "GROUP";       বিরতি;
      ক্ষেত্র TK_TIES:    zExclude = "TIES";        বিরতি;
      পূর্বনির্ধারিত:
        sqlite3_snprintf(আকার(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        বিরতি;
    }
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
    sqlite3_বৃক্ষ_View_সারি(pView, "EXCLUDE %s", zExclude);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
শূন্য sqlite3_বৃক্ষ_View_Win_Func(বৃক্ষ_View *pView, ধ্রুবক Window *pWin, u8 more){
  যদি( pWin==0 ) ফেরত;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, more);
  sqlite3_বৃক্ষ_View_সারি(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_বৃক্ষ_View_Window(pView, pWin, 0);
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
শূন্য sqlite3_বৃক্ষ_View_রাশিমালা(বৃক্ষ_View *pView, ধ্রুবক রাশিমালা *p_রাশিমালা, u8 moreToFollow){
  ধ্রুবক অক্ষর *zBinOp = 0;   /* Binary operator */
  ধ্রুবক অক্ষর *zUniOp = 0;   /* Unary operator */
  অক্ষর zFlgs[200];
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  যদি( p_রাশিমালা==0 ){
    sqlite3_বৃক্ষ_View_সারি(pView, "nil");
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
    ফেরত;
  }
  যদি( p_রাশিমালা->flags || p_রাশিমালা->aff_রাশিমালা || p_রাশিমালা->vvaFlags || p_রাশিমালা->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, আকার(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_রাশিমালা->flags, p_রাশিমালা->aff_রাশিমালা ? p_রাশিমালা->aff_রাশিমালা : 'n');
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_রাশিমালা->w.iJoin);
    }
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_রাশিমালা->w.iJoin);
    }
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    যদি( রাশিমালা_Has_VVAProperty(p_রাশিমালা, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    যদি( p_রাশিমালা->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_রাশিমালা->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }অন্যথা{
    zFlgs[0] = 0;
  }
  নির্বাচন( p_রাশিমালা->op ){
    ক্ষেত্র TK_AGG_COLUMN: {
      sqlite3_বৃক্ষ_View_সারি(pView, "AGG{%d:%d}%s",
            p_রাশিমালা->iTable, p_রাশিমালা->iColumn, zFlgs);
      বিরতি;
    }
    ক্ষেত্র TK_COLUMN: {
      যদি( p_রাশিমালা->iTable<0 ){
        /* This only happens when coding check constraints */
        অক্ষর zOp2[16];
        যদি( p_রাশিমালা->op2 ){
          sqlite3_snprintf(আকার(zOp2),zOp2," op2=0x%02x",p_রাশিমালা->op2);
        }অন্যথা{
          zOp2[0] = 0;
        }
        sqlite3_বৃক্ষ_View_সারি(pView, "COLUMN(%d)%s%s",
                                    p_রাশিমালা->iColumn, zFlgs, zOp2);
      }অন্যথা{
        প্রতিজ্ঞা( রাশিমালা_Use_YTab(p_রাশিমালা) );
        sqlite3_বৃক্ষ_View_সারি(pView, "{%d:%d} pTab=%p%s",
                        p_রাশিমালা->iTable, p_রাশিমালা->iColumn,
                        p_রাশিমালা->y.pTab, zFlgs);
      }
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_FixedCol) ){
        sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 0);
      }
      বিরতি;
    }
    ক্ষেত্র TK_INTEGER: {
      যদি( p_রাশিমালা->flags & EP_Int_মান ){
        sqlite3_বৃক্ষ_View_সারি(pView, "%d", p_রাশিমালা->u.i_মান);
      }অন্যথা{
        sqlite3_বৃক্ষ_View_সারি(pView, "%s", p_রাশিমালা->u.zToken);
      }
      বিরতি;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    ক্ষেত্র TK_FLOAT: {
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সারি(pView,"%s", p_রাশিমালা->u.zToken);
      বিরতি;
    }
#endif
    ক্ষেত্র TK_STRING: {
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সারি(pView,"%Q", p_রাশিমালা->u.zToken);
      বিরতি;
    }
    ক্ষেত্র TK_NULL: {
      sqlite3_বৃক্ষ_View_সারি(pView,"NULL");
      বিরতি;
    }
    ক্ষেত্র TK_TRUEFALSE: {
      sqlite3_বৃক্ষ_View_সারি(pView,"%s%s",
         sqlite3_রাশিমালা_Truth_মান(p_রাশিমালা) ? "TRUE" : "FALSE", zFlgs);
      বিরতি;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    ক্ষেত্র TK_BLOB: {
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সারি(pView,"%s", p_রাশিমালা->u.zToken);
      বিরতি;
    }
#endif
    ক্ষেত্র TK_VARIABLE: {
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সারি(pView,"VARIABLE(%s,%d)",
                          p_রাশিমালা->u.zToken, p_রাশিমালা->iColumn);
      বিরতি;
    }
    ক্ষেত্র TK_REGISTER: {
      sqlite3_বৃক্ষ_View_সারি(pView,"REGISTER(%d)", p_রাশিমালা->iTable);
      বিরতি;
    }
    ক্ষেত্র TK_ID: {
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সারি(pView,"ID \"%w\"", p_রাশিমালা->u.zToken);
      বিরতি;
    }
    ক্ষেত্র TK_ASTERISK: {
      sqlite3_বৃক্ষ_View_সারি(pView,"ASKTERISK");
      বিরতি;
    }
#ifndef SQLITE_OMIT_CAST
    ক্ষেত্র TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সারি(pView,"CAST %Q", p_রাশিমালা->u.zToken);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 0);
      বিরতি;
    }
#endif /* SQLITE_OMIT_CAST */
    ক্ষেত্র TK_LT:      zBinOp = "LT";     বিরতি;
    ক্ষেত্র TK_LE:      zBinOp = "LE";     বিরতি;
    ক্ষেত্র TK_GT:      zBinOp = "GT";     বিরতি;
    ক্ষেত্র TK_GE:      zBinOp = "GE";     বিরতি;
    ক্ষেত্র TK_NE:      zBinOp = "NE";     বিরতি;
    ক্ষেত্র TK_EQ:      zBinOp = "EQ";     বিরতি;
    ক্ষেত্র TK_IS:      zBinOp = "IS";     বিরতি;
    ক্ষেত্র TK_ISNOT:   zBinOp = "ISNOT";  বিরতি;
    ক্ষেত্র TK_AND:     zBinOp = "AND";    বিরতি;
    ক্ষেত্র TK_OR:      zBinOp = "OR";     বিরতি;
    ক্ষেত্র TK_PLUS:    zBinOp = "ADD";    বিরতি;
    ক্ষেত্র TK_STAR:    zBinOp = "MUL";    বিরতি;
    ক্ষেত্র TK_MINUS:   zBinOp = "SUB";    বিরতি;
    ক্ষেত্র TK_REM:     zBinOp = "REM";    বিরতি;
    ক্ষেত্র TK_BITAND:  zBinOp = "BITAND"; বিরতি;
    ক্ষেত্র TK_BITOR:   zBinOp = "BITOR";  বিরতি;
    ক্ষেত্র TK_SLASH:   zBinOp = "DIV";    বিরতি;
    ক্ষেত্র TK_LSHIFT:  zBinOp = "LSHIFT"; বিরতি;
    ক্ষেত্র TK_RSHIFT:  zBinOp = "RSHIFT"; বিরতি;
    ক্ষেত্র TK_CONCAT:  zBinOp = "CONCAT"; বিরতি;
    ক্ষেত্র TK_DOT:     zBinOp = "DOT";    বিরতি;
    ক্ষেত্র TK_LIMIT:   zBinOp = "LIMIT";  বিরতি;

    ক্ষেত্র TK_UMINUS:  zUniOp = "UMINUS"; বিরতি;
    ক্ষেত্র TK_UPLUS:   zUniOp = "UPLUS";  বিরতি;
    ক্ষেত্র TK_BITNOT:  zUniOp = "BITNOT"; বিরতি;
    ক্ষেত্র TK_NOT:     zUniOp = "NOT";    বিরতি;
    ক্ষেত্র TK_ISNULL:  zUniOp = "ISNULL"; বিরতি;
    ক্ষেত্র TK_NOTNULL: zUniOp = "NOTNULL"; বিরতি;

    ক্ষেত্র TK_TRUTH: {
      পূর্ণসংখ্যা x;
      ধ্রুবক অক্ষর *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      প্রতিজ্ঞা( p_রাশিমালা->op2==TK_IS || p_রাশিমালা->op2==TK_ISNOT );
      প্রতিজ্ঞা( p_রাশিমালা->p_ডান );
      প্রতিজ্ঞা( sqlite3_রাশিমালা_Skip_Collate_And_Likely(p_রাশিমালা->p_ডান)->op
                  == TK_TRUEFALSE );
      x = (p_রাশিমালা->op2==TK_ISNOT)*2 + sqlite3_রাশিমালা_Truth_মান(p_রাশিমালা->p_ডান);
      zUniOp = azOp[x];
      বিরতি;
    }

    ক্ষেত্র TK_SPAN: {
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সারি(pView, "SPAN %Q", p_রাশিমালা->u.zToken);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 0);
      বিরতি;
    }

    ক্ষেত্র TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সারি(pView, "%sCOLLATE %Q%s",
        !রাশিমালা_Has_Property(p_রাশিমালা, EP_Collate) ? "SOFT-" : "",
        p_রাশিমালা->u.zToken, zFlgs);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 0);
      বিরতি;
    }

    ক্ষেত্র TK_AGG_FUNCTION:
    ক্ষেত্র TK_FUNCTION: {
      রাশিমালা_List *pFarg;       /* List of function arguments */
      Window *pWin;
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }অন্যথা{
        প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
        pFarg = p_রাশিমালা->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_রাশিমালা) ? p_রাশিমালা->y.pWin : 0;
#অন্যথা
        pWin = 0;
#endif 
      }
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      যদি( p_রাশিমালা->op==TK_AGG_FUNCTION ){
        sqlite3_বৃক্ষ_View_সারি(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_রাশিমালা->op2, p_রাশিমালা->u.zToken, zFlgs,
                             p_রাশিমালা->pAggInfo ? p_রাশিমালা->pAggInfo->selId : 0,
                             p_রাশিমালা->iAgg, p_রাশিমালা->pAggInfo);
      }অন্যথা যদি( p_রাশিমালা->op2!=0 ){
        ধ্রুবক অক্ষর *zOp2;
        অক্ষর zBuf[8];
        sqlite3_snprintf(আকার(zBuf),zBuf,"0x%02x",p_রাশিমালা->op2);
        zOp2 = zBuf;
        যদি( p_রাশিমালা->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        যদি( p_রাশিমালা->op2==NC_Idx_রাশিমালা ) zOp2 = "NC_IdxExpr";
        যদি( p_রাশিমালা->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        যদি( p_রাশিমালা->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_বৃক্ষ_View_সারি(pView, "FUNCTION %Q%s op2=%s",
                            p_রাশিমালা->u.zToken, zFlgs, zOp2);
      }অন্যথা{
        sqlite3_বৃক্ষ_View_সারি(pView, "FUNCTION %Q%s", p_রাশিমালা->u.zToken, zFlgs);
      }
      যদি( pFarg ){
        sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pFarg, pWin!=0 || p_রাশিমালা->p_বাম, 0);
        যদি( p_রাশিমালা->p_বাম ){
          রাশিমালা *pOB = p_রাশিমালা->p_বাম;
          প্রতিজ্ঞা( pOB->op==TK_ORDER );
          প্রতিজ্ঞা( রাশিমালা_Use_XList(pOB) );
          sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      যদি( pWin ){
        sqlite3_বৃক্ষ_View_Window(pView, pWin, 0);
      }
#endif
      বিরতি;
    }
    ক্ষেত্র TK_ORDER: {
      sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, p_রাশিমালা->x.pList, 0, "ORDERBY");
      বিরতি;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ক্ষেত্র TK_EXISTS: {
      প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p_রাশিমালা) );
      sqlite3_বৃক্ষ_View_সারি(pView, "EXISTS-expr flags=0x%x", p_রাশিমালা->flags);
      sqlite3_বৃক্ষ_View_Select(pView, p_রাশিমালা->x.pSelect, 0);
      বিরতি;
    }
    ক্ষেত্র TK_SELECT: {
      প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p_রাশিমালা) );
      sqlite3_বৃক্ষ_View_সারি(pView, "subquery-expr flags=0x%x", p_রাশিমালা->flags);
      sqlite3_বৃক্ষ_View_Select(pView, p_রাশিমালা->x.pSelect, 0);
      বিরতি;
    }
    ক্ষেত্র TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      অক্ষর *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_রাশিমালা->flags);
      যদি( p_রাশিমালা->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_রাশিমালা->iTable);
      যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_রাশিমালা->y.sub.regReturn, p_রাশিমালা->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_বৃক্ষ_View_সারি(pView, z);
      sqlite3_free(z);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 1);
      যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
        sqlite3_বৃক্ষ_View_Select(pView, p_রাশিমালা->x.pSelect, 0);
      }অন্যথা{
        sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, p_রাশিমালা->x.pList, 0, 0);
      }
      বিরতি;
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
    ক্ষেত্র TK_BETWEEN: {
      ধ্রুবক রাশিমালা *pX, *pY, *pZ;
      pX = p_রাশিমালা->p_বাম;
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
      প্রতিজ্ঞা( p_রাশিমালা->x.pList->n_রাশিমালা==2 );
      pY = p_রাশিমালা->x.pList->a[0].p_রাশিমালা;
      pZ = p_রাশিমালা->x.pList->a[1].p_রাশিমালা;
      sqlite3_বৃক্ষ_View_সারি(pView, "BETWEEN%s", zFlgs);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, pX, 1);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, pY, 1);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, pZ, 0);
      বিরতি;
    }
    ক্ষেত্র TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_বৃক্ষ_View_সারি(pView, "%s(%d)", 
          p_রাশিমালা->iTable ? "NEW" : "OLD", p_রাশিমালা->iColumn);
      বিরতি;
    }
    ক্ষেত্র TK_CASE: {
      sqlite3_বৃক্ষ_View_সারি(pView, "CASE");
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 1);
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
      sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, p_রাশিমালা->x.pList, 0, 0);
      বিরতি;
    }
#ifndef SQLITE_OMIT_TRIGGER
    ক্ষেত্র TK_RAISE: {
      ধ্রুবক অক্ষর *zType = "unk";
      নির্বাচন( p_রাশিমালা->aff_রাশিমালা ){
        ক্ষেত্র OE_Rollback:   zType = "rollback";  বিরতি;
        ক্ষেত্র OE_Abort:      zType = "abort";     বিরতি;
        ক্ষেত্র OE_Fail:       zType = "fail";      বিরতি;
        ক্ষেত্র OE_Ignore:     zType = "ignore";    বিরতি;
      }
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      sqlite3_বৃক্ষ_View_সারি(pView, "RAISE %s", zType);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 0);
      বিরতি;
    }
#endif
    ক্ষেত্র TK_MATCH: {
      sqlite3_বৃক্ষ_View_সারি(pView, "MATCH {%d:%d}%s",
                          p_রাশিমালা->iTable, p_রাশিমালা->iColumn, zFlgs);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_ডান, 0);
      বিরতি;
    }
    ক্ষেত্র TK_VECTOR: {
      অক্ষর *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
      sqlite3_বৃক্ষ_View_Bare_রাশিমালা_List(pView, p_রাশিমালা->x.pList, z);
      sqlite3_free(z);
      বিরতি;
    }
    ক্ষেত্র TK_SELECT_COLUMN: {
      sqlite3_বৃক্ষ_View_সারি(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_রাশিমালা->iColumn, p_রাশিমালা->iTable-1,
              p_রাশিমালা->p_ডান==p_রাশিমালা->p_বাম ? " (SELECT-owner)" : "");
      প্রতিজ্ঞা( রাশিমালা_Use_XSelect(p_রাশিমালা->p_বাম) );
      sqlite3_বৃক্ষ_View_Select(pView, p_রাশিমালা->p_বাম->x.pSelect, 0);
      বিরতি;
    }
    ক্ষেত্র TK_IF_NULL_ROW: {
      sqlite3_বৃক্ষ_View_সারি(pView, "IF-NULL-ROW %d", p_রাশিমালা->iTable);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 0);
      বিরতি;
    }
    ক্ষেত্র TK_ERROR: {
      রাশিমালা tmp;
      sqlite3_বৃক্ষ_View_সারি(pView, "ERROR");
      tmp = *p_রাশিমালা;
      tmp.op = p_রাশিমালা->op2;
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, &tmp, 0);
      বিরতি;
    }
    ক্ষেত্র TK_ROW: {
      যদি( p_রাশিমালা->iColumn<=0 ){
        sqlite3_বৃক্ষ_View_সারি(pView, "First FROM table rowid");
      }অন্যথা{
        sqlite3_বৃক্ষ_View_সারি(pView, "First FROM table column %d",
            p_রাশিমালা->iColumn-1);
      }
      বিরতি;
    }
    পূর্বনির্ধারিত: {
      sqlite3_বৃক্ষ_View_সারি(pView, "op=%d", p_রাশিমালা->op);
      বিরতি;
    }
  }
  যদি( zBinOp ){
    sqlite3_বৃক্ষ_View_সারি(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 1);
    sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_ডান, 0);
  }অন্যথা যদি( zUniOp ){
    sqlite3_বৃক্ষ_View_সারি(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_বৃক্ষ_View_রাশিমালা(pView, p_রাশিমালা->p_বাম, 0);
  }
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
শূন্য sqlite3_বৃক্ষ_View_Bare_রাশিমালা_List(
  বৃক্ষ_View *pView,
  ধ্রুবক রাশিমালা_List *pList,
  ধ্রুবক অক্ষর *zLabel
){
  যদি( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  যদি( pList==0 ){
    sqlite3_বৃক্ষ_View_সারি(pView, "%s (empty)", zLabel);
  }অন্যথা{
    পূর্ণসংখ্যা i;
    sqlite3_বৃক্ষ_View_সারি(pView, "%s", zLabel);
    জন্য(i=0; i<pList->n_রাশিমালা; i++){
      পূর্ণসংখ্যা j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      অক্ষর *zName = pList->a[i].zEName;
      পূর্ণসংখ্যা moreToFollow = i<pList->n_রাশিমালা - 1;
      যদি( j || zName || sortFlags ){
        sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_বৃক্ষ_View_সারি(pView, 0);
        যদি( zName ){
          নির্বাচন( pList->a[i].fg.eEName ){
            পূর্বনির্ধারিত:
              fprintf(stdout, "AS %s ", zName);
              বিরতি;
            ক্ষেত্র ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              যদি( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              যদি( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              যদি( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              বিরতি;
            ক্ষেত্র ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              বিরতি;
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
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, pList->a[i].p_রাশিমালা, moreToFollow);
      যদি( j || zName || sortFlags ){
        sqlite3_বৃক্ষ_View_বের_করো(&pView);
      }
    }
  }
}
শূন্য sqlite3_বৃক্ষ_View_রাশিমালা_List(
  বৃক্ষ_View *pView,
  ধ্রুবক রাশিমালা_List *pList,
  u8 moreToFollow,
  ধ্রুবক অক্ষর *zLabel
){
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  sqlite3_বৃক্ষ_View_Bare_রাশিমালা_List(pView, pList, zLabel);
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
শূন্য sqlite3_বৃক্ষ_View_Bare_Id_List(
  বৃক্ষ_View *pView,
  ধ্রুবক IdList *pList,
  ধ্রুবক অক্ষর *zLabel
){
  যদি( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  যদি( pList==0 ){
    sqlite3_বৃক্ষ_View_সারি(pView, "%s (empty)", zLabel);
  }অন্যথা{
    পূর্ণসংখ্যা i;
    sqlite3_বৃক্ষ_View_সারি(pView, "%s", zLabel);
    জন্য(i=0; i<pList->nId; i++){
      অক্ষর *zName = pList->a[i].zName;
      পূর্ণসংখ্যা moreToFollow = i<pList->nId - 1;
      যদি( zName==0 ) zName = "(null)";
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
      sqlite3_বৃক্ষ_View_সারি(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
  }
}
শূন্য sqlite3_বৃক্ষ_View_Id_List(
  বৃক্ষ_View *pView,
  ধ্রুবক IdList *pList,
  u8 moreToFollow,
  ধ্রুবক অক্ষর *zLabel
){
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  sqlite3_বৃক্ষ_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
শূন্য sqlite3_বৃক্ষ_View_Upsert(
  বৃক্ষ_View *pView,
  ধ্রুবক Upsert *pUpsert,
  u8 moreToFollow
){
  যদি( pUpsert==0 ) ফেরত;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, moreToFollow);
  যখন( pUpsert ){
    পূর্ণসংখ্যা n;
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, pUpsert->p_পরবর্তী_Upsert!=0 || moreToFollow);
    sqlite3_বৃক্ষ_View_সারি(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    যদি( pUpsert->pUpsertWhere ){
      sqlite3_বৃক্ষ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_বৃক্ষ_View_রাশিমালা(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
    }
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
    pUpsert = pUpsert->p_পরবর্তী_Upsert;
  }
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}

#যদি TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
শূন্য sqlite3_বৃক্ষ_View_Delete(
  ধ্রুবক With *pWith,
  ধ্রুবক SrcList *pTabList,
  ধ্রুবক রাশিমালা *pWhere,
  ধ্রুবক রাশিমালা_List *pOrderBy,
  ধ্রুবক রাশিমালা *pLimit,
  ধ্রুবক Trigger *pTrigger
){
  পূর্ণসংখ্যা n = 0;
  বৃক্ষ_View *pView = 0;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
  sqlite3_বৃক্ষ_View_সারি(pView, "DELETE");
  যদি( pWith ) n++;
  যদি( pTabList ) n++;
  যদি( pWhere ) n++;
  যদি( pOrderBy ) n++;
  যদি( pLimit ) n++;
  যদি( pTrigger ) n++;
  যদি( pWith ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_With(pView, pWith, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pTabList ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "FROM");
    sqlite3_বৃক্ষ_View_Src_List(pView, pTabList);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pWhere ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "WHERE");
    sqlite3_বৃক্ষ_View_রাশিমালা(pView, pWhere, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pOrderBy ){
    sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  যদি( pLimit ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "LIMIT");
    sqlite3_বৃক্ষ_View_রাশিমালা(pView, pLimit, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pTrigger ){
    sqlite3_বৃক্ষ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}
#endif /* TREETRACE_ENABLED */

#যদি TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
শূন্য sqlite3_বৃক্ষ_View_ঢোকাও(
  ধ্রুবক With *pWith,
  ধ্রুবক SrcList *pTabList,
  ধ্রুবক IdList *pColumnList,
  ধ্রুবক Select *pSelect,
  ধ্রুবক রাশিমালা_List *p_রাশিমালা_List,
  পূর্ণসংখ্যা onError,
  ধ্রুবক Upsert *pUpsert,
  ধ্রুবক Trigger *pTrigger
){
  বৃক্ষ_View *pView = 0;
  পূর্ণসংখ্যা n = 0;
  ধ্রুবক অক্ষর *zLabel = "INSERT";
  নির্বাচন( onError ){
    ক্ষেত্র OE_Replace:  zLabel = "REPLACE";             বিরতি;
    ক্ষেত্র OE_Ignore:   zLabel = "INSERT OR IGNORE";    বিরতি;
    ক্ষেত্র OE_Rollback: zLabel = "INSERT OR ROLLBACK";  বিরতি;
    ক্ষেত্র OE_Abort:    zLabel = "INSERT OR ABORT";     বিরতি;
    ক্ষেত্র OE_Fail:     zLabel = "INSERT OR FAIL";      বিরতি;
  }
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
  sqlite3_বৃক্ষ_View_সারি(pView, zLabel);
  যদি( pWith ) n++;
  যদি( pTabList ) n++;
  যদি( pColumnList ) n++;
  যদি( pSelect ) n++;
  যদি( p_রাশিমালা_List ) n++;
  যদি( pUpsert ) n++;
  যদি( pTrigger ) n++;
  যদি( pWith ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_With(pView, pWith, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pTabList ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "INTO");
    sqlite3_বৃক্ষ_View_Src_List(pView, pTabList);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pColumnList ){
    sqlite3_বৃক্ষ_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  যদি( pSelect ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "DATA-SOURCE");
    sqlite3_বৃক্ষ_View_Select(pView, pSelect, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( p_রাশিমালা_List ){
    sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, p_রাশিমালা_List, (--n)>0, "VALUES");
  }
  যদি( pUpsert ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "UPSERT");
    sqlite3_বৃক্ষ_View_Upsert(pView, pUpsert, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pTrigger ){
    sqlite3_বৃক্ষ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}
#endif /* TREETRACE_ENABLED */

#যদি TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
শূন্য sqlite3_বৃক্ষ_View_Update(
  ধ্রুবক With *pWith,
  ধ্রুবক SrcList *pTabList,
  ধ্রুবক রাশিমালা_List *pChanges,
  ধ্রুবক রাশিমালা *pWhere,
  পূর্ণসংখ্যা onError,
  ধ্রুবক রাশিমালা_List *pOrderBy,
  ধ্রুবক রাশিমালা *pLimit,
  ধ্রুবক Upsert *pUpsert,
  ধ্রুবক Trigger *pTrigger
){
  পূর্ণসংখ্যা n = 0;
  বৃক্ষ_View *pView = 0;
  ধ্রুবক অক্ষর *zLabel = "UPDATE";
  নির্বাচন( onError ){
    ক্ষেত্র OE_Replace:  zLabel = "UPDATE OR REPLACE";   বিরতি;
    ক্ষেত্র OE_Ignore:   zLabel = "UPDATE OR IGNORE";    বিরতি;
    ক্ষেত্র OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  বিরতি;
    ক্ষেত্র OE_Abort:    zLabel = "UPDATE OR ABORT";     বিরতি;
    ক্ষেত্র OE_Fail:     zLabel = "UPDATE OR FAIL";      বিরতি;
  }
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
  sqlite3_বৃক্ষ_View_সারি(pView, zLabel);
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
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pTabList ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "FROM");
    sqlite3_বৃক্ষ_View_Src_List(pView, pTabList);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pChanges ){
    sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pChanges, (--n)>0, "SET");
  }
  যদি( pWhere ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "WHERE");
    sqlite3_বৃক্ষ_View_রাশিমালা(pView, pWhere, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pOrderBy ){
    sqlite3_বৃক্ষ_View_রাশিমালা_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  যদি( pLimit ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "LIMIT");
    sqlite3_বৃক্ষ_View_রাশিমালা(pView, pLimit, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pUpsert ){
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, (--n)>0);
    sqlite3_বৃক্ষ_View_সারি(pView, "UPSERT");
    sqlite3_বৃক্ষ_View_Upsert(pView, pUpsert, 0);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }
  যদি( pTrigger ){
    sqlite3_বৃক্ষ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
শূন্য sqlite3_বৃক্ষ_View_Trigger_Step(
  বৃক্ষ_View *pView,
  ধ্রুবক TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  পূর্ণসংখ্যা cnt = 0;
  যদি( pStep==0 ) ফেরত;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 
      moreToFollow || (showFullList && pStep->p_পরবর্তী!=0));
  করো{
    যদি( cnt++ && pStep->p_পরবর্তী==0 ){
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
    }
    sqlite3_বৃক্ষ_View_সারি(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }যখন( showFullList && (pStep = pStep->p_পরবর্তী)!=0 );
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
শূন্য sqlite3_বৃক্ষ_View_Trigger(
  বৃক্ষ_View *pView,
  ধ্রুবক Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  পূর্ণসংখ্যা cnt = 0;
  যদি( pTrigger==0 ) ফেরত;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView,
     moreToFollow || (showFullList && pTrigger->p_পরবর্তী!=0));
  করো{
    যদি( cnt++ && pTrigger->p_পরবর্তী==0 ){
      sqlite3_বৃক্ষ_View_বের_করো(&pView);
      sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
    }
    sqlite3_বৃক্ষ_View_সারি(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
    sqlite3_বৃক্ষ_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_বৃক্ষ_View_বের_করো(&pView);
  }যখন( showFullList && (pTrigger = pTrigger->p_পরবর্তী)!=0 );
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
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
শূন্য sqlite3_Show_রাশিমালা(ধ্রুবক রাশিমালা *p){ sqlite3_বৃক্ষ_View_রাশিমালা(0,p,0); }
শূন্য sqlite3_Show_রাশিমালা_List(ধ্রুবক রাশিমালা_List *p){ sqlite3_বৃক্ষ_View_রাশিমালা_List(0,p,0,0);}
শূন্য sqlite3ShowIdList(ধ্রুবক IdList *p){ sqlite3_বৃক্ষ_View_Id_List(0,p,0,0); }
শূন্য sqlite3ShowSrcList(ধ্রুবক SrcList *p){
  বৃক্ষ_View *pView = 0;
  sqlite3_বৃক্ষ_View_ঠেলো(&pView, 0);
  sqlite3_বৃক্ষ_View_সারি(pView, "SRCLIST");
  sqlite3_বৃক্ষ_View_Src_List(pView,p);
  sqlite3_বৃক্ষ_View_বের_করো(&pView);
}
শূন্য sqlite3ShowSelect(ধ্রুবক Select *p){ sqlite3_বৃক্ষ_View_Select(0,p,0); }
শূন্য sqlite3ShowWith(ধ্রুবক With *p){ sqlite3_বৃক্ষ_View_With(0,p,0); }
শূন্য sqlite3ShowUpsert(ধ্রুবক Upsert *p){ sqlite3_বৃক্ষ_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
শূন্য sqlite3ShowTriggerStep(ধ্রুবক TriggerStep *p){
  sqlite3_বৃক্ষ_View_Trigger_Step(0,p,0,0);
}
শূন্য sqlite3ShowTriggerStepList(ধ্রুবক TriggerStep *p){
  sqlite3_বৃক্ষ_View_Trigger_Step(0,p,0,1);
}
শূন্য sqlite3ShowTrigger(ধ্রুবক Trigger *p){ sqlite3_বৃক্ষ_View_Trigger(0,p,0,0); }
শূন্য sqlite3ShowTriggerList(ধ্রুবক Trigger *p){ sqlite3_বৃক্ষ_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
শূন্য sqlite3ShowWindow(ধ্রুবক Window *p){ sqlite3_বৃক্ষ_View_Window(0,p,0); }
শূন্য sqlite3ShowWinFunc(ধ্রুবক Window *p){ sqlite3_বৃক্ষ_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
