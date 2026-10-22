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
#ಅನ್ತರ್ಭಾವ "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(ವೃಕ್ಷ_View **pp, u8 moreToFollow){
  ವೃಕ್ಷ_View *p = *pp;
  ಯದಿ( p==0 ){
    *pp = p = sqlite3_malloc64( ಪರಿಮಾಣ(*p) );
    ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
    memset(p, 0, ಪರಿಮಾಣ(*p));
  }ಅನ್ಯಥಾ{
    p->iLevel++;
  }
  ಪ್ರತಿಜ್ಞಾ( moreToFollow==0 || moreToFollow==1 );
  ಯದಿ( p->iLevel<(ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(p->b_ಸಾಲು) ) p->b_ಸಾಲು[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(ವೃಕ್ಷ_View **pp){
  ವೃಕ್ಷ_View *p = *pp;
  ಯದಿ( p==0 ) ಪ್ರತ್ಯಾಗಮ;
  p->iLevel--;
  ಯದಿ( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(ವೃಕ್ಷ_View *p, ಧ್ರುವ ವರ್ಣ *zFormat, ...){
  va_list ap;
  ಪೂರ್ಣಾಙ್ಕ i;
  StrAccum acc;
  ವರ್ಣ zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, ಪರಿಮಾಣ(zBuf), 0);
  ಯದಿ( p ){
    ಪ್ರತಿ(i=0; i<p->iLevel && i<(ಪೂರ್ಣಾಙ್ಕ)ಪರಿಮಾಣ(p->b_ಸಾಲು)-1; i++){
      sqlite3_str_append(&acc, p->b_ಸಾಲು[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_ಸಾಲು[i] ? "|-- " : "'-- ", 4);
  }
  ಯದಿ( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    ಪ್ರತಿಜ್ಞಾ( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Item(ವೃಕ್ಷ_View *p, ಧ್ರುವ ವರ್ಣ *zLabel,u8 moreFollows){
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&p, moreFollows);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Column_List(
  ವೃಕ್ಷ_View *pView,
  ಧ್ರುವ Column *aCol,
  ಪೂರ್ಣಾಙ್ಕ nCol,
  u8 moreToFollow
){
  ಪೂರ್ಣಾಙ್ಕ i;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, moreToFollow);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "COLUMNS");
  ಪ್ರತಿ(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    ಪೂರ್ಣಾಙ್ಕ colMoreToFollow = i<(nCol - 1);
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, colMoreToFollow);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, 0);
    printf(" %s", aCol[i].zCnName);
    ವಿಕಲ್ಪ( aCol[i].eCType ){
      ಪಕ್ಷ COLTYPE_ANY:      printf(" ANY");        ಭಙ್ಗ;
      ಪಕ್ಷ COLTYPE_BLOB:     printf(" BLOB");       ಭಙ್ಗ;
      ಪಕ್ಷ COLTYPE_INT:      printf(" INT");        ಭಙ್ಗ;
      ಪಕ್ಷ COLTYPE_INTEGER:  printf(" INTEGER");    ಭಙ್ಗ;
      ಪಕ್ಷ COLTYPE_REAL:     printf(" REAL");       ಭಙ್ಗ;
      ಪಕ್ಷ COLTYPE_TEXT:     printf(" TEXT");       ಭಙ್ಗ;
      ಪಕ್ಷ COLTYPE_CUSTOM: {
        ಯದಿ( flg & COLFLAG_HASTYPE ){
          ಧ್ರುವ ವರ್ಣ *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          ಭಙ್ಗ;
        }
      }
    }
    ಯದಿ( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    ಯದಿ( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    ಯದಿ( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    ಯದಿ( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_With(ವೃಕ್ಷ_View *pView, ಧ್ರುವ With *pWith, u8 moreToFollow){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( pWith==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( pWith->nCte==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( pWith->pOuter ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }ಅನ್ಯಥಾ{
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WITH (0x%p)", pWith);
  }
  ಯದಿ( pWith->nCte>0 ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, moreToFollow);
    ಪ್ರತಿ(i=0; i<pWith->nCte; i++){
      StrAccum x;
      ವರ್ಣ z_ಸಾಲು[1000];
      ಧ್ರುವ ಸಂರಚನಾ Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_ಸಾಲು, ಪರಿಮಾಣ(z_ಸಾಲು), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      ಯದಿ( pCte->pCols && pCte->pCols->n_ಅಭಿವ್ಯಕ್ತಿ>0 ){
        ವರ್ಣ cSep = '(';
        ಪೂರ್ಣಾಙ್ಕ j;
        ಪ್ರತಿ(j=0; j<pCte->pCols->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      ಯದಿ( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      ಯದಿ( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_ವೃಕ್ಷ_View_Item(pView, z_ಸಾಲು, i<pWith->nCte-1);
      sqlite3_ವೃಕ್ಷ_View_Select(pView, pCte->pSelect, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Src_List(ವೃಕ್ಷ_View *pView, ಧ್ರುವ SrcList *pSrc){
  ಪೂರ್ಣಾಙ್ಕ i;
  ಯದಿ( pSrc==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ(i=0; i<pSrc->nSrc; i++){
    ಧ್ರುವ SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    ಪೂರ್ಣಾಙ್ಕ n = 0;
    ವರ್ಣ z_ಸಾಲು[1000];
    sqlite3StrAccumInit(&x, 0, z_ಸಾಲು, ಪರಿಮಾಣ(z_ಸಾಲು), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    ಯದಿ( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    ಯದಿ( (pItem->fg.jointype & (JT_ಎಡ|JT_ಬಲ))==(JT_ಎಡ|JT_ಬಲ) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }ಅನ್ಯಥಾ ಯದಿ( pItem->fg.jointype & JT_ಎಡ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }ಅನ್ಯಥಾ ಯದಿ( pItem->fg.jointype & JT_ಬಲ ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }ಅನ್ಯಥಾ ಯದಿ( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    ಯದಿ( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    ಯದಿ( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ಯದಿ( pItem->fg.isCte ){
      ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    ಯದಿ( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    ಯದಿ( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    ಯದಿ( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    ಯದಿ( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    ಯದಿ( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    ಯದಿ( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    ಯದಿ( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    ಯದಿ( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    ಯದಿ( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    ಯದಿ( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_ವೃಕ್ಷ_View_Item(pView, z_ಸಾಲು, i<pSrc->nSrc-1);
    n = 0;
    ಯದಿ( pItem->fg.isSubquery ) n++;
    ಯದಿ( pItem->fg.isTabFunc ) n++;
    ಯದಿ( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    ಯದಿ( pItem->fg.isUsing ){
      sqlite3_ವೃಕ್ಷ_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }ಅನ್ಯಥಾ ಯದಿ( pItem->u3.pOn!=0 ){
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "ON", (--n)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pItem->u3.pOn, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    ಯದಿ( pItem->fg.isSubquery ){
      ಪ್ರತಿಜ್ಞಾ( n==1 );
      ಯದಿ( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_ವೃಕ್ಷ_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಙ್ಕ)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_ವೃಕ್ಷ_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    ಯದಿ( pItem->fg.isTabFunc ){
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Select(ವೃಕ್ಷ_View *pView, ಧ್ರುವ Select *p, u8 moreToFollow){
  ಪೂರ್ಣಾಙ್ಕ n = 0;
  ಪೂರ್ಣಾಙ್ಕ cnt = 0;
  ಯದಿ( p==0 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "nil-SELECT");
    ಪ್ರತ್ಯಾಗಮ;
  } 
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, moreToFollow);
  ಯದಿ( p->pWith ){
    sqlite3_ವೃಕ್ಷ_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 1);
  }
  ಕುರು{
    ಯದಿ( p->selFlags & SF_WhereBegin ){
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "sqlite3WhereBegin()");
    }ಅನ್ಯಥಾ{
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (ಪೂರ್ಣಾಙ್ಕ)p->nSelectRow
      );
    }
    ಯದಿ( cnt++ ) sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    ಯದಿ( p->pPrior ){
      n = 1000;
    }ಅನ್ಯಥಾ{
      n = 0;
      ಯದಿ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      ಯದಿ( p->pWhere ) n++;
      ಯದಿ( p->pGroupBy ) n++;
      ಯದಿ( p->pHaving ) n++;
      ಯದಿ( p->pOrderBy ) n++;
      ಯದಿ( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      ಯದಿ( p->pWin ) n++;
      ಯದಿ( p->pWinDefn ) n++;
#endif
    }
    ಯದಿ( p->pEList ){
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( p->pWin ){
      Window *pX;
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "window-functions");
      ಪ್ರತಿ(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_ವೃಕ್ಷ_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
#endif
    ಯದಿ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FROM");
      sqlite3_ವೃಕ್ಷ_View_Src_List(pView, p->pSrc);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    ಯದಿ( p->pWhere ){
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p->pWhere, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    ಯದಿ( p->pGroupBy ){
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    ಯದಿ( p->pHaving ){
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p->pHaving, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ಯದಿ( p->pWinDefn ){
      Window *pX;
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "WINDOW", (n--)>0);
      ಪ್ರತಿ(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_ವೃಕ್ಷ_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
#endif
    ಯದಿ( p->pOrderBy ){
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    ಯದಿ( p->pLimit ){
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p->pLimit->p_ಎಡ, p->pLimit->p_ಬಲ!=0);
      ಯದಿ( p->pLimit->p_ಬಲ ){
        sqlite3_ವೃಕ್ಷ_View_Item(pView, "OFFSET", 0);
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p->pLimit->p_ಬಲ, 0);
        sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      }
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    ಯದಿ( p->pPrior ){
      ಧ್ರುವ ವರ್ಣ *zOp = "UNION";
      ವಿಕಲ್ಪ( p->op ){
        ಪಕ್ಷ TK_ALL:         zOp = "UNION ALL";  ಭಙ್ಗ;
        ಪಕ್ಷ TK_INTERSECT:   zOp = "INTERSECT";  ಭಙ್ಗ;
        ಪಕ್ಷ TK_EXCEPT:      zOp = "EXCEPT";     ಭಙ್ಗ;
      }
      sqlite3_ವೃಕ್ಷ_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }ಯಾವತ್( p!=0 );
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Bound(
  ವೃಕ್ಷ_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  ವಿಕಲ್ಪ( eBound ){
    ಪಕ್ಷ TK_UNBOUNDED: {
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_CURRENT: {
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_PRECEDING: {
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_FOLLOWING: {
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      ಭಙ್ಗ;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Window(ವೃಕ್ಷ_View *pView, ಧ್ರುವ Window *pWin, u8 more){
  ಪೂರ್ಣಾಙ್ಕ nElement = 0;
  ಯದಿ( pWin==0 ) ಪ್ರತ್ಯಾಗಮ;
  ಯದಿ( pWin->pFilter ){
    sqlite3_ವೃಕ್ಷ_View_Item(pView, "FILTER", 1);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pWin->pFilter, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    ಯದಿ( pWin->eFrmType==TK_FILTER ) ಪ್ರತ್ಯಾಗಮ;
  }
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, more);
  ಯದಿ( pWin->zName ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "OVER %s (%p)", pWin->zName, pWin);
  }ಅನ್ಯಥಾ{
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "OVER (%p)", pWin);
  }
  ಯದಿ( pWin->zBase )    nElement++;
  ಯದಿ( pWin->pOrderBy ) nElement++;
  ಯದಿ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  ಯದಿ( pWin->eExclude ) nElement++;
  ಯದಿ( pWin->zBase ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--nElement)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "window: %s", pWin->zBase);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pWin->pPartition ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  ಯದಿ( pWin->pOrderBy ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  ಯದಿ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    ವರ್ಣ zBuf[30];
    ಧ್ರುವ ವರ್ಣ *zFrmType = "ROWS";
    ಯದಿ( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    ಯದಿ( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(ಪರಿಮಾಣ(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_ವೃಕ್ಷ_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_ವೃಕ್ಷ_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_ವೃಕ್ಷ_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pWin->eExclude ){
    ವರ್ಣ zBuf[30];
    ಧ್ರುವ ವರ್ಣ *zExclude;
    ವಿಕಲ್ಪ( pWin->eExclude ){
      ಪಕ್ಷ TK_NO:      zExclude = "NO OTHERS";   ಭಙ್ಗ;
      ಪಕ್ಷ TK_CURRENT: zExclude = "CURRENT ROW"; ಭಙ್ಗ;
      ಪಕ್ಷ TK_GROUP:   zExclude = "GROUP";       ಭಙ್ಗ;
      ಪಕ್ಷ TK_TIES:    zExclude = "TIES";        ಭಙ್ಗ;
      ಸ್ವಭಾವ:
        sqlite3_snprintf(ಪರಿಮಾಣ(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        ಭಙ್ಗ;
    }
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "EXCLUDE %s", zExclude);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Win_Func(ವೃಕ್ಷ_View *pView, ಧ್ರುವ Window *pWin, u8 more){
  ಯದಿ( pWin==0 ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, more);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_ವೃಕ್ಷ_View_Window(pView, pWin, 0);
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(ವೃಕ್ಷ_View *pView, ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, u8 moreToFollow){
  ಧ್ರುವ ವರ್ಣ *zBinOp = 0;   /* Binary operator */
  ಧ್ರುವ ವರ್ಣ *zUniOp = 0;   /* Unary operator */
  ವರ್ಣ zFlgs[200];
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, moreToFollow);
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "nil");
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    ಪ್ರತ್ಯಾಗಮ;
  }
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->flags || p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ || p_ಅಭಿವ್ಯಕ್ತಿ->vvaFlags || p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, ಪರಿಮಾಣ(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_ಅಭಿವ್ಯಕ್ತಿ->flags, p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ ? p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ : 'n');
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin);
    }
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin);
    }
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_ಅಭಿವ್ಯಕ್ತಿ->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }ಅನ್ಯಥಾ{
    zFlgs[0] = 0;
  }
  ವಿಕಲ್ಪ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಪಕ್ಷ TK_AGG_COLUMN: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "AGG{%d:%d}%s",
            p_ಅಭಿವ್ಯಕ್ತಿ->iTable, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn, zFlgs);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_COLUMN: {
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable<0 ){
        /* This only happens when coding check constraints */
        ವರ್ಣ zOp2[16];
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op2 ){
          sqlite3_snprintf(ಪರಿಮಾಣ(zOp2),zOp2," op2=0x%02x",p_ಅಭಿವ್ಯಕ್ತಿ->op2);
        }ಅನ್ಯಥಾ{
          zOp2[0] = 0;
        }
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "COLUMN(%d)%s%s",
                                    p_ಅಭಿವ್ಯಕ್ತಿ->iColumn, zFlgs, zOp2);
      }ಅನ್ಯಥಾ{
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "{%d:%d} pTab=%p%s",
                        p_ಅಭಿವ್ಯಕ್ತಿ->iTable, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn,
                        p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab, zFlgs);
      }
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FixedCol) ){
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      }
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_INTEGER: {
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_Int_ಮೌಲ್ಯ ){
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%d", p_ಅಭಿವ್ಯಕ್ತಿ->u.i_ಮೌಲ್ಯ);
      }ಅನ್ಯಥಾ{
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      }
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    ಪಕ್ಷ TK_FLOAT: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"%s", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ಭಙ್ಗ;
    }
#endif
    ಪಕ್ಷ TK_STRING: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"%Q", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_NULL: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"NULL");
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_TRUEFALSE: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"%s%s",
         sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ) ? "TRUE" : "FALSE", zFlgs);
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    ಪಕ್ಷ TK_BLOB: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"%s", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ಭಙ್ಗ;
    }
#endif
    ಪಕ್ಷ TK_VARIABLE: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"VARIABLE(%s,%d)",
                          p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_REGISTER: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"REGISTER(%d)", p_ಅಭಿವ್ಯಕ್ತಿ->iTable);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_ID: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"ID \"%w\"", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_ASTERISK: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"ASKTERISK");
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_CAST
    ಪಕ್ಷ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"CAST %Q", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ಭಙ್ಗ;
    }
#endif /* SQLITE_OMIT_CAST */
    ಪಕ್ಷ TK_LT:      zBinOp = "LT";     ಭಙ್ಗ;
    ಪಕ್ಷ TK_LE:      zBinOp = "LE";     ಭಙ್ಗ;
    ಪಕ್ಷ TK_GT:      zBinOp = "GT";     ಭಙ್ಗ;
    ಪಕ್ಷ TK_GE:      zBinOp = "GE";     ಭಙ್ಗ;
    ಪಕ್ಷ TK_NE:      zBinOp = "NE";     ಭಙ್ಗ;
    ಪಕ್ಷ TK_EQ:      zBinOp = "EQ";     ಭಙ್ಗ;
    ಪಕ್ಷ TK_IS:      zBinOp = "IS";     ಭಙ್ಗ;
    ಪಕ್ಷ TK_ISNOT:   zBinOp = "ISNOT";  ಭಙ್ಗ;
    ಪಕ್ಷ TK_AND:     zBinOp = "AND";    ಭಙ್ಗ;
    ಪಕ್ಷ TK_OR:      zBinOp = "OR";     ಭಙ್ಗ;
    ಪಕ್ಷ TK_PLUS:    zBinOp = "ADD";    ಭಙ್ಗ;
    ಪಕ್ಷ TK_STAR:    zBinOp = "MUL";    ಭಙ್ಗ;
    ಪಕ್ಷ TK_MINUS:   zBinOp = "SUB";    ಭಙ್ಗ;
    ಪಕ್ಷ TK_REM:     zBinOp = "REM";    ಭಙ್ಗ;
    ಪಕ್ಷ TK_BITAND:  zBinOp = "BITAND"; ಭಙ್ಗ;
    ಪಕ್ಷ TK_BITOR:   zBinOp = "BITOR";  ಭಙ್ಗ;
    ಪಕ್ಷ TK_SLASH:   zBinOp = "DIV";    ಭಙ್ಗ;
    ಪಕ್ಷ TK_LSHIFT:  zBinOp = "LSHIFT"; ಭಙ್ಗ;
    ಪಕ್ಷ TK_RSHIFT:  zBinOp = "RSHIFT"; ಭಙ್ಗ;
    ಪಕ್ಷ TK_CONCAT:  zBinOp = "CONCAT"; ಭಙ್ಗ;
    ಪಕ್ಷ TK_DOT:     zBinOp = "DOT";    ಭಙ್ಗ;
    ಪಕ್ಷ TK_LIMIT:   zBinOp = "LIMIT";  ಭಙ್ಗ;

    ಪಕ್ಷ TK_UMINUS:  zUniOp = "UMINUS"; ಭಙ್ಗ;
    ಪಕ್ಷ TK_UPLUS:   zUniOp = "UPLUS";  ಭಙ್ಗ;
    ಪಕ್ಷ TK_BITNOT:  zUniOp = "BITNOT"; ಭಙ್ಗ;
    ಪಕ್ಷ TK_NOT:     zUniOp = "NOT";    ಭಙ್ಗ;
    ಪಕ್ಷ TK_ISNULL:  zUniOp = "ISNULL"; ಭಙ್ಗ;
    ಪಕ್ಷ TK_NOTNULL: zUniOp = "NOTNULL"; ಭಙ್ಗ;

    ಪಕ್ಷ TK_TRUTH: {
      ಪೂರ್ಣಾಙ್ಕ x;
      ಧ್ರುವ ವರ್ಣ *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_IS || p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_ISNOT );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ );
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ)->op
                  == TK_TRUEFALSE );
      x = (p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_ISNOT)*2 + sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      zUniOp = azOp[x];
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ TK_SPAN: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "SPAN %Q", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%sCOLLATE %Q%s",
        !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Collate) ? "SOFT-" : "",
        p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, zFlgs);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ಭಙ್ಗ;
    }

    ಪಕ್ಷ TK_AGG_FUNCTION:
    ಪಕ್ಷ TK_FUNCTION: {
      ಅಭಿವ್ಯಕ್ತಿ_List *pFarg;       /* List of function arguments */
      Window *pWin;
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }ಅನ್ಯಥಾ{
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
        pFarg = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_ಅಭಿವ್ಯಕ್ತಿ) ? p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin : 0;
#ಅನ್ಯಥಾ
        pWin = 0;
#endif 
      }
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION ){
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_ಅಭಿವ್ಯಕ್ತಿ->op2, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, zFlgs,
                             p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo ? p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo->selId : 0,
                             p_ಅಭಿವ್ಯಕ್ತಿ->iAgg, p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo);
      }ಅನ್ಯಥಾ ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op2!=0 ){
        ಧ್ರುವ ವರ್ಣ *zOp2;
        ವರ್ಣ zBuf[8];
        sqlite3_snprintf(ಪರಿಮಾಣ(zBuf),zBuf,"0x%02x",p_ಅಭಿವ್ಯಕ್ತಿ->op2);
        zOp2 = zBuf;
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==NC_Idx_ಅಭಿವ್ಯಕ್ತಿ ) zOp2 = "NC_IdxExpr";
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FUNCTION %Q%s op2=%s",
                            p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, zFlgs, zOp2);
      }ಅನ್ಯಥಾ{
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FUNCTION %Q%s", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, zFlgs);
      }
      ಯದಿ( pFarg ){
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pFarg, pWin!=0 || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
        ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
          ಅಭಿವ್ಯಕ್ತಿ *pOB = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
          ಪ್ರತಿಜ್ಞಾ( pOB->op==TK_ORDER );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pOB) );
          sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ಯದಿ( pWin ){
        sqlite3_ವೃಕ್ಷ_View_Window(pView, pWin, 0);
      }
#endif
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_ORDER: {
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, 0, "ORDERBY");
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ಪಕ್ಷ TK_EXISTS: {
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "EXISTS-expr flags=0x%x", p_ಅಭಿವ್ಯಕ್ತಿ->flags);
      sqlite3_ವೃಕ್ಷ_View_Select(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_SELECT: {
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "subquery-expr flags=0x%x", p_ಅಭಿವ್ಯಕ್ತಿ->flags);
      sqlite3_ವೃಕ್ಷ_View_Select(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      ವರ್ಣ *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_ಅಭಿವ್ಯಕ್ತಿ->flags);
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_ಅಭಿವ್ಯಕ್ತಿ->iTable);
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, z);
      sqlite3_free(z);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 1);
      ಯದಿ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        sqlite3_ವೃಕ್ಷ_View_Select(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect, 0);
      }ಅನ್ಯಥಾ{
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, 0, 0);
      }
      ಭಙ್ಗ;
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
    ಪಕ್ಷ TK_BETWEEN: {
      ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pX, *pY, *pZ;
      pX = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 );
      pY = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
      pZ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "BETWEEN%s", zFlgs);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pX, 1);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pY, 1);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pZ, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s(%d)", 
          p_ಅಭಿವ್ಯಕ್ತಿ->iTable ? "NEW" : "OLD", p_ಅಭಿವ್ಯಕ್ತಿ->iColumn);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_CASE: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "CASE");
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 1);
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, 0, 0);
      ಭಙ್ಗ;
    }
#ifndef SQLITE_OMIT_TRIGGER
    ಪಕ್ಷ TK_RAISE: {
      ಧ್ರುವ ವರ್ಣ *zType = "unk";
      ವಿಕಲ್ಪ( p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ ){
        ಪಕ್ಷ OE_Rollback:   zType = "rollback";  ಭಙ್ಗ;
        ಪಕ್ಷ OE_Abort:      zType = "abort";     ಭಙ್ಗ;
        ಪಕ್ಷ OE_Fail:       zType = "fail";      ಭಙ್ಗ;
        ಪಕ್ಷ OE_Ignore:     zType = "ignore";    ಭಙ್ಗ;
      }
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "RAISE %s", zType);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ಭಙ್ಗ;
    }
#endif
    ಪಕ್ಷ TK_MATCH: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "MATCH {%d:%d}%s",
                          p_ಅಭಿವ್ಯಕ್ತಿ->iTable, p_ಅಭಿವ್ಯಕ್ತಿ->iColumn, zFlgs);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_VECTOR: {
      ವರ್ಣ *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3_ವೃಕ್ಷ_View_Bare_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, z);
      sqlite3_free(z);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_SELECT_COLUMN: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_ಅಭಿವ್ಯಕ್ತಿ->iColumn, p_ಅಭಿವ್ಯಕ್ತಿ->iTable-1,
              p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ? " (SELECT-owner)" : "");
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
      sqlite3_ವೃಕ್ಷ_View_Select(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pSelect, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_IF_NULL_ROW: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "IF-NULL-ROW %d", p_ಅಭಿವ್ಯಕ್ತಿ->iTable);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_ERROR: {
      ಅಭಿವ್ಯಕ್ತಿ tmp;
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "ERROR");
      tmp = *p_ಅಭಿವ್ಯಕ್ತಿ;
      tmp.op = p_ಅಭಿವ್ಯಕ್ತಿ->op2;
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, &tmp, 0);
      ಭಙ್ಗ;
    }
    ಪಕ್ಷ TK_ROW: {
      ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ->iColumn<=0 ){
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "First FROM table rowid");
      }ಅನ್ಯಥಾ{
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "First FROM table column %d",
            p_ಅಭಿವ್ಯಕ್ತಿ->iColumn-1);
      }
      ಭಙ್ಗ;
    }
    ಸ್ವಭಾವ: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "op=%d", p_ಅಭಿವ್ಯಕ್ತಿ->op);
      ಭಙ್ಗ;
    }
  }
  ಯದಿ( zBinOp ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 1);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, 0);
  }ಅನ್ಯಥಾ ಯದಿ( zUniOp ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Bare_ಅಭಿವ್ಯಕ್ತಿ_List(
  ವೃಕ್ಷ_View *pView,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pList,
  ಧ್ರುವ ವರ್ಣ *zLabel
){
  ಯದಿ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ಯದಿ( pList==0 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s (empty)", zLabel);
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ i;
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s", zLabel);
    ಪ್ರತಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಪೂರ್ಣಾಙ್ಕ j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      ವರ್ಣ *zName = pList->a[i].zEName;
      ಪೂರ್ಣಾಙ್ಕ moreToFollow = i<pList->n_ಅಭಿವ್ಯಕ್ತಿ - 1;
      ಯದಿ( j || zName || sortFlags ){
        sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, 0);
        ಯದಿ( zName ){
          ವಿಕಲ್ಪ( pList->a[i].fg.eEName ){
            ಸ್ವಭಾವ:
              fprintf(stdout, "AS %s ", zName);
              ಭಙ್ಗ;
            ಪಕ್ಷ ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              ಯದಿ( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              ಯದಿ( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              ಯದಿ( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              ಭಙ್ಗ;
            ಪಕ್ಷ ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              ಭಙ್ಗ;
          }
        }
        ಯದಿ( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        ಯದಿ( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }ಅನ್ಯಥಾ ಯದಿ( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, moreToFollow);
      ಯದಿ( j || zName || sortFlags ){
        sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      }
    }
  }
}
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(
  ವೃಕ್ಷ_View *pView,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pList,
  u8 moreToFollow,
  ಧ್ರುವ ವರ್ಣ *zLabel
){
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, moreToFollow);
  sqlite3_ವೃಕ್ಷ_View_Bare_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pList, zLabel);
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Bare_Id_List(
  ವೃಕ್ಷ_View *pView,
  ಧ್ರುವ IdList *pList,
  ಧ್ರುವ ವರ್ಣ *zLabel
){
  ಯದಿ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ಯದಿ( pList==0 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s (empty)", zLabel);
  }ಅನ್ಯಥಾ{
    ಪೂರ್ಣಾಙ್ಕ i;
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s", zLabel);
    ಪ್ರತಿ(i=0; i<pList->nId; i++){
      ವರ್ಣ *zName = pList->a[i].zName;
      ಪೂರ್ಣಾಙ್ಕ moreToFollow = i<pList->nId - 1;
      ಯದಿ( zName==0 ) zName = "(null)";
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, moreToFollow);
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
  }
}
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Id_List(
  ವೃಕ್ಷ_View *pView,
  ಧ್ರುವ IdList *pList,
  u8 moreToFollow,
  ಧ್ರುವ ವರ್ಣ *zLabel
){
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, moreToFollow);
  sqlite3_ವೃಕ್ಷ_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Upsert(
  ವೃಕ್ಷ_View *pView,
  ಧ್ರುವ Upsert *pUpsert,
  u8 moreToFollow
){
  ಯದಿ( pUpsert==0 ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, moreToFollow);
  ಯಾವತ್( pUpsert ){
    ಪೂರ್ಣಾಙ್ಕ n;
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    ಯದಿ( pUpsert->pUpsertWhere ){
      sqlite3_ವೃಕ್ಷ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

#ಯದಿ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Delete(
  ಧ್ರುವ With *pWith,
  ಧ್ರುವ SrcList *pTabList,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pWhere,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pLimit,
  ಧ್ರುವ Trigger *pTrigger
){
  ಪೂರ್ಣಾಙ್ಕ n = 0;
  ವೃಕ್ಷ_View *pView = 0;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "DELETE");
  ಯದಿ( pWith ) n++;
  ಯದಿ( pTabList ) n++;
  ಯದಿ( pWhere ) n++;
  ಯದಿ( pOrderBy ) n++;
  ಯದಿ( pLimit ) n++;
  ಯದಿ( pTrigger ) n++;
  ಯದಿ( pWith ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_With(pView, pWith, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pTabList ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FROM");
    sqlite3_ವೃಕ್ಷ_View_Src_List(pView, pTabList);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pWhere ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WHERE");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pWhere, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pOrderBy ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  ಯದಿ( pLimit ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "LIMIT");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pLimit, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pTrigger ){
    sqlite3_ವೃಕ್ಷ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#endif /* TREETRACE_ENABLED */

#ಯದಿ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಸೇರಿಸು(
  ಧ್ರುವ With *pWith,
  ಧ್ರುವ SrcList *pTabList,
  ಧ್ರುವ IdList *pColumnList,
  ಧ್ರುವ Select *pSelect,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *p_ಅಭಿವ್ಯಕ್ತಿ_List,
  ಪೂರ್ಣಾಙ್ಕ onError,
  ಧ್ರುವ Upsert *pUpsert,
  ಧ್ರುವ Trigger *pTrigger
){
  ವೃಕ್ಷ_View *pView = 0;
  ಪೂರ್ಣಾಙ್ಕ n = 0;
  ಧ್ರುವ ವರ್ಣ *zLabel = "INSERT";
  ವಿಕಲ್ಪ( onError ){
    ಪಕ್ಷ OE_Replace:  zLabel = "REPLACE";             ಭಙ್ಗ;
    ಪಕ್ಷ OE_Ignore:   zLabel = "INSERT OR IGNORE";    ಭಙ್ಗ;
    ಪಕ್ಷ OE_Rollback: zLabel = "INSERT OR ROLLBACK";  ಭಙ್ಗ;
    ಪಕ್ಷ OE_Abort:    zLabel = "INSERT OR ABORT";     ಭಙ್ಗ;
    ಪಕ್ಷ OE_Fail:     zLabel = "INSERT OR FAIL";      ಭಙ್ಗ;
  }
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, zLabel);
  ಯದಿ( pWith ) n++;
  ಯದಿ( pTabList ) n++;
  ಯದಿ( pColumnList ) n++;
  ಯದಿ( pSelect ) n++;
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ_List ) n++;
  ಯದಿ( pUpsert ) n++;
  ಯದಿ( pTrigger ) n++;
  ಯದಿ( pWith ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_With(pView, pWith, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pTabList ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "INTO");
    sqlite3_ವೃಕ್ಷ_View_Src_List(pView, pTabList);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pColumnList ){
    sqlite3_ವೃಕ್ಷ_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  ಯದಿ( pSelect ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "DATA-SOURCE");
    sqlite3_ವೃಕ್ಷ_View_Select(pView, pSelect, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( p_ಅಭಿವ್ಯಕ್ತಿ_List ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ_List, (--n)>0, "VALUES");
  }
  ಯದಿ( pUpsert ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "UPSERT");
    sqlite3_ವೃಕ್ಷ_View_Upsert(pView, pUpsert, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pTrigger ){
    sqlite3_ವೃಕ್ಷ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#endif /* TREETRACE_ENABLED */

#ಯದಿ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Update(
  ಧ್ರುವ With *pWith,
  ಧ್ರುವ SrcList *pTabList,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pWhere,
  ಪೂರ್ಣಾಙ್ಕ onError,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *pOrderBy,
  ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *pLimit,
  ಧ್ರುವ Upsert *pUpsert,
  ಧ್ರುವ Trigger *pTrigger
){
  ಪೂರ್ಣಾಙ್ಕ n = 0;
  ವೃಕ್ಷ_View *pView = 0;
  ಧ್ರುವ ವರ್ಣ *zLabel = "UPDATE";
  ವಿಕಲ್ಪ( onError ){
    ಪಕ್ಷ OE_Replace:  zLabel = "UPDATE OR REPLACE";   ಭಙ್ಗ;
    ಪಕ್ಷ OE_Ignore:   zLabel = "UPDATE OR IGNORE";    ಭಙ್ಗ;
    ಪಕ್ಷ OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  ಭಙ್ಗ;
    ಪಕ್ಷ OE_Abort:    zLabel = "UPDATE OR ABORT";     ಭಙ್ಗ;
    ಪಕ್ಷ OE_Fail:     zLabel = "UPDATE OR FAIL";      ಭಙ್ಗ;
  }
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, zLabel);
  ಯದಿ( pWith ) n++;
  ಯದಿ( pTabList ) n++;
  ಯದಿ( pChanges ) n++;
  ಯದಿ( pWhere ) n++;
  ಯದಿ( pOrderBy ) n++;
  ಯದಿ( pLimit ) n++;
  ಯದಿ( pUpsert ) n++;
  ಯದಿ( pTrigger ) n++;
  ಯದಿ( pWith ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_With(pView, pWith, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pTabList ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FROM");
    sqlite3_ವೃಕ್ಷ_View_Src_List(pView, pTabList);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pChanges ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pChanges, (--n)>0, "SET");
  }
  ಯದಿ( pWhere ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WHERE");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pWhere, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pOrderBy ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  ಯದಿ( pLimit ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "LIMIT");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pLimit, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pUpsert ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "UPSERT");
    sqlite3_ವೃಕ್ಷ_View_Upsert(pView, pUpsert, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಯದಿ( pTrigger ){
    sqlite3_ವೃಕ್ಷ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Trigger_Step(
  ವೃಕ್ಷ_View *pView,
  ಧ್ರುವ TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  ಪೂರ್ಣಾಙ್ಕ cnt = 0;
  ಯದಿ( pStep==0 ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  ಕುರು{
    ಯದಿ( cnt++ && pStep->pNext==0 ){
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
    }
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }ಯಾವತ್( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Trigger(
  ವೃಕ್ಷ_View *pView,
  ಧ್ರುವ Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  ಪೂರ್ಣಾಙ್ಕ cnt = 0;
  ಯದಿ( pTrigger==0 ) ಪ್ರತ್ಯಾಗಮ;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  ಕುರು{
    ಯದಿ( cnt++ && pTrigger->pNext==0 ){
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
    }
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
    sqlite3_ವೃಕ್ಷ_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }ಯಾವತ್( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
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
ಶೂನ್ಯ sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ *p){ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0,p,0); }
ಶೂನ್ಯ sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ_List(ಧ್ರುವ ಅಭಿವ್ಯಕ್ತಿ_List *p){ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(0,p,0,0);}
ಶೂನ್ಯ sqlite3ShowIdList(ಧ್ರುವ IdList *p){ sqlite3_ವೃಕ್ಷ_View_Id_List(0,p,0,0); }
ಶೂನ್ಯ sqlite3ShowSrcList(ಧ್ರುವ SrcList *p){
  ವೃಕ್ಷ_View *pView = 0;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "SRCLIST");
  sqlite3_ವೃಕ್ಷ_View_Src_List(pView,p);
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
ಶೂನ್ಯ sqlite3ShowSelect(ಧ್ರುವ Select *p){ sqlite3_ವೃಕ್ಷ_View_Select(0,p,0); }
ಶೂನ್ಯ sqlite3ShowWith(ಧ್ರುವ With *p){ sqlite3_ವೃಕ್ಷ_View_With(0,p,0); }
ಶೂನ್ಯ sqlite3ShowUpsert(ಧ್ರುವ Upsert *p){ sqlite3_ವೃಕ್ಷ_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
ಶೂನ್ಯ sqlite3ShowTriggerStep(ಧ್ರುವ TriggerStep *p){
  sqlite3_ವೃಕ್ಷ_View_Trigger_Step(0,p,0,0);
}
ಶೂನ್ಯ sqlite3ShowTriggerStepList(ಧ್ರುವ TriggerStep *p){
  sqlite3_ವೃಕ್ಷ_View_Trigger_Step(0,p,0,1);
}
ಶೂನ್ಯ sqlite3ShowTrigger(ಧ್ರುವ Trigger *p){ sqlite3_ವೃಕ್ಷ_View_Trigger(0,p,0,0); }
ಶೂನ್ಯ sqlite3ShowTriggerList(ಧ್ರುವ Trigger *p){ sqlite3_ವೃಕ್ಷ_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
ಶೂನ್ಯ sqlite3ShowWindow(ಧ್ರುವ Window *p){ sqlite3_ವೃಕ್ಷ_View_Window(0,p,0); }
ಶೂನ್ಯ sqlite3ShowWinFunc(ಧ್ರುವ Window *p){ sqlite3_ವೃಕ್ಷ_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
