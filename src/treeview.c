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
स्थिर शून्य sqlite3_रूख_View_थप(रूख_View **pp, u8 moreToFollow){
  रूख_View *p = *pp;
  यदि( p==0 ){
    *pp = p = sqlite3_malloc64( परिमाण(*p) );
    यदि( p==0 ) फर्काऊ;
    स_रखो(p, 0, परिमाण(*p));
  }अन्यथा{
    p->iLevel++;
  }
  प्रतिज्ञा( moreToFollow==0 || moreToFollow==1 );
  यदि( p->iLevel<(पूर्णाङ्क)परिमाण(p->b_पङ्क्ति) ) p->b_पङ्क्ति[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
स्थिर शून्य sqlite3_रूख_View_झिक(रूख_View **pp){
  रूख_View *p = *pp;
  यदि( p==0 ) फर्काऊ;
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
शून्य sqlite3_रूख_View_पङ्क्ति(रूख_View *p, अचर वर्ण *zFormat, ...){
  बहु_सूची ap;
  पूर्णाङ्क i;
  StrAccum acc;
  वर्ण zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, परिमाण(zBuf), 0);
  यदि( p ){
    लागि(i=0; i<p->iLevel && i<(पूर्णाङ्क)परिमाण(p->b_पङ्क्ति)-1; i++){
      sqlite3_str_append(&acc, p->b_पङ्क्ति[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_पङ्क्ति[i] ? "|-- " : "'-- ", 4);
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
स्थिर शून्य sqlite3_रूख_View_Item(रूख_View *p, अचर वर्ण *zLabel,u8 moreFollows){
  sqlite3_रूख_View_थप(&p, moreFollows);
  sqlite3_रूख_View_पङ्क्ति(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
शून्य sqlite3_रूख_View_Column_List(
  रूख_View *pView,
  अचर Column *aCol,
  पूर्णाङ्क nCol,
  u8 moreToFollow
){
  पूर्णाङ्क i;
  sqlite3_रूख_View_थप(&pView, moreToFollow);
  sqlite3_रूख_View_पङ्क्ति(pView, "COLUMNS");
  लागि(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    पूर्णाङ्क colMoreToFollow = i<(nCol - 1);
    sqlite3_रूख_View_थप(&pView, colMoreToFollow);
    sqlite3_रूख_View_पङ्क्ति(pView, 0);
    म_लिखो(" %s", aCol[i].zCnName);
    विकल्प( aCol[i].eCType ){
      पक्ष COLTYPE_ANY:      म_लिखो(" ANY");        रोक;
      पक्ष COLTYPE_BLOB:     म_लिखो(" BLOB");       रोक;
      पक्ष COLTYPE_INT:      म_लिखो(" INT");        रोक;
      पक्ष COLTYPE_INTEGER:  म_लिखो(" INTEGER");    रोक;
      पक्ष COLTYPE_REAL:     म_लिखो(" REAL");       रोक;
      पक्ष COLTYPE_TEXT:     म_लिखो(" TEXT");       रोक;
      पक्ष COLTYPE_CUSTOM: {
        यदि( flg & COLFLAG_HASTYPE ){
          अचर वर्ण *z = aCol[i].zCnName;
          z += म_माप(z)+1;
          म_लिखो(" X-%s", z);
          रोक;
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
    sqlite3_रूख_View_झिक(&pView);
  }
  sqlite3_रूख_View_झिक(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
शून्य sqlite3_रूख_View_With(रूख_View *pView, अचर With *pWith, u8 moreToFollow){
  पूर्णाङ्क i;
  यदि( pWith==0 ) फर्काऊ;
  यदि( pWith->nCte==0 ) फर्काऊ;
  यदि( pWith->pOuter ){
    sqlite3_रूख_View_पङ्क्ति(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }अन्यथा{
    sqlite3_रूख_View_पङ्क्ति(pView, "WITH (0x%p)", pWith);
  }
  यदि( pWith->nCte>0 ){
    sqlite3_रूख_View_थप(&pView, moreToFollow);
    लागि(i=0; i<pWith->nCte; i++){
      StrAccum x;
      वर्ण z_पङ्क्ति[1000];
      अचर संरचना Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_पङ्क्ति, परिमाण(z_पङ्क्ति), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      यदि( pCte->pCols && pCte->pCols->n_अभिव्यक्ति>0 ){
        वर्ण cSep = '(';
        पूर्णाङ्क j;
        लागि(j=0; j<pCte->pCols->n_अभिव्यक्ति; j++){
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
      sqlite3_रूख_View_Item(pView, z_पङ्क्ति, i<pWith->nCte-1);
      sqlite3_रूख_View_Select(pView, pCte->pSelect, 0);
      sqlite3_रूख_View_झिक(&pView);
    }
    sqlite3_रूख_View_झिक(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
शून्य sqlite3_रूख_View_Src_List(रूख_View *pView, अचर SrcList *pSrc){
  पूर्णाङ्क i;
  यदि( pSrc==0 ) फर्काऊ;
  लागि(i=0; i<pSrc->nSrc; i++){
    अचर SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    पूर्णाङ्क n = 0;
    वर्ण z_पङ्क्ति[1000];
    sqlite3StrAccumInit(&x, 0, z_पङ्क्ति, परिमाण(z_पङ्क्ति), 0);
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
      स्थिर अचर वर्ण *aMat[] = {",MAT", "", ",NO-MAT"};
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
    sqlite3_रूख_View_Item(pView, z_पङ्क्ति, i<pSrc->nSrc-1);
    n = 0;
    यदि( pItem->fg.isSubquery ) n++;
    यदि( pItem->fg.isTabFunc ) n++;
    यदि( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    यदि( pItem->fg.isUsing ){
      sqlite3_रूख_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }अन्यथा यदि( pItem->u3.pOn!=0 ){
      sqlite3_रूख_View_Item(pView, "ON", (--n)>0);
      sqlite3_रूख_View_अभिव्यक्ति(pView, pItem->u3.pOn, 0);
      sqlite3_रूख_View_झिक(&pView);
    }
    यदि( pItem->fg.isSubquery ){
      प्रतिज्ञा( n==1 );
      यदि( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_रूख_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      प्रतिज्ञा( (पूर्णाङ्क)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_रूख_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    यदि( pItem->fg.isTabFunc ){
      sqlite3_रूख_View_अभिव्यक्ति_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_रूख_View_झिक(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
शून्य sqlite3_रूख_View_Select(रूख_View *pView, अचर Select *p, u8 moreToFollow){
  पूर्णाङ्क n = 0;
  पूर्णाङ्क cnt = 0;
  यदि( p==0 ){
    sqlite3_रूख_View_पङ्क्ति(pView, "nil-SELECT");
    फर्काऊ;
  } 
  sqlite3_रूख_View_थप(&pView, moreToFollow);
  यदि( p->pWith ){
    sqlite3_रूख_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_रूख_View_थप(&pView, 1);
  }
  गर{
    यदि( p->selFlags & SF_WhereBegin ){
      sqlite3_रूख_View_पङ्क्ति(pView, "sqlite3WhereBegin()");
    }अन्यथा{
      sqlite3_रूख_View_पङ्क्ति(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (पूर्णाङ्क)p->nSelectRow
      );
    }
    यदि( cnt++ ) sqlite3_रूख_View_झिक(&pView);
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
      sqlite3_रूख_View_अभिव्यक्ति_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    यदि( p->pWin ){
      Window *pX;
      sqlite3_रूख_View_थप(&pView, (n--)>0);
      sqlite3_रूख_View_पङ्क्ति(pView, "window-functions");
      लागि(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_रूख_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_रूख_View_झिक(&pView);
    }
#endif
    यदि( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_रूख_View_थप(&pView, (n--)>0);
      sqlite3_रूख_View_पङ्क्ति(pView, "FROM");
      sqlite3_रूख_View_Src_List(pView, p->pSrc);
      sqlite3_रूख_View_झिक(&pView);
    }
    यदि( p->pWhere ){
      sqlite3_रूख_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p->pWhere, 0);
      sqlite3_रूख_View_झिक(&pView);
    }
    यदि( p->pGroupBy ){
      sqlite3_रूख_View_अभिव्यक्ति_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    यदि( p->pHaving ){
      sqlite3_रूख_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p->pHaving, 0);
      sqlite3_रूख_View_झिक(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    यदि( p->pWinDefn ){
      Window *pX;
      sqlite3_रूख_View_Item(pView, "WINDOW", (n--)>0);
      लागि(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_रूख_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_रूख_View_झिक(&pView);
    }
#endif
    यदि( p->pOrderBy ){
      sqlite3_रूख_View_अभिव्यक्ति_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    यदि( p->pLimit ){
      sqlite3_रूख_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p->pLimit->p_बायाँ, p->pLimit->p_दायाँ!=0);
      यदि( p->pLimit->p_दायाँ ){
        sqlite3_रूख_View_Item(pView, "OFFSET", 0);
        sqlite3_रूख_View_अभिव्यक्ति(pView, p->pLimit->p_दायाँ, 0);
        sqlite3_रूख_View_झिक(&pView);
      }
      sqlite3_रूख_View_झिक(&pView);
    }
    यदि( p->pPrior ){
      अचर वर्ण *zOp = "UNION";
      विकल्प( p->op ){
        पक्ष TK_ALL:         zOp = "UNION ALL";  रोक;
        पक्ष TK_INTERSECT:   zOp = "INTERSECT";  रोक;
        पक्ष TK_EXCEPT:      zOp = "EXCEPT";     रोक;
      }
      sqlite3_रूख_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }जबसम्म( p!=0 );
  sqlite3_रूख_View_झिक(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
शून्य sqlite3_रूख_View_Bound(
  रूख_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  अभिव्यक्ति *p_अभिव्यक्ति,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  विकल्प( eBound ){
    पक्ष TK_UNBOUNDED: {
      sqlite3_रूख_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_रूख_View_झिक(&pView);
      रोक;
    }
    पक्ष TK_CURRENT: {
      sqlite3_रूख_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_रूख_View_झिक(&pView);
      रोक;
    }
    पक्ष TK_PRECEDING: {
      sqlite3_रूख_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति, 0);
      sqlite3_रूख_View_झिक(&pView);
      रोक;
    }
    पक्ष TK_FOLLOWING: {
      sqlite3_रूख_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति, 0);
      sqlite3_रूख_View_झिक(&pView);
      रोक;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
शून्य sqlite3_रूख_View_Window(रूख_View *pView, अचर Window *pWin, u8 more){
  पूर्णाङ्क nElement = 0;
  यदि( pWin==0 ) फर्काऊ;
  यदि( pWin->pFilter ){
    sqlite3_रूख_View_Item(pView, "FILTER", 1);
    sqlite3_रूख_View_अभिव्यक्ति(pView, pWin->pFilter, 0);
    sqlite3_रूख_View_झिक(&pView);
    यदि( pWin->eFrmType==TK_FILTER ) फर्काऊ;
  }
  sqlite3_रूख_View_थप(&pView, more);
  यदि( pWin->zName ){
    sqlite3_रूख_View_पङ्क्ति(pView, "OVER %s (%p)", pWin->zName, pWin);
  }अन्यथा{
    sqlite3_रूख_View_पङ्क्ति(pView, "OVER (%p)", pWin);
  }
  यदि( pWin->zBase )    nElement++;
  यदि( pWin->pOrderBy ) nElement++;
  यदि( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  यदि( pWin->eExclude ) nElement++;
  यदि( pWin->zBase ){
    sqlite3_रूख_View_थप(&pView, (--nElement)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "window: %s", pWin->zBase);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pWin->pPartition ){
    sqlite3_रूख_View_अभिव्यक्ति_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  यदि( pWin->pOrderBy ){
    sqlite3_रूख_View_अभिव्यक्ति_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  यदि( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    वर्ण zBuf[30];
    अचर वर्ण *zFrmType = "ROWS";
    यदि( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    यदि( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(परिमाण(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_रूख_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_रूख_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_रूख_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pWin->eExclude ){
    वर्ण zBuf[30];
    अचर वर्ण *zExclude;
    विकल्प( pWin->eExclude ){
      पक्ष TK_NO:      zExclude = "NO OTHERS";   रोक;
      पक्ष TK_CURRENT: zExclude = "CURRENT ROW"; रोक;
      पक्ष TK_GROUP:   zExclude = "GROUP";       रोक;
      पक्ष TK_TIES:    zExclude = "TIES";        रोक;
      स्वभाव:
        sqlite3_snprintf(परिमाण(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        रोक;
    }
    sqlite3_रूख_View_थप(&pView, 0);
    sqlite3_रूख_View_पङ्क्ति(pView, "EXCLUDE %s", zExclude);
    sqlite3_रूख_View_झिक(&pView);
  }
  sqlite3_रूख_View_झिक(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
शून्य sqlite3_रूख_View_Win_Func(रूख_View *pView, अचर Window *pWin, u8 more){
  यदि( pWin==0 ) फर्काऊ;
  sqlite3_रूख_View_थप(&pView, more);
  sqlite3_रूख_View_पङ्क्ति(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_रूख_View_Window(pView, pWin, 0);
  sqlite3_रूख_View_झिक(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
शून्य sqlite3_रूख_View_अभिव्यक्ति(रूख_View *pView, अचर अभिव्यक्ति *p_अभिव्यक्ति, u8 moreToFollow){
  अचर वर्ण *zBinOp = 0;   /* Binary operator */
  अचर वर्ण *zUniOp = 0;   /* Unary operator */
  वर्ण zFlgs[200];
  sqlite3_रूख_View_थप(&pView, moreToFollow);
  यदि( p_अभिव्यक्ति==0 ){
    sqlite3_रूख_View_पङ्क्ति(pView, "nil");
    sqlite3_रूख_View_झिक(&pView);
    फर्काऊ;
  }
  यदि( p_अभिव्यक्ति->flags || p_अभिव्यक्ति->aff_अभिव्यक्ति || p_अभिव्यक्ति->vvaFlags || p_अभिव्यक्ति->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, परिमाण(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_अभिव्यक्ति->flags, p_अभिव्यक्ति->aff_अभिव्यक्ति ? p_अभिव्यक्ति->aff_अभिव्यक्ति : 'n');
    यदि( अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_अभिव्यक्ति->w.iJoin);
    }
    यदि( अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_अभिव्यक्ति->w.iJoin);
    }
    यदि( अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    यदि( अभिव्यक्ति_Has_VVAProperty(p_अभिव्यक्ति, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    यदि( p_अभिव्यक्ति->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_अभिव्यक्ति->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }अन्यथा{
    zFlgs[0] = 0;
  }
  विकल्प( p_अभिव्यक्ति->op ){
    पक्ष TK_AGG_COLUMN: {
      sqlite3_रूख_View_पङ्क्ति(pView, "AGG{%d:%d}%s",
            p_अभिव्यक्ति->iTable, p_अभिव्यक्ति->iColumn, zFlgs);
      रोक;
    }
    पक्ष TK_COLUMN: {
      यदि( p_अभिव्यक्ति->iTable<0 ){
        /* This only happens when coding check constraints */
        वर्ण zOp2[16];
        यदि( p_अभिव्यक्ति->op2 ){
          sqlite3_snprintf(परिमाण(zOp2),zOp2," op2=0x%02x",p_अभिव्यक्ति->op2);
        }अन्यथा{
          zOp2[0] = 0;
        }
        sqlite3_रूख_View_पङ्क्ति(pView, "COLUMN(%d)%s%s",
                                    p_अभिव्यक्ति->iColumn, zFlgs, zOp2);
      }अन्यथा{
        प्रतिज्ञा( अभिव्यक्ति_Use_YTab(p_अभिव्यक्ति) );
        sqlite3_रूख_View_पङ्क्ति(pView, "{%d:%d} pTab=%p%s",
                        p_अभिव्यक्ति->iTable, p_अभिव्यक्ति->iColumn,
                        p_अभिव्यक्ति->y.pTab, zFlgs);
      }
      यदि( अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_FixedCol) ){
        sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 0);
      }
      रोक;
    }
    पक्ष TK_INTEGER: {
      यदि( p_अभिव्यक्ति->flags & EP_Int_मान ){
        sqlite3_रूख_View_पङ्क्ति(pView, "%d", p_अभिव्यक्ति->u.i_मान);
      }अन्यथा{
        sqlite3_रूख_View_पङ्क्ति(pView, "%s", p_अभिव्यक्ति->u.zToken);
      }
      रोक;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    पक्ष TK_FLOAT: {
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      sqlite3_रूख_View_पङ्क्ति(pView,"%s", p_अभिव्यक्ति->u.zToken);
      रोक;
    }
#endif
    पक्ष TK_STRING: {
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      sqlite3_रूख_View_पङ्क्ति(pView,"%Q", p_अभिव्यक्ति->u.zToken);
      रोक;
    }
    पक्ष TK_NULL: {
      sqlite3_रूख_View_पङ्क्ति(pView,"NULL");
      रोक;
    }
    पक्ष TK_TRUEFALSE: {
      sqlite3_रूख_View_पङ्क्ति(pView,"%s%s",
         sqlite3_अभिव्यक्ति_Truth_मान(p_अभिव्यक्ति) ? "TRUE" : "FALSE", zFlgs);
      रोक;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    पक्ष TK_BLOB: {
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      sqlite3_रूख_View_पङ्क्ति(pView,"%s", p_अभिव्यक्ति->u.zToken);
      रोक;
    }
#endif
    पक्ष TK_VARIABLE: {
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      sqlite3_रूख_View_पङ्क्ति(pView,"VARIABLE(%s,%d)",
                          p_अभिव्यक्ति->u.zToken, p_अभिव्यक्ति->iColumn);
      रोक;
    }
    पक्ष TK_REGISTER: {
      sqlite3_रूख_View_पङ्क्ति(pView,"REGISTER(%d)", p_अभिव्यक्ति->iTable);
      रोक;
    }
    पक्ष TK_ID: {
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      sqlite3_रूख_View_पङ्क्ति(pView,"ID \"%w\"", p_अभिव्यक्ति->u.zToken);
      रोक;
    }
    पक्ष TK_ASTERISK: {
      sqlite3_रूख_View_पङ्क्ति(pView,"ASKTERISK");
      रोक;
    }
#ifndef SQLITE_OMIT_CAST
    पक्ष TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      sqlite3_रूख_View_पङ्क्ति(pView,"CAST %Q", p_अभिव्यक्ति->u.zToken);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 0);
      रोक;
    }
#endif /* SQLITE_OMIT_CAST */
    पक्ष TK_LT:      zBinOp = "LT";     रोक;
    पक्ष TK_LE:      zBinOp = "LE";     रोक;
    पक्ष TK_GT:      zBinOp = "GT";     रोक;
    पक्ष TK_GE:      zBinOp = "GE";     रोक;
    पक्ष TK_NE:      zBinOp = "NE";     रोक;
    पक्ष TK_EQ:      zBinOp = "EQ";     रोक;
    पक्ष TK_IS:      zBinOp = "IS";     रोक;
    पक्ष TK_ISNOT:   zBinOp = "ISNOT";  रोक;
    पक्ष TK_AND:     zBinOp = "AND";    रोक;
    पक्ष TK_OR:      zBinOp = "OR";     रोक;
    पक्ष TK_PLUS:    zBinOp = "ADD";    रोक;
    पक्ष TK_STAR:    zBinOp = "MUL";    रोक;
    पक्ष TK_MINUS:   zBinOp = "SUB";    रोक;
    पक्ष TK_REM:     zBinOp = "REM";    रोक;
    पक्ष TK_BITAND:  zBinOp = "BITAND"; रोक;
    पक्ष TK_BITOR:   zBinOp = "BITOR";  रोक;
    पक्ष TK_SLASH:   zBinOp = "DIV";    रोक;
    पक्ष TK_LSHIFT:  zBinOp = "LSHIFT"; रोक;
    पक्ष TK_RSHIFT:  zBinOp = "RSHIFT"; रोक;
    पक्ष TK_CONCAT:  zBinOp = "CONCAT"; रोक;
    पक्ष TK_DOT:     zBinOp = "DOT";    रोक;
    पक्ष TK_LIMIT:   zBinOp = "LIMIT";  रोक;

    पक्ष TK_UMINUS:  zUniOp = "UMINUS"; रोक;
    पक्ष TK_UPLUS:   zUniOp = "UPLUS";  रोक;
    पक्ष TK_BITNOT:  zUniOp = "BITNOT"; रोक;
    पक्ष TK_NOT:     zUniOp = "NOT";    रोक;
    पक्ष TK_ISNULL:  zUniOp = "ISNULL"; रोक;
    पक्ष TK_NOTNULL: zUniOp = "NOTNULL"; रोक;

    पक्ष TK_TRUTH: {
      पूर्णाङ्क x;
      अचर वर्ण *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      प्रतिज्ञा( p_अभिव्यक्ति->op2==TK_IS || p_अभिव्यक्ति->op2==TK_ISNOT );
      प्रतिज्ञा( p_अभिव्यक्ति->p_दायाँ );
      प्रतिज्ञा( sqlite3_अभिव्यक्ति_Skip_Collate_And_Likely(p_अभिव्यक्ति->p_दायाँ)->op
                  == TK_TRUEFALSE );
      x = (p_अभिव्यक्ति->op2==TK_ISNOT)*2 + sqlite3_अभिव्यक्ति_Truth_मान(p_अभिव्यक्ति->p_दायाँ);
      zUniOp = azOp[x];
      रोक;
    }

    पक्ष TK_SPAN: {
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      sqlite3_रूख_View_पङ्क्ति(pView, "SPAN %Q", p_अभिव्यक्ति->u.zToken);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 0);
      रोक;
    }

    पक्ष TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      sqlite3_रूख_View_पङ्क्ति(pView, "%sCOLLATE %Q%s",
        !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Collate) ? "SOFT-" : "",
        p_अभिव्यक्ति->u.zToken, zFlgs);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 0);
      रोक;
    }

    पक्ष TK_AGG_FUNCTION:
    पक्ष TK_FUNCTION: {
      अभिव्यक्ति_List *pFarg;       /* List of function arguments */
      Window *pWin;
      यदि( अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }अन्यथा{
        प्रतिज्ञा( अभिव्यक्ति_Use_XList(p_अभिव्यक्ति) );
        pFarg = p_अभिव्यक्ति->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_अभिव्यक्ति) ? p_अभिव्यक्ति->y.pWin : 0;
#अन्यथा
        pWin = 0;
#endif 
      }
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      यदि( p_अभिव्यक्ति->op==TK_AGG_FUNCTION ){
        sqlite3_रूख_View_पङ्क्ति(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_अभिव्यक्ति->op2, p_अभिव्यक्ति->u.zToken, zFlgs,
                             p_अभिव्यक्ति->pAggInfo ? p_अभिव्यक्ति->pAggInfo->selId : 0,
                             p_अभिव्यक्ति->iAgg, p_अभिव्यक्ति->pAggInfo);
      }अन्यथा यदि( p_अभिव्यक्ति->op2!=0 ){
        अचर वर्ण *zOp2;
        वर्ण zBuf[8];
        sqlite3_snprintf(परिमाण(zBuf),zBuf,"0x%02x",p_अभिव्यक्ति->op2);
        zOp2 = zBuf;
        यदि( p_अभिव्यक्ति->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        यदि( p_अभिव्यक्ति->op2==NC_Idx_अभिव्यक्ति ) zOp2 = "NC_IdxExpr";
        यदि( p_अभिव्यक्ति->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        यदि( p_अभिव्यक्ति->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_रूख_View_पङ्क्ति(pView, "FUNCTION %Q%s op2=%s",
                            p_अभिव्यक्ति->u.zToken, zFlgs, zOp2);
      }अन्यथा{
        sqlite3_रूख_View_पङ्क्ति(pView, "FUNCTION %Q%s", p_अभिव्यक्ति->u.zToken, zFlgs);
      }
      यदि( pFarg ){
        sqlite3_रूख_View_अभिव्यक्ति_List(pView, pFarg, pWin!=0 || p_अभिव्यक्ति->p_बायाँ, 0);
        यदि( p_अभिव्यक्ति->p_बायाँ ){
          अभिव्यक्ति *pOB = p_अभिव्यक्ति->p_बायाँ;
          प्रतिज्ञा( pOB->op==TK_ORDER );
          प्रतिज्ञा( अभिव्यक्ति_Use_XList(pOB) );
          sqlite3_रूख_View_अभिव्यक्ति_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      यदि( pWin ){
        sqlite3_रूख_View_Window(pView, pWin, 0);
      }
#endif
      रोक;
    }
    पक्ष TK_ORDER: {
      sqlite3_रूख_View_अभिव्यक्ति_List(pView, p_अभिव्यक्ति->x.pList, 0, "ORDERBY");
      रोक;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    पक्ष TK_EXISTS: {
      प्रतिज्ञा( अभिव्यक्ति_Use_XSelect(p_अभिव्यक्ति) );
      sqlite3_रूख_View_पङ्क्ति(pView, "EXISTS-expr flags=0x%x", p_अभिव्यक्ति->flags);
      sqlite3_रूख_View_Select(pView, p_अभिव्यक्ति->x.pSelect, 0);
      रोक;
    }
    पक्ष TK_SELECT: {
      प्रतिज्ञा( अभिव्यक्ति_Use_XSelect(p_अभिव्यक्ति) );
      sqlite3_रूख_View_पङ्क्ति(pView, "subquery-expr flags=0x%x", p_अभिव्यक्ति->flags);
      sqlite3_रूख_View_Select(pView, p_अभिव्यक्ति->x.pSelect, 0);
      रोक;
    }
    पक्ष TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      वर्ण *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_अभिव्यक्ति->flags);
      यदि( p_अभिव्यक्ति->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_अभिव्यक्ति->iTable);
      यदि( अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_अभिव्यक्ति->y.sub.regReturn, p_अभिव्यक्ति->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_रूख_View_पङ्क्ति(pView, z);
      sqlite3_free(z);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 1);
      यदि( अभिव्यक्ति_Use_XSelect(p_अभिव्यक्ति) ){
        sqlite3_रूख_View_Select(pView, p_अभिव्यक्ति->x.pSelect, 0);
      }अन्यथा{
        sqlite3_रूख_View_अभिव्यक्ति_List(pView, p_अभिव्यक्ति->x.pList, 0, 0);
      }
      रोक;
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
      अचर अभिव्यक्ति *pX, *pY, *pZ;
      pX = p_अभिव्यक्ति->p_बायाँ;
      प्रतिज्ञा( अभिव्यक्ति_Use_XList(p_अभिव्यक्ति) );
      प्रतिज्ञा( p_अभिव्यक्ति->x.pList->n_अभिव्यक्ति==2 );
      pY = p_अभिव्यक्ति->x.pList->a[0].p_अभिव्यक्ति;
      pZ = p_अभिव्यक्ति->x.pList->a[1].p_अभिव्यक्ति;
      sqlite3_रूख_View_पङ्क्ति(pView, "BETWEEN%s", zFlgs);
      sqlite3_रूख_View_अभिव्यक्ति(pView, pX, 1);
      sqlite3_रूख_View_अभिव्यक्ति(pView, pY, 1);
      sqlite3_रूख_View_अभिव्यक्ति(pView, pZ, 0);
      रोक;
    }
    पक्ष TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_रूख_View_पङ्क्ति(pView, "%s(%d)", 
          p_अभिव्यक्ति->iTable ? "NEW" : "OLD", p_अभिव्यक्ति->iColumn);
      रोक;
    }
    पक्ष TK_CASE: {
      sqlite3_रूख_View_पङ्क्ति(pView, "CASE");
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 1);
      प्रतिज्ञा( अभिव्यक्ति_Use_XList(p_अभिव्यक्ति) );
      sqlite3_रूख_View_अभिव्यक्ति_List(pView, p_अभिव्यक्ति->x.pList, 0, 0);
      रोक;
    }
#ifndef SQLITE_OMIT_TRIGGER
    पक्ष TK_RAISE: {
      अचर वर्ण *zType = "unk";
      विकल्प( p_अभिव्यक्ति->aff_अभिव्यक्ति ){
        पक्ष OE_Rollback:   zType = "rollback";  रोक;
        पक्ष OE_Abort:      zType = "abort";     रोक;
        पक्ष OE_Fail:       zType = "fail";      रोक;
        पक्ष OE_Ignore:     zType = "ignore";    रोक;
      }
      प्रतिज्ञा( !अभिव्यक्ति_Has_Property(p_अभिव्यक्ति, EP_Int_मान) );
      sqlite3_रूख_View_पङ्क्ति(pView, "RAISE %s", zType);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 0);
      रोक;
    }
#endif
    पक्ष TK_MATCH: {
      sqlite3_रूख_View_पङ्क्ति(pView, "MATCH {%d:%d}%s",
                          p_अभिव्यक्ति->iTable, p_अभिव्यक्ति->iColumn, zFlgs);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_दायाँ, 0);
      रोक;
    }
    पक्ष TK_VECTOR: {
      वर्ण *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      प्रतिज्ञा( अभिव्यक्ति_Use_XList(p_अभिव्यक्ति) );
      sqlite3_रूख_View_Bare_अभिव्यक्ति_List(pView, p_अभिव्यक्ति->x.pList, z);
      sqlite3_free(z);
      रोक;
    }
    पक्ष TK_SELECT_COLUMN: {
      sqlite3_रूख_View_पङ्क्ति(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_अभिव्यक्ति->iColumn, p_अभिव्यक्ति->iTable-1,
              p_अभिव्यक्ति->p_दायाँ==p_अभिव्यक्ति->p_बायाँ ? " (SELECT-owner)" : "");
      प्रतिज्ञा( अभिव्यक्ति_Use_XSelect(p_अभिव्यक्ति->p_बायाँ) );
      sqlite3_रूख_View_Select(pView, p_अभिव्यक्ति->p_बायाँ->x.pSelect, 0);
      रोक;
    }
    पक्ष TK_IF_NULL_ROW: {
      sqlite3_रूख_View_पङ्क्ति(pView, "IF-NULL-ROW %d", p_अभिव्यक्ति->iTable);
      sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 0);
      रोक;
    }
    पक्ष TK_ERROR: {
      अभिव्यक्ति tmp;
      sqlite3_रूख_View_पङ्क्ति(pView, "ERROR");
      tmp = *p_अभिव्यक्ति;
      tmp.op = p_अभिव्यक्ति->op2;
      sqlite3_रूख_View_अभिव्यक्ति(pView, &tmp, 0);
      रोक;
    }
    पक्ष TK_ROW: {
      यदि( p_अभिव्यक्ति->iColumn<=0 ){
        sqlite3_रूख_View_पङ्क्ति(pView, "First FROM table rowid");
      }अन्यथा{
        sqlite3_रूख_View_पङ्क्ति(pView, "First FROM table column %d",
            p_अभिव्यक्ति->iColumn-1);
      }
      रोक;
    }
    स्वभाव: {
      sqlite3_रूख_View_पङ्क्ति(pView, "op=%d", p_अभिव्यक्ति->op);
      रोक;
    }
  }
  यदि( zBinOp ){
    sqlite3_रूख_View_पङ्क्ति(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 1);
    sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_दायाँ, 0);
  }अन्यथा यदि( zUniOp ){
    sqlite3_रूख_View_पङ्क्ति(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_रूख_View_अभिव्यक्ति(pView, p_अभिव्यक्ति->p_बायाँ, 0);
  }
  sqlite3_रूख_View_झिक(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
शून्य sqlite3_रूख_View_Bare_अभिव्यक्ति_List(
  रूख_View *pView,
  अचर अभिव्यक्ति_List *pList,
  अचर वर्ण *zLabel
){
  यदि( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  यदि( pList==0 ){
    sqlite3_रूख_View_पङ्क्ति(pView, "%s (empty)", zLabel);
  }अन्यथा{
    पूर्णाङ्क i;
    sqlite3_रूख_View_पङ्क्ति(pView, "%s", zLabel);
    लागि(i=0; i<pList->n_अभिव्यक्ति; i++){
      पूर्णाङ्क j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      वर्ण *zName = pList->a[i].zEName;
      पूर्णाङ्क moreToFollow = i<pList->n_अभिव्यक्ति - 1;
      यदि( j || zName || sortFlags ){
        sqlite3_रूख_View_थप(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_रूख_View_पङ्क्ति(pView, 0);
        यदि( zName ){
          विकल्प( pList->a[i].fg.eEName ){
            स्वभाव:
              ख_लिखो(मानक_निकास, "AS %s ", zName);
              रोक;
            पक्ष ENAME_TAB:
              ख_लिखो(मानक_निकास, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              यदि( pList->a[i].fg.bUsed ) ख_लिखो(मानक_निकास, "(used) ");
              यदि( pList->a[i].fg.bUsingTerm ) ख_लिखो(मानक_निकास, "(USING-term) ");
              यदि( pList->a[i].fg.bNoExpand ) ख_लिखो(मानक_निकास, "(NoExpand) ");
              रोक;
            पक्ष ENAME_SPAN:
              ख_लिखो(मानक_निकास, "SPAN(\"%s\") ", zName);
              रोक;
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
      sqlite3_रूख_View_अभिव्यक्ति(pView, pList->a[i].p_अभिव्यक्ति, moreToFollow);
      यदि( j || zName || sortFlags ){
        sqlite3_रूख_View_झिक(&pView);
      }
    }
  }
}
शून्य sqlite3_रूख_View_अभिव्यक्ति_List(
  रूख_View *pView,
  अचर अभिव्यक्ति_List *pList,
  u8 moreToFollow,
  अचर वर्ण *zLabel
){
  sqlite3_रूख_View_थप(&pView, moreToFollow);
  sqlite3_रूख_View_Bare_अभिव्यक्ति_List(pView, pList, zLabel);
  sqlite3_रूख_View_झिक(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
शून्य sqlite3_रूख_View_Bare_Id_List(
  रूख_View *pView,
  अचर IdList *pList,
  अचर वर्ण *zLabel
){
  यदि( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  यदि( pList==0 ){
    sqlite3_रूख_View_पङ्क्ति(pView, "%s (empty)", zLabel);
  }अन्यथा{
    पूर्णाङ्क i;
    sqlite3_रूख_View_पङ्क्ति(pView, "%s", zLabel);
    लागि(i=0; i<pList->nId; i++){
      वर्ण *zName = pList->a[i].zName;
      पूर्णाङ्क moreToFollow = i<pList->nId - 1;
      यदि( zName==0 ) zName = "(null)";
      sqlite3_रूख_View_थप(&pView, moreToFollow);
      sqlite3_रूख_View_पङ्क्ति(pView, 0);
      ख_लिखो(मानक_निकास, "%s\n", zName);
      sqlite3_रूख_View_झिक(&pView);
    }
  }
}
शून्य sqlite3_रूख_View_Id_List(
  रूख_View *pView,
  अचर IdList *pList,
  u8 moreToFollow,
  अचर वर्ण *zLabel
){
  sqlite3_रूख_View_थप(&pView, moreToFollow);
  sqlite3_रूख_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_रूख_View_झिक(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
शून्य sqlite3_रूख_View_Upsert(
  रूख_View *pView,
  अचर Upsert *pUpsert,
  u8 moreToFollow
){
  यदि( pUpsert==0 ) फर्काऊ;
  sqlite3_रूख_View_थप(&pView, moreToFollow);
  जबसम्म( pUpsert ){
    पूर्णाङ्क n;
    sqlite3_रूख_View_थप(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_रूख_View_पङ्क्ति(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_रूख_View_अभिव्यक्ति_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_रूख_View_अभिव्यक्ति_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    यदि( pUpsert->pUpsertWhere ){
      sqlite3_रूख_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_रूख_View_अभिव्यक्ति(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_रूख_View_झिक(&pView);
    }
    sqlite3_रूख_View_झिक(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_रूख_View_झिक(&pView);
}

#यदि TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
शून्य sqlite3_रूख_View_Delete(
  अचर With *pWith,
  अचर SrcList *pTabList,
  अचर अभिव्यक्ति *pWhere,
  अचर अभिव्यक्ति_List *pOrderBy,
  अचर अभिव्यक्ति *pLimit,
  अचर Trigger *pTrigger
){
  पूर्णाङ्क n = 0;
  रूख_View *pView = 0;
  sqlite3_रूख_View_थप(&pView, 0);
  sqlite3_रूख_View_पङ्क्ति(pView, "DELETE");
  यदि( pWith ) n++;
  यदि( pTabList ) n++;
  यदि( pWhere ) n++;
  यदि( pOrderBy ) n++;
  यदि( pLimit ) n++;
  यदि( pTrigger ) n++;
  यदि( pWith ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_With(pView, pWith, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pTabList ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "FROM");
    sqlite3_रूख_View_Src_List(pView, pTabList);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pWhere ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "WHERE");
    sqlite3_रूख_View_अभिव्यक्ति(pView, pWhere, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pOrderBy ){
    sqlite3_रूख_View_अभिव्यक्ति_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  यदि( pLimit ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "LIMIT");
    sqlite3_रूख_View_अभिव्यक्ति(pView, pLimit, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pTrigger ){
    sqlite3_रूख_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_रूख_View_झिक(&pView);
}
#endif /* TREETRACE_ENABLED */

#यदि TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
शून्य sqlite3_रूख_View_घुसाउ(
  अचर With *pWith,
  अचर SrcList *pTabList,
  अचर IdList *pColumnList,
  अचर Select *pSelect,
  अचर अभिव्यक्ति_List *p_अभिव्यक्ति_List,
  पूर्णाङ्क onError,
  अचर Upsert *pUpsert,
  अचर Trigger *pTrigger
){
  रूख_View *pView = 0;
  पूर्णाङ्क n = 0;
  अचर वर्ण *zLabel = "INSERT";
  विकल्प( onError ){
    पक्ष OE_Replace:  zLabel = "REPLACE";             रोक;
    पक्ष OE_Ignore:   zLabel = "INSERT OR IGNORE";    रोक;
    पक्ष OE_Rollback: zLabel = "INSERT OR ROLLBACK";  रोक;
    पक्ष OE_Abort:    zLabel = "INSERT OR ABORT";     रोक;
    पक्ष OE_Fail:     zLabel = "INSERT OR FAIL";      रोक;
  }
  sqlite3_रूख_View_थप(&pView, 0);
  sqlite3_रूख_View_पङ्क्ति(pView, zLabel);
  यदि( pWith ) n++;
  यदि( pTabList ) n++;
  यदि( pColumnList ) n++;
  यदि( pSelect ) n++;
  यदि( p_अभिव्यक्ति_List ) n++;
  यदि( pUpsert ) n++;
  यदि( pTrigger ) n++;
  यदि( pWith ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_With(pView, pWith, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pTabList ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "INTO");
    sqlite3_रूख_View_Src_List(pView, pTabList);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pColumnList ){
    sqlite3_रूख_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  यदि( pSelect ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "DATA-SOURCE");
    sqlite3_रूख_View_Select(pView, pSelect, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( p_अभिव्यक्ति_List ){
    sqlite3_रूख_View_अभिव्यक्ति_List(pView, p_अभिव्यक्ति_List, (--n)>0, "VALUES");
  }
  यदि( pUpsert ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "UPSERT");
    sqlite3_रूख_View_Upsert(pView, pUpsert, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pTrigger ){
    sqlite3_रूख_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_रूख_View_झिक(&pView);
}
#endif /* TREETRACE_ENABLED */

#यदि TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
शून्य sqlite3_रूख_View_Update(
  अचर With *pWith,
  अचर SrcList *pTabList,
  अचर अभिव्यक्ति_List *pChanges,
  अचर अभिव्यक्ति *pWhere,
  पूर्णाङ्क onError,
  अचर अभिव्यक्ति_List *pOrderBy,
  अचर अभिव्यक्ति *pLimit,
  अचर Upsert *pUpsert,
  अचर Trigger *pTrigger
){
  पूर्णाङ्क n = 0;
  रूख_View *pView = 0;
  अचर वर्ण *zLabel = "UPDATE";
  विकल्प( onError ){
    पक्ष OE_Replace:  zLabel = "UPDATE OR REPLACE";   रोक;
    पक्ष OE_Ignore:   zLabel = "UPDATE OR IGNORE";    रोक;
    पक्ष OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  रोक;
    पक्ष OE_Abort:    zLabel = "UPDATE OR ABORT";     रोक;
    पक्ष OE_Fail:     zLabel = "UPDATE OR FAIL";      रोक;
  }
  sqlite3_रूख_View_थप(&pView, 0);
  sqlite3_रूख_View_पङ्क्ति(pView, zLabel);
  यदि( pWith ) n++;
  यदि( pTabList ) n++;
  यदि( pChanges ) n++;
  यदि( pWhere ) n++;
  यदि( pOrderBy ) n++;
  यदि( pLimit ) n++;
  यदि( pUpsert ) n++;
  यदि( pTrigger ) n++;
  यदि( pWith ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_With(pView, pWith, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pTabList ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "FROM");
    sqlite3_रूख_View_Src_List(pView, pTabList);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pChanges ){
    sqlite3_रूख_View_अभिव्यक्ति_List(pView, pChanges, (--n)>0, "SET");
  }
  यदि( pWhere ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "WHERE");
    sqlite3_रूख_View_अभिव्यक्ति(pView, pWhere, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pOrderBy ){
    sqlite3_रूख_View_अभिव्यक्ति_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  यदि( pLimit ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "LIMIT");
    sqlite3_रूख_View_अभिव्यक्ति(pView, pLimit, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pUpsert ){
    sqlite3_रूख_View_थप(&pView, (--n)>0);
    sqlite3_रूख_View_पङ्क्ति(pView, "UPSERT");
    sqlite3_रूख_View_Upsert(pView, pUpsert, 0);
    sqlite3_रूख_View_झिक(&pView);
  }
  यदि( pTrigger ){
    sqlite3_रूख_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_रूख_View_झिक(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
शून्य sqlite3_रूख_View_Trigger_Step(
  रूख_View *pView,
  अचर TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णाङ्क cnt = 0;
  यदि( pStep==0 ) फर्काऊ;
  sqlite3_रूख_View_थप(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  गर{
    यदि( cnt++ && pStep->pNext==0 ){
      sqlite3_रूख_View_झिक(&pView);
      sqlite3_रूख_View_थप(&pView, 0);
    }
    sqlite3_रूख_View_पङ्क्ति(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }जबसम्म( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_रूख_View_झिक(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
शून्य sqlite3_रूख_View_Trigger(
  रूख_View *pView,
  अचर Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णाङ्क cnt = 0;
  यदि( pTrigger==0 ) फर्काऊ;
  sqlite3_रूख_View_थप(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  गर{
    यदि( cnt++ && pTrigger->pNext==0 ){
      sqlite3_रूख_View_झिक(&pView);
      sqlite3_रूख_View_थप(&pView, 0);
    }
    sqlite3_रूख_View_पङ्क्ति(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_रूख_View_थप(&pView, 0);
    sqlite3_रूख_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_रूख_View_झिक(&pView);
  }जबसम्म( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_रूख_View_झिक(&pView);
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
शून्य sqlite3_Show_अभिव्यक्ति(अचर अभिव्यक्ति *p){ sqlite3_रूख_View_अभिव्यक्ति(0,p,0); }
शून्य sqlite3_Show_अभिव्यक्ति_List(अचर अभिव्यक्ति_List *p){ sqlite3_रूख_View_अभिव्यक्ति_List(0,p,0,0);}
शून्य sqlite3ShowIdList(अचर IdList *p){ sqlite3_रूख_View_Id_List(0,p,0,0); }
शून्य sqlite3ShowSrcList(अचर SrcList *p){
  रूख_View *pView = 0;
  sqlite3_रूख_View_थप(&pView, 0);
  sqlite3_रूख_View_पङ्क्ति(pView, "SRCLIST");
  sqlite3_रूख_View_Src_List(pView,p);
  sqlite3_रूख_View_झिक(&pView);
}
शून्य sqlite3ShowSelect(अचर Select *p){ sqlite3_रूख_View_Select(0,p,0); }
शून्य sqlite3ShowWith(अचर With *p){ sqlite3_रूख_View_With(0,p,0); }
शून्य sqlite3ShowUpsert(अचर Upsert *p){ sqlite3_रूख_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
शून्य sqlite3ShowTriggerStep(अचर TriggerStep *p){
  sqlite3_रूख_View_Trigger_Step(0,p,0,0);
}
शून्य sqlite3ShowTriggerStepList(अचर TriggerStep *p){
  sqlite3_रूख_View_Trigger_Step(0,p,0,1);
}
शून्य sqlite3ShowTrigger(अचर Trigger *p){ sqlite3_रूख_View_Trigger(0,p,0,0); }
शून्य sqlite3ShowTriggerList(अचर Trigger *p){ sqlite3_रूख_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
शून्य sqlite3ShowWindow(अचर Window *p){ sqlite3_रूख_View_Window(0,p,0); }
शून्य sqlite3ShowWinFunc(अचर Window *p){ sqlite3_रूख_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
