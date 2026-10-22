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
#अन्तर्भाव "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
स्थिर शून्य sqlite3_वृक्षः_View_प्रक्षिप(वृक्षः_View **pp, u8 moreToFollow){
  वृक्षः_View *p = *pp;
  जइ( p==0 ){
    *pp = p = sqlite3_malloc64( परिमाण(*p) );
    जइ( p==0 ) प्रत्यागम;
    memset(p, 0, परिमाण(*p));
  }अण्णहा{
    p->iLevel++;
  }
  प्रतिज्ञा( moreToFollow==0 || moreToFollow==1 );
  जइ( p->iLevel<(पूर्णाङ्क)परिमाण(p->b_पङ्क्तिः) ) p->b_पङ्क्तिः[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
स्थिर शून्य sqlite3_वृक्षः_View_निष्कासय(वृक्षः_View **pp){
  वृक्षः_View *p = *pp;
  जइ( p==0 ) प्रत्यागम;
  p->iLevel--;
  जइ( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
शून्य sqlite3_वृक्षः_View_पङ्क्तिः(वृक्षः_View *p, ध्रुव वर्ण *zFormat, ...){
  va_list ap;
  पूर्णाङ्क i;
  StrAccum acc;
  वर्ण zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, परिमाण(zBuf), 0);
  जइ( p ){
    प्रति(i=0; i<p->iLevel && i<(पूर्णाङ्क)परिमाण(p->b_पङ्क्तिः)-1; i++){
      sqlite3_str_append(&acc, p->b_पङ्क्तिः[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_पङ्क्तिः[i] ? "|-- " : "'-- ", 4);
  }
  जइ( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    प्रतिज्ञा( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
स्थिर शून्य sqlite3_वृक्षः_View_Item(वृक्षः_View *p, ध्रुव वर्ण *zLabel,u8 moreFollows){
  sqlite3_वृक्षः_View_प्रक्षिप(&p, moreFollows);
  sqlite3_वृक्षः_View_पङ्क्तिः(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
शून्य sqlite3_वृक्षः_View_Column_List(
  वृक्षः_View *pView,
  ध्रुव Column *aCol,
  पूर्णाङ्क nCol,
  u8 moreToFollow
){
  पूर्णाङ्क i;
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, moreToFollow);
  sqlite3_वृक्षः_View_पङ्क्तिः(pView, "COLUMNS");
  प्रति(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    पूर्णाङ्क colMoreToFollow = i<(nCol - 1);
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, colMoreToFollow);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, 0);
    printf(" %s", aCol[i].zCnName);
    विकल्प( aCol[i].eCType ){
      पक्ष COLTYPE_ANY:      printf(" ANY");        भङ्ग;
      पक्ष COLTYPE_BLOB:     printf(" BLOB");       भङ्ग;
      पक्ष COLTYPE_INT:      printf(" INT");        भङ्ग;
      पक्ष COLTYPE_INTEGER:  printf(" INTEGER");    भङ्ग;
      पक्ष COLTYPE_REAL:     printf(" REAL");       भङ्ग;
      पक्ष COLTYPE_TEXT:     printf(" TEXT");       भङ्ग;
      पक्ष COLTYPE_CUSTOM: {
        जइ( flg & COLFLAG_HASTYPE ){
          ध्रुव वर्ण *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          भङ्ग;
        }
      }
    }
    जइ( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    जइ( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    जइ( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    जइ( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
शून्य sqlite3_वृक्षः_View_With(वृक्षः_View *pView, ध्रुव With *pWith, u8 moreToFollow){
  पूर्णाङ्क i;
  जइ( pWith==0 ) प्रत्यागम;
  जइ( pWith->nCte==0 ) प्रत्यागम;
  जइ( pWith->pOuter ){
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }अण्णहा{
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "WITH (0x%p)", pWith);
  }
  जइ( pWith->nCte>0 ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, moreToFollow);
    प्रति(i=0; i<pWith->nCte; i++){
      StrAccum x;
      वर्ण z_पङ्क्तिः[1000];
      ध्रुव संरचना Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_पङ्क्तिः, परिमाण(z_पङ्क्तिः), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      जइ( pCte->pCols && pCte->pCols->n_व्यञ्जकम्>0 ){
        वर्ण cSep = '(';
        पूर्णाङ्क j;
        प्रति(j=0; j<pCte->pCols->n_व्यञ्जकम्; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      जइ( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      जइ( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_वृक्षः_View_Item(pView, z_पङ्क्तिः, i<pWith->nCte-1);
      sqlite3_वृक्षः_View_Select(pView, pCte->pSelect, 0);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
शून्य sqlite3_वृक्षः_View_Src_List(वृक्षः_View *pView, ध्रुव SrcList *pSrc){
  पूर्णाङ्क i;
  जइ( pSrc==0 ) प्रत्यागम;
  प्रति(i=0; i<pSrc->nSrc; i++){
    ध्रुव SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    पूर्णाङ्क n = 0;
    वर्ण z_पङ्क्तिः[1000];
    sqlite3StrAccumInit(&x, 0, z_पङ्क्तिः, परिमाण(z_पङ्क्तिः), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    जइ( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    जइ( (pItem->fg.jointype & (JT_वामः|JT_दक्षिणः))==(JT_वामः|JT_दक्षिणः) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }अण्णहा जइ( pItem->fg.jointype & JT_वामः ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }अण्णहा जइ( pItem->fg.jointype & JT_दक्षिणः ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }अण्णहा जइ( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    जइ( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    जइ( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    जइ( pItem->fg.isCte ){
      स्थिर ध्रुव वर्ण *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    जइ( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    जइ( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    जइ( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    जइ( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    जइ( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    जइ( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    जइ( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    जइ( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    जइ( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    जइ( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_वृक्षः_View_Item(pView, z_पङ्क्तिः, i<pSrc->nSrc-1);
    n = 0;
    जइ( pItem->fg.isSubquery ) n++;
    जइ( pItem->fg.isTabFunc ) n++;
    जइ( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    जइ( pItem->fg.isUsing ){
      sqlite3_वृक्षः_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }अण्णहा जइ( pItem->u3.pOn!=0 ){
      sqlite3_वृक्षः_View_Item(pView, "ON", (--n)>0);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pItem->u3.pOn, 0);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
    जइ( pItem->fg.isSubquery ){
      प्रतिज्ञा( n==1 );
      जइ( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_वृक्षः_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      प्रतिज्ञा( (पूर्णाङ्क)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_वृक्षः_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    जइ( pItem->fg.isTabFunc ){
      sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
शून्य sqlite3_वृक्षः_View_Select(वृक्षः_View *pView, ध्रुव Select *p, u8 moreToFollow){
  पूर्णाङ्क n = 0;
  पूर्णाङ्क cnt = 0;
  जइ( p==0 ){
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "nil-SELECT");
    प्रत्यागम;
  } 
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, moreToFollow);
  जइ( p->pWith ){
    sqlite3_वृक्षः_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, 1);
  }
  करेहि{
    जइ( p->selFlags & SF_WhereBegin ){
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "sqlite3WhereBegin()");
    }अण्णहा{
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (पूर्णाङ्क)p->nSelectRow
      );
    }
    जइ( cnt++ ) sqlite3_वृक्षः_View_निष्कासय(&pView);
    जइ( p->pPrior ){
      n = 1000;
    }अण्णहा{
      n = 0;
      जइ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      जइ( p->pWhere ) n++;
      जइ( p->pGroupBy ) n++;
      जइ( p->pHaving ) n++;
      जइ( p->pOrderBy ) n++;
      जइ( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      जइ( p->pWin ) n++;
      जइ( p->pWinDefn ) n++;
#endif
    }
    जइ( p->pEList ){
      sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    जइ( p->pWin ){
      Window *pX;
      sqlite3_वृक्षः_View_प्रक्षिप(&pView, (n--)>0);
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "window-functions");
      प्रति(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_वृक्षः_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
#endif
    जइ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_वृक्षः_View_प्रक्षिप(&pView, (n--)>0);
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "FROM");
      sqlite3_वृक्षः_View_Src_List(pView, p->pSrc);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
    जइ( p->pWhere ){
      sqlite3_वृक्षः_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p->pWhere, 0);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
    जइ( p->pGroupBy ){
      sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    जइ( p->pHaving ){
      sqlite3_वृक्षः_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p->pHaving, 0);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    जइ( p->pWinDefn ){
      Window *pX;
      sqlite3_वृक्षः_View_Item(pView, "WINDOW", (n--)>0);
      प्रति(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_वृक्षः_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
#endif
    जइ( p->pOrderBy ){
      sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    जइ( p->pLimit ){
      sqlite3_वृक्षः_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p->pLimit->p_वामः, p->pLimit->p_दक्षिणः!=0);
      जइ( p->pLimit->p_दक्षिणः ){
        sqlite3_वृक्षः_View_Item(pView, "OFFSET", 0);
        sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p->pLimit->p_दक्षिणः, 0);
        sqlite3_वृक्षः_View_निष्कासय(&pView);
      }
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
    जइ( p->pPrior ){
      ध्रुव वर्ण *zOp = "UNION";
      विकल्प( p->op ){
        पक्ष TK_ALL:         zOp = "UNION ALL";  भङ्ग;
        पक्ष TK_INTERSECT:   zOp = "INTERSECT";  भङ्ग;
        पक्ष TK_EXCEPT:      zOp = "EXCEPT";     भङ्ग;
      }
      sqlite3_वृक्षः_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }जाव( p!=0 );
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
शून्य sqlite3_वृक्षः_View_Bound(
  वृक्षः_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  व्यञ्जकम् *p_व्यञ्जकम्,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  विकल्प( eBound ){
    पक्ष TK_UNBOUNDED: {
      sqlite3_वृक्षः_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
      भङ्ग;
    }
    पक्ष TK_CURRENT: {
      sqlite3_वृक्षः_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
      भङ्ग;
    }
    पक्ष TK_PRECEDING: {
      sqlite3_वृक्षः_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्, 0);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
      भङ्ग;
    }
    पक्ष TK_FOLLOWING: {
      sqlite3_वृक्षः_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्, 0);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
      भङ्ग;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
शून्य sqlite3_वृक्षः_View_Window(वृक्षः_View *pView, ध्रुव Window *pWin, u8 more){
  पूर्णाङ्क nElement = 0;
  जइ( pWin==0 ) प्रत्यागम;
  जइ( pWin->pFilter ){
    sqlite3_वृक्षः_View_Item(pView, "FILTER", 1);
    sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pWin->pFilter, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
    जइ( pWin->eFrmType==TK_FILTER ) प्रत्यागम;
  }
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, more);
  जइ( pWin->zName ){
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "OVER %s (%p)", pWin->zName, pWin);
  }अण्णहा{
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "OVER (%p)", pWin);
  }
  जइ( pWin->zBase )    nElement++;
  जइ( pWin->pOrderBy ) nElement++;
  जइ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  जइ( pWin->eExclude ) nElement++;
  जइ( pWin->zBase ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--nElement)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "window: %s", pWin->zBase);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pWin->pPartition ){
    sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  जइ( pWin->pOrderBy ){
    sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  जइ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    वर्ण zBuf[30];
    ध्रुव वर्ण *zFrmType = "ROWS";
    जइ( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    जइ( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(परिमाण(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_वृक्षः_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_वृक्षः_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_वृक्षः_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pWin->eExclude ){
    वर्ण zBuf[30];
    ध्रुव वर्ण *zExclude;
    विकल्प( pWin->eExclude ){
      पक्ष TK_NO:      zExclude = "NO OTHERS";   भङ्ग;
      पक्ष TK_CURRENT: zExclude = "CURRENT ROW"; भङ्ग;
      पक्ष TK_GROUP:   zExclude = "GROUP";       भङ्ग;
      पक्ष TK_TIES:    zExclude = "TIES";        भङ्ग;
      स्वभाव:
        sqlite3_snprintf(परिमाण(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        भङ्ग;
    }
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, 0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "EXCLUDE %s", zExclude);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
शून्य sqlite3_वृक्षः_View_Win_Func(वृक्षः_View *pView, ध्रुव Window *pWin, u8 more){
  जइ( pWin==0 ) प्रत्यागम;
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, more);
  sqlite3_वृक्षः_View_पङ्क्तिः(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_वृक्षः_View_Window(pView, pWin, 0);
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
शून्य sqlite3_वृक्षः_View_व्यञ्जकम्(वृक्षः_View *pView, ध्रुव व्यञ्जकम् *p_व्यञ्जकम्, u8 moreToFollow){
  ध्रुव वर्ण *zBinOp = 0;   /* Binary operator */
  ध्रुव वर्ण *zUniOp = 0;   /* Unary operator */
  वर्ण zFlgs[200];
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, moreToFollow);
  जइ( p_व्यञ्जकम्==0 ){
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "nil");
    sqlite3_वृक्षः_View_निष्कासय(&pView);
    प्रत्यागम;
  }
  जइ( p_व्यञ्जकम्->flags || p_व्यञ्जकम्->aff_व्यञ्जकम् || p_व्यञ्जकम्->vvaFlags || p_व्यञ्जकम्->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, परिमाण(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_व्यञ्जकम्->flags, p_व्यञ्जकम्->aff_व्यञ्जकम् ? p_व्यञ्जकम्->aff_व्यञ्जकम् : 'n');
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_व्यञ्जकम्->w.iJoin);
    }
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_व्यञ्जकम्->w.iJoin);
    }
    जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    जइ( व्यञ्जकम्_Has_VVAProperty(p_व्यञ्जकम्, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    जइ( p_व्यञ्जकम्->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_व्यञ्जकम्->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }अण्णहा{
    zFlgs[0] = 0;
  }
  विकल्प( p_व्यञ्जकम्->op ){
    पक्ष TK_AGG_COLUMN: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "AGG{%d:%d}%s",
            p_व्यञ्जकम्->iTable, p_व्यञ्जकम्->iColumn, zFlgs);
      भङ्ग;
    }
    पक्ष TK_COLUMN: {
      जइ( p_व्यञ्जकम्->iTable<0 ){
        /* This only happens when coding check constraints */
        वर्ण zOp2[16];
        जइ( p_व्यञ्जकम्->op2 ){
          sqlite3_snprintf(परिमाण(zOp2),zOp2," op2=0x%02x",p_व्यञ्जकम्->op2);
        }अण्णहा{
          zOp2[0] = 0;
        }
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, "COLUMN(%d)%s%s",
                                    p_व्यञ्जकम्->iColumn, zFlgs, zOp2);
      }अण्णहा{
        प्रतिज्ञा( व्यञ्जकम्_Use_YTab(p_व्यञ्जकम्) );
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, "{%d:%d} pTab=%p%s",
                        p_व्यञ्जकम्->iTable, p_व्यञ्जकम्->iColumn,
                        p_व्यञ्जकम्->y.pTab, zFlgs);
      }
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_FixedCol) ){
        sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 0);
      }
      भङ्ग;
    }
    पक्ष TK_INTEGER: {
      जइ( p_व्यञ्जकम्->flags & EP_Int_मानम् ){
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%d", p_व्यञ्जकम्->u.i_मानम्);
      }अण्णहा{
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%s", p_व्यञ्जकम्->u.zToken);
      }
      भङ्ग;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    पक्ष TK_FLOAT: {
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"%s", p_व्यञ्जकम्->u.zToken);
      भङ्ग;
    }
#endif
    पक्ष TK_STRING: {
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"%Q", p_व्यञ्जकम्->u.zToken);
      भङ्ग;
    }
    पक्ष TK_NULL: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"NULL");
      भङ्ग;
    }
    पक्ष TK_TRUEFALSE: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"%s%s",
         sqlite3_व्यञ्जकम्_Truth_मानम्(p_व्यञ्जकम्) ? "TRUE" : "FALSE", zFlgs);
      भङ्ग;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    पक्ष TK_BLOB: {
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"%s", p_व्यञ्जकम्->u.zToken);
      भङ्ग;
    }
#endif
    पक्ष TK_VARIABLE: {
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"VARIABLE(%s,%d)",
                          p_व्यञ्जकम्->u.zToken, p_व्यञ्जकम्->iColumn);
      भङ्ग;
    }
    पक्ष TK_REGISTER: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"REGISTER(%d)", p_व्यञ्जकम्->iTable);
      भङ्ग;
    }
    पक्ष TK_ID: {
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"ID \"%w\"", p_व्यञ्जकम्->u.zToken);
      भङ्ग;
    }
    पक्ष TK_ASTERISK: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"ASKTERISK");
      भङ्ग;
    }
#ifndef SQLITE_OMIT_CAST
    पक्ष TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView,"CAST %Q", p_व्यञ्जकम्->u.zToken);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 0);
      भङ्ग;
    }
#endif /* SQLITE_OMIT_CAST */
    पक्ष TK_LT:      zBinOp = "LT";     भङ्ग;
    पक्ष TK_LE:      zBinOp = "LE";     भङ्ग;
    पक्ष TK_GT:      zBinOp = "GT";     भङ्ग;
    पक्ष TK_GE:      zBinOp = "GE";     भङ्ग;
    पक्ष TK_NE:      zBinOp = "NE";     भङ्ग;
    पक्ष TK_EQ:      zBinOp = "EQ";     भङ्ग;
    पक्ष TK_IS:      zBinOp = "IS";     भङ्ग;
    पक्ष TK_ISNOT:   zBinOp = "ISNOT";  भङ्ग;
    पक्ष TK_AND:     zBinOp = "AND";    भङ्ग;
    पक्ष TK_OR:      zBinOp = "OR";     भङ्ग;
    पक्ष TK_PLUS:    zBinOp = "ADD";    भङ्ग;
    पक्ष TK_STAR:    zBinOp = "MUL";    भङ्ग;
    पक्ष TK_MINUS:   zBinOp = "SUB";    भङ्ग;
    पक्ष TK_REM:     zBinOp = "REM";    भङ्ग;
    पक्ष TK_BITAND:  zBinOp = "BITAND"; भङ्ग;
    पक्ष TK_BITOR:   zBinOp = "BITOR";  भङ्ग;
    पक्ष TK_SLASH:   zBinOp = "DIV";    भङ्ग;
    पक्ष TK_LSHIFT:  zBinOp = "LSHIFT"; भङ्ग;
    पक्ष TK_RSHIFT:  zBinOp = "RSHIFT"; भङ्ग;
    पक्ष TK_CONCAT:  zBinOp = "CONCAT"; भङ्ग;
    पक्ष TK_DOT:     zBinOp = "DOT";    भङ्ग;
    पक्ष TK_LIMIT:   zBinOp = "LIMIT";  भङ्ग;

    पक्ष TK_UMINUS:  zUniOp = "UMINUS"; भङ्ग;
    पक्ष TK_UPLUS:   zUniOp = "UPLUS";  भङ्ग;
    पक्ष TK_BITNOT:  zUniOp = "BITNOT"; भङ्ग;
    पक्ष TK_NOT:     zUniOp = "NOT";    भङ्ग;
    पक्ष TK_ISNULL:  zUniOp = "ISNULL"; भङ्ग;
    पक्ष TK_NOTNULL: zUniOp = "NOTNULL"; भङ्ग;

    पक्ष TK_TRUTH: {
      पूर्णाङ्क x;
      ध्रुव वर्ण *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      प्रतिज्ञा( p_व्यञ्जकम्->op2==TK_IS || p_व्यञ्जकम्->op2==TK_ISNOT );
      प्रतिज्ञा( p_व्यञ्जकम्->p_दक्षिणः );
      प्रतिज्ञा( sqlite3_व्यञ्जकम्_Skip_Collate_And_Likely(p_व्यञ्जकम्->p_दक्षिणः)->op
                  == TK_TRUEFALSE );
      x = (p_व्यञ्जकम्->op2==TK_ISNOT)*2 + sqlite3_व्यञ्जकम्_Truth_मानम्(p_व्यञ्जकम्->p_दक्षिणः);
      zUniOp = azOp[x];
      भङ्ग;
    }

    पक्ष TK_SPAN: {
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "SPAN %Q", p_व्यञ्जकम्->u.zToken);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 0);
      भङ्ग;
    }

    पक्ष TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%sCOLLATE %Q%s",
        !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Collate) ? "SOFT-" : "",
        p_व्यञ्जकम्->u.zToken, zFlgs);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 0);
      भङ्ग;
    }

    पक्ष TK_AGG_FUNCTION:
    पक्ष TK_FUNCTION: {
      व्यञ्जकम्_List *pFarg;       /* List of function arguments */
      Window *pWin;
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }अण्णहा{
        प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
        pFarg = p_व्यञ्जकम्->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_व्यञ्जकम्) ? p_व्यञ्जकम्->y.pWin : 0;
#अण्णहा
        pWin = 0;
#endif 
      }
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      जइ( p_व्यञ्जकम्->op==TK_AGG_FUNCTION ){
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_व्यञ्जकम्->op2, p_व्यञ्जकम्->u.zToken, zFlgs,
                             p_व्यञ्जकम्->pAggInfo ? p_व्यञ्जकम्->pAggInfo->selId : 0,
                             p_व्यञ्जकम्->iAgg, p_व्यञ्जकम्->pAggInfo);
      }अण्णहा जइ( p_व्यञ्जकम्->op2!=0 ){
        ध्रुव वर्ण *zOp2;
        वर्ण zBuf[8];
        sqlite3_snprintf(परिमाण(zBuf),zBuf,"0x%02x",p_व्यञ्जकम्->op2);
        zOp2 = zBuf;
        जइ( p_व्यञ्जकम्->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        जइ( p_व्यञ्जकम्->op2==NC_Idx_व्यञ्जकम् ) zOp2 = "NC_IdxExpr";
        जइ( p_व्यञ्जकम्->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        जइ( p_व्यञ्जकम्->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, "FUNCTION %Q%s op2=%s",
                            p_व्यञ्जकम्->u.zToken, zFlgs, zOp2);
      }अण्णहा{
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, "FUNCTION %Q%s", p_व्यञ्जकम्->u.zToken, zFlgs);
      }
      जइ( pFarg ){
        sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pFarg, pWin!=0 || p_व्यञ्जकम्->p_वामः, 0);
        जइ( p_व्यञ्जकम्->p_वामः ){
          व्यञ्जकम् *pOB = p_व्यञ्जकम्->p_वामः;
          प्रतिज्ञा( pOB->op==TK_ORDER );
          प्रतिज्ञा( व्यञ्जकम्_Use_XList(pOB) );
          sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      जइ( pWin ){
        sqlite3_वृक्षः_View_Window(pView, pWin, 0);
      }
#endif
      भङ्ग;
    }
    पक्ष TK_ORDER: {
      sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, p_व्यञ्जकम्->x.pList, 0, "ORDERBY");
      भङ्ग;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    पक्ष TK_EXISTS: {
      प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "EXISTS-expr flags=0x%x", p_व्यञ्जकम्->flags);
      sqlite3_वृक्षः_View_Select(pView, p_व्यञ्जकम्->x.pSelect, 0);
      भङ्ग;
    }
    पक्ष TK_SELECT: {
      प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "subquery-expr flags=0x%x", p_व्यञ्जकम्->flags);
      sqlite3_वृक्षः_View_Select(pView, p_व्यञ्जकम्->x.pSelect, 0);
      भङ्ग;
    }
    पक्ष TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      वर्ण *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_व्यञ्जकम्->flags);
      जइ( p_व्यञ्जकम्->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_व्यञ्जकम्->iTable);
      जइ( व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_व्यञ्जकम्->y.sub.regReturn, p_व्यञ्जकम्->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, z);
      sqlite3_free(z);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 1);
      जइ( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्) ){
        sqlite3_वृक्षः_View_Select(pView, p_व्यञ्जकम्->x.pSelect, 0);
      }अण्णहा{
        sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, p_व्यञ्जकम्->x.pList, 0, 0);
      }
      भङ्ग;
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
    पक्ष TK_BETWEEN: {
      ध्रुव व्यञ्जकम् *pX, *pY, *pZ;
      pX = p_व्यञ्जकम्->p_वामः;
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
      प्रतिज्ञा( p_व्यञ्जकम्->x.pList->n_व्यञ्जकम्==2 );
      pY = p_व्यञ्जकम्->x.pList->a[0].p_व्यञ्जकम्;
      pZ = p_व्यञ्जकम्->x.pList->a[1].p_व्यञ्जकम्;
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "BETWEEN%s", zFlgs);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pX, 1);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pY, 1);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pZ, 0);
      भङ्ग;
    }
    पक्ष TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%s(%d)", 
          p_व्यञ्जकम्->iTable ? "NEW" : "OLD", p_व्यञ्जकम्->iColumn);
      भङ्ग;
    }
    पक्ष TK_CASE: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "CASE");
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 1);
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
      sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, p_व्यञ्जकम्->x.pList, 0, 0);
      भङ्ग;
    }
#ifndef SQLITE_OMIT_TRIGGER
    पक्ष TK_RAISE: {
      ध्रुव वर्ण *zType = "unk";
      विकल्प( p_व्यञ्जकम्->aff_व्यञ्जकम् ){
        पक्ष OE_Rollback:   zType = "rollback";  भङ्ग;
        पक्ष OE_Abort:      zType = "abort";     भङ्ग;
        पक्ष OE_Fail:       zType = "fail";      भङ्ग;
        पक्ष OE_Ignore:     zType = "ignore";    भङ्ग;
      }
      प्रतिज्ञा( !व्यञ्जकम्_Has_Property(p_व्यञ्जकम्, EP_Int_मानम्) );
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "RAISE %s", zType);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 0);
      भङ्ग;
    }
#endif
    पक्ष TK_MATCH: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "MATCH {%d:%d}%s",
                          p_व्यञ्जकम्->iTable, p_व्यञ्जकम्->iColumn, zFlgs);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_दक्षिणः, 0);
      भङ्ग;
    }
    पक्ष TK_VECTOR: {
      वर्ण *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      प्रतिज्ञा( व्यञ्जकम्_Use_XList(p_व्यञ्जकम्) );
      sqlite3_वृक्षः_View_Bare_व्यञ्जकम्_List(pView, p_व्यञ्जकम्->x.pList, z);
      sqlite3_free(z);
      भङ्ग;
    }
    पक्ष TK_SELECT_COLUMN: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_व्यञ्जकम्->iColumn, p_व्यञ्जकम्->iTable-1,
              p_व्यञ्जकम्->p_दक्षिणः==p_व्यञ्जकम्->p_वामः ? " (SELECT-owner)" : "");
      प्रतिज्ञा( व्यञ्जकम्_Use_XSelect(p_व्यञ्जकम्->p_वामः) );
      sqlite3_वृक्षः_View_Select(pView, p_व्यञ्जकम्->p_वामः->x.pSelect, 0);
      भङ्ग;
    }
    पक्ष TK_IF_NULL_ROW: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "IF-NULL-ROW %d", p_व्यञ्जकम्->iTable);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 0);
      भङ्ग;
    }
    पक्ष TK_ERROR: {
      व्यञ्जकम् tmp;
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "ERROR");
      tmp = *p_व्यञ्जकम्;
      tmp.op = p_व्यञ्जकम्->op2;
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, &tmp, 0);
      भङ्ग;
    }
    पक्ष TK_ROW: {
      जइ( p_व्यञ्जकम्->iColumn<=0 ){
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, "First FROM table rowid");
      }अण्णहा{
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, "First FROM table column %d",
            p_व्यञ्जकम्->iColumn-1);
      }
      भङ्ग;
    }
    स्वभाव: {
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, "op=%d", p_व्यञ्जकम्->op);
      भङ्ग;
    }
  }
  जइ( zBinOp ){
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 1);
    sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_दक्षिणः, 0);
  }अण्णहा जइ( zUniOp ){
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_वृक्षः_View_व्यञ्जकम्(pView, p_व्यञ्जकम्->p_वामः, 0);
  }
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
शून्य sqlite3_वृक्षः_View_Bare_व्यञ्जकम्_List(
  वृक्षः_View *pView,
  ध्रुव व्यञ्जकम्_List *pList,
  ध्रुव वर्ण *zLabel
){
  जइ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  जइ( pList==0 ){
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%s (empty)", zLabel);
  }अण्णहा{
    पूर्णाङ्क i;
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%s", zLabel);
    प्रति(i=0; i<pList->n_व्यञ्जकम्; i++){
      पूर्णाङ्क j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      वर्ण *zName = pList->a[i].zEName;
      पूर्णाङ्क moreToFollow = i<pList->n_व्यञ्जकम् - 1;
      जइ( j || zName || sortFlags ){
        sqlite3_वृक्षः_View_प्रक्षिप(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_वृक्षः_View_पङ्क्तिः(pView, 0);
        जइ( zName ){
          विकल्प( pList->a[i].fg.eEName ){
            स्वभाव:
              fprintf(stdout, "AS %s ", zName);
              भङ्ग;
            पक्ष ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              जइ( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              जइ( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              जइ( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              भङ्ग;
            पक्ष ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              भङ्ग;
          }
        }
        जइ( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        जइ( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }अण्णहा जइ( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pList->a[i].p_व्यञ्जकम्, moreToFollow);
      जइ( j || zName || sortFlags ){
        sqlite3_वृक्षः_View_निष्कासय(&pView);
      }
    }
  }
}
शून्य sqlite3_वृक्षः_View_व्यञ्जकम्_List(
  वृक्षः_View *pView,
  ध्रुव व्यञ्जकम्_List *pList,
  u8 moreToFollow,
  ध्रुव वर्ण *zLabel
){
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, moreToFollow);
  sqlite3_वृक्षः_View_Bare_व्यञ्जकम्_List(pView, pList, zLabel);
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
शून्य sqlite3_वृक्षः_View_Bare_Id_List(
  वृक्षः_View *pView,
  ध्रुव IdList *pList,
  ध्रुव वर्ण *zLabel
){
  जइ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  जइ( pList==0 ){
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%s (empty)", zLabel);
  }अण्णहा{
    पूर्णाङ्क i;
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%s", zLabel);
    प्रति(i=0; i<pList->nId; i++){
      वर्ण *zName = pList->a[i].zName;
      पूर्णाङ्क moreToFollow = i<pList->nId - 1;
      जइ( zName==0 ) zName = "(null)";
      sqlite3_वृक्षः_View_प्रक्षिप(&pView, moreToFollow);
      sqlite3_वृक्षः_View_पङ्क्तिः(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
  }
}
शून्य sqlite3_वृक्षः_View_Id_List(
  वृक्षः_View *pView,
  ध्रुव IdList *pList,
  u8 moreToFollow,
  ध्रुव वर्ण *zLabel
){
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, moreToFollow);
  sqlite3_वृक्षः_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
शून्य sqlite3_वृक्षः_View_Upsert(
  वृक्षः_View *pView,
  ध्रुव Upsert *pUpsert,
  u8 moreToFollow
){
  जइ( pUpsert==0 ) प्रत्यागम;
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, moreToFollow);
  जाव( pUpsert ){
    पूर्णाङ्क n;
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    जइ( pUpsert->pUpsertWhere ){
      sqlite3_वृक्षः_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_वृक्षः_View_निष्कासय(&pView);
    }
    sqlite3_वृक्षः_View_निष्कासय(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}

#जइ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
शून्य sqlite3_वृक्षः_View_Delete(
  ध्रुव With *pWith,
  ध्रुव SrcList *pTabList,
  ध्रुव व्यञ्जकम् *pWhere,
  ध्रुव व्यञ्जकम्_List *pOrderBy,
  ध्रुव व्यञ्जकम् *pLimit,
  ध्रुव Trigger *pTrigger
){
  पूर्णाङ्क n = 0;
  वृक्षः_View *pView = 0;
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, 0);
  sqlite3_वृक्षः_View_पङ्क्तिः(pView, "DELETE");
  जइ( pWith ) n++;
  जइ( pTabList ) n++;
  जइ( pWhere ) n++;
  जइ( pOrderBy ) n++;
  जइ( pLimit ) n++;
  जइ( pTrigger ) n++;
  जइ( pWith ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_With(pView, pWith, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pTabList ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "FROM");
    sqlite3_वृक्षः_View_Src_List(pView, pTabList);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pWhere ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "WHERE");
    sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pWhere, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pOrderBy ){
    sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  जइ( pLimit ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "LIMIT");
    sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pLimit, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pTrigger ){
    sqlite3_वृक्षः_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}
#endif /* TREETRACE_ENABLED */

#जइ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
शून्य sqlite3_वृक्षः_View_निवेशय(
  ध्रुव With *pWith,
  ध्रुव SrcList *pTabList,
  ध्रुव IdList *pColumnList,
  ध्रुव Select *pSelect,
  ध्रुव व्यञ्जकम्_List *p_व्यञ्जकम्_List,
  पूर्णाङ्क onError,
  ध्रुव Upsert *pUpsert,
  ध्रुव Trigger *pTrigger
){
  वृक्षः_View *pView = 0;
  पूर्णाङ्क n = 0;
  ध्रुव वर्ण *zLabel = "INSERT";
  विकल्प( onError ){
    पक्ष OE_Replace:  zLabel = "REPLACE";             भङ्ग;
    पक्ष OE_Ignore:   zLabel = "INSERT OR IGNORE";    भङ्ग;
    पक्ष OE_Rollback: zLabel = "INSERT OR ROLLBACK";  भङ्ग;
    पक्ष OE_Abort:    zLabel = "INSERT OR ABORT";     भङ्ग;
    पक्ष OE_Fail:     zLabel = "INSERT OR FAIL";      भङ्ग;
  }
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, 0);
  sqlite3_वृक्षः_View_पङ्क्तिः(pView, zLabel);
  जइ( pWith ) n++;
  जइ( pTabList ) n++;
  जइ( pColumnList ) n++;
  जइ( pSelect ) n++;
  जइ( p_व्यञ्जकम्_List ) n++;
  जइ( pUpsert ) n++;
  जइ( pTrigger ) n++;
  जइ( pWith ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_With(pView, pWith, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pTabList ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "INTO");
    sqlite3_वृक्षः_View_Src_List(pView, pTabList);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pColumnList ){
    sqlite3_वृक्षः_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  जइ( pSelect ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "DATA-SOURCE");
    sqlite3_वृक्षः_View_Select(pView, pSelect, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( p_व्यञ्जकम्_List ){
    sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, p_व्यञ्जकम्_List, (--n)>0, "VALUES");
  }
  जइ( pUpsert ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "UPSERT");
    sqlite3_वृक्षः_View_Upsert(pView, pUpsert, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pTrigger ){
    sqlite3_वृक्षः_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}
#endif /* TREETRACE_ENABLED */

#जइ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
शून्य sqlite3_वृक्षः_View_Update(
  ध्रुव With *pWith,
  ध्रुव SrcList *pTabList,
  ध्रुव व्यञ्जकम्_List *pChanges,
  ध्रुव व्यञ्जकम् *pWhere,
  पूर्णाङ्क onError,
  ध्रुव व्यञ्जकम्_List *pOrderBy,
  ध्रुव व्यञ्जकम् *pLimit,
  ध्रुव Upsert *pUpsert,
  ध्रुव Trigger *pTrigger
){
  पूर्णाङ्क n = 0;
  वृक्षः_View *pView = 0;
  ध्रुव वर्ण *zLabel = "UPDATE";
  विकल्प( onError ){
    पक्ष OE_Replace:  zLabel = "UPDATE OR REPLACE";   भङ्ग;
    पक्ष OE_Ignore:   zLabel = "UPDATE OR IGNORE";    भङ्ग;
    पक्ष OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  भङ्ग;
    पक्ष OE_Abort:    zLabel = "UPDATE OR ABORT";     भङ्ग;
    पक्ष OE_Fail:     zLabel = "UPDATE OR FAIL";      भङ्ग;
  }
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, 0);
  sqlite3_वृक्षः_View_पङ्क्तिः(pView, zLabel);
  जइ( pWith ) n++;
  जइ( pTabList ) n++;
  जइ( pChanges ) n++;
  जइ( pWhere ) n++;
  जइ( pOrderBy ) n++;
  जइ( pLimit ) n++;
  जइ( pUpsert ) n++;
  जइ( pTrigger ) n++;
  जइ( pWith ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_With(pView, pWith, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pTabList ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "FROM");
    sqlite3_वृक्षः_View_Src_List(pView, pTabList);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pChanges ){
    sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pChanges, (--n)>0, "SET");
  }
  जइ( pWhere ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "WHERE");
    sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pWhere, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pOrderBy ){
    sqlite3_वृक्षः_View_व्यञ्जकम्_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  जइ( pLimit ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "LIMIT");
    sqlite3_वृक्षः_View_व्यञ्जकम्(pView, pLimit, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pUpsert ){
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, (--n)>0);
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "UPSERT");
    sqlite3_वृक्षः_View_Upsert(pView, pUpsert, 0);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }
  जइ( pTrigger ){
    sqlite3_वृक्षः_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
शून्य sqlite3_वृक्षः_View_Trigger_Step(
  वृक्षः_View *pView,
  ध्रुव TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णाङ्क cnt = 0;
  जइ( pStep==0 ) प्रत्यागम;
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  करेहि{
    जइ( cnt++ && pStep->pNext==0 ){
      sqlite3_वृक्षः_View_निष्कासय(&pView);
      sqlite3_वृक्षः_View_प्रक्षिप(&pView, 0);
    }
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }जाव( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
शून्य sqlite3_वृक्षः_View_Trigger(
  वृक्षः_View *pView,
  ध्रुव Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णाङ्क cnt = 0;
  जइ( pTrigger==0 ) प्रत्यागम;
  sqlite3_वृक्षः_View_प्रक्षिप(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  करेहि{
    जइ( cnt++ && pTrigger->pNext==0 ){
      sqlite3_वृक्षः_View_निष्कासय(&pView);
      sqlite3_वृक्षः_View_प्रक्षिप(&pView, 0);
    }
    sqlite3_वृक्षः_View_पङ्क्तिः(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_वृक्षः_View_प्रक्षिप(&pView, 0);
    sqlite3_वृक्षः_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_वृक्षः_View_निष्कासय(&pView);
  }जाव( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_वृक्षः_View_निष्कासय(&pView);
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
शून्य sqlite3_Show_व्यञ्जकम्(ध्रुव व्यञ्जकम् *p){ sqlite3_वृक्षः_View_व्यञ्जकम्(0,p,0); }
शून्य sqlite3_Show_व्यञ्जकम्_List(ध्रुव व्यञ्जकम्_List *p){ sqlite3_वृक्षः_View_व्यञ्जकम्_List(0,p,0,0);}
शून्य sqlite3ShowIdList(ध्रुव IdList *p){ sqlite3_वृक्षः_View_Id_List(0,p,0,0); }
शून्य sqlite3ShowSrcList(ध्रुव SrcList *p){
  वृक्षः_View *pView = 0;
  sqlite3_वृक्षः_View_प्रक्षिप(&pView, 0);
  sqlite3_वृक्षः_View_पङ्क्तिः(pView, "SRCLIST");
  sqlite3_वृक्षः_View_Src_List(pView,p);
  sqlite3_वृक्षः_View_निष्कासय(&pView);
}
शून्य sqlite3ShowSelect(ध्रुव Select *p){ sqlite3_वृक्षः_View_Select(0,p,0); }
शून्य sqlite3ShowWith(ध्रुव With *p){ sqlite3_वृक्षः_View_With(0,p,0); }
शून्य sqlite3ShowUpsert(ध्रुव Upsert *p){ sqlite3_वृक्षः_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
शून्य sqlite3ShowTriggerStep(ध्रुव TriggerStep *p){
  sqlite3_वृक्षः_View_Trigger_Step(0,p,0,0);
}
शून्य sqlite3ShowTriggerStepList(ध्रुव TriggerStep *p){
  sqlite3_वृक्षः_View_Trigger_Step(0,p,0,1);
}
शून्य sqlite3ShowTrigger(ध्रुव Trigger *p){ sqlite3_वृक्षः_View_Trigger(0,p,0,0); }
शून्य sqlite3ShowTriggerList(ध्रुव Trigger *p){ sqlite3_वृक्षः_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
शून्य sqlite3ShowWindow(ध्रुव Window *p){ sqlite3_वृक्षः_View_Window(0,p,0); }
शून्य sqlite3ShowWinFunc(ध्रुव Window *p){ sqlite3_वृक्षः_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
