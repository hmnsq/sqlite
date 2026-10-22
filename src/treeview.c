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
स्थिर शून्य sqlite3_वृक्ष_View_ढकला(वृक्ष_View **pp, u8 moreToFollow){
  वृक्ष_View *p = *pp;
  जर( p==0 ){
    *pp = p = sqlite3_malloc64( आकार(*p) );
    जर( p==0 ) परत;
    स_रखो(p, 0, आकार(*p));
  }अन्यथा{
    p->iLevel++;
  }
  प्रतिज्ञा( moreToFollow==0 || moreToFollow==1 );
  जर( p->iLevel<(पूर्णांक)आकार(p->b_ओळ) ) p->b_ओळ[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
स्थिर शून्य sqlite3_वृक्ष_View_काढा(वृक्ष_View **pp){
  वृक्ष_View *p = *pp;
  जर( p==0 ) परत;
  p->iLevel--;
  जर( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
शून्य sqlite3_वृक्ष_View_ओळ(वृक्ष_View *p, स्थिरांक अक्षर *zFormat, ...){
  बहु_सूची ap;
  पूर्णांक i;
  StrAccum acc;
  अक्षर zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, आकार(zBuf), 0);
  जर( p ){
    साठी(i=0; i<p->iLevel && i<(पूर्णांक)आकार(p->b_ओळ)-1; i++){
      sqlite3_str_append(&acc, p->b_ओळ[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_ओळ[i] ? "|-- " : "'-- ", 4);
  }
  जर( zFormat!=0 ){
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
स्थिर शून्य sqlite3_वृक्ष_View_Item(वृक्ष_View *p, स्थिरांक अक्षर *zLabel,u8 moreFollows){
  sqlite3_वृक्ष_View_ढकला(&p, moreFollows);
  sqlite3_वृक्ष_View_ओळ(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
शून्य sqlite3_वृक्ष_View_Column_List(
  वृक्ष_View *pView,
  स्थिरांक Column *aCol,
  पूर्णांक nCol,
  u8 moreToFollow
){
  पूर्णांक i;
  sqlite3_वृक्ष_View_ढकला(&pView, moreToFollow);
  sqlite3_वृक्ष_View_ओळ(pView, "COLUMNS");
  साठी(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    पूर्णांक colMoreToFollow = i<(nCol - 1);
    sqlite3_वृक्ष_View_ढकला(&pView, colMoreToFollow);
    sqlite3_वृक्ष_View_ओळ(pView, 0);
    म_लिखो(" %s", aCol[i].zCnName);
    निवड( aCol[i].eCType ){
      स्थिती COLTYPE_ANY:      म_लिखो(" ANY");        थांबा;
      स्थिती COLTYPE_BLOB:     म_लिखो(" BLOB");       थांबा;
      स्थिती COLTYPE_INT:      म_लिखो(" INT");        थांबा;
      स्थिती COLTYPE_INTEGER:  म_लिखो(" INTEGER");    थांबा;
      स्थिती COLTYPE_REAL:     म_लिखो(" REAL");       थांबा;
      स्थिती COLTYPE_TEXT:     म_लिखो(" TEXT");       थांबा;
      स्थिती COLTYPE_CUSTOM: {
        जर( flg & COLFLAG_HASTYPE ){
          स्थिरांक अक्षर *z = aCol[i].zCnName;
          z += म_माप(z)+1;
          म_लिखो(" X-%s", z);
          थांबा;
        }
      }
    }
    जर( flg & COLFLAG_PRIMKEY ) म_लिखो(" PRIMARY KEY");
    जर( flg & COLFLAG_HIDDEN ) म_लिखो(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    जर( flg & COLFLAG_NOEXPAND ) म_लिखो(" NO-EXPAND");
#endif
    जर( flg ) म_लिखो(" flags=%04x", flg);
    म_लिखो("\n");      
    ख_साफ(मानक_निकास);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  sqlite3_वृक्ष_View_काढा(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
शून्य sqlite3_वृक्ष_View_With(वृक्ष_View *pView, स्थिरांक With *pWith, u8 moreToFollow){
  पूर्णांक i;
  जर( pWith==0 ) परत;
  जर( pWith->nCte==0 ) परत;
  जर( pWith->pOuter ){
    sqlite3_वृक्ष_View_ओळ(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }अन्यथा{
    sqlite3_वृक्ष_View_ओळ(pView, "WITH (0x%p)", pWith);
  }
  जर( pWith->nCte>0 ){
    sqlite3_वृक्ष_View_ढकला(&pView, moreToFollow);
    साठी(i=0; i<pWith->nCte; i++){
      StrAccum x;
      अक्षर z_ओळ[1000];
      स्थिरांक संरचना Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_ओळ, आकार(z_ओळ), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      जर( pCte->pCols && pCte->pCols->n_अभिव्यक्ती>0 ){
        अक्षर cSep = '(';
        पूर्णांक j;
        साठी(j=0; j<pCte->pCols->n_अभिव्यक्ती; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      जर( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      जर( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_वृक्ष_View_Item(pView, z_ओळ, i<pWith->nCte-1);
      sqlite3_वृक्ष_View_Select(pView, pCte->pSelect, 0);
      sqlite3_वृक्ष_View_काढा(&pView);
    }
    sqlite3_वृक्ष_View_काढा(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
शून्य sqlite3_वृक्ष_View_Src_List(वृक्ष_View *pView, स्थिरांक SrcList *pSrc){
  पूर्णांक i;
  जर( pSrc==0 ) परत;
  साठी(i=0; i<pSrc->nSrc; i++){
    स्थिरांक SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    पूर्णांक n = 0;
    अक्षर z_ओळ[1000];
    sqlite3StrAccumInit(&x, 0, z_ओळ, आकार(z_ओळ), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    जर( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    जर( (pItem->fg.jointype & (JT_डावा|JT_उजवा))==(JT_डावा|JT_उजवा) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }अन्यथा जर( pItem->fg.jointype & JT_डावा ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }अन्यथा जर( pItem->fg.jointype & JT_उजवा ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }अन्यथा जर( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    जर( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    जर( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    जर( pItem->fg.isCte ){
      स्थिर स्थिरांक अक्षर *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    जर( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    जर( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    जर( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    जर( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    जर( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    जर( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    जर( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    जर( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    जर( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    जर( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_वृक्ष_View_Item(pView, z_ओळ, i<pSrc->nSrc-1);
    n = 0;
    जर( pItem->fg.isSubquery ) n++;
    जर( pItem->fg.isTabFunc ) n++;
    जर( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    जर( pItem->fg.isUsing ){
      sqlite3_वृक्ष_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }अन्यथा जर( pItem->u3.pOn!=0 ){
      sqlite3_वृक्ष_View_Item(pView, "ON", (--n)>0);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pItem->u3.pOn, 0);
      sqlite3_वृक्ष_View_काढा(&pView);
    }
    जर( pItem->fg.isSubquery ){
      प्रतिज्ञा( n==1 );
      जर( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_वृक्ष_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      प्रतिज्ञा( (पूर्णांक)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_वृक्ष_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    जर( pItem->fg.isTabFunc ){
      sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_वृक्ष_View_काढा(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
शून्य sqlite3_वृक्ष_View_Select(वृक्ष_View *pView, स्थिरांक Select *p, u8 moreToFollow){
  पूर्णांक n = 0;
  पूर्णांक cnt = 0;
  जर( p==0 ){
    sqlite3_वृक्ष_View_ओळ(pView, "nil-SELECT");
    परत;
  } 
  sqlite3_वृक्ष_View_ढकला(&pView, moreToFollow);
  जर( p->pWith ){
    sqlite3_वृक्ष_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_वृक्ष_View_ढकला(&pView, 1);
  }
  करा{
    जर( p->selFlags & SF_WhereBegin ){
      sqlite3_वृक्ष_View_ओळ(pView, "sqlite3WhereBegin()");
    }अन्यथा{
      sqlite3_वृक्ष_View_ओळ(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (पूर्णांक)p->nSelectRow
      );
    }
    जर( cnt++ ) sqlite3_वृक्ष_View_काढा(&pView);
    जर( p->pPrior ){
      n = 1000;
    }अन्यथा{
      n = 0;
      जर( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      जर( p->pWhere ) n++;
      जर( p->pGroupBy ) n++;
      जर( p->pHaving ) n++;
      जर( p->pOrderBy ) n++;
      जर( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      जर( p->pWin ) n++;
      जर( p->pWinDefn ) n++;
#endif
    }
    जर( p->pEList ){
      sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    जर( p->pWin ){
      Window *pX;
      sqlite3_वृक्ष_View_ढकला(&pView, (n--)>0);
      sqlite3_वृक्ष_View_ओळ(pView, "window-functions");
      साठी(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_वृक्ष_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_वृक्ष_View_काढा(&pView);
    }
#endif
    जर( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_वृक्ष_View_ढकला(&pView, (n--)>0);
      sqlite3_वृक्ष_View_ओळ(pView, "FROM");
      sqlite3_वृक्ष_View_Src_List(pView, p->pSrc);
      sqlite3_वृक्ष_View_काढा(&pView);
    }
    जर( p->pWhere ){
      sqlite3_वृक्ष_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p->pWhere, 0);
      sqlite3_वृक्ष_View_काढा(&pView);
    }
    जर( p->pGroupBy ){
      sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    जर( p->pHaving ){
      sqlite3_वृक्ष_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p->pHaving, 0);
      sqlite3_वृक्ष_View_काढा(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    जर( p->pWinDefn ){
      Window *pX;
      sqlite3_वृक्ष_View_Item(pView, "WINDOW", (n--)>0);
      साठी(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_वृक्ष_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_वृक्ष_View_काढा(&pView);
    }
#endif
    जर( p->pOrderBy ){
      sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    जर( p->pLimit ){
      sqlite3_वृक्ष_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p->pLimit->p_डावा, p->pLimit->p_उजवा!=0);
      जर( p->pLimit->p_उजवा ){
        sqlite3_वृक्ष_View_Item(pView, "OFFSET", 0);
        sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p->pLimit->p_उजवा, 0);
        sqlite3_वृक्ष_View_काढा(&pView);
      }
      sqlite3_वृक्ष_View_काढा(&pView);
    }
    जर( p->pPrior ){
      स्थिरांक अक्षर *zOp = "UNION";
      निवड( p->op ){
        स्थिती TK_ALL:         zOp = "UNION ALL";  थांबा;
        स्थिती TK_INTERSECT:   zOp = "INTERSECT";  थांबा;
        स्थिती TK_EXCEPT:      zOp = "EXCEPT";     थांबा;
      }
      sqlite3_वृक्ष_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }पर्यंत( p!=0 );
  sqlite3_वृक्ष_View_काढा(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
शून्य sqlite3_वृक्ष_View_Bound(
  वृक्ष_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  अभिव्यक्ती *p_अभिव्यक्ती,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  निवड( eBound ){
    स्थिती TK_UNBOUNDED: {
      sqlite3_वृक्ष_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_वृक्ष_View_काढा(&pView);
      थांबा;
    }
    स्थिती TK_CURRENT: {
      sqlite3_वृक्ष_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_वृक्ष_View_काढा(&pView);
      थांबा;
    }
    स्थिती TK_PRECEDING: {
      sqlite3_वृक्ष_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती, 0);
      sqlite3_वृक्ष_View_काढा(&pView);
      थांबा;
    }
    स्थिती TK_FOLLOWING: {
      sqlite3_वृक्ष_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती, 0);
      sqlite3_वृक्ष_View_काढा(&pView);
      थांबा;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
शून्य sqlite3_वृक्ष_View_Window(वृक्ष_View *pView, स्थिरांक Window *pWin, u8 more){
  पूर्णांक nElement = 0;
  जर( pWin==0 ) परत;
  जर( pWin->pFilter ){
    sqlite3_वृक्ष_View_Item(pView, "FILTER", 1);
    sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pWin->pFilter, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
    जर( pWin->eFrmType==TK_FILTER ) परत;
  }
  sqlite3_वृक्ष_View_ढकला(&pView, more);
  जर( pWin->zName ){
    sqlite3_वृक्ष_View_ओळ(pView, "OVER %s (%p)", pWin->zName, pWin);
  }अन्यथा{
    sqlite3_वृक्ष_View_ओळ(pView, "OVER (%p)", pWin);
  }
  जर( pWin->zBase )    nElement++;
  जर( pWin->pOrderBy ) nElement++;
  जर( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  जर( pWin->eExclude ) nElement++;
  जर( pWin->zBase ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--nElement)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "window: %s", pWin->zBase);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pWin->pPartition ){
    sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  जर( pWin->pOrderBy ){
    sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  जर( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    अक्षर zBuf[30];
    स्थिरांक अक्षर *zFrmType = "ROWS";
    जर( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    जर( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(आकार(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_वृक्ष_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_वृक्ष_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_वृक्ष_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pWin->eExclude ){
    अक्षर zBuf[30];
    स्थिरांक अक्षर *zExclude;
    निवड( pWin->eExclude ){
      स्थिती TK_NO:      zExclude = "NO OTHERS";   थांबा;
      स्थिती TK_CURRENT: zExclude = "CURRENT ROW"; थांबा;
      स्थिती TK_GROUP:   zExclude = "GROUP";       थांबा;
      स्थिती TK_TIES:    zExclude = "TIES";        थांबा;
      पूर्वनिर्धारित:
        sqlite3_snprintf(आकार(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        थांबा;
    }
    sqlite3_वृक्ष_View_ढकला(&pView, 0);
    sqlite3_वृक्ष_View_ओळ(pView, "EXCLUDE %s", zExclude);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  sqlite3_वृक्ष_View_काढा(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
शून्य sqlite3_वृक्ष_View_Win_Func(वृक्ष_View *pView, स्थिरांक Window *pWin, u8 more){
  जर( pWin==0 ) परत;
  sqlite3_वृक्ष_View_ढकला(&pView, more);
  sqlite3_वृक्ष_View_ओळ(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_वृक्ष_View_Window(pView, pWin, 0);
  sqlite3_वृक्ष_View_काढा(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
शून्य sqlite3_वृक्ष_View_अभिव्यक्ती(वृक्ष_View *pView, स्थिरांक अभिव्यक्ती *p_अभिव्यक्ती, u8 moreToFollow){
  स्थिरांक अक्षर *zBinOp = 0;   /* Binary operator */
  स्थिरांक अक्षर *zUniOp = 0;   /* Unary operator */
  अक्षर zFlgs[200];
  sqlite3_वृक्ष_View_ढकला(&pView, moreToFollow);
  जर( p_अभिव्यक्ती==0 ){
    sqlite3_वृक्ष_View_ओळ(pView, "nil");
    sqlite3_वृक्ष_View_काढा(&pView);
    परत;
  }
  जर( p_अभिव्यक्ती->flags || p_अभिव्यक्ती->aff_अभिव्यक्ती || p_अभिव्यक्ती->vvaFlags || p_अभिव्यक्ती->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, आकार(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_अभिव्यक्ती->flags, p_अभिव्यक्ती->aff_अभिव्यक्ती ? p_अभिव्यक्ती->aff_अभिव्यक्ती : 'n');
    जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_अभिव्यक्ती->w.iJoin);
    }
    जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_अभिव्यक्ती->w.iJoin);
    }
    जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    जर( अभिव्यक्ती_Has_VVAProperty(p_अभिव्यक्ती, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    जर( p_अभिव्यक्ती->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_अभिव्यक्ती->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }अन्यथा{
    zFlgs[0] = 0;
  }
  निवड( p_अभिव्यक्ती->op ){
    स्थिती TK_AGG_COLUMN: {
      sqlite3_वृक्ष_View_ओळ(pView, "AGG{%d:%d}%s",
            p_अभिव्यक्ती->iTable, p_अभिव्यक्ती->iColumn, zFlgs);
      थांबा;
    }
    स्थिती TK_COLUMN: {
      जर( p_अभिव्यक्ती->iTable<0 ){
        /* This only happens when coding check constraints */
        अक्षर zOp2[16];
        जर( p_अभिव्यक्ती->op2 ){
          sqlite3_snprintf(आकार(zOp2),zOp2," op2=0x%02x",p_अभिव्यक्ती->op2);
        }अन्यथा{
          zOp2[0] = 0;
        }
        sqlite3_वृक्ष_View_ओळ(pView, "COLUMN(%d)%s%s",
                                    p_अभिव्यक्ती->iColumn, zFlgs, zOp2);
      }अन्यथा{
        प्रतिज्ञा( अभिव्यक्ती_Use_YTab(p_अभिव्यक्ती) );
        sqlite3_वृक्ष_View_ओळ(pView, "{%d:%d} pTab=%p%s",
                        p_अभिव्यक्ती->iTable, p_अभिव्यक्ती->iColumn,
                        p_अभिव्यक्ती->y.pTab, zFlgs);
      }
      जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_FixedCol) ){
        sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 0);
      }
      थांबा;
    }
    स्थिती TK_INTEGER: {
      जर( p_अभिव्यक्ती->flags & EP_Int_मूल्य ){
        sqlite3_वृक्ष_View_ओळ(pView, "%d", p_अभिव्यक्ती->u.i_मूल्य);
      }अन्यथा{
        sqlite3_वृक्ष_View_ओळ(pView, "%s", p_अभिव्यक्ती->u.zToken);
      }
      थांबा;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    स्थिती TK_FLOAT: {
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_ओळ(pView,"%s", p_अभिव्यक्ती->u.zToken);
      थांबा;
    }
#endif
    स्थिती TK_STRING: {
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_ओळ(pView,"%Q", p_अभिव्यक्ती->u.zToken);
      थांबा;
    }
    स्थिती TK_NULL: {
      sqlite3_वृक्ष_View_ओळ(pView,"NULL");
      थांबा;
    }
    स्थिती TK_TRUEFALSE: {
      sqlite3_वृक्ष_View_ओळ(pView,"%s%s",
         sqlite3_अभिव्यक्ती_Truth_मूल्य(p_अभिव्यक्ती) ? "TRUE" : "FALSE", zFlgs);
      थांबा;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    स्थिती TK_BLOB: {
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_ओळ(pView,"%s", p_अभिव्यक्ती->u.zToken);
      थांबा;
    }
#endif
    स्थिती TK_VARIABLE: {
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_ओळ(pView,"VARIABLE(%s,%d)",
                          p_अभिव्यक्ती->u.zToken, p_अभिव्यक्ती->iColumn);
      थांबा;
    }
    स्थिती TK_REGISTER: {
      sqlite3_वृक्ष_View_ओळ(pView,"REGISTER(%d)", p_अभिव्यक्ती->iTable);
      थांबा;
    }
    स्थिती TK_ID: {
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_ओळ(pView,"ID \"%w\"", p_अभिव्यक्ती->u.zToken);
      थांबा;
    }
    स्थिती TK_ASTERISK: {
      sqlite3_वृक्ष_View_ओळ(pView,"ASKTERISK");
      थांबा;
    }
#ifndef SQLITE_OMIT_CAST
    स्थिती TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_ओळ(pView,"CAST %Q", p_अभिव्यक्ती->u.zToken);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 0);
      थांबा;
    }
#endif /* SQLITE_OMIT_CAST */
    स्थिती TK_LT:      zBinOp = "LT";     थांबा;
    स्थिती TK_LE:      zBinOp = "LE";     थांबा;
    स्थिती TK_GT:      zBinOp = "GT";     थांबा;
    स्थिती TK_GE:      zBinOp = "GE";     थांबा;
    स्थिती TK_NE:      zBinOp = "NE";     थांबा;
    स्थिती TK_EQ:      zBinOp = "EQ";     थांबा;
    स्थिती TK_IS:      zBinOp = "IS";     थांबा;
    स्थिती TK_ISNOT:   zBinOp = "ISNOT";  थांबा;
    स्थिती TK_AND:     zBinOp = "AND";    थांबा;
    स्थिती TK_OR:      zBinOp = "OR";     थांबा;
    स्थिती TK_PLUS:    zBinOp = "ADD";    थांबा;
    स्थिती TK_STAR:    zBinOp = "MUL";    थांबा;
    स्थिती TK_MINUS:   zBinOp = "SUB";    थांबा;
    स्थिती TK_REM:     zBinOp = "REM";    थांबा;
    स्थिती TK_BITAND:  zBinOp = "BITAND"; थांबा;
    स्थिती TK_BITOR:   zBinOp = "BITOR";  थांबा;
    स्थिती TK_SLASH:   zBinOp = "DIV";    थांबा;
    स्थिती TK_LSHIFT:  zBinOp = "LSHIFT"; थांबा;
    स्थिती TK_RSHIFT:  zBinOp = "RSHIFT"; थांबा;
    स्थिती TK_CONCAT:  zBinOp = "CONCAT"; थांबा;
    स्थिती TK_DOT:     zBinOp = "DOT";    थांबा;
    स्थिती TK_LIMIT:   zBinOp = "LIMIT";  थांबा;

    स्थिती TK_UMINUS:  zUniOp = "UMINUS"; थांबा;
    स्थिती TK_UPLUS:   zUniOp = "UPLUS";  थांबा;
    स्थिती TK_BITNOT:  zUniOp = "BITNOT"; थांबा;
    स्थिती TK_NOT:     zUniOp = "NOT";    थांबा;
    स्थिती TK_ISNULL:  zUniOp = "ISNULL"; थांबा;
    स्थिती TK_NOTNULL: zUniOp = "NOTNULL"; थांबा;

    स्थिती TK_TRUTH: {
      पूर्णांक x;
      स्थिरांक अक्षर *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      प्रतिज्ञा( p_अभिव्यक्ती->op2==TK_IS || p_अभिव्यक्ती->op2==TK_ISNOT );
      प्रतिज्ञा( p_अभिव्यक्ती->p_उजवा );
      प्रतिज्ञा( sqlite3_अभिव्यक्ती_Skip_Collate_And_Likely(p_अभिव्यक्ती->p_उजवा)->op
                  == TK_TRUEFALSE );
      x = (p_अभिव्यक्ती->op2==TK_ISNOT)*2 + sqlite3_अभिव्यक्ती_Truth_मूल्य(p_अभिव्यक्ती->p_उजवा);
      zUniOp = azOp[x];
      थांबा;
    }

    स्थिती TK_SPAN: {
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_ओळ(pView, "SPAN %Q", p_अभिव्यक्ती->u.zToken);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 0);
      थांबा;
    }

    स्थिती TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_ओळ(pView, "%sCOLLATE %Q%s",
        !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Collate) ? "SOFT-" : "",
        p_अभिव्यक्ती->u.zToken, zFlgs);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 0);
      थांबा;
    }

    स्थिती TK_AGG_FUNCTION:
    स्थिती TK_FUNCTION: {
      अभिव्यक्ती_List *pFarg;       /* List of function arguments */
      Window *pWin;
      जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }अन्यथा{
        प्रतिज्ञा( अभिव्यक्ती_Use_XList(p_अभिव्यक्ती) );
        pFarg = p_अभिव्यक्ती->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_अभिव्यक्ती) ? p_अभिव्यक्ती->y.pWin : 0;
#अन्यथा
        pWin = 0;
#endif 
      }
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      जर( p_अभिव्यक्ती->op==TK_AGG_FUNCTION ){
        sqlite3_वृक्ष_View_ओळ(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_अभिव्यक्ती->op2, p_अभिव्यक्ती->u.zToken, zFlgs,
                             p_अभिव्यक्ती->pAggInfo ? p_अभिव्यक्ती->pAggInfo->selId : 0,
                             p_अभिव्यक्ती->iAgg, p_अभिव्यक्ती->pAggInfo);
      }अन्यथा जर( p_अभिव्यक्ती->op2!=0 ){
        स्थिरांक अक्षर *zOp2;
        अक्षर zBuf[8];
        sqlite3_snprintf(आकार(zBuf),zBuf,"0x%02x",p_अभिव्यक्ती->op2);
        zOp2 = zBuf;
        जर( p_अभिव्यक्ती->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        जर( p_अभिव्यक्ती->op2==NC_Idx_अभिव्यक्ती ) zOp2 = "NC_IdxExpr";
        जर( p_अभिव्यक्ती->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        जर( p_अभिव्यक्ती->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_वृक्ष_View_ओळ(pView, "FUNCTION %Q%s op2=%s",
                            p_अभिव्यक्ती->u.zToken, zFlgs, zOp2);
      }अन्यथा{
        sqlite3_वृक्ष_View_ओळ(pView, "FUNCTION %Q%s", p_अभिव्यक्ती->u.zToken, zFlgs);
      }
      जर( pFarg ){
        sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pFarg, pWin!=0 || p_अभिव्यक्ती->p_डावा, 0);
        जर( p_अभिव्यक्ती->p_डावा ){
          अभिव्यक्ती *pOB = p_अभिव्यक्ती->p_डावा;
          प्रतिज्ञा( pOB->op==TK_ORDER );
          प्रतिज्ञा( अभिव्यक्ती_Use_XList(pOB) );
          sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      जर( pWin ){
        sqlite3_वृक्ष_View_Window(pView, pWin, 0);
      }
#endif
      थांबा;
    }
    स्थिती TK_ORDER: {
      sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, p_अभिव्यक्ती->x.pList, 0, "ORDERBY");
      थांबा;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    स्थिती TK_EXISTS: {
      प्रतिज्ञा( अभिव्यक्ती_Use_XSelect(p_अभिव्यक्ती) );
      sqlite3_वृक्ष_View_ओळ(pView, "EXISTS-expr flags=0x%x", p_अभिव्यक्ती->flags);
      sqlite3_वृक्ष_View_Select(pView, p_अभिव्यक्ती->x.pSelect, 0);
      थांबा;
    }
    स्थिती TK_SELECT: {
      प्रतिज्ञा( अभिव्यक्ती_Use_XSelect(p_अभिव्यक्ती) );
      sqlite3_वृक्ष_View_ओळ(pView, "subquery-expr flags=0x%x", p_अभिव्यक्ती->flags);
      sqlite3_वृक्ष_View_Select(pView, p_अभिव्यक्ती->x.pSelect, 0);
      थांबा;
    }
    स्थिती TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      अक्षर *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_अभिव्यक्ती->flags);
      जर( p_अभिव्यक्ती->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_अभिव्यक्ती->iTable);
      जर( अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_अभिव्यक्ती->y.sub.regReturn, p_अभिव्यक्ती->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_वृक्ष_View_ओळ(pView, z);
      sqlite3_free(z);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 1);
      जर( अभिव्यक्ती_Use_XSelect(p_अभिव्यक्ती) ){
        sqlite3_वृक्ष_View_Select(pView, p_अभिव्यक्ती->x.pSelect, 0);
      }अन्यथा{
        sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, p_अभिव्यक्ती->x.pList, 0, 0);
      }
      थांबा;
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
    स्थिती TK_BETWEEN: {
      स्थिरांक अभिव्यक्ती *pX, *pY, *pZ;
      pX = p_अभिव्यक्ती->p_डावा;
      प्रतिज्ञा( अभिव्यक्ती_Use_XList(p_अभिव्यक्ती) );
      प्रतिज्ञा( p_अभिव्यक्ती->x.pList->n_अभिव्यक्ती==2 );
      pY = p_अभिव्यक्ती->x.pList->a[0].p_अभिव्यक्ती;
      pZ = p_अभिव्यक्ती->x.pList->a[1].p_अभिव्यक्ती;
      sqlite3_वृक्ष_View_ओळ(pView, "BETWEEN%s", zFlgs);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pX, 1);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pY, 1);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pZ, 0);
      थांबा;
    }
    स्थिती TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_वृक्ष_View_ओळ(pView, "%s(%d)", 
          p_अभिव्यक्ती->iTable ? "NEW" : "OLD", p_अभिव्यक्ती->iColumn);
      थांबा;
    }
    स्थिती TK_CASE: {
      sqlite3_वृक्ष_View_ओळ(pView, "CASE");
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 1);
      प्रतिज्ञा( अभिव्यक्ती_Use_XList(p_अभिव्यक्ती) );
      sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, p_अभिव्यक्ती->x.pList, 0, 0);
      थांबा;
    }
#ifndef SQLITE_OMIT_TRIGGER
    स्थिती TK_RAISE: {
      स्थिरांक अक्षर *zType = "unk";
      निवड( p_अभिव्यक्ती->aff_अभिव्यक्ती ){
        स्थिती OE_Rollback:   zType = "rollback";  थांबा;
        स्थिती OE_Abort:      zType = "abort";     थांबा;
        स्थिती OE_Fail:       zType = "fail";      थांबा;
        स्थिती OE_Ignore:     zType = "ignore";    थांबा;
      }
      प्रतिज्ञा( !अभिव्यक्ती_Has_Property(p_अभिव्यक्ती, EP_Int_मूल्य) );
      sqlite3_वृक्ष_View_ओळ(pView, "RAISE %s", zType);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 0);
      थांबा;
    }
#endif
    स्थिती TK_MATCH: {
      sqlite3_वृक्ष_View_ओळ(pView, "MATCH {%d:%d}%s",
                          p_अभिव्यक्ती->iTable, p_अभिव्यक्ती->iColumn, zFlgs);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_उजवा, 0);
      थांबा;
    }
    स्थिती TK_VECTOR: {
      अक्षर *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      प्रतिज्ञा( अभिव्यक्ती_Use_XList(p_अभिव्यक्ती) );
      sqlite3_वृक्ष_View_Bare_अभिव्यक्ती_List(pView, p_अभिव्यक्ती->x.pList, z);
      sqlite3_free(z);
      थांबा;
    }
    स्थिती TK_SELECT_COLUMN: {
      sqlite3_वृक्ष_View_ओळ(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_अभिव्यक्ती->iColumn, p_अभिव्यक्ती->iTable-1,
              p_अभिव्यक्ती->p_उजवा==p_अभिव्यक्ती->p_डावा ? " (SELECT-owner)" : "");
      प्रतिज्ञा( अभिव्यक्ती_Use_XSelect(p_अभिव्यक्ती->p_डावा) );
      sqlite3_वृक्ष_View_Select(pView, p_अभिव्यक्ती->p_डावा->x.pSelect, 0);
      थांबा;
    }
    स्थिती TK_IF_NULL_ROW: {
      sqlite3_वृक्ष_View_ओळ(pView, "IF-NULL-ROW %d", p_अभिव्यक्ती->iTable);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 0);
      थांबा;
    }
    स्थिती TK_ERROR: {
      अभिव्यक्ती tmp;
      sqlite3_वृक्ष_View_ओळ(pView, "ERROR");
      tmp = *p_अभिव्यक्ती;
      tmp.op = p_अभिव्यक्ती->op2;
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, &tmp, 0);
      थांबा;
    }
    स्थिती TK_ROW: {
      जर( p_अभिव्यक्ती->iColumn<=0 ){
        sqlite3_वृक्ष_View_ओळ(pView, "First FROM table rowid");
      }अन्यथा{
        sqlite3_वृक्ष_View_ओळ(pView, "First FROM table column %d",
            p_अभिव्यक्ती->iColumn-1);
      }
      थांबा;
    }
    पूर्वनिर्धारित: {
      sqlite3_वृक्ष_View_ओळ(pView, "op=%d", p_अभिव्यक्ती->op);
      थांबा;
    }
  }
  जर( zBinOp ){
    sqlite3_वृक्ष_View_ओळ(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 1);
    sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_उजवा, 0);
  }अन्यथा जर( zUniOp ){
    sqlite3_वृक्ष_View_ओळ(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_वृक्ष_View_अभिव्यक्ती(pView, p_अभिव्यक्ती->p_डावा, 0);
  }
  sqlite3_वृक्ष_View_काढा(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
शून्य sqlite3_वृक्ष_View_Bare_अभिव्यक्ती_List(
  वृक्ष_View *pView,
  स्थिरांक अभिव्यक्ती_List *pList,
  स्थिरांक अक्षर *zLabel
){
  जर( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  जर( pList==0 ){
    sqlite3_वृक्ष_View_ओळ(pView, "%s (empty)", zLabel);
  }अन्यथा{
    पूर्णांक i;
    sqlite3_वृक्ष_View_ओळ(pView, "%s", zLabel);
    साठी(i=0; i<pList->n_अभिव्यक्ती; i++){
      पूर्णांक j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      अक्षर *zName = pList->a[i].zEName;
      पूर्णांक moreToFollow = i<pList->n_अभिव्यक्ती - 1;
      जर( j || zName || sortFlags ){
        sqlite3_वृक्ष_View_ढकला(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_वृक्ष_View_ओळ(pView, 0);
        जर( zName ){
          निवड( pList->a[i].fg.eEName ){
            पूर्वनिर्धारित:
              ख_लिखो(मानक_निकास, "AS %s ", zName);
              थांबा;
            स्थिती ENAME_TAB:
              ख_लिखो(मानक_निकास, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              जर( pList->a[i].fg.bUsed ) ख_लिखो(मानक_निकास, "(used) ");
              जर( pList->a[i].fg.bUsingTerm ) ख_लिखो(मानक_निकास, "(USING-term) ");
              जर( pList->a[i].fg.bNoExpand ) ख_लिखो(मानक_निकास, "(NoExpand) ");
              थांबा;
            स्थिती ENAME_SPAN:
              ख_लिखो(मानक_निकास, "SPAN(\"%s\") ", zName);
              थांबा;
          }
        }
        जर( j ){
          ख_लिखो(मानक_निकास, "iOrderByCol=%d ", j);
        }
        जर( sortFlags & KEYINFO_ORDER_DESC ){
          ख_लिखो(मानक_निकास, "DESC ");
        }अन्यथा जर( sortFlags & KEYINFO_ORDER_BIGNULL ){
          ख_लिखो(मानक_निकास, "NULLS-LAST");
        }
        ख_लिखो(मानक_निकास, "\n");
        ख_साफ(मानक_निकास);
      }
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pList->a[i].p_अभिव्यक्ती, moreToFollow);
      जर( j || zName || sortFlags ){
        sqlite3_वृक्ष_View_काढा(&pView);
      }
    }
  }
}
शून्य sqlite3_वृक्ष_View_अभिव्यक्ती_List(
  वृक्ष_View *pView,
  स्थिरांक अभिव्यक्ती_List *pList,
  u8 moreToFollow,
  स्थिरांक अक्षर *zLabel
){
  sqlite3_वृक्ष_View_ढकला(&pView, moreToFollow);
  sqlite3_वृक्ष_View_Bare_अभिव्यक्ती_List(pView, pList, zLabel);
  sqlite3_वृक्ष_View_काढा(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
शून्य sqlite3_वृक्ष_View_Bare_Id_List(
  वृक्ष_View *pView,
  स्थिरांक IdList *pList,
  स्थिरांक अक्षर *zLabel
){
  जर( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  जर( pList==0 ){
    sqlite3_वृक्ष_View_ओळ(pView, "%s (empty)", zLabel);
  }अन्यथा{
    पूर्णांक i;
    sqlite3_वृक्ष_View_ओळ(pView, "%s", zLabel);
    साठी(i=0; i<pList->nId; i++){
      अक्षर *zName = pList->a[i].zName;
      पूर्णांक moreToFollow = i<pList->nId - 1;
      जर( zName==0 ) zName = "(null)";
      sqlite3_वृक्ष_View_ढकला(&pView, moreToFollow);
      sqlite3_वृक्ष_View_ओळ(pView, 0);
      ख_लिखो(मानक_निकास, "%s\n", zName);
      sqlite3_वृक्ष_View_काढा(&pView);
    }
  }
}
शून्य sqlite3_वृक्ष_View_Id_List(
  वृक्ष_View *pView,
  स्थिरांक IdList *pList,
  u8 moreToFollow,
  स्थिरांक अक्षर *zLabel
){
  sqlite3_वृक्ष_View_ढकला(&pView, moreToFollow);
  sqlite3_वृक्ष_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_वृक्ष_View_काढा(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
शून्य sqlite3_वृक्ष_View_Upsert(
  वृक्ष_View *pView,
  स्थिरांक Upsert *pUpsert,
  u8 moreToFollow
){
  जर( pUpsert==0 ) परत;
  sqlite3_वृक्ष_View_ढकला(&pView, moreToFollow);
  पर्यंत( pUpsert ){
    पूर्णांक n;
    sqlite3_वृक्ष_View_ढकला(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_वृक्ष_View_ओळ(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    जर( pUpsert->pUpsertWhere ){
      sqlite3_वृक्ष_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_वृक्ष_View_काढा(&pView);
    }
    sqlite3_वृक्ष_View_काढा(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_वृक्ष_View_काढा(&pView);
}

#जर TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
शून्य sqlite3_वृक्ष_View_Delete(
  स्थिरांक With *pWith,
  स्थिरांक SrcList *pTabList,
  स्थिरांक अभिव्यक्ती *pWhere,
  स्थिरांक अभिव्यक्ती_List *pOrderBy,
  स्थिरांक अभिव्यक्ती *pLimit,
  स्थिरांक Trigger *pTrigger
){
  पूर्णांक n = 0;
  वृक्ष_View *pView = 0;
  sqlite3_वृक्ष_View_ढकला(&pView, 0);
  sqlite3_वृक्ष_View_ओळ(pView, "DELETE");
  जर( pWith ) n++;
  जर( pTabList ) n++;
  जर( pWhere ) n++;
  जर( pOrderBy ) n++;
  जर( pLimit ) n++;
  जर( pTrigger ) n++;
  जर( pWith ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_With(pView, pWith, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pTabList ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "FROM");
    sqlite3_वृक्ष_View_Src_List(pView, pTabList);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pWhere ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "WHERE");
    sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pWhere, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pOrderBy ){
    sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  जर( pLimit ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "LIMIT");
    sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pLimit, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pTrigger ){
    sqlite3_वृक्ष_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्ष_View_काढा(&pView);
}
#endif /* TREETRACE_ENABLED */

#जर TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
शून्य sqlite3_वृक्ष_View_घाला(
  स्थिरांक With *pWith,
  स्थिरांक SrcList *pTabList,
  स्थिरांक IdList *pColumnList,
  स्थिरांक Select *pSelect,
  स्थिरांक अभिव्यक्ती_List *p_अभिव्यक्ती_List,
  पूर्णांक onError,
  स्थिरांक Upsert *pUpsert,
  स्थिरांक Trigger *pTrigger
){
  वृक्ष_View *pView = 0;
  पूर्णांक n = 0;
  स्थिरांक अक्षर *zLabel = "INSERT";
  निवड( onError ){
    स्थिती OE_Replace:  zLabel = "REPLACE";             थांबा;
    स्थिती OE_Ignore:   zLabel = "INSERT OR IGNORE";    थांबा;
    स्थिती OE_Rollback: zLabel = "INSERT OR ROLLBACK";  थांबा;
    स्थिती OE_Abort:    zLabel = "INSERT OR ABORT";     थांबा;
    स्थिती OE_Fail:     zLabel = "INSERT OR FAIL";      थांबा;
  }
  sqlite3_वृक्ष_View_ढकला(&pView, 0);
  sqlite3_वृक्ष_View_ओळ(pView, zLabel);
  जर( pWith ) n++;
  जर( pTabList ) n++;
  जर( pColumnList ) n++;
  जर( pSelect ) n++;
  जर( p_अभिव्यक्ती_List ) n++;
  जर( pUpsert ) n++;
  जर( pTrigger ) n++;
  जर( pWith ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_With(pView, pWith, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pTabList ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "INTO");
    sqlite3_वृक्ष_View_Src_List(pView, pTabList);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pColumnList ){
    sqlite3_वृक्ष_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  जर( pSelect ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "DATA-SOURCE");
    sqlite3_वृक्ष_View_Select(pView, pSelect, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( p_अभिव्यक्ती_List ){
    sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, p_अभिव्यक्ती_List, (--n)>0, "VALUES");
  }
  जर( pUpsert ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "UPSERT");
    sqlite3_वृक्ष_View_Upsert(pView, pUpsert, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pTrigger ){
    sqlite3_वृक्ष_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्ष_View_काढा(&pView);
}
#endif /* TREETRACE_ENABLED */

#जर TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
शून्य sqlite3_वृक्ष_View_Update(
  स्थिरांक With *pWith,
  स्थिरांक SrcList *pTabList,
  स्थिरांक अभिव्यक्ती_List *pChanges,
  स्थिरांक अभिव्यक्ती *pWhere,
  पूर्णांक onError,
  स्थिरांक अभिव्यक्ती_List *pOrderBy,
  स्थिरांक अभिव्यक्ती *pLimit,
  स्थिरांक Upsert *pUpsert,
  स्थिरांक Trigger *pTrigger
){
  पूर्णांक n = 0;
  वृक्ष_View *pView = 0;
  स्थिरांक अक्षर *zLabel = "UPDATE";
  निवड( onError ){
    स्थिती OE_Replace:  zLabel = "UPDATE OR REPLACE";   थांबा;
    स्थिती OE_Ignore:   zLabel = "UPDATE OR IGNORE";    थांबा;
    स्थिती OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  थांबा;
    स्थिती OE_Abort:    zLabel = "UPDATE OR ABORT";     थांबा;
    स्थिती OE_Fail:     zLabel = "UPDATE OR FAIL";      थांबा;
  }
  sqlite3_वृक्ष_View_ढकला(&pView, 0);
  sqlite3_वृक्ष_View_ओळ(pView, zLabel);
  जर( pWith ) n++;
  जर( pTabList ) n++;
  जर( pChanges ) n++;
  जर( pWhere ) n++;
  जर( pOrderBy ) n++;
  जर( pLimit ) n++;
  जर( pUpsert ) n++;
  जर( pTrigger ) n++;
  जर( pWith ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_With(pView, pWith, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pTabList ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "FROM");
    sqlite3_वृक्ष_View_Src_List(pView, pTabList);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pChanges ){
    sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pChanges, (--n)>0, "SET");
  }
  जर( pWhere ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "WHERE");
    sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pWhere, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pOrderBy ){
    sqlite3_वृक्ष_View_अभिव्यक्ती_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  जर( pLimit ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "LIMIT");
    sqlite3_वृक्ष_View_अभिव्यक्ती(pView, pLimit, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pUpsert ){
    sqlite3_वृक्ष_View_ढकला(&pView, (--n)>0);
    sqlite3_वृक्ष_View_ओळ(pView, "UPSERT");
    sqlite3_वृक्ष_View_Upsert(pView, pUpsert, 0);
    sqlite3_वृक्ष_View_काढा(&pView);
  }
  जर( pTrigger ){
    sqlite3_वृक्ष_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_वृक्ष_View_काढा(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
शून्य sqlite3_वृक्ष_View_Trigger_Step(
  वृक्ष_View *pView,
  स्थिरांक TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णांक cnt = 0;
  जर( pStep==0 ) परत;
  sqlite3_वृक्ष_View_ढकला(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  करा{
    जर( cnt++ && pStep->pNext==0 ){
      sqlite3_वृक्ष_View_काढा(&pView);
      sqlite3_वृक्ष_View_ढकला(&pView, 0);
    }
    sqlite3_वृक्ष_View_ओळ(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }पर्यंत( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_वृक्ष_View_काढा(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
शून्य sqlite3_वृक्ष_View_Trigger(
  वृक्ष_View *pView,
  स्थिरांक Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  पूर्णांक cnt = 0;
  जर( pTrigger==0 ) परत;
  sqlite3_वृक्ष_View_ढकला(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  करा{
    जर( cnt++ && pTrigger->pNext==0 ){
      sqlite3_वृक्ष_View_काढा(&pView);
      sqlite3_वृक्ष_View_ढकला(&pView, 0);
    }
    sqlite3_वृक्ष_View_ओळ(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_वृक्ष_View_ढकला(&pView, 0);
    sqlite3_वृक्ष_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_वृक्ष_View_काढा(&pView);
  }पर्यंत( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_वृक्ष_View_काढा(&pView);
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
शून्य sqlite3_Show_अभिव्यक्ती(स्थिरांक अभिव्यक्ती *p){ sqlite3_वृक्ष_View_अभिव्यक्ती(0,p,0); }
शून्य sqlite3_Show_अभिव्यक्ती_List(स्थिरांक अभिव्यक्ती_List *p){ sqlite3_वृक्ष_View_अभिव्यक्ती_List(0,p,0,0);}
शून्य sqlite3ShowIdList(स्थिरांक IdList *p){ sqlite3_वृक्ष_View_Id_List(0,p,0,0); }
शून्य sqlite3ShowSrcList(स्थिरांक SrcList *p){
  वृक्ष_View *pView = 0;
  sqlite3_वृक्ष_View_ढकला(&pView, 0);
  sqlite3_वृक्ष_View_ओळ(pView, "SRCLIST");
  sqlite3_वृक्ष_View_Src_List(pView,p);
  sqlite3_वृक्ष_View_काढा(&pView);
}
शून्य sqlite3ShowSelect(स्थिरांक Select *p){ sqlite3_वृक्ष_View_Select(0,p,0); }
शून्य sqlite3ShowWith(स्थिरांक With *p){ sqlite3_वृक्ष_View_With(0,p,0); }
शून्य sqlite3ShowUpsert(स्थिरांक Upsert *p){ sqlite3_वृक्ष_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
शून्य sqlite3ShowTriggerStep(स्थिरांक TriggerStep *p){
  sqlite3_वृक्ष_View_Trigger_Step(0,p,0,0);
}
शून्य sqlite3ShowTriggerStepList(स्थिरांक TriggerStep *p){
  sqlite3_वृक्ष_View_Trigger_Step(0,p,0,1);
}
शून्य sqlite3ShowTrigger(स्थिरांक Trigger *p){ sqlite3_वृक्ष_View_Trigger(0,p,0,0); }
शून्य sqlite3ShowTriggerList(स्थिरांक Trigger *p){ sqlite3_वृक्ष_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
शून्य sqlite3ShowWindow(स्थिरांक Window *p){ sqlite3_वृक्ष_View_Window(0,p,0); }
शून्य sqlite3ShowWinFunc(स्थिरांक Window *p){ sqlite3_वृक्ष_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
