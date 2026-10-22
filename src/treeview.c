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
#समावेश "sqliteInt.h"
#अगर_घोषित SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
अटल व्योम sqlite3_वृक्ष_View_धकेलो(वृक्ष_View **pp, u8 moreToFollow){
  वृक्ष_View *p = *pp;
  अगर( p==0 ){
    *pp = p = sqlite3_malloc64( माप(*p) );
    अगर( p==0 ) वापस;
    स_रखो(p, 0, माप(*p));
  }अन्यथा{
    p->iLevel++;
  }
  निश्चित( moreToFollow==0 || moreToFollow==1 );
  अगर( p->iLevel<(पूर्णांक)माप(p->b_पंक्ति) ) p->b_पंक्ति[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
अटल व्योम sqlite3_वृक्ष_View_निकालो(वृक्ष_View **pp){
  वृक्ष_View *p = *pp;
  अगर( p==0 ) वापस;
  p->iLevel--;
  अगर( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
व्योम sqlite3_वृक्ष_View_पंक्ति(वृक्ष_View *p, स्थिर अक्षर *zFormat, ...){
  बहु_सूची ap;
  पूर्णांक i;
  StrAccum acc;
  अक्षर zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, माप(zBuf), 0);
  अगर( p ){
    क्रम(i=0; i<p->iLevel && i<(पूर्णांक)माप(p->b_पंक्ति)-1; i++){
      sqlite3_str_append(&acc, p->b_पंक्ति[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_पंक्ति[i] ? "|-- " : "'-- ", 4);
  }
  अगर( zFormat!=0 ){
    बहु_शुरू(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    बहु_पूर्ण(ap);
    निश्चित( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  ख_लिखो(मानक_निकास,"%s", zBuf);
  ख_साफ(मानक_निकास);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
अटल व्योम sqlite3_वृक्ष_View_Item(वृक्ष_View *p, स्थिर अक्षर *zLabel,u8 moreFollows){
  sqlite3_वृक्ष_View_धकेलो(&p, moreFollows);
  sqlite3_वृक्ष_View_पंक्ति(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
व्योम sqlite3_वृक्ष_View_Column_List(
  वृक्ष_View *pView,
  स्थिर Column *aCol,
  पूर्णांक nCol,
  u8 moreToFollow
){
  पूर्णांक i;
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  sqlite3_वृक्ष_View_पंक्ति(pView, "COLUMNS");
  क्रम(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    पूर्णांक colMoreToFollow = i<(nCol - 1);
    sqlite3_वृक्ष_View_धकेलो(&pView, colMoreToFollow);
    sqlite3_वृक्ष_View_पंक्ति(pView, 0);
    म_लिखो(" %s", aCol[i].zCnName);
    चयन( aCol[i].eCType ){
      हाल COLTYPE_ANY:      म_लिखो(" ANY");        अवरोध;
      हाल COLTYPE_BLOB:     म_लिखो(" BLOB");       अवरोध;
      हाल COLTYPE_INT:      म_लिखो(" INT");        अवरोध;
      हाल COLTYPE_INTEGER:  म_लिखो(" INTEGER");    अवरोध;
      हाल COLTYPE_REAL:     म_लिखो(" REAL");       अवरोध;
      हाल COLTYPE_TEXT:     म_लिखो(" TEXT");       अवरोध;
      हाल COLTYPE_CUSTOM: {
        अगर( flg & COLFLAG_HASTYPE ){
          स्थिर अक्षर *z = aCol[i].zCnName;
          z += म_माप(z)+1;
          म_लिखो(" X-%s", z);
          अवरोध;
        }
      }
    }
    अगर( flg & COLFLAG_PRIMKEY ) म_लिखो(" PRIMARY KEY");
    अगर( flg & COLFLAG_HIDDEN ) म_लिखो(" HIDDEN");
#अगर_घोषित COLFLAG_NOEXPAND
    अगर( flg & COLFLAG_NOEXPAND ) म_लिखो(" NO-EXPAND");
#पूर्ण_अगर
    अगर( flg ) म_लिखो(" flags=%04x", flg);
    म_लिखो("\n");      
    ख_साफ(मानक_निकास);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
व्योम sqlite3_वृक्ष_View_With(वृक्ष_View *pView, स्थिर With *pWith, u8 moreToFollow){
  पूर्णांक i;
  अगर( pWith==0 ) वापस;
  अगर( pWith->nCte==0 ) वापस;
  अगर( pWith->pOuter ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }अन्यथा{
    sqlite3_वृक्ष_View_पंक्ति(pView, "WITH (0x%p)", pWith);
  }
  अगर( pWith->nCte>0 ){
    sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
    क्रम(i=0; i<pWith->nCte; i++){
      StrAccum x;
      अक्षर z_पंक्ति[1000];
      स्थिर काष्ठा Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_पंक्ति, माप(z_पंक्ति), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      अगर( pCte->pCols && pCte->pCols->n_व्यंजक>0 ){
        अक्षर cSep = '(';
        पूर्णांक j;
        क्रम(j=0; j<pCte->pCols->n_व्यंजक; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      अगर( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      अगर( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_वृक्ष_View_Item(pView, z_पंक्ति, i<pWith->nCte-1);
      sqlite3_वृक्ष_View_Select(pView, pCte->pSelect, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
व्योम sqlite3_वृक्ष_View_Src_List(वृक्ष_View *pView, स्थिर SrcList *pSrc){
  पूर्णांक i;
  अगर( pSrc==0 ) वापस;
  क्रम(i=0; i<pSrc->nSrc; i++){
    स्थिर SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    पूर्णांक n = 0;
    अक्षर z_पंक्ति[1000];
    sqlite3StrAccumInit(&x, 0, z_पंक्ति, माप(z_पंक्ति), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    अगर( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    अगर( (pItem->fg.jointype & (JT_बायाँ|JT_दायाँ))==(JT_बायाँ|JT_दायाँ) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }अन्यथा अगर( pItem->fg.jointype & JT_बायाँ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }अन्यथा अगर( pItem->fg.jointype & JT_दायाँ ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }अन्यथा अगर( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    अगर( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    अगर( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    अगर( pItem->fg.isCte ){
      अटल स्थिर अक्षर *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    अगर( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    अगर( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    अगर( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    अगर( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    अगर( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    अगर( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    अगर( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    अगर( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    अगर( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    अगर( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_वृक्ष_View_Item(pView, z_पंक्ति, i<pSrc->nSrc-1);
    n = 0;
    अगर( pItem->fg.isSubquery ) n++;
    अगर( pItem->fg.isTabFunc ) n++;
    अगर( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    अगर( pItem->fg.isUsing ){
      sqlite3_वृक्ष_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }अन्यथा अगर( pItem->u3.pOn!=0 ){
      sqlite3_वृक्ष_View_Item(pView, "ON", (--n)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, pItem->u3.pOn, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    अगर( pItem->fg.isSubquery ){
      निश्चित( n==1 );
      अगर( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_वृक्ष_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      निश्चित( (पूर्णांक)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_वृक्ष_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    अगर( pItem->fg.isTabFunc ){
      sqlite3_वृक्ष_View_व्यंजक_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
व्योम sqlite3_वृक्ष_View_Select(वृक्ष_View *pView, स्थिर Select *p, u8 moreToFollow){
  पूर्णांक n = 0;
  पूर्णांक cnt = 0;
  अगर( p==0 ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "nil-SELECT");
    वापस;
  } 
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  अगर( p->pWith ){
    sqlite3_वृक्ष_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_वृक्ष_View_धकेलो(&pView, 1);
  }
  करो{
    अगर( p->selFlags & SF_WhereBegin ){
      sqlite3_वृक्ष_View_पंक्ति(pView, "sqlite3WhereBegin()");
    }अन्यथा{
      sqlite3_वृक्ष_View_पंक्ति(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (पूर्णांक)p->nSelectRow
      );
    }
    अगर( cnt++ ) sqlite3_वृक्ष_View_निकालो(&pView);
    अगर( p->pPrior ){
      n = 1000;
    }अन्यथा{
      n = 0;
      अगर( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      अगर( p->pWhere ) n++;
      अगर( p->pGroupBy ) n++;
      अगर( p->pHaving ) n++;
      अगर( p->pOrderBy ) n++;
      अगर( p->pLimit ) n++;
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
      अगर( p->pWin ) n++;
      अगर( p->pWinDefn ) n++;
#पूर्ण_अगर
    }
    अगर( p->pEList ){
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
    अगर( p->pWin ){
      Window *pX;
      sqlite3_वृक्ष_View_धकेलो(&pView, (n--)>0);
      sqlite3_वृक्ष_View_पंक्ति(pView, "window-functions");
      क्रम(pX=p->pWin; pX; pX=pX->p_अग्र_Win){
        sqlite3_वृक्ष_View_Win_Func(pView, pX, pX->p_अग्र_Win!=0);
      }
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
#पूर्ण_अगर
    अगर( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_वृक्ष_View_धकेलो(&pView, (n--)>0);
      sqlite3_वृक्ष_View_पंक्ति(pView, "FROM");
      sqlite3_वृक्ष_View_Src_List(pView, p->pSrc);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    अगर( p->pWhere ){
      sqlite3_वृक्ष_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, p->pWhere, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    अगर( p->pGroupBy ){
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    अगर( p->pHaving ){
      sqlite3_वृक्ष_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, p->pHaving, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
    अगर( p->pWinDefn ){
      Window *pX;
      sqlite3_वृक्ष_View_Item(pView, "WINDOW", (n--)>0);
      क्रम(pX=p->pWinDefn; pX; pX=pX->p_अग्र_Win){
        sqlite3_वृक्ष_View_Window(pView, pX, pX->p_अग्र_Win!=0);
      }
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
#पूर्ण_अगर
    अगर( p->pOrderBy ){
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    अगर( p->pLimit ){
      sqlite3_वृक्ष_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, p->pLimit->p_बायाँ, p->pLimit->p_दायाँ!=0);
      अगर( p->pLimit->p_दायाँ ){
        sqlite3_वृक्ष_View_Item(pView, "OFFSET", 0);
        sqlite3_वृक्ष_View_व्यंजक(pView, p->pLimit->p_दायाँ, 0);
        sqlite3_वृक्ष_View_निकालो(&pView);
      }
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    अगर( p->pPrior ){
      स्थिर अक्षर *zOp = "UNION";
      चयन( p->op ){
        हाल TK_ALL:         zOp = "UNION ALL";  अवरोध;
        हाल TK_INTERSECT:   zOp = "INTERSECT";  अवरोध;
        हाल TK_EXCEPT:      zOp = "EXCEPT";     अवरोध;
      }
      sqlite3_वृक्ष_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }जबतक( p!=0 );
  sqlite3_वृक्ष_View_निकालो(&pView);
}

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
व्योम sqlite3_वृक्ष_View_Bound(
  वृक्ष_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  व्यंजक *p_व्यंजक,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  चयन( eBound ){
    हाल TK_UNBOUNDED: {
      sqlite3_वृक्ष_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_वृक्ष_View_निकालो(&pView);
      अवरोध;
    }
    हाल TK_CURRENT: {
      sqlite3_वृक्ष_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_वृक्ष_View_निकालो(&pView);
      अवरोध;
    }
    हाल TK_PRECEDING: {
      sqlite3_वृक्ष_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
      अवरोध;
    }
    हाल TK_FOLLOWING: {
      sqlite3_वृक्ष_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
      अवरोध;
    }
  }
}
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
व्योम sqlite3_वृक्ष_View_Window(वृक्ष_View *pView, स्थिर Window *pWin, u8 more){
  पूर्णांक nElement = 0;
  अगर( pWin==0 ) वापस;
  अगर( pWin->pFilter ){
    sqlite3_वृक्ष_View_Item(pView, "FILTER", 1);
    sqlite3_वृक्ष_View_व्यंजक(pView, pWin->pFilter, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
    अगर( pWin->eFrmType==TK_FILTER ) वापस;
  }
  sqlite3_वृक्ष_View_धकेलो(&pView, more);
  अगर( pWin->zName ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "OVER %s (%p)", pWin->zName, pWin);
  }अन्यथा{
    sqlite3_वृक्ष_View_पंक्ति(pView, "OVER (%p)", pWin);
  }
  अगर( pWin->zBase )    nElement++;
  अगर( pWin->pOrderBy ) nElement++;
  अगर( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  अगर( pWin->eExclude ) nElement++;
  अगर( pWin->zBase ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--nElement)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "window: %s", pWin->zBase);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pWin->pPartition ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  अगर( pWin->pOrderBy ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  अगर( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    अक्षर zBuf[30];
    स्थिर अक्षर *zFrmType = "ROWS";
    अगर( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    अगर( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(माप(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_वृक्ष_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_वृक्ष_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_वृक्ष_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pWin->eExclude ){
    अक्षर zBuf[30];
    स्थिर अक्षर *zExclude;
    चयन( pWin->eExclude ){
      हाल TK_NO:      zExclude = "NO OTHERS";   अवरोध;
      हाल TK_CURRENT: zExclude = "CURRENT ROW"; अवरोध;
      हाल TK_GROUP:   zExclude = "GROUP";       अवरोध;
      हाल TK_TIES:    zExclude = "TIES";        अवरोध;
      शेष:
        sqlite3_snprintf(माप(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        अवरोध;
    }
    sqlite3_वृक्ष_View_धकेलो(&pView, 0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "EXCLUDE %s", zExclude);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */

#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
व्योम sqlite3_वृक्ष_View_Win_Func(वृक्ष_View *pView, स्थिर Window *pWin, u8 more){
  अगर( pWin==0 ) वापस;
  sqlite3_वृक्ष_View_धकेलो(&pView, more);
  sqlite3_वृक्ष_View_पंक्ति(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_वृक्ष_View_Window(pView, pWin, 0);
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#पूर्ण_अगर /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
व्योम sqlite3_वृक्ष_View_व्यंजक(वृक्ष_View *pView, स्थिर व्यंजक *p_व्यंजक, u8 moreToFollow){
  स्थिर अक्षर *zBinOp = 0;   /* Binary operator */
  स्थिर अक्षर *zUniOp = 0;   /* Unary operator */
  अक्षर zFlgs[200];
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  अगर( p_व्यंजक==0 ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "nil");
    sqlite3_वृक्ष_View_निकालो(&pView);
    वापस;
  }
  अगर( p_व्यंजक->flags || p_व्यंजक->aff_व्यंजक || p_व्यंजक->vvaFlags || p_व्यंजक->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, माप(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_व्यंजक->flags, p_व्यंजक->aff_व्यंजक ? p_व्यंजक->aff_व्यंजक : 'n');
    अगर( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_व्यंजक->w.iJoin);
    }
    अगर( व्यंजक_Has_Property(p_व्यंजक, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_व्यंजक->w.iJoin);
    }
    अगर( व्यंजक_Has_Property(p_व्यंजक, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    अगर( व्यंजक_Has_VVAProperty(p_व्यंजक, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    अगर( p_व्यंजक->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_व्यंजक->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }अन्यथा{
    zFlgs[0] = 0;
  }
  चयन( p_व्यंजक->op ){
    हाल TK_AGG_COLUMN: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "AGG{%d:%d}%s",
            p_व्यंजक->iTable, p_व्यंजक->iColumn, zFlgs);
      अवरोध;
    }
    हाल TK_COLUMN: {
      अगर( p_व्यंजक->iTable<0 ){
        /* This only happens when coding check constraints */
        अक्षर zOp2[16];
        अगर( p_व्यंजक->op2 ){
          sqlite3_snprintf(माप(zOp2),zOp2," op2=0x%02x",p_व्यंजक->op2);
        }अन्यथा{
          zOp2[0] = 0;
        }
        sqlite3_वृक्ष_View_पंक्ति(pView, "COLUMN(%d)%s%s",
                                    p_व्यंजक->iColumn, zFlgs, zOp2);
      }अन्यथा{
        निश्चित( व्यंजक_Use_YTab(p_व्यंजक) );
        sqlite3_वृक्ष_View_पंक्ति(pView, "{%d:%d} pTab=%p%s",
                        p_व्यंजक->iTable, p_व्यंजक->iColumn,
                        p_व्यंजक->y.pTab, zFlgs);
      }
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_FixedCol) ){
        sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      }
      अवरोध;
    }
    हाल TK_INTEGER: {
      अगर( p_व्यंजक->flags & EP_Int_मूल्य ){
        sqlite3_वृक्ष_View_पंक्ति(pView, "%d", p_व्यंजक->u.i_मूल्य);
      }अन्यथा{
        sqlite3_वृक्ष_View_पंक्ति(pView, "%s", p_व्यंजक->u.zToken);
      }
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_FLOATING_POINT
    हाल TK_FLOAT: {
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"%s", p_व्यंजक->u.zToken);
      अवरोध;
    }
#पूर्ण_अगर
    हाल TK_STRING: {
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"%Q", p_व्यंजक->u.zToken);
      अवरोध;
    }
    हाल TK_NULL: {
      sqlite3_वृक्ष_View_पंक्ति(pView,"NULL");
      अवरोध;
    }
    हाल TK_TRUEFALSE: {
      sqlite3_वृक्ष_View_पंक्ति(pView,"%s%s",
         sqlite3_व्यंजक_Truth_मूल्य(p_व्यंजक) ? "TRUE" : "FALSE", zFlgs);
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_BLOB_LITERAL
    हाल TK_BLOB: {
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"%s", p_व्यंजक->u.zToken);
      अवरोध;
    }
#पूर्ण_अगर
    हाल TK_VARIABLE: {
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"VARIABLE(%s,%d)",
                          p_व्यंजक->u.zToken, p_व्यंजक->iColumn);
      अवरोध;
    }
    हाल TK_REGISTER: {
      sqlite3_वृक्ष_View_पंक्ति(pView,"REGISTER(%d)", p_व्यंजक->iTable);
      अवरोध;
    }
    हाल TK_ID: {
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"ID \"%w\"", p_व्यंजक->u.zToken);
      अवरोध;
    }
    हाल TK_ASTERISK: {
      sqlite3_वृक्ष_View_पंक्ति(pView,"ASKTERISK");
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_CAST
    हाल TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"CAST %Q", p_व्यंजक->u.zToken);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      अवरोध;
    }
#पूर्ण_अगर /* SQLITE_OMIT_CAST */
    हाल TK_LT:      zBinOp = "LT";     अवरोध;
    हाल TK_LE:      zBinOp = "LE";     अवरोध;
    हाल TK_GT:      zBinOp = "GT";     अवरोध;
    हाल TK_GE:      zBinOp = "GE";     अवरोध;
    हाल TK_NE:      zBinOp = "NE";     अवरोध;
    हाल TK_EQ:      zBinOp = "EQ";     अवरोध;
    हाल TK_IS:      zBinOp = "IS";     अवरोध;
    हाल TK_ISNOT:   zBinOp = "ISNOT";  अवरोध;
    हाल TK_AND:     zBinOp = "AND";    अवरोध;
    हाल TK_OR:      zBinOp = "OR";     अवरोध;
    हाल TK_PLUS:    zBinOp = "ADD";    अवरोध;
    हाल TK_STAR:    zBinOp = "MUL";    अवरोध;
    हाल TK_MINUS:   zBinOp = "SUB";    अवरोध;
    हाल TK_REM:     zBinOp = "REM";    अवरोध;
    हाल TK_BITAND:  zBinOp = "BITAND"; अवरोध;
    हाल TK_BITOR:   zBinOp = "BITOR";  अवरोध;
    हाल TK_SLASH:   zBinOp = "DIV";    अवरोध;
    हाल TK_LSHIFT:  zBinOp = "LSHIFT"; अवरोध;
    हाल TK_RSHIFT:  zBinOp = "RSHIFT"; अवरोध;
    हाल TK_CONCAT:  zBinOp = "CONCAT"; अवरोध;
    हाल TK_DOT:     zBinOp = "DOT";    अवरोध;
    हाल TK_LIMIT:   zBinOp = "LIMIT";  अवरोध;

    हाल TK_UMINUS:  zUniOp = "UMINUS"; अवरोध;
    हाल TK_UPLUS:   zUniOp = "UPLUS";  अवरोध;
    हाल TK_BITNOT:  zUniOp = "BITNOT"; अवरोध;
    हाल TK_NOT:     zUniOp = "NOT";    अवरोध;
    हाल TK_ISNULL:  zUniOp = "ISNULL"; अवरोध;
    हाल TK_NOTNULL: zUniOp = "NOTNULL"; अवरोध;

    हाल TK_TRUTH: {
      पूर्णांक x;
      स्थिर अक्षर *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      निश्चित( p_व्यंजक->op2==TK_IS || p_व्यंजक->op2==TK_ISNOT );
      निश्चित( p_व्यंजक->p_दायाँ );
      निश्चित( sqlite3_व्यंजक_Skip_Collate_And_Likely(p_व्यंजक->p_दायाँ)->op
                  == TK_TRUEFALSE );
      x = (p_व्यंजक->op2==TK_ISNOT)*2 + sqlite3_व्यंजक_Truth_मूल्य(p_व्यंजक->p_दायाँ);
      zUniOp = azOp[x];
      अवरोध;
    }

    हाल TK_SPAN: {
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "SPAN %Q", p_व्यंजक->u.zToken);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      अवरोध;
    }

    हाल TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "%sCOLLATE %Q%s",
        !व्यंजक_Has_Property(p_व्यंजक, EP_Collate) ? "SOFT-" : "",
        p_व्यंजक->u.zToken, zFlgs);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      अवरोध;
    }

    हाल TK_AGG_FUNCTION:
    हाल TK_FUNCTION: {
      व्यंजक_List *pFarg;       /* List of function arguments */
      Window *pWin;
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }अन्यथा{
        निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
        pFarg = p_व्यंजक->x.pList;
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_व्यंजक) ? p_व्यंजक->y.pWin : 0;
#अन्यथा
        pWin = 0;
#पूर्ण_अगर 
      }
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      अगर( p_व्यंजक->op==TK_AGG_FUNCTION ){
        sqlite3_वृक्ष_View_पंक्ति(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_व्यंजक->op2, p_व्यंजक->u.zToken, zFlgs,
                             p_व्यंजक->pAggInfo ? p_व्यंजक->pAggInfo->selId : 0,
                             p_व्यंजक->iAgg, p_व्यंजक->pAggInfo);
      }अन्यथा अगर( p_व्यंजक->op2!=0 ){
        स्थिर अक्षर *zOp2;
        अक्षर zBuf[8];
        sqlite3_snprintf(माप(zBuf),zBuf,"0x%02x",p_व्यंजक->op2);
        zOp2 = zBuf;
        अगर( p_व्यंजक->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        अगर( p_व्यंजक->op2==NC_Idx_व्यंजक ) zOp2 = "NC_IdxExpr";
        अगर( p_व्यंजक->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        अगर( p_व्यंजक->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_वृक्ष_View_पंक्ति(pView, "FUNCTION %Q%s op2=%s",
                            p_व्यंजक->u.zToken, zFlgs, zOp2);
      }अन्यथा{
        sqlite3_वृक्ष_View_पंक्ति(pView, "FUNCTION %Q%s", p_व्यंजक->u.zToken, zFlgs);
      }
      अगर( pFarg ){
        sqlite3_वृक्ष_View_व्यंजक_List(pView, pFarg, pWin!=0 || p_व्यंजक->p_बायाँ, 0);
        अगर( p_व्यंजक->p_बायाँ ){
          व्यंजक *pOB = p_व्यंजक->p_बायाँ;
          निश्चित( pOB->op==TK_ORDER );
          निश्चित( व्यंजक_Use_XList(pOB) );
          sqlite3_वृक्ष_View_व्यंजक_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
      अगर( pWin ){
        sqlite3_वृक्ष_View_Window(pView, pWin, 0);
      }
#पूर्ण_अगर
      अवरोध;
    }
    हाल TK_ORDER: {
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p_व्यंजक->x.pList, 0, "ORDERBY");
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_SUBQUERY
    हाल TK_EXISTS: {
      निश्चित( व्यंजक_Use_XSelect(p_व्यंजक) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "EXISTS-expr flags=0x%x", p_व्यंजक->flags);
      sqlite3_वृक्ष_View_Select(pView, p_व्यंजक->x.pSelect, 0);
      अवरोध;
    }
    हाल TK_SELECT: {
      निश्चित( व्यंजक_Use_XSelect(p_व्यंजक) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "subquery-expr flags=0x%x", p_व्यंजक->flags);
      sqlite3_वृक्ष_View_Select(pView, p_व्यंजक->x.pSelect, 0);
      अवरोध;
    }
    हाल TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      अक्षर *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_व्यंजक->flags);
      अगर( p_व्यंजक->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_व्यंजक->iTable);
      अगर( व्यंजक_Has_Property(p_व्यंजक, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_व्यंजक->y.sub.regReturn, p_व्यंजक->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_वृक्ष_View_पंक्ति(pView, z);
      sqlite3_free(z);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 1);
      अगर( व्यंजक_Use_XSelect(p_व्यंजक) ){
        sqlite3_वृक्ष_View_Select(pView, p_व्यंजक->x.pSelect, 0);
      }अन्यथा{
        sqlite3_वृक्ष_View_व्यंजक_List(pView, p_व्यंजक->x.pList, 0, 0);
      }
      अवरोध;
    }
#पूर्ण_अगर /* SQLITE_OMIT_SUBQUERY */

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
    हाल TK_BETWEEN: {
      स्थिर व्यंजक *pX, *pY, *pZ;
      pX = p_व्यंजक->p_बायाँ;
      निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
      निश्चित( p_व्यंजक->x.pList->n_व्यंजक==2 );
      pY = p_व्यंजक->x.pList->a[0].p_व्यंजक;
      pZ = p_व्यंजक->x.pList->a[1].p_व्यंजक;
      sqlite3_वृक्ष_View_पंक्ति(pView, "BETWEEN%s", zFlgs);
      sqlite3_वृक्ष_View_व्यंजक(pView, pX, 1);
      sqlite3_वृक्ष_View_व्यंजक(pView, pY, 1);
      sqlite3_वृक्ष_View_व्यंजक(pView, pZ, 0);
      अवरोध;
    }
    हाल TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_वृक्ष_View_पंक्ति(pView, "%s(%d)", 
          p_व्यंजक->iTable ? "NEW" : "OLD", p_व्यंजक->iColumn);
      अवरोध;
    }
    हाल TK_CASE: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "CASE");
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 1);
      निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p_व्यंजक->x.pList, 0, 0);
      अवरोध;
    }
#अगर_अघोषित SQLITE_OMIT_TRIGGER
    हाल TK_RAISE: {
      स्थिर अक्षर *zType = "unk";
      चयन( p_व्यंजक->aff_व्यंजक ){
        हाल OE_Rollback:   zType = "rollback";  अवरोध;
        हाल OE_Abort:      zType = "abort";     अवरोध;
        हाल OE_Fail:       zType = "fail";      अवरोध;
        हाल OE_Ignore:     zType = "ignore";    अवरोध;
      }
      निश्चित( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "RAISE %s", zType);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      अवरोध;
    }
#पूर्ण_अगर
    हाल TK_MATCH: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "MATCH {%d:%d}%s",
                          p_व्यंजक->iTable, p_व्यंजक->iColumn, zFlgs);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_दायाँ, 0);
      अवरोध;
    }
    हाल TK_VECTOR: {
      अक्षर *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      निश्चित( व्यंजक_Use_XList(p_व्यंजक) );
      sqlite3_वृक्ष_View_Bare_व्यंजक_List(pView, p_व्यंजक->x.pList, z);
      sqlite3_free(z);
      अवरोध;
    }
    हाल TK_SELECT_COLUMN: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_व्यंजक->iColumn, p_व्यंजक->iTable-1,
              p_व्यंजक->p_दायाँ==p_व्यंजक->p_बायाँ ? " (SELECT-owner)" : "");
      निश्चित( व्यंजक_Use_XSelect(p_व्यंजक->p_बायाँ) );
      sqlite3_वृक्ष_View_Select(pView, p_व्यंजक->p_बायाँ->x.pSelect, 0);
      अवरोध;
    }
    हाल TK_IF_NULL_ROW: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "IF-NULL-ROW %d", p_व्यंजक->iTable);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      अवरोध;
    }
    हाल TK_ERROR: {
      व्यंजक tmp;
      sqlite3_वृक्ष_View_पंक्ति(pView, "ERROR");
      tmp = *p_व्यंजक;
      tmp.op = p_व्यंजक->op2;
      sqlite3_वृक्ष_View_व्यंजक(pView, &tmp, 0);
      अवरोध;
    }
    हाल TK_ROW: {
      अगर( p_व्यंजक->iColumn<=0 ){
        sqlite3_वृक्ष_View_पंक्ति(pView, "First FROM table rowid");
      }अन्यथा{
        sqlite3_वृक्ष_View_पंक्ति(pView, "First FROM table column %d",
            p_व्यंजक->iColumn-1);
      }
      अवरोध;
    }
    शेष: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "op=%d", p_व्यंजक->op);
      अवरोध;
    }
  }
  अगर( zBinOp ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 1);
    sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_दायाँ, 0);
  }अन्यथा अगर( zUniOp ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
व्योम sqlite3_वृक्ष_View_Bare_व्यंजक_List(
  वृक्ष_View *pView,
  स्थिर व्यंजक_List *pList,
  स्थिर अक्षर *zLabel
){
  अगर( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  अगर( pList==0 ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s (empty)", zLabel);
  }अन्यथा{
    पूर्णांक i;
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s", zLabel);
    क्रम(i=0; i<pList->n_व्यंजक; i++){
      पूर्णांक j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      अक्षर *zName = pList->a[i].zEName;
      पूर्णांक moreToFollow = i<pList->n_व्यंजक - 1;
      अगर( j || zName || sortFlags ){
        sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_वृक्ष_View_पंक्ति(pView, 0);
        अगर( zName ){
          चयन( pList->a[i].fg.eEName ){
            शेष:
              ख_लिखो(मानक_निकास, "AS %s ", zName);
              अवरोध;
            हाल ENAME_TAB:
              ख_लिखो(मानक_निकास, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              अगर( pList->a[i].fg.bUsed ) ख_लिखो(मानक_निकास, "(used) ");
              अगर( pList->a[i].fg.bUsingTerm ) ख_लिखो(मानक_निकास, "(USING-term) ");
              अगर( pList->a[i].fg.bNoExpand ) ख_लिखो(मानक_निकास, "(NoExpand) ");
              अवरोध;
            हाल ENAME_SPAN:
              ख_लिखो(मानक_निकास, "SPAN(\"%s\") ", zName);
              अवरोध;
          }
        }
        अगर( j ){
          ख_लिखो(मानक_निकास, "iOrderByCol=%d ", j);
        }
        अगर( sortFlags & KEYINFO_ORDER_DESC ){
          ख_लिखो(मानक_निकास, "DESC ");
        }अन्यथा अगर( sortFlags & KEYINFO_ORDER_BIGNULL ){
          ख_लिखो(मानक_निकास, "NULLS-LAST");
        }
        ख_लिखो(मानक_निकास, "\n");
        ख_साफ(मानक_निकास);
      }
      sqlite3_वृक्ष_View_व्यंजक(pView, pList->a[i].p_व्यंजक, moreToFollow);
      अगर( j || zName || sortFlags ){
        sqlite3_वृक्ष_View_निकालो(&pView);
      }
    }
  }
}
व्योम sqlite3_वृक्ष_View_व्यंजक_List(
  वृक्ष_View *pView,
  स्थिर व्यंजक_List *pList,
  u8 moreToFollow,
  स्थिर अक्षर *zLabel
){
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  sqlite3_वृक्ष_View_Bare_व्यंजक_List(pView, pList, zLabel);
  sqlite3_वृक्ष_View_निकालो(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
व्योम sqlite3_वृक्ष_View_Bare_Id_List(
  वृक्ष_View *pView,
  स्थिर IdList *pList,
  स्थिर अक्षर *zLabel
){
  अगर( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  अगर( pList==0 ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s (empty)", zLabel);
  }अन्यथा{
    पूर्णांक i;
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s", zLabel);
    क्रम(i=0; i<pList->nId; i++){
      अक्षर *zName = pList->a[i].zName;
      पूर्णांक moreToFollow = i<pList->nId - 1;
      अगर( zName==0 ) zName = "(null)";
      sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
      sqlite3_वृक्ष_View_पंक्ति(pView, 0);
      ख_लिखो(मानक_निकास, "%s\n", zName);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
  }
}
व्योम sqlite3_वृक्ष_View_Id_List(
  वृक्ष_View *pView,
  स्थिर IdList *pList,
  u8 moreToFollow,
  स्थिर अक्षर *zLabel
){
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  sqlite3_वृक्ष_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_वृक्ष_View_निकालो(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
व्योम sqlite3_वृक्ष_View_Upsert(
  वृक्ष_View *pView,
  स्थिर Upsert *pUpsert,
  u8 moreToFollow
){
  अगर( pUpsert==0 ) वापस;
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  जबतक( pUpsert ){
    पूर्णांक n;
    sqlite3_वृक्ष_View_धकेलो(&pView, pUpsert->p_अग्र_Upsert!=0 || moreToFollow);
    sqlite3_वृक्ष_View_पंक्ति(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    अगर( pUpsert->pUpsertWhere ){
      sqlite3_वृक्ष_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    sqlite3_वृक्ष_View_निकालो(&pView);
    pUpsert = pUpsert->p_अग्र_Upsert;
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}

#अगर TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
व्योम sqlite3_वृक्ष_View_Delete(
  स्थिर With *pWith,
  स्थिर SrcList *pTabList,
  स्थिर व्यंजक *pWhere,
  स्थिर व्यंजक_List *pOrderBy,
  स्थिर व्यंजक *pLimit,
  स्थिर Trigger *pTrigger
){
  पूर्णांक n = 0;
  वृक्ष_View *pView = 0;
  sqlite3_वृक्ष_View_धकेलो(&pView, 0);
  sqlite3_वृक्ष_View_पंक्ति(pView, "DELETE");
  अगर( pWith ) n++;
  अगर( pTabList ) n++;
  अगर( pWhere ) n++;
  अगर( pOrderBy ) n++;
  अगर( pLimit ) n++;
  अगर( pTrigger ) n++;
  अगर( pWith ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_With(pView, pWith, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pTabList ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "FROM");
    sqlite3_वृक्ष_View_Src_List(pView, pTabList);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pWhere ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "WHERE");
    sqlite3_वृक्ष_View_व्यंजक(pView, pWhere, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pOrderBy ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  अगर( pLimit ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "LIMIT");
    sqlite3_वृक्ष_View_व्यंजक(pView, pLimit, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pTrigger ){
    sqlite3_वृक्ष_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#पूर्ण_अगर /* TREETRACE_ENABLED */

#अगर TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
व्योम sqlite3_वृक्ष_View_जोड़ो(
  स्थिर With *pWith,
  स्थिर SrcList *pTabList,
  स्थिर IdList *pColumnList,
  स्थिर Select *pSelect,
  स्थिर व्यंजक_List *p_व्यंजक_List,
  पूर्णांक onError,
  स्थिर Upsert *pUpsert,
  स्थिर Trigger *pTrigger
){
  वृक्ष_View *pView = 0;
  पूर्णांक n = 0;
  स्थिर अक्षर *zLabel = "INSERT";
  चयन( onError ){
    हाल OE_Replace:  zLabel = "REPLACE";             अवरोध;
    हाल OE_Ignore:   zLabel = "INSERT OR IGNORE";    अवरोध;
    हाल OE_Rollback: zLabel = "INSERT OR ROLLBACK";  अवरोध;
    हाल OE_Abort:    zLabel = "INSERT OR ABORT";     अवरोध;
    हाल OE_Fail:     zLabel = "INSERT OR FAIL";      अवरोध;
  }
  sqlite3_वृक्ष_View_धकेलो(&pView, 0);
  sqlite3_वृक्ष_View_पंक्ति(pView, zLabel);
  अगर( pWith ) n++;
  अगर( pTabList ) n++;
  अगर( pColumnList ) n++;
  अगर( pSelect ) n++;
  अगर( p_व्यंजक_List ) n++;
  अगर( pUpsert ) n++;
  अगर( pTrigger ) n++;
  अगर( pWith ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_With(pView, pWith, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pTabList ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "INTO");
    sqlite3_वृक्ष_View_Src_List(pView, pTabList);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pColumnList ){
    sqlite3_वृक्ष_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  अगर( pSelect ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "DATA-SOURCE");
    sqlite3_वृक्ष_View_Select(pView, pSelect, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( p_व्यंजक_List ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, p_व्यंजक_List, (--n)>0, "VALUES");
  }
  अगर( pUpsert ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "UPSERT");
    sqlite3_वृक्ष_View_Upsert(pView, pUpsert, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pTrigger ){
    sqlite3_वृक्ष_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#पूर्ण_अगर /* TREETRACE_ENABLED */

#अगर TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
व्योम sqlite3_वृक्ष_View_Update(
  स्थिर With *pWith,
  स्थिर SrcList *pTabList,
  स्थिर व्यंजक_List *pChanges,
  स्थिर व्यंजक *pWhere,
  पूर्णांक onError,
  स्थिर व्यंजक_List *pOrderBy,
  स्थिर व्यंजक *pLimit,
  स्थिर Upsert *pUpsert,
  स्थिर Trigger *pTrigger
){
  पूर्णांक n = 0;
  वृक्ष_View *pView = 0;
  स्थिर अक्षर *zLabel = "UPDATE";
  चयन( onError ){
    हाल OE_Replace:  zLabel = "UPDATE OR REPLACE";   अवरोध;
    हाल OE_Ignore:   zLabel = "UPDATE OR IGNORE";    अवरोध;
    हाल OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  अवरोध;
    हाल OE_Abort:    zLabel = "UPDATE OR ABORT";     अवरोध;
    हाल OE_Fail:     zLabel = "UPDATE OR FAIL";      अवरोध;
  }
  sqlite3_वृक्ष_View_धकेलो(&pView, 0);
  sqlite3_वृक्ष_View_पंक्ति(pView, zLabel);
  अगर( pWith ) n++;
  अगर( pTabList ) n++;
  अगर( pChanges ) n++;
  अगर( pWhere ) n++;
  अगर( pOrderBy ) n++;
  अगर( pLimit ) n++;
  अगर( pUpsert ) n++;
  अगर( pTrigger ) n++;
  अगर( pWith ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_With(pView, pWith, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pTabList ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "FROM");
    sqlite3_वृक्ष_View_Src_List(pView, pTabList);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pChanges ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pChanges, (--n)>0, "SET");
  }
  अगर( pWhere ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "WHERE");
    sqlite3_वृक्ष_View_व्यंजक(pView, pWhere, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pOrderBy ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  अगर( pLimit ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "LIMIT");
    sqlite3_वृक्ष_View_व्यंजक(pView, pLimit, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pUpsert ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "UPSERT");
    sqlite3_वृक्ष_View_Upsert(pView, pUpsert, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  अगर( pTrigger ){
    sqlite3_वृक्ष_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#पूर्ण_अगर /* TREETRACE_ENABLED */

#अगर_अघोषित SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
व्योम sqlite3_वृक्ष_View_Trigger_Step(
  वृक्ष_View *pView,
  स्थिर TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णांक cnt = 0;
  अगर( pStep==0 ) वापस;
  sqlite3_वृक्ष_View_धकेलो(&pView, 
      moreToFollow || (showFullList && pStep->p_अग्र!=0));
  करो{
    अगर( cnt++ && pStep->p_अग्र==0 ){
      sqlite3_वृक्ष_View_निकालो(&pView);
      sqlite3_वृक्ष_View_धकेलो(&pView, 0);
    }
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }जबतक( showFullList && (pStep = pStep->p_अग्र)!=0 );
  sqlite3_वृक्ष_View_निकालो(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
व्योम sqlite3_वृक्ष_View_Trigger(
  वृक्ष_View *pView,
  स्थिर Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णांक cnt = 0;
  अगर( pTrigger==0 ) वापस;
  sqlite3_वृक्ष_View_धकेलो(&pView,
     moreToFollow || (showFullList && pTrigger->p_अग्र!=0));
  करो{
    अगर( cnt++ && pTrigger->p_अग्र==0 ){
      sqlite3_वृक्ष_View_निकालो(&pView);
      sqlite3_वृक्ष_View_धकेलो(&pView, 0);
    }
    sqlite3_वृक्ष_View_पंक्ति(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_वृक्ष_View_धकेलो(&pView, 0);
    sqlite3_वृक्ष_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }जबतक( showFullList && (pTrigger = pTrigger->p_अग्र)!=0 );
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#पूर्ण_अगर /* SQLITE_OMIT_TRIGGER */
  

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
व्योम sqlite3_Show_व्यंजक(स्थिर व्यंजक *p){ sqlite3_वृक्ष_View_व्यंजक(0,p,0); }
व्योम sqlite3_Show_व्यंजक_List(स्थिर व्यंजक_List *p){ sqlite3_वृक्ष_View_व्यंजक_List(0,p,0,0);}
व्योम sqlite3ShowIdList(स्थिर IdList *p){ sqlite3_वृक्ष_View_Id_List(0,p,0,0); }
व्योम sqlite3ShowSrcList(स्थिर SrcList *p){
  वृक्ष_View *pView = 0;
  sqlite3_वृक्ष_View_धकेलो(&pView, 0);
  sqlite3_वृक्ष_View_पंक्ति(pView, "SRCLIST");
  sqlite3_वृक्ष_View_Src_List(pView,p);
  sqlite3_वृक्ष_View_निकालो(&pView);
}
व्योम sqlite3ShowSelect(स्थिर Select *p){ sqlite3_वृक्ष_View_Select(0,p,0); }
व्योम sqlite3ShowWith(स्थिर With *p){ sqlite3_वृक्ष_View_With(0,p,0); }
व्योम sqlite3ShowUpsert(स्थिर Upsert *p){ sqlite3_वृक्ष_View_Upsert(0,p,0); }
#अगर_अघोषित SQLITE_OMIT_TRIGGER
व्योम sqlite3ShowTriggerStep(स्थिर TriggerStep *p){
  sqlite3_वृक्ष_View_Trigger_Step(0,p,0,0);
}
व्योम sqlite3ShowTriggerStepList(स्थिर TriggerStep *p){
  sqlite3_वृक्ष_View_Trigger_Step(0,p,0,1);
}
व्योम sqlite3ShowTrigger(स्थिर Trigger *p){ sqlite3_वृक्ष_View_Trigger(0,p,0,0); }
व्योम sqlite3ShowTriggerList(स्थिर Trigger *p){ sqlite3_वृक्ष_View_Trigger(0,p,0,1);}
#पूर्ण_अगर
#अगर_अघोषित SQLITE_OMIT_WINDOWFUNC
व्योम sqlite3ShowWindow(स्थिर Window *p){ sqlite3_वृक्ष_View_Window(0,p,0); }
व्योम sqlite3ShowWinFunc(स्थिर Window *p){ sqlite3_वृक्ष_View_Win_Func(0,p,0); }
#पूर्ण_अगर

#पूर्ण_अगर /* SQLITE_DEBUG */
