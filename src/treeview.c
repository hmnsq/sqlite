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
स्थिर शून्य sqlite3_वृक्ष_View_धकेलो(वृक्ष_View **pp, u8 moreToFollow){
  वृक्ष_View *p = *pp;
  यदि( p==0 ){
    *pp = p = sqlite3_malloc64( परिमाण(*p) );
    यदि( p==0 ) प्रत्यागम;
    स_रखो(p, 0, परिमाण(*p));
  }अन्यथा{
    p->iLevel++;
  }
  प्रतिज्ञा( moreToFollow==0 || moreToFollow==1 );
  यदि( p->iLevel<(पूर्णाङ्क)परिमाण(p->b_पंक्ति) ) p->b_पंक्ति[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
स्थिर शून्य sqlite3_वृक्ष_View_निकालो(वृक्ष_View **pp){
  वृक्ष_View *p = *pp;
  यदि( p==0 ) प्रत्यागम;
  p->iLevel--;
  यदि( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
शून्य sqlite3_वृक्ष_View_पंक्ति(वृक्ष_View *p, ध्रुव वर्ण *zFormat, ...){
  बहु_सूची ap;
  पूर्णाङ्क i;
  StrAccum acc;
  वर्ण zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, परिमाण(zBuf), 0);
  यदि( p ){
    प्रति(i=0; i<p->iLevel && i<(पूर्णाङ्क)परिमाण(p->b_पंक्ति)-1; i++){
      sqlite3_str_append(&acc, p->b_पंक्ति[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_पंक्ति[i] ? "|-- " : "'-- ", 4);
  }
  यदि( zFormat!=0 ){
    बहु_शुरू(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    बहु_पूर्ण(ap);
    प्रतिज्ञा( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  ख_लिखो(मानक_निकास,"%s", zBuf);
  ख_साफ(मानक_निकास);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
स्थिर शून्य sqlite3_वृक्ष_View_Item(वृक्ष_View *p, ध्रुव वर्ण *zLabel,u8 moreFollows){
  sqlite3_वृक्ष_View_धकेलो(&p, moreFollows);
  sqlite3_वृक्ष_View_पंक्ति(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
शून्य sqlite3_वृक्ष_View_Column_List(
  वृक्ष_View *pView,
  ध्रुव Column *aCol,
  पूर्णाङ्क nCol,
  u8 moreToFollow
){
  पूर्णाङ्क i;
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  sqlite3_वृक्ष_View_पंक्ति(pView, "COLUMNS");
  प्रति(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    पूर्णाङ्क colMoreToFollow = i<(nCol - 1);
    sqlite3_वृक्ष_View_धकेलो(&pView, colMoreToFollow);
    sqlite3_वृक्ष_View_पंक्ति(pView, 0);
    म_लिखो(" %s", aCol[i].zCnName);
    विकल्प( aCol[i].eCType ){
      पक्ष COLTYPE_ANY:      म_लिखो(" ANY");        भङ्ग;
      पक्ष COLTYPE_BLOB:     म_लिखो(" BLOB");       भङ्ग;
      पक्ष COLTYPE_INT:      म_लिखो(" INT");        भङ्ग;
      पक्ष COLTYPE_INTEGER:  म_लिखो(" INTEGER");    भङ्ग;
      पक्ष COLTYPE_REAL:     म_लिखो(" REAL");       भङ्ग;
      पक्ष COLTYPE_TEXT:     म_लिखो(" TEXT");       भङ्ग;
      पक्ष COLTYPE_CUSTOM: {
        यदि( flg & COLFLAG_HASTYPE ){
          ध्रुव वर्ण *z = aCol[i].zCnName;
          z += म_माप(z)+1;
          म_लिखो(" X-%s", z);
          भङ्ग;
        }
      }
    }
    यदि( flg & COLFLAG_PRIMKEY ) म_लिखो(" PRIMARY KEY");
    यदि( flg & COLFLAG_HIDDEN ) म_लिखो(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    यदि( flg & COLFLAG_NOEXPAND ) म_लिखो(" NO-EXPAND");
#endif
    यदि( flg ) म_लिखो(" flags=%04x", flg);
    म_लिखो("\n");      
    ख_साफ(मानक_निकास);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
शून्य sqlite3_वृक्ष_View_With(वृक्ष_View *pView, ध्रुव With *pWith, u8 moreToFollow){
  पूर्णाङ्क i;
  यदि( pWith==0 ) प्रत्यागम;
  यदि( pWith->nCte==0 ) प्रत्यागम;
  यदि( pWith->pOuter ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }अन्यथा{
    sqlite3_वृक्ष_View_पंक्ति(pView, "WITH (0x%p)", pWith);
  }
  यदि( pWith->nCte>0 ){
    sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
    प्रति(i=0; i<pWith->nCte; i++){
      StrAccum x;
      वर्ण z_पंक्ति[1000];
      ध्रुव संरचना Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_पंक्ति, परिमाण(z_पंक्ति), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      यदि( pCte->pCols && pCte->pCols->n_व्यंजक>0 ){
        वर्ण cSep = '(';
        पूर्णाङ्क j;
        प्रति(j=0; j<pCte->pCols->n_व्यंजक; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      यदि( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      यदि( pCte->pUse ){
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
शून्य sqlite3_वृक्ष_View_Src_List(वृक्ष_View *pView, ध्रुव SrcList *pSrc){
  पूर्णाङ्क i;
  यदि( pSrc==0 ) प्रत्यागम;
  प्रति(i=0; i<pSrc->nSrc; i++){
    ध्रुव SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    पूर्णाङ्क n = 0;
    वर्ण z_पंक्ति[1000];
    sqlite3StrAccumInit(&x, 0, z_पंक्ति, परिमाण(z_पंक्ति), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    यदि( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    यदि( (pItem->fg.jointype & (JT_बायाँ|JT_दायाँ))==(JT_बायाँ|JT_दायाँ) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }अन्यथा यदि( pItem->fg.jointype & JT_बायाँ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }अन्यथा यदि( pItem->fg.jointype & JT_दायाँ ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }अन्यथा यदि( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    यदि( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    यदि( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    यदि( pItem->fg.isCte ){
      स्थिर ध्रुव वर्ण *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    यदि( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    यदि( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    यदि( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    यदि( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    यदि( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    यदि( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    यदि( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    यदि( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    यदि( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    यदि( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_वृक्ष_View_Item(pView, z_पंक्ति, i<pSrc->nSrc-1);
    n = 0;
    यदि( pItem->fg.isSubquery ) n++;
    यदि( pItem->fg.isTabFunc ) n++;
    यदि( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    यदि( pItem->fg.isUsing ){
      sqlite3_वृक्ष_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }अन्यथा यदि( pItem->u3.pOn!=0 ){
      sqlite3_वृक्ष_View_Item(pView, "ON", (--n)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, pItem->u3.pOn, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    यदि( pItem->fg.isSubquery ){
      प्रतिज्ञा( n==1 );
      यदि( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_वृक्ष_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      प्रतिज्ञा( (पूर्णाङ्क)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_वृक्ष_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    यदि( pItem->fg.isTabFunc ){
      sqlite3_वृक्ष_View_व्यंजक_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
शून्य sqlite3_वृक्ष_View_Select(वृक्ष_View *pView, ध्रुव Select *p, u8 moreToFollow){
  पूर्णाङ्क n = 0;
  पूर्णाङ्क cnt = 0;
  यदि( p==0 ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "nil-SELECT");
    प्रत्यागम;
  } 
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  यदि( p->pWith ){
    sqlite3_वृक्ष_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_वृक्ष_View_धकेलो(&pView, 1);
  }
  कुरु{
    यदि( p->selFlags & SF_WhereBegin ){
      sqlite3_वृक्ष_View_पंक्ति(pView, "sqlite3WhereBegin()");
    }अन्यथा{
      sqlite3_वृक्ष_View_पंक्ति(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (पूर्णाङ्क)p->nSelectRow
      );
    }
    यदि( cnt++ ) sqlite3_वृक्ष_View_निकालो(&pView);
    यदि( p->pPrior ){
      n = 1000;
    }अन्यथा{
      n = 0;
      यदि( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      यदि( p->pWhere ) n++;
      यदि( p->pGroupBy ) n++;
      यदि( p->pHaving ) n++;
      यदि( p->pOrderBy ) n++;
      यदि( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      यदि( p->pWin ) n++;
      यदि( p->pWinDefn ) n++;
#endif
    }
    यदि( p->pEList ){
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    यदि( p->pWin ){
      Window *pX;
      sqlite3_वृक्ष_View_धकेलो(&pView, (n--)>0);
      sqlite3_वृक्ष_View_पंक्ति(pView, "window-functions");
      प्रति(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_वृक्ष_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
#endif
    यदि( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_वृक्ष_View_धकेलो(&pView, (n--)>0);
      sqlite3_वृक्ष_View_पंक्ति(pView, "FROM");
      sqlite3_वृक्ष_View_Src_List(pView, p->pSrc);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    यदि( p->pWhere ){
      sqlite3_वृक्ष_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, p->pWhere, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    यदि( p->pGroupBy ){
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    यदि( p->pHaving ){
      sqlite3_वृक्ष_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, p->pHaving, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    यदि( p->pWinDefn ){
      Window *pX;
      sqlite3_वृक्ष_View_Item(pView, "WINDOW", (n--)>0);
      प्रति(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_वृक्ष_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
#endif
    यदि( p->pOrderBy ){
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    यदि( p->pLimit ){
      sqlite3_वृक्ष_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, p->pLimit->p_बायाँ, p->pLimit->p_दायाँ!=0);
      यदि( p->pLimit->p_दायाँ ){
        sqlite3_वृक्ष_View_Item(pView, "OFFSET", 0);
        sqlite3_वृक्ष_View_व्यंजक(pView, p->pLimit->p_दायाँ, 0);
        sqlite3_वृक्ष_View_निकालो(&pView);
      }
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    यदि( p->pPrior ){
      ध्रुव वर्ण *zOp = "UNION";
      विकल्प( p->op ){
        पक्ष TK_ALL:         zOp = "UNION ALL";  भङ्ग;
        पक्ष TK_INTERSECT:   zOp = "INTERSECT";  भङ्ग;
        पक्ष TK_EXCEPT:      zOp = "EXCEPT";     भङ्ग;
      }
      sqlite3_वृक्ष_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }यावत्( p!=0 );
  sqlite3_वृक्ष_View_निकालो(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
शून्य sqlite3_वृक्ष_View_Bound(
  वृक्ष_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  व्यंजक *p_व्यंजक,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  विकल्प( eBound ){
    पक्ष TK_UNBOUNDED: {
      sqlite3_वृक्ष_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_वृक्ष_View_निकालो(&pView);
      भङ्ग;
    }
    पक्ष TK_CURRENT: {
      sqlite3_वृक्ष_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_वृक्ष_View_निकालो(&pView);
      भङ्ग;
    }
    पक्ष TK_PRECEDING: {
      sqlite3_वृक्ष_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
      भङ्ग;
    }
    पक्ष TK_FOLLOWING: {
      sqlite3_वृक्ष_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
      भङ्ग;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
शून्य sqlite3_वृक्ष_View_Window(वृक्ष_View *pView, ध्रुव Window *pWin, u8 more){
  पूर्णाङ्क nElement = 0;
  यदि( pWin==0 ) प्रत्यागम;
  यदि( pWin->pFilter ){
    sqlite3_वृक्ष_View_Item(pView, "FILTER", 1);
    sqlite3_वृक्ष_View_व्यंजक(pView, pWin->pFilter, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
    यदि( pWin->eFrmType==TK_FILTER ) प्रत्यागम;
  }
  sqlite3_वृक्ष_View_धकेलो(&pView, more);
  यदि( pWin->zName ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "OVER %s (%p)", pWin->zName, pWin);
  }अन्यथा{
    sqlite3_वृक्ष_View_पंक्ति(pView, "OVER (%p)", pWin);
  }
  यदि( pWin->zBase )    nElement++;
  यदि( pWin->pOrderBy ) nElement++;
  यदि( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  यदि( pWin->eExclude ) nElement++;
  यदि( pWin->zBase ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--nElement)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "window: %s", pWin->zBase);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pWin->pPartition ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  यदि( pWin->pOrderBy ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  यदि( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    वर्ण zBuf[30];
    ध्रुव वर्ण *zFrmType = "ROWS";
    यदि( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    यदि( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(परिमाण(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_वृक्ष_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_वृक्ष_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_वृक्ष_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pWin->eExclude ){
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
    sqlite3_वृक्ष_View_धकेलो(&pView, 0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "EXCLUDE %s", zExclude);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
शून्य sqlite3_वृक्ष_View_Win_Func(वृक्ष_View *pView, ध्रुव Window *pWin, u8 more){
  यदि( pWin==0 ) प्रत्यागम;
  sqlite3_वृक्ष_View_धकेलो(&pView, more);
  sqlite3_वृक्ष_View_पंक्ति(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_वृक्ष_View_Window(pView, pWin, 0);
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
शून्य sqlite3_वृक्ष_View_व्यंजक(वृक्ष_View *pView, ध्रुव व्यंजक *p_व्यंजक, u8 moreToFollow){
  ध्रुव वर्ण *zBinOp = 0;   /* Binary operator */
  ध्रुव वर्ण *zUniOp = 0;   /* Unary operator */
  वर्ण zFlgs[200];
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  यदि( p_व्यंजक==0 ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "nil");
    sqlite3_वृक्ष_View_निकालो(&pView);
    प्रत्यागम;
  }
  यदि( p_व्यंजक->flags || p_व्यंजक->aff_व्यंजक || p_व्यंजक->vvaFlags || p_व्यंजक->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, परिमाण(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_व्यंजक->flags, p_व्यंजक->aff_व्यंजक ? p_व्यंजक->aff_व्यंजक : 'n');
    यदि( व्यंजक_Has_Property(p_व्यंजक, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_व्यंजक->w.iJoin);
    }
    यदि( व्यंजक_Has_Property(p_व्यंजक, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_व्यंजक->w.iJoin);
    }
    यदि( व्यंजक_Has_Property(p_व्यंजक, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    यदि( व्यंजक_Has_VVAProperty(p_व्यंजक, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    यदि( p_व्यंजक->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_व्यंजक->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }अन्यथा{
    zFlgs[0] = 0;
  }
  विकल्प( p_व्यंजक->op ){
    पक्ष TK_AGG_COLUMN: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "AGG{%d:%d}%s",
            p_व्यंजक->iTable, p_व्यंजक->iColumn, zFlgs);
      भङ्ग;
    }
    पक्ष TK_COLUMN: {
      यदि( p_व्यंजक->iTable<0 ){
        /* This only happens when coding check constraints */
        वर्ण zOp2[16];
        यदि( p_व्यंजक->op2 ){
          sqlite3_snprintf(परिमाण(zOp2),zOp2," op2=0x%02x",p_व्यंजक->op2);
        }अन्यथा{
          zOp2[0] = 0;
        }
        sqlite3_वृक्ष_View_पंक्ति(pView, "COLUMN(%d)%s%s",
                                    p_व्यंजक->iColumn, zFlgs, zOp2);
      }अन्यथा{
        प्रतिज्ञा( व्यंजक_Use_YTab(p_व्यंजक) );
        sqlite3_वृक्ष_View_पंक्ति(pView, "{%d:%d} pTab=%p%s",
                        p_व्यंजक->iTable, p_व्यंजक->iColumn,
                        p_व्यंजक->y.pTab, zFlgs);
      }
      यदि( व्यंजक_Has_Property(p_व्यंजक, EP_FixedCol) ){
        sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      }
      भङ्ग;
    }
    पक्ष TK_INTEGER: {
      यदि( p_व्यंजक->flags & EP_Int_मूल्य ){
        sqlite3_वृक्ष_View_पंक्ति(pView, "%d", p_व्यंजक->u.i_मूल्य);
      }अन्यथा{
        sqlite3_वृक्ष_View_पंक्ति(pView, "%s", p_व्यंजक->u.zToken);
      }
      भङ्ग;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    पक्ष TK_FLOAT: {
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"%s", p_व्यंजक->u.zToken);
      भङ्ग;
    }
#endif
    पक्ष TK_STRING: {
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"%Q", p_व्यंजक->u.zToken);
      भङ्ग;
    }
    पक्ष TK_NULL: {
      sqlite3_वृक्ष_View_पंक्ति(pView,"NULL");
      भङ्ग;
    }
    पक्ष TK_TRUEFALSE: {
      sqlite3_वृक्ष_View_पंक्ति(pView,"%s%s",
         sqlite3_व्यंजक_Truth_मूल्य(p_व्यंजक) ? "TRUE" : "FALSE", zFlgs);
      भङ्ग;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    पक्ष TK_BLOB: {
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"%s", p_व्यंजक->u.zToken);
      भङ्ग;
    }
#endif
    पक्ष TK_VARIABLE: {
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"VARIABLE(%s,%d)",
                          p_व्यंजक->u.zToken, p_व्यंजक->iColumn);
      भङ्ग;
    }
    पक्ष TK_REGISTER: {
      sqlite3_वृक्ष_View_पंक्ति(pView,"REGISTER(%d)", p_व्यंजक->iTable);
      भङ्ग;
    }
    पक्ष TK_ID: {
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"ID \"%w\"", p_व्यंजक->u.zToken);
      भङ्ग;
    }
    पक्ष TK_ASTERISK: {
      sqlite3_वृक्ष_View_पंक्ति(pView,"ASKTERISK");
      भङ्ग;
    }
#ifndef SQLITE_OMIT_CAST
    पक्ष TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView,"CAST %Q", p_व्यंजक->u.zToken);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
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
      प्रतिज्ञा( p_व्यंजक->op2==TK_IS || p_व्यंजक->op2==TK_ISNOT );
      प्रतिज्ञा( p_व्यंजक->p_दायाँ );
      प्रतिज्ञा( sqlite3_व्यंजक_Skip_Collate_And_Likely(p_व्यंजक->p_दायाँ)->op
                  == TK_TRUEFALSE );
      x = (p_व्यंजक->op2==TK_ISNOT)*2 + sqlite3_व्यंजक_Truth_मूल्य(p_व्यंजक->p_दायाँ);
      zUniOp = azOp[x];
      भङ्ग;
    }

    पक्ष TK_SPAN: {
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "SPAN %Q", p_व्यंजक->u.zToken);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      भङ्ग;
    }

    पक्ष TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "%sCOLLATE %Q%s",
        !व्यंजक_Has_Property(p_व्यंजक, EP_Collate) ? "SOFT-" : "",
        p_व्यंजक->u.zToken, zFlgs);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      भङ्ग;
    }

    पक्ष TK_AGG_FUNCTION:
    पक्ष TK_FUNCTION: {
      व्यंजक_List *pFarg;       /* List of function arguments */
      Window *pWin;
      यदि( व्यंजक_Has_Property(p_व्यंजक, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }अन्यथा{
        प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
        pFarg = p_व्यंजक->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_व्यंजक) ? p_व्यंजक->y.pWin : 0;
#अन्यथा
        pWin = 0;
#endif 
      }
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      यदि( p_व्यंजक->op==TK_AGG_FUNCTION ){
        sqlite3_वृक्ष_View_पंक्ति(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_व्यंजक->op2, p_व्यंजक->u.zToken, zFlgs,
                             p_व्यंजक->pAggInfo ? p_व्यंजक->pAggInfo->selId : 0,
                             p_व्यंजक->iAgg, p_व्यंजक->pAggInfo);
      }अन्यथा यदि( p_व्यंजक->op2!=0 ){
        ध्रुव वर्ण *zOp2;
        वर्ण zBuf[8];
        sqlite3_snprintf(परिमाण(zBuf),zBuf,"0x%02x",p_व्यंजक->op2);
        zOp2 = zBuf;
        यदि( p_व्यंजक->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        यदि( p_व्यंजक->op2==NC_Idx_व्यंजक ) zOp2 = "NC_IdxExpr";
        यदि( p_व्यंजक->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        यदि( p_व्यंजक->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_वृक्ष_View_पंक्ति(pView, "FUNCTION %Q%s op2=%s",
                            p_व्यंजक->u.zToken, zFlgs, zOp2);
      }अन्यथा{
        sqlite3_वृक्ष_View_पंक्ति(pView, "FUNCTION %Q%s", p_व्यंजक->u.zToken, zFlgs);
      }
      यदि( pFarg ){
        sqlite3_वृक्ष_View_व्यंजक_List(pView, pFarg, pWin!=0 || p_व्यंजक->p_बायाँ, 0);
        यदि( p_व्यंजक->p_बायाँ ){
          व्यंजक *pOB = p_व्यंजक->p_बायाँ;
          प्रतिज्ञा( pOB->op==TK_ORDER );
          प्रतिज्ञा( व्यंजक_Use_XList(pOB) );
          sqlite3_वृक्ष_View_व्यंजक_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      यदि( pWin ){
        sqlite3_वृक्ष_View_Window(pView, pWin, 0);
      }
#endif
      भङ्ग;
    }
    पक्ष TK_ORDER: {
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p_व्यंजक->x.pList, 0, "ORDERBY");
      भङ्ग;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    पक्ष TK_EXISTS: {
      प्रतिज्ञा( व्यंजक_Use_XSelect(p_व्यंजक) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "EXISTS-expr flags=0x%x", p_व्यंजक->flags);
      sqlite3_वृक्ष_View_Select(pView, p_व्यंजक->x.pSelect, 0);
      भङ्ग;
    }
    पक्ष TK_SELECT: {
      प्रतिज्ञा( व्यंजक_Use_XSelect(p_व्यंजक) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "subquery-expr flags=0x%x", p_व्यंजक->flags);
      sqlite3_वृक्ष_View_Select(pView, p_व्यंजक->x.pSelect, 0);
      भङ्ग;
    }
    पक्ष TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      वर्ण *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_व्यंजक->flags);
      यदि( p_व्यंजक->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_व्यंजक->iTable);
      यदि( व्यंजक_Has_Property(p_व्यंजक, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_व्यंजक->y.sub.regReturn, p_व्यंजक->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_वृक्ष_View_पंक्ति(pView, z);
      sqlite3_free(z);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 1);
      यदि( व्यंजक_Use_XSelect(p_व्यंजक) ){
        sqlite3_वृक्ष_View_Select(pView, p_व्यंजक->x.pSelect, 0);
      }अन्यथा{
        sqlite3_वृक्ष_View_व्यंजक_List(pView, p_व्यंजक->x.pList, 0, 0);
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
      ध्रुव व्यंजक *pX, *pY, *pZ;
      pX = p_व्यंजक->p_बायाँ;
      प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
      प्रतिज्ञा( p_व्यंजक->x.pList->n_व्यंजक==2 );
      pY = p_व्यंजक->x.pList->a[0].p_व्यंजक;
      pZ = p_व्यंजक->x.pList->a[1].p_व्यंजक;
      sqlite3_वृक्ष_View_पंक्ति(pView, "BETWEEN%s", zFlgs);
      sqlite3_वृक्ष_View_व्यंजक(pView, pX, 1);
      sqlite3_वृक्ष_View_व्यंजक(pView, pY, 1);
      sqlite3_वृक्ष_View_व्यंजक(pView, pZ, 0);
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
      sqlite3_वृक्ष_View_पंक्ति(pView, "%s(%d)", 
          p_व्यंजक->iTable ? "NEW" : "OLD", p_व्यंजक->iColumn);
      भङ्ग;
    }
    पक्ष TK_CASE: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "CASE");
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 1);
      प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
      sqlite3_वृक्ष_View_व्यंजक_List(pView, p_व्यंजक->x.pList, 0, 0);
      भङ्ग;
    }
#ifndef SQLITE_OMIT_TRIGGER
    पक्ष TK_RAISE: {
      ध्रुव वर्ण *zType = "unk";
      विकल्प( p_व्यंजक->aff_व्यंजक ){
        पक्ष OE_Rollback:   zType = "rollback";  भङ्ग;
        पक्ष OE_Abort:      zType = "abort";     भङ्ग;
        पक्ष OE_Fail:       zType = "fail";      भङ्ग;
        पक्ष OE_Ignore:     zType = "ignore";    भङ्ग;
      }
      प्रतिज्ञा( !व्यंजक_Has_Property(p_व्यंजक, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_पंक्ति(pView, "RAISE %s", zType);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      भङ्ग;
    }
#endif
    पक्ष TK_MATCH: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "MATCH {%d:%d}%s",
                          p_व्यंजक->iTable, p_व्यंजक->iColumn, zFlgs);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_दायाँ, 0);
      भङ्ग;
    }
    पक्ष TK_VECTOR: {
      वर्ण *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      प्रतिज्ञा( व्यंजक_Use_XList(p_व्यंजक) );
      sqlite3_वृक्ष_View_Bare_व्यंजक_List(pView, p_व्यंजक->x.pList, z);
      sqlite3_free(z);
      भङ्ग;
    }
    पक्ष TK_SELECT_COLUMN: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_व्यंजक->iColumn, p_व्यंजक->iTable-1,
              p_व्यंजक->p_दायाँ==p_व्यंजक->p_बायाँ ? " (SELECT-owner)" : "");
      प्रतिज्ञा( व्यंजक_Use_XSelect(p_व्यंजक->p_बायाँ) );
      sqlite3_वृक्ष_View_Select(pView, p_व्यंजक->p_बायाँ->x.pSelect, 0);
      भङ्ग;
    }
    पक्ष TK_IF_NULL_ROW: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "IF-NULL-ROW %d", p_व्यंजक->iTable);
      sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
      भङ्ग;
    }
    पक्ष TK_ERROR: {
      व्यंजक tmp;
      sqlite3_वृक्ष_View_पंक्ति(pView, "ERROR");
      tmp = *p_व्यंजक;
      tmp.op = p_व्यंजक->op2;
      sqlite3_वृक्ष_View_व्यंजक(pView, &tmp, 0);
      भङ्ग;
    }
    पक्ष TK_ROW: {
      यदि( p_व्यंजक->iColumn<=0 ){
        sqlite3_वृक्ष_View_पंक्ति(pView, "First FROM table rowid");
      }अन्यथा{
        sqlite3_वृक्ष_View_पंक्ति(pView, "First FROM table column %d",
            p_व्यंजक->iColumn-1);
      }
      भङ्ग;
    }
    स्वभाव: {
      sqlite3_वृक्ष_View_पंक्ति(pView, "op=%d", p_व्यंजक->op);
      भङ्ग;
    }
  }
  यदि( zBinOp ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 1);
    sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_दायाँ, 0);
  }अन्यथा यदि( zUniOp ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_वृक्ष_View_व्यंजक(pView, p_व्यंजक->p_बायाँ, 0);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
शून्य sqlite3_वृक्ष_View_Bare_व्यंजक_List(
  वृक्ष_View *pView,
  ध्रुव व्यंजक_List *pList,
  ध्रुव वर्ण *zLabel
){
  यदि( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  यदि( pList==0 ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s (empty)", zLabel);
  }अन्यथा{
    पूर्णाङ्क i;
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s", zLabel);
    प्रति(i=0; i<pList->n_व्यंजक; i++){
      पूर्णाङ्क j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      वर्ण *zName = pList->a[i].zEName;
      पूर्णाङ्क moreToFollow = i<pList->n_व्यंजक - 1;
      यदि( j || zName || sortFlags ){
        sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_वृक्ष_View_पंक्ति(pView, 0);
        यदि( zName ){
          विकल्प( pList->a[i].fg.eEName ){
            स्वभाव:
              ख_लिखो(मानक_निकास, "AS %s ", zName);
              भङ्ग;
            पक्ष ENAME_TAB:
              ख_लिखो(मानक_निकास, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              यदि( pList->a[i].fg.bUsed ) ख_लिखो(मानक_निकास, "(used) ");
              यदि( pList->a[i].fg.bUsingTerm ) ख_लिखो(मानक_निकास, "(USING-term) ");
              यदि( pList->a[i].fg.bNoExpand ) ख_लिखो(मानक_निकास, "(NoExpand) ");
              भङ्ग;
            पक्ष ENAME_SPAN:
              ख_लिखो(मानक_निकास, "SPAN(\"%s\") ", zName);
              भङ्ग;
          }
        }
        यदि( j ){
          ख_लिखो(मानक_निकास, "iOrderByCol=%d ", j);
        }
        यदि( sortFlags & KEYINFO_ORDER_DESC ){
          ख_लिखो(मानक_निकास, "DESC ");
        }अन्यथा यदि( sortFlags & KEYINFO_ORDER_BIGNULL ){
          ख_लिखो(मानक_निकास, "NULLS-LAST");
        }
        ख_लिखो(मानक_निकास, "\n");
        ख_साफ(मानक_निकास);
      }
      sqlite3_वृक्ष_View_व्यंजक(pView, pList->a[i].p_व्यंजक, moreToFollow);
      यदि( j || zName || sortFlags ){
        sqlite3_वृक्ष_View_निकालो(&pView);
      }
    }
  }
}
शून्य sqlite3_वृक्ष_View_व्यंजक_List(
  वृक्ष_View *pView,
  ध्रुव व्यंजक_List *pList,
  u8 moreToFollow,
  ध्रुव वर्ण *zLabel
){
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  sqlite3_वृक्ष_View_Bare_व्यंजक_List(pView, pList, zLabel);
  sqlite3_वृक्ष_View_निकालो(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
शून्य sqlite3_वृक्ष_View_Bare_Id_List(
  वृक्ष_View *pView,
  ध्रुव IdList *pList,
  ध्रुव वर्ण *zLabel
){
  यदि( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  यदि( pList==0 ){
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s (empty)", zLabel);
  }अन्यथा{
    पूर्णाङ्क i;
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s", zLabel);
    प्रति(i=0; i<pList->nId; i++){
      वर्ण *zName = pList->a[i].zName;
      पूर्णाङ्क moreToFollow = i<pList->nId - 1;
      यदि( zName==0 ) zName = "(null)";
      sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
      sqlite3_वृक्ष_View_पंक्ति(pView, 0);
      ख_लिखो(मानक_निकास, "%s\n", zName);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
  }
}
शून्य sqlite3_वृक्ष_View_Id_List(
  वृक्ष_View *pView,
  ध्रुव IdList *pList,
  u8 moreToFollow,
  ध्रुव वर्ण *zLabel
){
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  sqlite3_वृक्ष_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_वृक्ष_View_निकालो(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
शून्य sqlite3_वृक्ष_View_Upsert(
  वृक्ष_View *pView,
  ध्रुव Upsert *pUpsert,
  u8 moreToFollow
){
  यदि( pUpsert==0 ) प्रत्यागम;
  sqlite3_वृक्ष_View_धकेलो(&pView, moreToFollow);
  यावत्( pUpsert ){
    पूर्णाङ्क n;
    sqlite3_वृक्ष_View_धकेलो(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_वृक्ष_View_पंक्ति(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    यदि( pUpsert->pUpsertWhere ){
      sqlite3_वृक्ष_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_वृक्ष_View_व्यंजक(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_वृक्ष_View_निकालो(&pView);
    }
    sqlite3_वृक्ष_View_निकालो(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}

#यदि TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
शून्य sqlite3_वृक्ष_View_Delete(
  ध्रुव With *pWith,
  ध्रुव SrcList *pTabList,
  ध्रुव व्यंजक *pWhere,
  ध्रुव व्यंजक_List *pOrderBy,
  ध्रुव व्यंजक *pLimit,
  ध्रुव Trigger *pTrigger
){
  पूर्णाङ्क n = 0;
  वृक्ष_View *pView = 0;
  sqlite3_वृक्ष_View_धकेलो(&pView, 0);
  sqlite3_वृक्ष_View_पंक्ति(pView, "DELETE");
  यदि( pWith ) n++;
  यदि( pTabList ) n++;
  यदि( pWhere ) n++;
  यदि( pOrderBy ) n++;
  यदि( pLimit ) n++;
  यदि( pTrigger ) n++;
  यदि( pWith ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_With(pView, pWith, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pTabList ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "FROM");
    sqlite3_वृक्ष_View_Src_List(pView, pTabList);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pWhere ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "WHERE");
    sqlite3_वृक्ष_View_व्यंजक(pView, pWhere, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pOrderBy ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  यदि( pLimit ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "LIMIT");
    sqlite3_वृक्ष_View_व्यंजक(pView, pLimit, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pTrigger ){
    sqlite3_वृक्ष_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#endif /* TREETRACE_ENABLED */

#यदि TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
शून्य sqlite3_वृक्ष_View_जोड़ो(
  ध्रुव With *pWith,
  ध्रुव SrcList *pTabList,
  ध्रुव IdList *pColumnList,
  ध्रुव Select *pSelect,
  ध्रुव व्यंजक_List *p_व्यंजक_List,
  पूर्णाङ्क onError,
  ध्रुव Upsert *pUpsert,
  ध्रुव Trigger *pTrigger
){
  वृक्ष_View *pView = 0;
  पूर्णाङ्क n = 0;
  ध्रुव वर्ण *zLabel = "INSERT";
  विकल्प( onError ){
    पक्ष OE_Replace:  zLabel = "REPLACE";             भङ्ग;
    पक्ष OE_Ignore:   zLabel = "INSERT OR IGNORE";    भङ्ग;
    पक्ष OE_Rollback: zLabel = "INSERT OR ROLLBACK";  भङ्ग;
    पक्ष OE_Abort:    zLabel = "INSERT OR ABORT";     भङ्ग;
    पक्ष OE_Fail:     zLabel = "INSERT OR FAIL";      भङ्ग;
  }
  sqlite3_वृक्ष_View_धकेलो(&pView, 0);
  sqlite3_वृक्ष_View_पंक्ति(pView, zLabel);
  यदि( pWith ) n++;
  यदि( pTabList ) n++;
  यदि( pColumnList ) n++;
  यदि( pSelect ) n++;
  यदि( p_व्यंजक_List ) n++;
  यदि( pUpsert ) n++;
  यदि( pTrigger ) n++;
  यदि( pWith ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_With(pView, pWith, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pTabList ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "INTO");
    sqlite3_वृक्ष_View_Src_List(pView, pTabList);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pColumnList ){
    sqlite3_वृक्ष_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  यदि( pSelect ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "DATA-SOURCE");
    sqlite3_वृक्ष_View_Select(pView, pSelect, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( p_व्यंजक_List ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, p_व्यंजक_List, (--n)>0, "VALUES");
  }
  यदि( pUpsert ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "UPSERT");
    sqlite3_वृक्ष_View_Upsert(pView, pUpsert, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pTrigger ){
    sqlite3_वृक्ष_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#endif /* TREETRACE_ENABLED */

#यदि TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
शून्य sqlite3_वृक्ष_View_Update(
  ध्रुव With *pWith,
  ध्रुव SrcList *pTabList,
  ध्रुव व्यंजक_List *pChanges,
  ध्रुव व्यंजक *pWhere,
  पूर्णाङ्क onError,
  ध्रुव व्यंजक_List *pOrderBy,
  ध्रुव व्यंजक *pLimit,
  ध्रुव Upsert *pUpsert,
  ध्रुव Trigger *pTrigger
){
  पूर्णाङ्क n = 0;
  वृक्ष_View *pView = 0;
  ध्रुव वर्ण *zLabel = "UPDATE";
  विकल्प( onError ){
    पक्ष OE_Replace:  zLabel = "UPDATE OR REPLACE";   भङ्ग;
    पक्ष OE_Ignore:   zLabel = "UPDATE OR IGNORE";    भङ्ग;
    पक्ष OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  भङ्ग;
    पक्ष OE_Abort:    zLabel = "UPDATE OR ABORT";     भङ्ग;
    पक्ष OE_Fail:     zLabel = "UPDATE OR FAIL";      भङ्ग;
  }
  sqlite3_वृक्ष_View_धकेलो(&pView, 0);
  sqlite3_वृक्ष_View_पंक्ति(pView, zLabel);
  यदि( pWith ) n++;
  यदि( pTabList ) n++;
  यदि( pChanges ) n++;
  यदि( pWhere ) n++;
  यदि( pOrderBy ) n++;
  यदि( pLimit ) n++;
  यदि( pUpsert ) n++;
  यदि( pTrigger ) n++;
  यदि( pWith ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_With(pView, pWith, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pTabList ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "FROM");
    sqlite3_वृक्ष_View_Src_List(pView, pTabList);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pChanges ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pChanges, (--n)>0, "SET");
  }
  यदि( pWhere ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "WHERE");
    sqlite3_वृक्ष_View_व्यंजक(pView, pWhere, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pOrderBy ){
    sqlite3_वृक्ष_View_व्यंजक_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  यदि( pLimit ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "LIMIT");
    sqlite3_वृक्ष_View_व्यंजक(pView, pLimit, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pUpsert ){
    sqlite3_वृक्ष_View_धकेलो(&pView, (--n)>0);
    sqlite3_वृक्ष_View_पंक्ति(pView, "UPSERT");
    sqlite3_वृक्ष_View_Upsert(pView, pUpsert, 0);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }
  यदि( pTrigger ){
    sqlite3_वृक्ष_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्ष_View_निकालो(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
शून्य sqlite3_वृक्ष_View_Trigger_Step(
  वृक्ष_View *pView,
  ध्रुव TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णाङ्क cnt = 0;
  यदि( pStep==0 ) प्रत्यागम;
  sqlite3_वृक्ष_View_धकेलो(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  कुरु{
    यदि( cnt++ && pStep->pNext==0 ){
      sqlite3_वृक्ष_View_निकालो(&pView);
      sqlite3_वृक्ष_View_धकेलो(&pView, 0);
    }
    sqlite3_वृक्ष_View_पंक्ति(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }यावत्( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_वृक्ष_View_निकालो(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
शून्य sqlite3_वृक्ष_View_Trigger(
  वृक्ष_View *pView,
  ध्रुव Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णाङ्क cnt = 0;
  यदि( pTrigger==0 ) प्रत्यागम;
  sqlite3_वृक्ष_View_धकेलो(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  कुरु{
    यदि( cnt++ && pTrigger->pNext==0 ){
      sqlite3_वृक्ष_View_निकालो(&pView);
      sqlite3_वृक्ष_View_धकेलो(&pView, 0);
    }
    sqlite3_वृक्ष_View_पंक्ति(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_वृक्ष_View_धकेलो(&pView, 0);
    sqlite3_वृक्ष_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_वृक्ष_View_निकालो(&pView);
  }यावत्( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_वृक्ष_View_निकालो(&pView);
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
शून्य sqlite3_Show_व्यंजक(ध्रुव व्यंजक *p){ sqlite3_वृक्ष_View_व्यंजक(0,p,0); }
शून्य sqlite3_Show_व्यंजक_List(ध्रुव व्यंजक_List *p){ sqlite3_वृक्ष_View_व्यंजक_List(0,p,0,0);}
शून्य sqlite3ShowIdList(ध्रुव IdList *p){ sqlite3_वृक्ष_View_Id_List(0,p,0,0); }
शून्य sqlite3ShowSrcList(ध्रुव SrcList *p){
  वृक्ष_View *pView = 0;
  sqlite3_वृक्ष_View_धकेलो(&pView, 0);
  sqlite3_वृक्ष_View_पंक्ति(pView, "SRCLIST");
  sqlite3_वृक्ष_View_Src_List(pView,p);
  sqlite3_वृक्ष_View_निकालो(&pView);
}
शून्य sqlite3ShowSelect(ध्रुव Select *p){ sqlite3_वृक्ष_View_Select(0,p,0); }
शून्य sqlite3ShowWith(ध्रुव With *p){ sqlite3_वृक्ष_View_With(0,p,0); }
शून्य sqlite3ShowUpsert(ध्रुव Upsert *p){ sqlite3_वृक्ष_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
शून्य sqlite3ShowTriggerStep(ध्रुव TriggerStep *p){
  sqlite3_वृक्ष_View_Trigger_Step(0,p,0,0);
}
शून्य sqlite3ShowTriggerStepList(ध्रुव TriggerStep *p){
  sqlite3_वृक्ष_View_Trigger_Step(0,p,0,1);
}
शून्य sqlite3ShowTrigger(ध्रुव Trigger *p){ sqlite3_वृक्ष_View_Trigger(0,p,0,0); }
शून्य sqlite3ShowTriggerList(ध्रुव Trigger *p){ sqlite3_वृक्ष_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
शून्य sqlite3ShowWindow(ध्रुव Window *p){ sqlite3_वृक्ष_View_Window(0,p,0); }
शून्य sqlite3ShowWinFunc(ध्रुव Window *p){ sqlite3_वृक्ष_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
