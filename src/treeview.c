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
#ଅନ୍ତର୍ଭାଵ "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
ସ୍ଥିର ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_ଠେଲ(ବୃକ୍ଷ_View **pp, u8 moreToFollow){
  ବୃକ୍ଷ_View *p = *pp;
  ଯଦି( p==0 ){
    *pp = p = sqlite3_malloc64( ଆକାର(*p) );
    ଯଦି( p==0 ) ଫେରନ୍ତୁ;
    memset(p, 0, ଆକାର(*p));
  }ଅନ୍ୟଥା{
    p->iLevel++;
  }
  ପ୍ରତିଜ୍ଞା( moreToFollow==0 || moreToFollow==1 );
  ଯଦି( p->iLevel<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(p->b_ଧାଡ଼ି) ) p->b_ଧାଡ଼ି[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
ସ୍ଥିର ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_କାଢ଼(ବୃକ୍ଷ_View **pp){
  ବୃକ୍ଷ_View *p = *pp;
  ଯଦି( p==0 ) ଫେରନ୍ତୁ;
  p->iLevel--;
  ଯଦି( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(ବୃକ୍ଷ_View *p, ଅଚଳ ଅକ୍ଷର *zFormat, ...){
  va_list ap;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  StrAccum acc;
  ଅକ୍ଷର zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, ଆକାର(zBuf), 0);
  ଯଦି( p ){
    ପାଇଁ(i=0; i<p->iLevel && i<(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)ଆକାର(p->b_ଧାଡ଼ି)-1; i++){
      sqlite3_str_append(&acc, p->b_ଧାଡ଼ି[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_ଧାଡ଼ି[i] ? "|-- " : "'-- ", 4);
  }
  ଯଦି( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    ପ୍ରତିଜ୍ଞା( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
ସ୍ଥିର ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Item(ବୃକ୍ଷ_View *p, ଅଚଳ ଅକ୍ଷର *zLabel,u8 moreFollows){
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&p, moreFollows);
  sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Column_List(
  ବୃକ୍ଷ_View *pView,
  ଅଚଳ Column *aCol,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nCol,
  u8 moreToFollow
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, moreToFollow);
  sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "COLUMNS");
  ପାଇଁ(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା colMoreToFollow = i<(nCol - 1);
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, colMoreToFollow);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, 0);
    printf(" %s", aCol[i].zCnName);
    ବାଛ( aCol[i].eCType ){
      ସ୍ଥିତି COLTYPE_ANY:      printf(" ANY");        ଭାଙ୍ଗ;
      ସ୍ଥିତି COLTYPE_BLOB:     printf(" BLOB");       ଭାଙ୍ଗ;
      ସ୍ଥିତି COLTYPE_INT:      printf(" INT");        ଭାଙ୍ଗ;
      ସ୍ଥିତି COLTYPE_INTEGER:  printf(" INTEGER");    ଭାଙ୍ଗ;
      ସ୍ଥିତି COLTYPE_REAL:     printf(" REAL");       ଭାଙ୍ଗ;
      ସ୍ଥିତି COLTYPE_TEXT:     printf(" TEXT");       ଭାଙ୍ଗ;
      ସ୍ଥିତି COLTYPE_CUSTOM: {
        ଯଦି( flg & COLFLAG_HASTYPE ){
          ଅଚଳ ଅକ୍ଷର *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          ଭାଙ୍ଗ;
        }
      }
    }
    ଯଦି( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    ଯଦି( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    ଯଦି( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    ଯଦି( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_With(ବୃକ୍ଷ_View *pView, ଅଚଳ With *pWith, u8 moreToFollow){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( pWith==0 ) ଫେରନ୍ତୁ;
  ଯଦି( pWith->nCte==0 ) ଫେରନ୍ତୁ;
  ଯଦି( pWith->pOuter ){
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }ଅନ୍ୟଥା{
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "WITH (0x%p)", pWith);
  }
  ଯଦି( pWith->nCte>0 ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, moreToFollow);
    ପାଇଁ(i=0; i<pWith->nCte; i++){
      StrAccum x;
      ଅକ୍ଷର z_ଧାଡ଼ି[1000];
      ଅଚଳ ସଂରଚନା Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_ଧାଡ଼ି, ଆକାର(z_ଧାଡ଼ି), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      ଯଦି( pCte->pCols && pCte->pCols->n_ପଦସମଷ୍ଟି>0 ){
        ଅକ୍ଷର cSep = '(';
        ପୂର୍ଣ୍ଣସଂଖ୍ୟା j;
        ପାଇଁ(j=0; j<pCte->pCols->n_ପଦସମଷ୍ଟି; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      ଯଦି( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      ଯଦି( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_ବୃକ୍ଷ_View_Item(pView, z_ଧାଡ଼ି, i<pWith->nCte-1);
      sqlite3_ବୃକ୍ଷ_View_Select(pView, pCte->pSelect, 0);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Src_List(ବୃକ୍ଷ_View *pView, ଅଚଳ SrcList *pSrc){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  ଯଦି( pSrc==0 ) ଫେରନ୍ତୁ;
  ପାଇଁ(i=0; i<pSrc->nSrc; i++){
    ଅଚଳ SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
    ଅକ୍ଷର z_ଧାଡ଼ି[1000];
    sqlite3StrAccumInit(&x, 0, z_ଧାଡ଼ି, ଆକାର(z_ଧାଡ଼ି), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    ଯଦି( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    ଯଦି( (pItem->fg.jointype & (JT_ବାମ|JT_ଡାହାଣ))==(JT_ବାମ|JT_ଡାହାଣ) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }ଅନ୍ୟଥା ଯଦି( pItem->fg.jointype & JT_ବାମ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }ଅନ୍ୟଥା ଯଦି( pItem->fg.jointype & JT_ଡାହାଣ ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }ଅନ୍ୟଥା ଯଦି( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    ଯଦି( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    ଯଦି( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ଯଦି( pItem->fg.isCte ){
      ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    ଯଦି( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    ଯଦି( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    ଯଦି( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    ଯଦି( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    ଯଦି( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    ଯଦି( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    ଯଦି( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    ଯଦି( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    ଯଦି( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    ଯଦି( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_ବୃକ୍ଷ_View_Item(pView, z_ଧାଡ଼ି, i<pSrc->nSrc-1);
    n = 0;
    ଯଦି( pItem->fg.isSubquery ) n++;
    ଯଦି( pItem->fg.isTabFunc ) n++;
    ଯଦି( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    ଯଦି( pItem->fg.isUsing ){
      sqlite3_ବୃକ୍ଷ_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }ଅନ୍ୟଥା ଯଦି( pItem->u3.pOn!=0 ){
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "ON", (--n)>0);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pItem->u3.pOn, 0);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
    ଯଦି( pItem->fg.isSubquery ){
      ପ୍ରତିଜ୍ଞା( n==1 );
      ଯଦି( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_ବୃକ୍ଷ_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      ପ୍ରତିଜ୍ଞା( (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_ବୃକ୍ଷ_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    ଯଦି( pItem->fg.isTabFunc ){
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Select(ବୃକ୍ଷ_View *pView, ଅଚଳ Select *p, u8 moreToFollow){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 0;
  ଯଦି( p==0 ){
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "nil-SELECT");
    ଫେରନ୍ତୁ;
  } 
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, moreToFollow);
  ଯଦି( p->pWith ){
    sqlite3_ବୃକ୍ଷ_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 1);
  }
  କର{
    ଯଦି( p->selFlags & SF_WhereBegin ){
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "sqlite3WhereBegin()");
    }ଅନ୍ୟଥା{
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)p->nSelectRow
      );
    }
    ଯଦି( cnt++ ) sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    ଯଦି( p->pPrior ){
      n = 1000;
    }ଅନ୍ୟଥା{
      n = 0;
      ଯଦି( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      ଯଦି( p->pWhere ) n++;
      ଯଦି( p->pGroupBy ) n++;
      ଯଦି( p->pHaving ) n++;
      ଯଦି( p->pOrderBy ) n++;
      ଯଦି( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      ଯଦି( p->pWin ) n++;
      ଯଦି( p->pWinDefn ) n++;
#endif
    }
    ଯଦି( p->pEList ){
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( p->pWin ){
      Window *pX;
      sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (n--)>0);
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "window-functions");
      ପାଇଁ(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_ବୃକ୍ଷ_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
#endif
    ଯଦି( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (n--)>0);
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "FROM");
      sqlite3_ବୃକ୍ଷ_View_Src_List(pView, p->pSrc);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
    ଯଦି( p->pWhere ){
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p->pWhere, 0);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
    ଯଦି( p->pGroupBy ){
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    ଯଦି( p->pHaving ){
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p->pHaving, 0);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    ଯଦି( p->pWinDefn ){
      Window *pX;
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "WINDOW", (n--)>0);
      ପାଇଁ(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_ବୃକ୍ଷ_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
#endif
    ଯଦି( p->pOrderBy ){
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    ଯଦି( p->pLimit ){
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p->pLimit->p_ବାମ, p->pLimit->p_ଡାହାଣ!=0);
      ଯଦି( p->pLimit->p_ଡାହାଣ ){
        sqlite3_ବୃକ୍ଷ_View_Item(pView, "OFFSET", 0);
        sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p->pLimit->p_ଡାହାଣ, 0);
        sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
      }
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
    ଯଦି( p->pPrior ){
      ଅଚଳ ଅକ୍ଷର *zOp = "UNION";
      ବାଛ( p->op ){
        ସ୍ଥିତି TK_ALL:         zOp = "UNION ALL";  ଭାଙ୍ଗ;
        ସ୍ଥିତି TK_INTERSECT:   zOp = "INTERSECT";  ଭାଙ୍ଗ;
        ସ୍ଥିତି TK_EXCEPT:      zOp = "EXCEPT";     ଭାଙ୍ଗ;
      }
      sqlite3_ବୃକ୍ଷ_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }ଯେତେବେଳେ( p!=0 );
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Bound(
  ବୃକ୍ଷ_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  ବାଛ( eBound ){
    ସ୍ଥିତି TK_UNBOUNDED: {
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_CURRENT: {
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_PRECEDING: {
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି, 0);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_FOLLOWING: {
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି, 0);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
      ଭାଙ୍ଗ;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Window(ବୃକ୍ଷ_View *pView, ଅଚଳ Window *pWin, u8 more){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା nElement = 0;
  ଯଦି( pWin==0 ) ଫେରନ୍ତୁ;
  ଯଦି( pWin->pFilter ){
    sqlite3_ବୃକ୍ଷ_View_Item(pView, "FILTER", 1);
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pWin->pFilter, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    ଯଦି( pWin->eFrmType==TK_FILTER ) ଫେରନ୍ତୁ;
  }
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, more);
  ଯଦି( pWin->zName ){
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "OVER %s (%p)", pWin->zName, pWin);
  }ଅନ୍ୟଥା{
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "OVER (%p)", pWin);
  }
  ଯଦି( pWin->zBase )    nElement++;
  ଯଦି( pWin->pOrderBy ) nElement++;
  ଯଦି( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  ଯଦି( pWin->eExclude ) nElement++;
  ଯଦି( pWin->zBase ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--nElement)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "window: %s", pWin->zBase);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pWin->pPartition ){
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  ଯଦି( pWin->pOrderBy ){
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  ଯଦି( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    ଅକ୍ଷର zBuf[30];
    ଅଚଳ ଅକ୍ଷର *zFrmType = "ROWS";
    ଯଦି( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    ଯଦି( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(ଆକାର(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_ବୃକ୍ଷ_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_ବୃକ୍ଷ_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_ବୃକ୍ଷ_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pWin->eExclude ){
    ଅକ୍ଷର zBuf[30];
    ଅଚଳ ଅକ୍ଷର *zExclude;
    ବାଛ( pWin->eExclude ){
      ସ୍ଥିତି TK_NO:      zExclude = "NO OTHERS";   ଭାଙ୍ଗ;
      ସ୍ଥିତି TK_CURRENT: zExclude = "CURRENT ROW"; ଭାଙ୍ଗ;
      ସ୍ଥିତି TK_GROUP:   zExclude = "GROUP";       ଭାଙ୍ଗ;
      ସ୍ଥିତି TK_TIES:    zExclude = "TIES";        ଭାଙ୍ଗ;
      ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
        sqlite3_snprintf(ଆକାର(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        ଭାଙ୍ଗ;
    }
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "EXCLUDE %s", zExclude);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Win_Func(ବୃକ୍ଷ_View *pView, ଅଚଳ Window *pWin, u8 more){
  ଯଦି( pWin==0 ) ଫେରନ୍ତୁ;
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, more);
  sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_ବୃକ୍ଷ_View_Window(pView, pWin, 0);
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(ବୃକ୍ଷ_View *pView, ଅଚଳ ପଦସମଷ୍ଟି *p_ପଦସମଷ୍ଟି, u8 moreToFollow){
  ଅଚଳ ଅକ୍ଷର *zBinOp = 0;   /* Binary operator */
  ଅଚଳ ଅକ୍ଷର *zUniOp = 0;   /* Unary operator */
  ଅକ୍ଷର zFlgs[200];
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, moreToFollow);
  ଯଦି( p_ପଦସମଷ୍ଟି==0 ){
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "nil");
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    ଫେରନ୍ତୁ;
  }
  ଯଦି( p_ପଦସମଷ୍ଟି->flags || p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି || p_ପଦସମଷ୍ଟି->vvaFlags || p_ପଦସମଷ୍ଟି->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, ଆକାର(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_ପଦସମଷ୍ଟି->flags, p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି ? p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି : 'n');
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_ପଦସମଷ୍ଟି->w.iJoin);
    }
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_ପଦସମଷ୍ଟି->w.iJoin);
    }
    ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ଯଦି( ପଦସମଷ୍ଟି_Has_VVAProperty(p_ପଦସମଷ୍ଟି, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    ଯଦି( p_ପଦସମଷ୍ଟି->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_ପଦସମଷ୍ଟି->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }ଅନ୍ୟଥା{
    zFlgs[0] = 0;
  }
  ବାଛ( p_ପଦସମଷ୍ଟି->op ){
    ସ୍ଥିତି TK_AGG_COLUMN: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "AGG{%d:%d}%s",
            p_ପଦସମଷ୍ଟି->iTable, p_ପଦସମଷ୍ଟି->iColumn, zFlgs);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_COLUMN: {
      ଯଦି( p_ପଦସମଷ୍ଟି->iTable<0 ){
        /* This only happens when coding check constraints */
        ଅକ୍ଷର zOp2[16];
        ଯଦି( p_ପଦସମଷ୍ଟି->op2 ){
          sqlite3_snprintf(ଆକାର(zOp2),zOp2," op2=0x%02x",p_ପଦସମଷ୍ଟି->op2);
        }ଅନ୍ୟଥା{
          zOp2[0] = 0;
        }
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "COLUMN(%d)%s%s",
                                    p_ପଦସମଷ୍ଟି->iColumn, zFlgs, zOp2);
      }ଅନ୍ୟଥା{
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_YTab(p_ପଦସମଷ୍ଟି) );
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "{%d:%d} pTab=%p%s",
                        p_ପଦସମଷ୍ଟି->iTable, p_ପଦସମଷ୍ଟି->iColumn,
                        p_ପଦସମଷ୍ଟି->y.pTab, zFlgs);
      }
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_FixedCol) ){
        sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
      }
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_INTEGER: {
      ଯଦି( p_ପଦସମଷ୍ଟି->flags & EP_Int_ମୂଲ୍ୟ ){
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%d", p_ପଦସମଷ୍ଟି->u.i_ମୂଲ୍ୟ);
      }ଅନ୍ୟଥା{
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%s", p_ପଦସମଷ୍ଟି->u.zToken);
      }
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    ସ୍ଥିତି TK_FLOAT: {
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"%s", p_ପଦସମଷ୍ଟି->u.zToken);
      ଭାଙ୍ଗ;
    }
#endif
    ସ୍ଥିତି TK_STRING: {
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"%Q", p_ପଦସମଷ୍ଟି->u.zToken);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_NULL: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"NULL");
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_TRUEFALSE: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"%s%s",
         sqlite3_ପଦସମଷ୍ଟି_Truth_ମୂଲ୍ୟ(p_ପଦସମଷ୍ଟି) ? "TRUE" : "FALSE", zFlgs);
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    ସ୍ଥିତି TK_BLOB: {
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"%s", p_ପଦସମଷ୍ଟି->u.zToken);
      ଭାଙ୍ଗ;
    }
#endif
    ସ୍ଥିତି TK_VARIABLE: {
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"VARIABLE(%s,%d)",
                          p_ପଦସମଷ୍ଟି->u.zToken, p_ପଦସମଷ୍ଟି->iColumn);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_REGISTER: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"REGISTER(%d)", p_ପଦସମଷ୍ଟି->iTable);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_ID: {
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"ID \"%w\"", p_ପଦସମଷ୍ଟି->u.zToken);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_ASTERISK: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"ASKTERISK");
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_CAST
    ସ୍ଥିତି TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView,"CAST %Q", p_ପଦସମଷ୍ଟି->u.zToken);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
      ଭାଙ୍ଗ;
    }
#endif /* SQLITE_OMIT_CAST */
    ସ୍ଥିତି TK_LT:      zBinOp = "LT";     ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_LE:      zBinOp = "LE";     ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_GT:      zBinOp = "GT";     ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_GE:      zBinOp = "GE";     ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_NE:      zBinOp = "NE";     ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_EQ:      zBinOp = "EQ";     ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_IS:      zBinOp = "IS";     ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_ISNOT:   zBinOp = "ISNOT";  ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_AND:     zBinOp = "AND";    ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_OR:      zBinOp = "OR";     ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_PLUS:    zBinOp = "ADD";    ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_STAR:    zBinOp = "MUL";    ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_MINUS:   zBinOp = "SUB";    ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_REM:     zBinOp = "REM";    ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_BITAND:  zBinOp = "BITAND"; ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_BITOR:   zBinOp = "BITOR";  ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_SLASH:   zBinOp = "DIV";    ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_LSHIFT:  zBinOp = "LSHIFT"; ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_RSHIFT:  zBinOp = "RSHIFT"; ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_CONCAT:  zBinOp = "CONCAT"; ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_DOT:     zBinOp = "DOT";    ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_LIMIT:   zBinOp = "LIMIT";  ଭାଙ୍ଗ;

    ସ୍ଥିତି TK_UMINUS:  zUniOp = "UMINUS"; ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_UPLUS:   zUniOp = "UPLUS";  ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_BITNOT:  zUniOp = "BITNOT"; ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_NOT:     zUniOp = "NOT";    ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_ISNULL:  zUniOp = "ISNULL"; ଭାଙ୍ଗ;
    ସ୍ଥିତି TK_NOTNULL: zUniOp = "NOTNULL"; ଭାଙ୍ଗ;

    ସ୍ଥିତି TK_TRUTH: {
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା x;
      ଅଚଳ ଅକ୍ଷର *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->op2==TK_IS || p_ପଦସମଷ୍ଟି->op2==TK_ISNOT );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->p_ଡାହାଣ );
      ପ୍ରତିଜ୍ଞା( sqlite3_ପଦସମଷ୍ଟି_Skip_Collate_And_Likely(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ)->op
                  == TK_TRUEFALSE );
      x = (p_ପଦସମଷ୍ଟି->op2==TK_ISNOT)*2 + sqlite3_ପଦସମଷ୍ଟି_Truth_ମୂଲ୍ୟ(p_ପଦସମଷ୍ଟି->p_ଡାହାଣ);
      zUniOp = azOp[x];
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି TK_SPAN: {
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "SPAN %Q", p_ପଦସମଷ୍ଟି->u.zToken);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%sCOLLATE %Q%s",
        !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Collate) ? "SOFT-" : "",
        p_ପଦସମଷ୍ଟି->u.zToken, zFlgs);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
      ଭାଙ୍ଗ;
    }

    ସ୍ଥିତି TK_AGG_FUNCTION:
    ସ୍ଥିତି TK_FUNCTION: {
      ପଦସମଷ୍ଟି_List *pFarg;       /* List of function arguments */
      Window *pWin;
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }ଅନ୍ୟଥା{
        ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
        pFarg = p_ପଦସମଷ୍ଟି->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_ପଦସମଷ୍ଟି) ? p_ପଦସମଷ୍ଟି->y.pWin : 0;
#ଅନ୍ୟଥା
        pWin = 0;
#endif 
      }
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      ଯଦି( p_ପଦସମଷ୍ଟି->op==TK_AGG_FUNCTION ){
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_ପଦସମଷ୍ଟି->op2, p_ପଦସମଷ୍ଟି->u.zToken, zFlgs,
                             p_ପଦସମଷ୍ଟି->pAggInfo ? p_ପଦସମଷ୍ଟି->pAggInfo->selId : 0,
                             p_ପଦସମଷ୍ଟି->iAgg, p_ପଦସମଷ୍ଟି->pAggInfo);
      }ଅନ୍ୟଥା ଯଦି( p_ପଦସମଷ୍ଟି->op2!=0 ){
        ଅଚଳ ଅକ୍ଷର *zOp2;
        ଅକ୍ଷର zBuf[8];
        sqlite3_snprintf(ଆକାର(zBuf),zBuf,"0x%02x",p_ପଦସମଷ୍ଟି->op2);
        zOp2 = zBuf;
        ଯଦି( p_ପଦସମଷ୍ଟି->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        ଯଦି( p_ପଦସମଷ୍ଟି->op2==NC_Idx_ପଦସମଷ୍ଟି ) zOp2 = "NC_IdxExpr";
        ଯଦି( p_ପଦସମଷ୍ଟି->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        ଯଦି( p_ପଦସମଷ୍ଟି->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "FUNCTION %Q%s op2=%s",
                            p_ପଦସମଷ୍ଟି->u.zToken, zFlgs, zOp2);
      }ଅନ୍ୟଥା{
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "FUNCTION %Q%s", p_ପଦସମଷ୍ଟି->u.zToken, zFlgs);
      }
      ଯଦି( pFarg ){
        sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pFarg, pWin!=0 || p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
        ଯଦି( p_ପଦସମଷ୍ଟି->p_ବାମ ){
          ପଦସମଷ୍ଟି *pOB = p_ପଦସମଷ୍ଟି->p_ବାମ;
          ପ୍ରତିଜ୍ଞା( pOB->op==TK_ORDER );
          ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(pOB) );
          sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      ଯଦି( pWin ){
        sqlite3_ବୃକ୍ଷ_View_Window(pView, pWin, 0);
      }
#endif
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_ORDER: {
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, p_ପଦସମଷ୍ଟି->x.pList, 0, "ORDERBY");
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    ସ୍ଥିତି TK_EXISTS: {
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "EXISTS-expr flags=0x%x", p_ପଦସମଷ୍ଟି->flags);
      sqlite3_ବୃକ୍ଷ_View_Select(pView, p_ପଦସମଷ୍ଟି->x.pSelect, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_SELECT: {
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "subquery-expr flags=0x%x", p_ପଦସମଷ୍ଟି->flags);
      sqlite3_ବୃକ୍ଷ_View_Select(pView, p_ପଦସମଷ୍ଟି->x.pSelect, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      ଅକ୍ଷର *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_ପଦସମଷ୍ଟି->flags);
      ଯଦି( p_ପଦସମଷ୍ଟି->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_ପଦସମଷ୍ଟି->iTable);
      ଯଦି( ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_ପଦସମଷ୍ଟି->y.sub.regReturn, p_ପଦସମଷ୍ଟି->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, z);
      sqlite3_free(z);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 1);
      ଯଦି( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି) ){
        sqlite3_ବୃକ୍ଷ_View_Select(pView, p_ପଦସମଷ୍ଟି->x.pSelect, 0);
      }ଅନ୍ୟଥା{
        sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, p_ପଦସମଷ୍ଟି->x.pList, 0, 0);
      }
      ଭାଙ୍ଗ;
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
    ସ୍ଥିତି TK_BETWEEN: {
      ଅଚଳ ପଦସମଷ୍ଟି *pX, *pY, *pZ;
      pX = p_ପଦସମଷ୍ଟି->p_ବାମ;
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
      ପ୍ରତିଜ୍ଞା( p_ପଦସମଷ୍ଟି->x.pList->n_ପଦସମଷ୍ଟି==2 );
      pY = p_ପଦସମଷ୍ଟି->x.pList->a[0].p_ପଦସମଷ୍ଟି;
      pZ = p_ପଦସମଷ୍ଟି->x.pList->a[1].p_ପଦସମଷ୍ଟି;
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "BETWEEN%s", zFlgs);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pX, 1);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pY, 1);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pZ, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%s(%d)", 
          p_ପଦସମଷ୍ଟି->iTable ? "NEW" : "OLD", p_ପଦସମଷ୍ଟି->iColumn);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_CASE: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "CASE");
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 1);
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, p_ପଦସମଷ୍ଟି->x.pList, 0, 0);
      ଭାଙ୍ଗ;
    }
#ifndef SQLITE_OMIT_TRIGGER
    ସ୍ଥିତି TK_RAISE: {
      ଅଚଳ ଅକ୍ଷର *zType = "unk";
      ବାଛ( p_ପଦସମଷ୍ଟି->aff_ପଦସମଷ୍ଟି ){
        ସ୍ଥିତି OE_Rollback:   zType = "rollback";  ଭାଙ୍ଗ;
        ସ୍ଥିତି OE_Abort:      zType = "abort";     ଭାଙ୍ଗ;
        ସ୍ଥିତି OE_Fail:       zType = "fail";      ଭାଙ୍ଗ;
        ସ୍ଥିତି OE_Ignore:     zType = "ignore";    ଭାଙ୍ଗ;
      }
      ପ୍ରତିଜ୍ଞା( !ପଦସମଷ୍ଟି_Has_Property(p_ପଦସମଷ୍ଟି, EP_Int_ମୂଲ୍ୟ) );
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "RAISE %s", zType);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
      ଭାଙ୍ଗ;
    }
#endif
    ସ୍ଥିତି TK_MATCH: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "MATCH {%d:%d}%s",
                          p_ପଦସମଷ୍ଟି->iTable, p_ପଦସମଷ୍ଟି->iColumn, zFlgs);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_VECTOR: {
      ଅକ୍ଷର *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XList(p_ପଦସମଷ୍ଟି) );
      sqlite3_ବୃକ୍ଷ_View_Bare_ପଦସମଷ୍ଟି_List(pView, p_ପଦସମଷ୍ଟି->x.pList, z);
      sqlite3_free(z);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_SELECT_COLUMN: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_ପଦସମଷ୍ଟି->iColumn, p_ପଦସମଷ୍ଟି->iTable-1,
              p_ପଦସମଷ୍ଟି->p_ଡାହାଣ==p_ପଦସମଷ୍ଟି->p_ବାମ ? " (SELECT-owner)" : "");
      ପ୍ରତିଜ୍ଞା( ପଦସମଷ୍ଟି_Use_XSelect(p_ପଦସମଷ୍ଟି->p_ବାମ) );
      sqlite3_ବୃକ୍ଷ_View_Select(pView, p_ପଦସମଷ୍ଟି->p_ବାମ->x.pSelect, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_IF_NULL_ROW: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "IF-NULL-ROW %d", p_ପଦସମଷ୍ଟି->iTable);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_ERROR: {
      ପଦସମଷ୍ଟି tmp;
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "ERROR");
      tmp = *p_ପଦସମଷ୍ଟି;
      tmp.op = p_ପଦସମଷ୍ଟି->op2;
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, &tmp, 0);
      ଭାଙ୍ଗ;
    }
    ସ୍ଥିତି TK_ROW: {
      ଯଦି( p_ପଦସମଷ୍ଟି->iColumn<=0 ){
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "First FROM table rowid");
      }ଅନ୍ୟଥା{
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "First FROM table column %d",
            p_ପଦସମଷ୍ଟି->iColumn-1);
      }
      ଭାଙ୍ଗ;
    }
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ: {
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "op=%d", p_ପଦସମଷ୍ଟି->op);
      ଭାଙ୍ଗ;
    }
  }
  ଯଦି( zBinOp ){
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 1);
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ଡାହାଣ, 0);
  }ଅନ୍ୟଥା ଯଦି( zUniOp ){
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, p_ପଦସମଷ୍ଟି->p_ବାମ, 0);
  }
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Bare_ପଦସମଷ୍ଟି_List(
  ବୃକ୍ଷ_View *pView,
  ଅଚଳ ପଦସମଷ୍ଟି_List *pList,
  ଅଚଳ ଅକ୍ଷର *zLabel
){
  ଯଦି( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ଯଦି( pList==0 ){
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%s (empty)", zLabel);
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%s", zLabel);
    ପାଇଁ(i=0; i<pList->n_ପଦସମଷ୍ଟି; i++){
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      ଅକ୍ଷର *zName = pList->a[i].zEName;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା moreToFollow = i<pList->n_ପଦସମଷ୍ଟି - 1;
      ଯଦି( j || zName || sortFlags ){
        sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, 0);
        ଯଦି( zName ){
          ବାଛ( pList->a[i].fg.eEName ){
            ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
              fprintf(stdout, "AS %s ", zName);
              ଭାଙ୍ଗ;
            ସ୍ଥିତି ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              ଯଦି( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              ଯଦି( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              ଯଦି( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              ଭାଙ୍ଗ;
            ସ୍ଥିତି ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              ଭାଙ୍ଗ;
          }
        }
        ଯଦି( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        ଯଦି( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }ଅନ୍ୟଥା ଯଦି( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pList->a[i].p_ପଦସମଷ୍ଟି, moreToFollow);
      ଯଦି( j || zName || sortFlags ){
        sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
      }
    }
  }
}
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(
  ବୃକ୍ଷ_View *pView,
  ଅଚଳ ପଦସମଷ୍ଟି_List *pList,
  u8 moreToFollow,
  ଅଚଳ ଅକ୍ଷର *zLabel
){
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, moreToFollow);
  sqlite3_ବୃକ୍ଷ_View_Bare_ପଦସମଷ୍ଟି_List(pView, pList, zLabel);
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Bare_Id_List(
  ବୃକ୍ଷ_View *pView,
  ଅଚଳ IdList *pList,
  ଅଚଳ ଅକ୍ଷର *zLabel
){
  ଯଦି( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ଯଦି( pList==0 ){
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%s (empty)", zLabel);
  }ଅନ୍ୟଥା{
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%s", zLabel);
    ପାଇଁ(i=0; i<pList->nId; i++){
      ଅକ୍ଷର *zName = pList->a[i].zName;
      ପୂର୍ଣ୍ଣସଂଖ୍ୟା moreToFollow = i<pList->nId - 1;
      ଯଦି( zName==0 ) zName = "(null)";
      sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, moreToFollow);
      sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
  }
}
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Id_List(
  ବୃକ୍ଷ_View *pView,
  ଅଚଳ IdList *pList,
  u8 moreToFollow,
  ଅଚଳ ଅକ୍ଷର *zLabel
){
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, moreToFollow);
  sqlite3_ବୃକ୍ଷ_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Upsert(
  ବୃକ୍ଷ_View *pView,
  ଅଚଳ Upsert *pUpsert,
  u8 moreToFollow
){
  ଯଦି( pUpsert==0 ) ଫେରନ୍ତୁ;
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, moreToFollow);
  ଯେତେବେଳେ( pUpsert ){
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    ଯଦି( pUpsert->pUpsertWhere ){
      sqlite3_ବୃକ୍ଷ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    }
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}

#ଯଦି TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Delete(
  ଅଚଳ With *pWith,
  ଅଚଳ SrcList *pTabList,
  ଅଚଳ ପଦସମଷ୍ଟି *pWhere,
  ଅଚଳ ପଦସମଷ୍ଟି_List *pOrderBy,
  ଅଚଳ ପଦସମଷ୍ଟି *pLimit,
  ଅଚଳ Trigger *pTrigger
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
  ବୃକ୍ଷ_View *pView = 0;
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 0);
  sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "DELETE");
  ଯଦି( pWith ) n++;
  ଯଦି( pTabList ) n++;
  ଯଦି( pWhere ) n++;
  ଯଦି( pOrderBy ) n++;
  ଯଦି( pLimit ) n++;
  ଯଦି( pTrigger ) n++;
  ଯଦି( pWith ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_With(pView, pWith, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pTabList ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "FROM");
    sqlite3_ବୃକ୍ଷ_View_Src_List(pView, pTabList);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pWhere ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "WHERE");
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pWhere, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pOrderBy ){
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  ଯଦି( pLimit ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "LIMIT");
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pLimit, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pTrigger ){
    sqlite3_ବୃକ୍ଷ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}
#endif /* TREETRACE_ENABLED */

#ଯଦି TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_ଭର୍ତ୍ତି(
  ଅଚଳ With *pWith,
  ଅଚଳ SrcList *pTabList,
  ଅଚଳ IdList *pColumnList,
  ଅଚଳ Select *pSelect,
  ଅଚଳ ପଦସମଷ୍ଟି_List *p_ପଦସମଷ୍ଟି_List,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା onError,
  ଅଚଳ Upsert *pUpsert,
  ଅଚଳ Trigger *pTrigger
){
  ବୃକ୍ଷ_View *pView = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
  ଅଚଳ ଅକ୍ଷର *zLabel = "INSERT";
  ବାଛ( onError ){
    ସ୍ଥିତି OE_Replace:  zLabel = "REPLACE";             ଭାଙ୍ଗ;
    ସ୍ଥିତି OE_Ignore:   zLabel = "INSERT OR IGNORE";    ଭାଙ୍ଗ;
    ସ୍ଥିତି OE_Rollback: zLabel = "INSERT OR ROLLBACK";  ଭାଙ୍ଗ;
    ସ୍ଥିତି OE_Abort:    zLabel = "INSERT OR ABORT";     ଭାଙ୍ଗ;
    ସ୍ଥିତି OE_Fail:     zLabel = "INSERT OR FAIL";      ଭାଙ୍ଗ;
  }
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 0);
  sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, zLabel);
  ଯଦି( pWith ) n++;
  ଯଦି( pTabList ) n++;
  ଯଦି( pColumnList ) n++;
  ଯଦି( pSelect ) n++;
  ଯଦି( p_ପଦସମଷ୍ଟି_List ) n++;
  ଯଦି( pUpsert ) n++;
  ଯଦି( pTrigger ) n++;
  ଯଦି( pWith ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_With(pView, pWith, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pTabList ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "INTO");
    sqlite3_ବୃକ୍ଷ_View_Src_List(pView, pTabList);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pColumnList ){
    sqlite3_ବୃକ୍ଷ_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  ଯଦି( pSelect ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "DATA-SOURCE");
    sqlite3_ବୃକ୍ଷ_View_Select(pView, pSelect, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( p_ପଦସମଷ୍ଟି_List ){
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, p_ପଦସମଷ୍ଟି_List, (--n)>0, "VALUES");
  }
  ଯଦି( pUpsert ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "UPSERT");
    sqlite3_ବୃକ୍ଷ_View_Upsert(pView, pUpsert, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pTrigger ){
    sqlite3_ବୃକ୍ଷ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}
#endif /* TREETRACE_ENABLED */

#ଯଦି TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Update(
  ଅଚଳ With *pWith,
  ଅଚଳ SrcList *pTabList,
  ଅଚଳ ପଦସମଷ୍ଟି_List *pChanges,
  ଅଚଳ ପଦସମଷ୍ଟି *pWhere,
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା onError,
  ଅଚଳ ପଦସମଷ୍ଟି_List *pOrderBy,
  ଅଚଳ ପଦସମଷ୍ଟି *pLimit,
  ଅଚଳ Upsert *pUpsert,
  ଅଚଳ Trigger *pTrigger
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
  ବୃକ୍ଷ_View *pView = 0;
  ଅଚଳ ଅକ୍ଷର *zLabel = "UPDATE";
  ବାଛ( onError ){
    ସ୍ଥିତି OE_Replace:  zLabel = "UPDATE OR REPLACE";   ଭାଙ୍ଗ;
    ସ୍ଥିତି OE_Ignore:   zLabel = "UPDATE OR IGNORE";    ଭାଙ୍ଗ;
    ସ୍ଥିତି OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  ଭାଙ୍ଗ;
    ସ୍ଥିତି OE_Abort:    zLabel = "UPDATE OR ABORT";     ଭାଙ୍ଗ;
    ସ୍ଥିତି OE_Fail:     zLabel = "UPDATE OR FAIL";      ଭାଙ୍ଗ;
  }
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 0);
  sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, zLabel);
  ଯଦି( pWith ) n++;
  ଯଦି( pTabList ) n++;
  ଯଦି( pChanges ) n++;
  ଯଦି( pWhere ) n++;
  ଯଦି( pOrderBy ) n++;
  ଯଦି( pLimit ) n++;
  ଯଦି( pUpsert ) n++;
  ଯଦି( pTrigger ) n++;
  ଯଦି( pWith ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_With(pView, pWith, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pTabList ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "FROM");
    sqlite3_ବୃକ୍ଷ_View_Src_List(pView, pTabList);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pChanges ){
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pChanges, (--n)>0, "SET");
  }
  ଯଦି( pWhere ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "WHERE");
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pWhere, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pOrderBy ){
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  ଯଦି( pLimit ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "LIMIT");
    sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(pView, pLimit, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pUpsert ){
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, (--n)>0);
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "UPSERT");
    sqlite3_ବୃକ୍ଷ_View_Upsert(pView, pUpsert, 0);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }
  ଯଦି( pTrigger ){
    sqlite3_ବୃକ୍ଷ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Trigger_Step(
  ବୃକ୍ଷ_View *pView,
  ଅଚଳ TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 0;
  ଯଦି( pStep==0 ) ଫେରନ୍ତୁ;
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  କର{
    ଯଦି( cnt++ && pStep->pNext==0 ){
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
      sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 0);
    }
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }ଯେତେବେଳେ( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
ଶୂନ୍ୟ sqlite3_ବୃକ୍ଷ_View_Trigger(
  ବୃକ୍ଷ_View *pView,
  ଅଚଳ Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 0;
  ଯଦି( pTrigger==0 ) ଫେରନ୍ତୁ;
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  କର{
    ଯଦି( cnt++ && pTrigger->pNext==0 ){
      sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
      sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 0);
    }
    sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 0);
    sqlite3_ବୃକ୍ଷ_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
  }ଯେତେବେଳେ( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
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
ଶୂନ୍ୟ sqlite3_Show_ପଦସମଷ୍ଟି(ଅଚଳ ପଦସମଷ୍ଟି *p){ sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି(0,p,0); }
ଶୂନ୍ୟ sqlite3_Show_ପଦସମଷ୍ଟି_List(ଅଚଳ ପଦସମଷ୍ଟି_List *p){ sqlite3_ବୃକ୍ଷ_View_ପଦସମଷ୍ଟି_List(0,p,0,0);}
ଶୂନ୍ୟ sqlite3ShowIdList(ଅଚଳ IdList *p){ sqlite3_ବୃକ୍ଷ_View_Id_List(0,p,0,0); }
ଶୂନ୍ୟ sqlite3ShowSrcList(ଅଚଳ SrcList *p){
  ବୃକ୍ଷ_View *pView = 0;
  sqlite3_ବୃକ୍ଷ_View_ଠେଲ(&pView, 0);
  sqlite3_ବୃକ୍ଷ_View_ଧାଡ଼ି(pView, "SRCLIST");
  sqlite3_ବୃକ୍ଷ_View_Src_List(pView,p);
  sqlite3_ବୃକ୍ଷ_View_କାଢ଼(&pView);
}
ଶୂନ୍ୟ sqlite3ShowSelect(ଅଚଳ Select *p){ sqlite3_ବୃକ୍ଷ_View_Select(0,p,0); }
ଶୂନ୍ୟ sqlite3ShowWith(ଅଚଳ With *p){ sqlite3_ବୃକ୍ଷ_View_With(0,p,0); }
ଶୂନ୍ୟ sqlite3ShowUpsert(ଅଚଳ Upsert *p){ sqlite3_ବୃକ୍ଷ_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
ଶୂନ୍ୟ sqlite3ShowTriggerStep(ଅଚଳ TriggerStep *p){
  sqlite3_ବୃକ୍ଷ_View_Trigger_Step(0,p,0,0);
}
ଶୂନ୍ୟ sqlite3ShowTriggerStepList(ଅଚଳ TriggerStep *p){
  sqlite3_ବୃକ୍ଷ_View_Trigger_Step(0,p,0,1);
}
ଶୂନ୍ୟ sqlite3ShowTrigger(ଅଚଳ Trigger *p){ sqlite3_ବୃକ୍ଷ_View_Trigger(0,p,0,0); }
ଶୂନ୍ୟ sqlite3ShowTriggerList(ଅଚଳ Trigger *p){ sqlite3_ବୃକ୍ଷ_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
ଶୂନ୍ୟ sqlite3ShowWindow(ଅଚଳ Window *p){ sqlite3_ବୃକ୍ଷ_View_Window(0,p,0); }
ଶୂନ୍ୟ sqlite3ShowWinFunc(ଅଚଳ Window *p){ sqlite3_ବୃକ୍ଷ_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
