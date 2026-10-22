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
#அந்தர்பாவ "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
நிலையான வெறுமை sqlite3_மரம்_View_தள்ளு(மரம்_View **pp, u8 moreToFollow){
  மரம்_View *p = *pp;
  எனில்( p==0 ){
    *pp = p = sqlite3_malloc64( அளவு(*p) );
    எனில்( p==0 ) திரும்பு;
    memset(p, 0, அளவு(*p));
  }இல்லையெனில்{
    p->iLevel++;
  }
  ப்ரதிஜ்ஞா( moreToFollow==0 || moreToFollow==1 );
  எனில்( p->iLevel<(முழு)அளவு(p->b_வரி) ) p->b_வரி[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
நிலையான வெறுமை sqlite3_மரம்_View_எடு(மரம்_View **pp){
  மரம்_View *p = *pp;
  எனில்( p==0 ) திரும்பு;
  p->iLevel--;
  எனில்( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
வெறுமை sqlite3_மரம்_View_வரி(மரம்_View *p, மாறிலி எழுத்து *zFormat, ...){
  va_list ap;
  முழு i;
  StrAccum acc;
  எழுத்து zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, அளவு(zBuf), 0);
  எனில்( p ){
    க்கு(i=0; i<p->iLevel && i<(முழு)அளவு(p->b_வரி)-1; i++){
      sqlite3_str_append(&acc, p->b_வரி[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_வரி[i] ? "|-- " : "'-- ", 4);
  }
  எனில்( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    ப்ரதிஜ்ஞா( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
நிலையான வெறுமை sqlite3_மரம்_View_Item(மரம்_View *p, மாறிலி எழுத்து *zLabel,u8 moreFollows){
  sqlite3_மரம்_View_தள்ளு(&p, moreFollows);
  sqlite3_மரம்_View_வரி(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
வெறுமை sqlite3_மரம்_View_Column_List(
  மரம்_View *pView,
  மாறிலி Column *aCol,
  முழு nCol,
  u8 moreToFollow
){
  முழு i;
  sqlite3_மரம்_View_தள்ளு(&pView, moreToFollow);
  sqlite3_மரம்_View_வரி(pView, "COLUMNS");
  க்கு(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    முழு colMoreToFollow = i<(nCol - 1);
    sqlite3_மரம்_View_தள்ளு(&pView, colMoreToFollow);
    sqlite3_மரம்_View_வரி(pView, 0);
    printf(" %s", aCol[i].zCnName);
    தேர்வு( aCol[i].eCType ){
      நிலை COLTYPE_ANY:      printf(" ANY");        நிறுத்து;
      நிலை COLTYPE_BLOB:     printf(" BLOB");       நிறுத்து;
      நிலை COLTYPE_INT:      printf(" INT");        நிறுத்து;
      நிலை COLTYPE_INTEGER:  printf(" INTEGER");    நிறுத்து;
      நிலை COLTYPE_REAL:     printf(" REAL");       நிறுத்து;
      நிலை COLTYPE_TEXT:     printf(" TEXT");       நிறுத்து;
      நிலை COLTYPE_CUSTOM: {
        எனில்( flg & COLFLAG_HASTYPE ){
          மாறிலி எழுத்து *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          நிறுத்து;
        }
      }
    }
    எனில்( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    எனில்( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    எனில்( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    எனில்( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_மரம்_View_எடு(&pView);
  }
  sqlite3_மரம்_View_எடு(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
வெறுமை sqlite3_மரம்_View_With(மரம்_View *pView, மாறிலி With *pWith, u8 moreToFollow){
  முழு i;
  எனில்( pWith==0 ) திரும்பு;
  எனில்( pWith->nCte==0 ) திரும்பு;
  எனில்( pWith->pOuter ){
    sqlite3_மரம்_View_வரி(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }இல்லையெனில்{
    sqlite3_மரம்_View_வரி(pView, "WITH (0x%p)", pWith);
  }
  எனில்( pWith->nCte>0 ){
    sqlite3_மரம்_View_தள்ளு(&pView, moreToFollow);
    க்கு(i=0; i<pWith->nCte; i++){
      StrAccum x;
      எழுத்து z_வரி[1000];
      மாறிலி கட்டமைப்பு Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_வரி, அளவு(z_வரி), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      எனில்( pCte->pCols && pCte->pCols->n_கோவை>0 ){
        எழுத்து cSep = '(';
        முழு j;
        க்கு(j=0; j<pCte->pCols->n_கோவை; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      எனில்( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      எனில்( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_மரம்_View_Item(pView, z_வரி, i<pWith->nCte-1);
      sqlite3_மரம்_View_Select(pView, pCte->pSelect, 0);
      sqlite3_மரம்_View_எடு(&pView);
    }
    sqlite3_மரம்_View_எடு(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
வெறுமை sqlite3_மரம்_View_Src_List(மரம்_View *pView, மாறிலி SrcList *pSrc){
  முழு i;
  எனில்( pSrc==0 ) திரும்பு;
  க்கு(i=0; i<pSrc->nSrc; i++){
    மாறிலி SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    முழு n = 0;
    எழுத்து z_வரி[1000];
    sqlite3StrAccumInit(&x, 0, z_வரி, அளவு(z_வரி), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    எனில்( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    எனில்( (pItem->fg.jointype & (JT_இடது|JT_வலது))==(JT_இடது|JT_வலது) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }இல்லையெனில் எனில்( pItem->fg.jointype & JT_இடது ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }இல்லையெனில் எனில்( pItem->fg.jointype & JT_வலது ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }இல்லையெனில் எனில்( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    எனில்( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    எனில்( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    எனில்( pItem->fg.isCte ){
      நிலையான மாறிலி எழுத்து *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    எனில்( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    எனில்( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    எனில்( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    எனில்( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    எனில்( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    எனில்( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    எனில்( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    எனில்( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    எனில்( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    எனில்( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_மரம்_View_Item(pView, z_வரி, i<pSrc->nSrc-1);
    n = 0;
    எனில்( pItem->fg.isSubquery ) n++;
    எனில்( pItem->fg.isTabFunc ) n++;
    எனில்( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    எனில்( pItem->fg.isUsing ){
      sqlite3_மரம்_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }இல்லையெனில் எனில்( pItem->u3.pOn!=0 ){
      sqlite3_மரம்_View_Item(pView, "ON", (--n)>0);
      sqlite3_மரம்_View_கோவை(pView, pItem->u3.pOn, 0);
      sqlite3_மரம்_View_எடு(&pView);
    }
    எனில்( pItem->fg.isSubquery ){
      ப்ரதிஜ்ஞா( n==1 );
      எனில்( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_மரம்_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      ப்ரதிஜ்ஞா( (முழு)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_மரம்_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    எனில்( pItem->fg.isTabFunc ){
      sqlite3_மரம்_View_கோவை_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_மரம்_View_எடு(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
வெறுமை sqlite3_மரம்_View_Select(மரம்_View *pView, மாறிலி Select *p, u8 moreToFollow){
  முழு n = 0;
  முழு cnt = 0;
  எனில்( p==0 ){
    sqlite3_மரம்_View_வரி(pView, "nil-SELECT");
    திரும்பு;
  } 
  sqlite3_மரம்_View_தள்ளு(&pView, moreToFollow);
  எனில்( p->pWith ){
    sqlite3_மரம்_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_மரம்_View_தள்ளு(&pView, 1);
  }
  செய்{
    எனில்( p->selFlags & SF_WhereBegin ){
      sqlite3_மரம்_View_வரி(pView, "sqlite3WhereBegin()");
    }இல்லையெனில்{
      sqlite3_மரம்_View_வரி(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (முழு)p->nSelectRow
      );
    }
    எனில்( cnt++ ) sqlite3_மரம்_View_எடு(&pView);
    எனில்( p->pPrior ){
      n = 1000;
    }இல்லையெனில்{
      n = 0;
      எனில்( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      எனில்( p->pWhere ) n++;
      எனில்( p->pGroupBy ) n++;
      எனில்( p->pHaving ) n++;
      எனில்( p->pOrderBy ) n++;
      எனில்( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      எனில்( p->pWin ) n++;
      எனில்( p->pWinDefn ) n++;
#endif
    }
    எனில்( p->pEList ){
      sqlite3_மரம்_View_கோவை_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    எனில்( p->pWin ){
      Window *pX;
      sqlite3_மரம்_View_தள்ளு(&pView, (n--)>0);
      sqlite3_மரம்_View_வரி(pView, "window-functions");
      க்கு(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_மரம்_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_மரம்_View_எடு(&pView);
    }
#endif
    எனில்( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_மரம்_View_தள்ளு(&pView, (n--)>0);
      sqlite3_மரம்_View_வரி(pView, "FROM");
      sqlite3_மரம்_View_Src_List(pView, p->pSrc);
      sqlite3_மரம்_View_எடு(&pView);
    }
    எனில்( p->pWhere ){
      sqlite3_மரம்_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_மரம்_View_கோவை(pView, p->pWhere, 0);
      sqlite3_மரம்_View_எடு(&pView);
    }
    எனில்( p->pGroupBy ){
      sqlite3_மரம்_View_கோவை_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    எனில்( p->pHaving ){
      sqlite3_மரம்_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_மரம்_View_கோவை(pView, p->pHaving, 0);
      sqlite3_மரம்_View_எடு(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    எனில்( p->pWinDefn ){
      Window *pX;
      sqlite3_மரம்_View_Item(pView, "WINDOW", (n--)>0);
      க்கு(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_மரம்_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_மரம்_View_எடு(&pView);
    }
#endif
    எனில்( p->pOrderBy ){
      sqlite3_மரம்_View_கோவை_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    எனில்( p->pLimit ){
      sqlite3_மரம்_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_மரம்_View_கோவை(pView, p->pLimit->p_இடது, p->pLimit->p_வலது!=0);
      எனில்( p->pLimit->p_வலது ){
        sqlite3_மரம்_View_Item(pView, "OFFSET", 0);
        sqlite3_மரம்_View_கோவை(pView, p->pLimit->p_வலது, 0);
        sqlite3_மரம்_View_எடு(&pView);
      }
      sqlite3_மரம்_View_எடு(&pView);
    }
    எனில்( p->pPrior ){
      மாறிலி எழுத்து *zOp = "UNION";
      தேர்வு( p->op ){
        நிலை TK_ALL:         zOp = "UNION ALL";  நிறுத்து;
        நிலை TK_INTERSECT:   zOp = "INTERSECT";  நிறுத்து;
        நிலை TK_EXCEPT:      zOp = "EXCEPT";     நிறுத்து;
      }
      sqlite3_மரம்_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }போது( p!=0 );
  sqlite3_மரம்_View_எடு(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
வெறுமை sqlite3_மரம்_View_Bound(
  மரம்_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  கோவை *p_கோவை,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  தேர்வு( eBound ){
    நிலை TK_UNBOUNDED: {
      sqlite3_மரம்_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_மரம்_View_எடு(&pView);
      நிறுத்து;
    }
    நிலை TK_CURRENT: {
      sqlite3_மரம்_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_மரம்_View_எடு(&pView);
      நிறுத்து;
    }
    நிலை TK_PRECEDING: {
      sqlite3_மரம்_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_மரம்_View_கோவை(pView, p_கோவை, 0);
      sqlite3_மரம்_View_எடு(&pView);
      நிறுத்து;
    }
    நிலை TK_FOLLOWING: {
      sqlite3_மரம்_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_மரம்_View_கோவை(pView, p_கோவை, 0);
      sqlite3_மரம்_View_எடு(&pView);
      நிறுத்து;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
வெறுமை sqlite3_மரம்_View_Window(மரம்_View *pView, மாறிலி Window *pWin, u8 more){
  முழு nElement = 0;
  எனில்( pWin==0 ) திரும்பு;
  எனில்( pWin->pFilter ){
    sqlite3_மரம்_View_Item(pView, "FILTER", 1);
    sqlite3_மரம்_View_கோவை(pView, pWin->pFilter, 0);
    sqlite3_மரம்_View_எடு(&pView);
    எனில்( pWin->eFrmType==TK_FILTER ) திரும்பு;
  }
  sqlite3_மரம்_View_தள்ளு(&pView, more);
  எனில்( pWin->zName ){
    sqlite3_மரம்_View_வரி(pView, "OVER %s (%p)", pWin->zName, pWin);
  }இல்லையெனில்{
    sqlite3_மரம்_View_வரி(pView, "OVER (%p)", pWin);
  }
  எனில்( pWin->zBase )    nElement++;
  எனில்( pWin->pOrderBy ) nElement++;
  எனில்( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  எனில்( pWin->eExclude ) nElement++;
  எனில்( pWin->zBase ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--nElement)>0);
    sqlite3_மரம்_View_வரி(pView, "window: %s", pWin->zBase);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pWin->pPartition ){
    sqlite3_மரம்_View_கோவை_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  எனில்( pWin->pOrderBy ){
    sqlite3_மரம்_View_கோவை_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  எனில்( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    எழுத்து zBuf[30];
    மாறிலி எழுத்து *zFrmType = "ROWS";
    எனில்( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    எனில்( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(அளவு(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_மரம்_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_மரம்_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_மரம்_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pWin->eExclude ){
    எழுத்து zBuf[30];
    மாறிலி எழுத்து *zExclude;
    தேர்வு( pWin->eExclude ){
      நிலை TK_NO:      zExclude = "NO OTHERS";   நிறுத்து;
      நிலை TK_CURRENT: zExclude = "CURRENT ROW"; நிறுத்து;
      நிலை TK_GROUP:   zExclude = "GROUP";       நிறுத்து;
      நிலை TK_TIES:    zExclude = "TIES";        நிறுத்து;
      இயல்பு:
        sqlite3_snprintf(அளவு(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        நிறுத்து;
    }
    sqlite3_மரம்_View_தள்ளு(&pView, 0);
    sqlite3_மரம்_View_வரி(pView, "EXCLUDE %s", zExclude);
    sqlite3_மரம்_View_எடு(&pView);
  }
  sqlite3_மரம்_View_எடு(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
வெறுமை sqlite3_மரம்_View_Win_Func(மரம்_View *pView, மாறிலி Window *pWin, u8 more){
  எனில்( pWin==0 ) திரும்பு;
  sqlite3_மரம்_View_தள்ளு(&pView, more);
  sqlite3_மரம்_View_வரி(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_மரம்_View_Window(pView, pWin, 0);
  sqlite3_மரம்_View_எடு(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
வெறுமை sqlite3_மரம்_View_கோவை(மரம்_View *pView, மாறிலி கோவை *p_கோவை, u8 moreToFollow){
  மாறிலி எழுத்து *zBinOp = 0;   /* Binary operator */
  மாறிலி எழுத்து *zUniOp = 0;   /* Unary operator */
  எழுத்து zFlgs[200];
  sqlite3_மரம்_View_தள்ளு(&pView, moreToFollow);
  எனில்( p_கோவை==0 ){
    sqlite3_மரம்_View_வரி(pView, "nil");
    sqlite3_மரம்_View_எடு(&pView);
    திரும்பு;
  }
  எனில்( p_கோவை->flags || p_கோவை->aff_கோவை || p_கோவை->vvaFlags || p_கோவை->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, அளவு(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_கோவை->flags, p_கோவை->aff_கோவை ? p_கோவை->aff_கோவை : 'n');
    எனில்( கோவை_Has_Property(p_கோவை, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_கோவை->w.iJoin);
    }
    எனில்( கோவை_Has_Property(p_கோவை, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_கோவை->w.iJoin);
    }
    எனில்( கோவை_Has_Property(p_கோவை, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    எனில்( கோவை_Has_VVAProperty(p_கோவை, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    எனில்( p_கோவை->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_கோவை->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }இல்லையெனில்{
    zFlgs[0] = 0;
  }
  தேர்வு( p_கோவை->op ){
    நிலை TK_AGG_COLUMN: {
      sqlite3_மரம்_View_வரி(pView, "AGG{%d:%d}%s",
            p_கோவை->iTable, p_கோவை->iColumn, zFlgs);
      நிறுத்து;
    }
    நிலை TK_COLUMN: {
      எனில்( p_கோவை->iTable<0 ){
        /* This only happens when coding check constraints */
        எழுத்து zOp2[16];
        எனில்( p_கோவை->op2 ){
          sqlite3_snprintf(அளவு(zOp2),zOp2," op2=0x%02x",p_கோவை->op2);
        }இல்லையெனில்{
          zOp2[0] = 0;
        }
        sqlite3_மரம்_View_வரி(pView, "COLUMN(%d)%s%s",
                                    p_கோவை->iColumn, zFlgs, zOp2);
      }இல்லையெனில்{
        ப்ரதிஜ்ஞா( கோவை_Use_YTab(p_கோவை) );
        sqlite3_மரம்_View_வரி(pView, "{%d:%d} pTab=%p%s",
                        p_கோவை->iTable, p_கோவை->iColumn,
                        p_கோவை->y.pTab, zFlgs);
      }
      எனில்( கோவை_Has_Property(p_கோவை, EP_FixedCol) ){
        sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 0);
      }
      நிறுத்து;
    }
    நிலை TK_INTEGER: {
      எனில்( p_கோவை->flags & EP_Int_மதிப்பு ){
        sqlite3_மரம்_View_வரி(pView, "%d", p_கோவை->u.i_மதிப்பு);
      }இல்லையெனில்{
        sqlite3_மரம்_View_வரி(pView, "%s", p_கோவை->u.zToken);
      }
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    நிலை TK_FLOAT: {
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_மரம்_View_வரி(pView,"%s", p_கோவை->u.zToken);
      நிறுத்து;
    }
#endif
    நிலை TK_STRING: {
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_மரம்_View_வரி(pView,"%Q", p_கோவை->u.zToken);
      நிறுத்து;
    }
    நிலை TK_NULL: {
      sqlite3_மரம்_View_வரி(pView,"NULL");
      நிறுத்து;
    }
    நிலை TK_TRUEFALSE: {
      sqlite3_மரம்_View_வரி(pView,"%s%s",
         sqlite3_கோவை_Truth_மதிப்பு(p_கோவை) ? "TRUE" : "FALSE", zFlgs);
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    நிலை TK_BLOB: {
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_மரம்_View_வரி(pView,"%s", p_கோவை->u.zToken);
      நிறுத்து;
    }
#endif
    நிலை TK_VARIABLE: {
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_மரம்_View_வரி(pView,"VARIABLE(%s,%d)",
                          p_கோவை->u.zToken, p_கோவை->iColumn);
      நிறுத்து;
    }
    நிலை TK_REGISTER: {
      sqlite3_மரம்_View_வரி(pView,"REGISTER(%d)", p_கோவை->iTable);
      நிறுத்து;
    }
    நிலை TK_ID: {
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_மரம்_View_வரி(pView,"ID \"%w\"", p_கோவை->u.zToken);
      நிறுத்து;
    }
    நிலை TK_ASTERISK: {
      sqlite3_மரம்_View_வரி(pView,"ASKTERISK");
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_CAST
    நிலை TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_மரம்_View_வரி(pView,"CAST %Q", p_கோவை->u.zToken);
      sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 0);
      நிறுத்து;
    }
#endif /* SQLITE_OMIT_CAST */
    நிலை TK_LT:      zBinOp = "LT";     நிறுத்து;
    நிலை TK_LE:      zBinOp = "LE";     நிறுத்து;
    நிலை TK_GT:      zBinOp = "GT";     நிறுத்து;
    நிலை TK_GE:      zBinOp = "GE";     நிறுத்து;
    நிலை TK_NE:      zBinOp = "NE";     நிறுத்து;
    நிலை TK_EQ:      zBinOp = "EQ";     நிறுத்து;
    நிலை TK_IS:      zBinOp = "IS";     நிறுத்து;
    நிலை TK_ISNOT:   zBinOp = "ISNOT";  நிறுத்து;
    நிலை TK_AND:     zBinOp = "AND";    நிறுத்து;
    நிலை TK_OR:      zBinOp = "OR";     நிறுத்து;
    நிலை TK_PLUS:    zBinOp = "ADD";    நிறுத்து;
    நிலை TK_STAR:    zBinOp = "MUL";    நிறுத்து;
    நிலை TK_MINUS:   zBinOp = "SUB";    நிறுத்து;
    நிலை TK_REM:     zBinOp = "REM";    நிறுத்து;
    நிலை TK_BITAND:  zBinOp = "BITAND"; நிறுத்து;
    நிலை TK_BITOR:   zBinOp = "BITOR";  நிறுத்து;
    நிலை TK_SLASH:   zBinOp = "DIV";    நிறுத்து;
    நிலை TK_LSHIFT:  zBinOp = "LSHIFT"; நிறுத்து;
    நிலை TK_RSHIFT:  zBinOp = "RSHIFT"; நிறுத்து;
    நிலை TK_CONCAT:  zBinOp = "CONCAT"; நிறுத்து;
    நிலை TK_DOT:     zBinOp = "DOT";    நிறுத்து;
    நிலை TK_LIMIT:   zBinOp = "LIMIT";  நிறுத்து;

    நிலை TK_UMINUS:  zUniOp = "UMINUS"; நிறுத்து;
    நிலை TK_UPLUS:   zUniOp = "UPLUS";  நிறுத்து;
    நிலை TK_BITNOT:  zUniOp = "BITNOT"; நிறுத்து;
    நிலை TK_NOT:     zUniOp = "NOT";    நிறுத்து;
    நிலை TK_ISNULL:  zUniOp = "ISNULL"; நிறுத்து;
    நிலை TK_NOTNULL: zUniOp = "NOTNULL"; நிறுத்து;

    நிலை TK_TRUTH: {
      முழு x;
      மாறிலி எழுத்து *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      ப்ரதிஜ்ஞா( p_கோவை->op2==TK_IS || p_கோவை->op2==TK_ISNOT );
      ப்ரதிஜ்ஞா( p_கோவை->p_வலது );
      ப்ரதிஜ்ஞா( sqlite3_கோவை_Skip_Collate_And_Likely(p_கோவை->p_வலது)->op
                  == TK_TRUEFALSE );
      x = (p_கோவை->op2==TK_ISNOT)*2 + sqlite3_கோவை_Truth_மதிப்பு(p_கோவை->p_வலது);
      zUniOp = azOp[x];
      நிறுத்து;
    }

    நிலை TK_SPAN: {
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_மரம்_View_வரி(pView, "SPAN %Q", p_கோவை->u.zToken);
      sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 0);
      நிறுத்து;
    }

    நிலை TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_மரம்_View_வரி(pView, "%sCOLLATE %Q%s",
        !கோவை_Has_Property(p_கோவை, EP_Collate) ? "SOFT-" : "",
        p_கோவை->u.zToken, zFlgs);
      sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 0);
      நிறுத்து;
    }

    நிலை TK_AGG_FUNCTION:
    நிலை TK_FUNCTION: {
      கோவை_List *pFarg;       /* List of function arguments */
      Window *pWin;
      எனில்( கோவை_Has_Property(p_கோவை, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }இல்லையெனில்{
        ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
        pFarg = p_கோவை->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_கோவை) ? p_கோவை->y.pWin : 0;
#இல்லையெனில்
        pWin = 0;
#endif 
      }
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      எனில்( p_கோவை->op==TK_AGG_FUNCTION ){
        sqlite3_மரம்_View_வரி(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_கோவை->op2, p_கோவை->u.zToken, zFlgs,
                             p_கோவை->pAggInfo ? p_கோவை->pAggInfo->selId : 0,
                             p_கோவை->iAgg, p_கோவை->pAggInfo);
      }இல்லையெனில் எனில்( p_கோவை->op2!=0 ){
        மாறிலி எழுத்து *zOp2;
        எழுத்து zBuf[8];
        sqlite3_snprintf(அளவு(zBuf),zBuf,"0x%02x",p_கோவை->op2);
        zOp2 = zBuf;
        எனில்( p_கோவை->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        எனில்( p_கோவை->op2==NC_Idx_கோவை ) zOp2 = "NC_IdxExpr";
        எனில்( p_கோவை->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        எனில்( p_கோவை->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_மரம்_View_வரி(pView, "FUNCTION %Q%s op2=%s",
                            p_கோவை->u.zToken, zFlgs, zOp2);
      }இல்லையெனில்{
        sqlite3_மரம்_View_வரி(pView, "FUNCTION %Q%s", p_கோவை->u.zToken, zFlgs);
      }
      எனில்( pFarg ){
        sqlite3_மரம்_View_கோவை_List(pView, pFarg, pWin!=0 || p_கோவை->p_இடது, 0);
        எனில்( p_கோவை->p_இடது ){
          கோவை *pOB = p_கோவை->p_இடது;
          ப்ரதிஜ்ஞா( pOB->op==TK_ORDER );
          ப்ரதிஜ்ஞா( கோவை_Use_XList(pOB) );
          sqlite3_மரம்_View_கோவை_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      எனில்( pWin ){
        sqlite3_மரம்_View_Window(pView, pWin, 0);
      }
#endif
      நிறுத்து;
    }
    நிலை TK_ORDER: {
      sqlite3_மரம்_View_கோவை_List(pView, p_கோவை->x.pList, 0, "ORDERBY");
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    நிலை TK_EXISTS: {
      ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p_கோவை) );
      sqlite3_மரம்_View_வரி(pView, "EXISTS-expr flags=0x%x", p_கோவை->flags);
      sqlite3_மரம்_View_Select(pView, p_கோவை->x.pSelect, 0);
      நிறுத்து;
    }
    நிலை TK_SELECT: {
      ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p_கோவை) );
      sqlite3_மரம்_View_வரி(pView, "subquery-expr flags=0x%x", p_கோவை->flags);
      sqlite3_மரம்_View_Select(pView, p_கோவை->x.pSelect, 0);
      நிறுத்து;
    }
    நிலை TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      எழுத்து *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_கோவை->flags);
      எனில்( p_கோவை->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_கோவை->iTable);
      எனில்( கோவை_Has_Property(p_கோவை, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_கோவை->y.sub.regReturn, p_கோவை->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_மரம்_View_வரி(pView, z);
      sqlite3_free(z);
      sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 1);
      எனில்( கோவை_Use_XSelect(p_கோவை) ){
        sqlite3_மரம்_View_Select(pView, p_கோவை->x.pSelect, 0);
      }இல்லையெனில்{
        sqlite3_மரம்_View_கோவை_List(pView, p_கோவை->x.pList, 0, 0);
      }
      நிறுத்து;
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
    நிலை TK_BETWEEN: {
      மாறிலி கோவை *pX, *pY, *pZ;
      pX = p_கோவை->p_இடது;
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
      ப்ரதிஜ்ஞா( p_கோவை->x.pList->n_கோவை==2 );
      pY = p_கோவை->x.pList->a[0].p_கோவை;
      pZ = p_கோவை->x.pList->a[1].p_கோவை;
      sqlite3_மரம்_View_வரி(pView, "BETWEEN%s", zFlgs);
      sqlite3_மரம்_View_கோவை(pView, pX, 1);
      sqlite3_மரம்_View_கோவை(pView, pY, 1);
      sqlite3_மரம்_View_கோவை(pView, pZ, 0);
      நிறுத்து;
    }
    நிலை TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_மரம்_View_வரி(pView, "%s(%d)", 
          p_கோவை->iTable ? "NEW" : "OLD", p_கோவை->iColumn);
      நிறுத்து;
    }
    நிலை TK_CASE: {
      sqlite3_மரம்_View_வரி(pView, "CASE");
      sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 1);
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
      sqlite3_மரம்_View_கோவை_List(pView, p_கோவை->x.pList, 0, 0);
      நிறுத்து;
    }
#ifndef SQLITE_OMIT_TRIGGER
    நிலை TK_RAISE: {
      மாறிலி எழுத்து *zType = "unk";
      தேர்வு( p_கோவை->aff_கோவை ){
        நிலை OE_Rollback:   zType = "rollback";  நிறுத்து;
        நிலை OE_Abort:      zType = "abort";     நிறுத்து;
        நிலை OE_Fail:       zType = "fail";      நிறுத்து;
        நிலை OE_Ignore:     zType = "ignore";    நிறுத்து;
      }
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      sqlite3_மரம்_View_வரி(pView, "RAISE %s", zType);
      sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 0);
      நிறுத்து;
    }
#endif
    நிலை TK_MATCH: {
      sqlite3_மரம்_View_வரி(pView, "MATCH {%d:%d}%s",
                          p_கோவை->iTable, p_கோவை->iColumn, zFlgs);
      sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_வலது, 0);
      நிறுத்து;
    }
    நிலை TK_VECTOR: {
      எழுத்து *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
      sqlite3_மரம்_View_Bare_கோவை_List(pView, p_கோவை->x.pList, z);
      sqlite3_free(z);
      நிறுத்து;
    }
    நிலை TK_SELECT_COLUMN: {
      sqlite3_மரம்_View_வரி(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_கோவை->iColumn, p_கோவை->iTable-1,
              p_கோவை->p_வலது==p_கோவை->p_இடது ? " (SELECT-owner)" : "");
      ப்ரதிஜ்ஞா( கோவை_Use_XSelect(p_கோவை->p_இடது) );
      sqlite3_மரம்_View_Select(pView, p_கோவை->p_இடது->x.pSelect, 0);
      நிறுத்து;
    }
    நிலை TK_IF_NULL_ROW: {
      sqlite3_மரம்_View_வரி(pView, "IF-NULL-ROW %d", p_கோவை->iTable);
      sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 0);
      நிறுத்து;
    }
    நிலை TK_ERROR: {
      கோவை tmp;
      sqlite3_மரம்_View_வரி(pView, "ERROR");
      tmp = *p_கோவை;
      tmp.op = p_கோவை->op2;
      sqlite3_மரம்_View_கோவை(pView, &tmp, 0);
      நிறுத்து;
    }
    நிலை TK_ROW: {
      எனில்( p_கோவை->iColumn<=0 ){
        sqlite3_மரம்_View_வரி(pView, "First FROM table rowid");
      }இல்லையெனில்{
        sqlite3_மரம்_View_வரி(pView, "First FROM table column %d",
            p_கோவை->iColumn-1);
      }
      நிறுத்து;
    }
    இயல்பு: {
      sqlite3_மரம்_View_வரி(pView, "op=%d", p_கோவை->op);
      நிறுத்து;
    }
  }
  எனில்( zBinOp ){
    sqlite3_மரம்_View_வரி(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 1);
    sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_வலது, 0);
  }இல்லையெனில் எனில்( zUniOp ){
    sqlite3_மரம்_View_வரி(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_மரம்_View_கோவை(pView, p_கோவை->p_இடது, 0);
  }
  sqlite3_மரம்_View_எடு(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
வெறுமை sqlite3_மரம்_View_Bare_கோவை_List(
  மரம்_View *pView,
  மாறிலி கோவை_List *pList,
  மாறிலி எழுத்து *zLabel
){
  எனில்( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  எனில்( pList==0 ){
    sqlite3_மரம்_View_வரி(pView, "%s (empty)", zLabel);
  }இல்லையெனில்{
    முழு i;
    sqlite3_மரம்_View_வரி(pView, "%s", zLabel);
    க்கு(i=0; i<pList->n_கோவை; i++){
      முழு j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      எழுத்து *zName = pList->a[i].zEName;
      முழு moreToFollow = i<pList->n_கோவை - 1;
      எனில்( j || zName || sortFlags ){
        sqlite3_மரம்_View_தள்ளு(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_மரம்_View_வரி(pView, 0);
        எனில்( zName ){
          தேர்வு( pList->a[i].fg.eEName ){
            இயல்பு:
              fprintf(stdout, "AS %s ", zName);
              நிறுத்து;
            நிலை ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              எனில்( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              எனில்( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              எனில்( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              நிறுத்து;
            நிலை ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              நிறுத்து;
          }
        }
        எனில்( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        எனில்( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }இல்லையெனில் எனில்( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_மரம்_View_கோவை(pView, pList->a[i].p_கோவை, moreToFollow);
      எனில்( j || zName || sortFlags ){
        sqlite3_மரம்_View_எடு(&pView);
      }
    }
  }
}
வெறுமை sqlite3_மரம்_View_கோவை_List(
  மரம்_View *pView,
  மாறிலி கோவை_List *pList,
  u8 moreToFollow,
  மாறிலி எழுத்து *zLabel
){
  sqlite3_மரம்_View_தள்ளு(&pView, moreToFollow);
  sqlite3_மரம்_View_Bare_கோவை_List(pView, pList, zLabel);
  sqlite3_மரம்_View_எடு(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
வெறுமை sqlite3_மரம்_View_Bare_Id_List(
  மரம்_View *pView,
  மாறிலி IdList *pList,
  மாறிலி எழுத்து *zLabel
){
  எனில்( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  எனில்( pList==0 ){
    sqlite3_மரம்_View_வரி(pView, "%s (empty)", zLabel);
  }இல்லையெனில்{
    முழு i;
    sqlite3_மரம்_View_வரி(pView, "%s", zLabel);
    க்கு(i=0; i<pList->nId; i++){
      எழுத்து *zName = pList->a[i].zName;
      முழு moreToFollow = i<pList->nId - 1;
      எனில்( zName==0 ) zName = "(null)";
      sqlite3_மரம்_View_தள்ளு(&pView, moreToFollow);
      sqlite3_மரம்_View_வரி(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_மரம்_View_எடு(&pView);
    }
  }
}
வெறுமை sqlite3_மரம்_View_Id_List(
  மரம்_View *pView,
  மாறிலி IdList *pList,
  u8 moreToFollow,
  மாறிலி எழுத்து *zLabel
){
  sqlite3_மரம்_View_தள்ளு(&pView, moreToFollow);
  sqlite3_மரம்_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_மரம்_View_எடு(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
வெறுமை sqlite3_மரம்_View_Upsert(
  மரம்_View *pView,
  மாறிலி Upsert *pUpsert,
  u8 moreToFollow
){
  எனில்( pUpsert==0 ) திரும்பு;
  sqlite3_மரம்_View_தள்ளு(&pView, moreToFollow);
  போது( pUpsert ){
    முழு n;
    sqlite3_மரம்_View_தள்ளு(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_மரம்_View_வரி(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_மரம்_View_கோவை_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_மரம்_View_கோவை_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    எனில்( pUpsert->pUpsertWhere ){
      sqlite3_மரம்_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_மரம்_View_கோவை(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_மரம்_View_எடு(&pView);
    }
    sqlite3_மரம்_View_எடு(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_மரம்_View_எடு(&pView);
}

#எனில் TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
வெறுமை sqlite3_மரம்_View_Delete(
  மாறிலி With *pWith,
  மாறிலி SrcList *pTabList,
  மாறிலி கோவை *pWhere,
  மாறிலி கோவை_List *pOrderBy,
  மாறிலி கோவை *pLimit,
  மாறிலி Trigger *pTrigger
){
  முழு n = 0;
  மரம்_View *pView = 0;
  sqlite3_மரம்_View_தள்ளு(&pView, 0);
  sqlite3_மரம்_View_வரி(pView, "DELETE");
  எனில்( pWith ) n++;
  எனில்( pTabList ) n++;
  எனில்( pWhere ) n++;
  எனில்( pOrderBy ) n++;
  எனில்( pLimit ) n++;
  எனில்( pTrigger ) n++;
  எனில்( pWith ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_With(pView, pWith, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pTabList ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "FROM");
    sqlite3_மரம்_View_Src_List(pView, pTabList);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pWhere ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "WHERE");
    sqlite3_மரம்_View_கோவை(pView, pWhere, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pOrderBy ){
    sqlite3_மரம்_View_கோவை_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  எனில்( pLimit ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "LIMIT");
    sqlite3_மரம்_View_கோவை(pView, pLimit, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pTrigger ){
    sqlite3_மரம்_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_மரம்_View_எடு(&pView);
}
#endif /* TREETRACE_ENABLED */

#எனில் TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
வெறுமை sqlite3_மரம்_View_செருகு(
  மாறிலி With *pWith,
  மாறிலி SrcList *pTabList,
  மாறிலி IdList *pColumnList,
  மாறிலி Select *pSelect,
  மாறிலி கோவை_List *p_கோவை_List,
  முழு onError,
  மாறிலி Upsert *pUpsert,
  மாறிலி Trigger *pTrigger
){
  மரம்_View *pView = 0;
  முழு n = 0;
  மாறிலி எழுத்து *zLabel = "INSERT";
  தேர்வு( onError ){
    நிலை OE_Replace:  zLabel = "REPLACE";             நிறுத்து;
    நிலை OE_Ignore:   zLabel = "INSERT OR IGNORE";    நிறுத்து;
    நிலை OE_Rollback: zLabel = "INSERT OR ROLLBACK";  நிறுத்து;
    நிலை OE_Abort:    zLabel = "INSERT OR ABORT";     நிறுத்து;
    நிலை OE_Fail:     zLabel = "INSERT OR FAIL";      நிறுத்து;
  }
  sqlite3_மரம்_View_தள்ளு(&pView, 0);
  sqlite3_மரம்_View_வரி(pView, zLabel);
  எனில்( pWith ) n++;
  எனில்( pTabList ) n++;
  எனில்( pColumnList ) n++;
  எனில்( pSelect ) n++;
  எனில்( p_கோவை_List ) n++;
  எனில்( pUpsert ) n++;
  எனில்( pTrigger ) n++;
  எனில்( pWith ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_With(pView, pWith, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pTabList ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "INTO");
    sqlite3_மரம்_View_Src_List(pView, pTabList);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pColumnList ){
    sqlite3_மரம்_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  எனில்( pSelect ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "DATA-SOURCE");
    sqlite3_மரம்_View_Select(pView, pSelect, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( p_கோவை_List ){
    sqlite3_மரம்_View_கோவை_List(pView, p_கோவை_List, (--n)>0, "VALUES");
  }
  எனில்( pUpsert ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "UPSERT");
    sqlite3_மரம்_View_Upsert(pView, pUpsert, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pTrigger ){
    sqlite3_மரம்_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_மரம்_View_எடு(&pView);
}
#endif /* TREETRACE_ENABLED */

#எனில் TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
வெறுமை sqlite3_மரம்_View_Update(
  மாறிலி With *pWith,
  மாறிலி SrcList *pTabList,
  மாறிலி கோவை_List *pChanges,
  மாறிலி கோவை *pWhere,
  முழு onError,
  மாறிலி கோவை_List *pOrderBy,
  மாறிலி கோவை *pLimit,
  மாறிலி Upsert *pUpsert,
  மாறிலி Trigger *pTrigger
){
  முழு n = 0;
  மரம்_View *pView = 0;
  மாறிலி எழுத்து *zLabel = "UPDATE";
  தேர்வு( onError ){
    நிலை OE_Replace:  zLabel = "UPDATE OR REPLACE";   நிறுத்து;
    நிலை OE_Ignore:   zLabel = "UPDATE OR IGNORE";    நிறுத்து;
    நிலை OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  நிறுத்து;
    நிலை OE_Abort:    zLabel = "UPDATE OR ABORT";     நிறுத்து;
    நிலை OE_Fail:     zLabel = "UPDATE OR FAIL";      நிறுத்து;
  }
  sqlite3_மரம்_View_தள்ளு(&pView, 0);
  sqlite3_மரம்_View_வரி(pView, zLabel);
  எனில்( pWith ) n++;
  எனில்( pTabList ) n++;
  எனில்( pChanges ) n++;
  எனில்( pWhere ) n++;
  எனில்( pOrderBy ) n++;
  எனில்( pLimit ) n++;
  எனில்( pUpsert ) n++;
  எனில்( pTrigger ) n++;
  எனில்( pWith ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_With(pView, pWith, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pTabList ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "FROM");
    sqlite3_மரம்_View_Src_List(pView, pTabList);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pChanges ){
    sqlite3_மரம்_View_கோவை_List(pView, pChanges, (--n)>0, "SET");
  }
  எனில்( pWhere ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "WHERE");
    sqlite3_மரம்_View_கோவை(pView, pWhere, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pOrderBy ){
    sqlite3_மரம்_View_கோவை_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  எனில்( pLimit ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "LIMIT");
    sqlite3_மரம்_View_கோவை(pView, pLimit, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pUpsert ){
    sqlite3_மரம்_View_தள்ளு(&pView, (--n)>0);
    sqlite3_மரம்_View_வரி(pView, "UPSERT");
    sqlite3_மரம்_View_Upsert(pView, pUpsert, 0);
    sqlite3_மரம்_View_எடு(&pView);
  }
  எனில்( pTrigger ){
    sqlite3_மரம்_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_மரம்_View_எடு(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
வெறுமை sqlite3_மரம்_View_Trigger_Step(
  மரம்_View *pView,
  மாறிலி TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  முழு cnt = 0;
  எனில்( pStep==0 ) திரும்பு;
  sqlite3_மரம்_View_தள்ளு(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  செய்{
    எனில்( cnt++ && pStep->pNext==0 ){
      sqlite3_மரம்_View_எடு(&pView);
      sqlite3_மரம்_View_தள்ளு(&pView, 0);
    }
    sqlite3_மரம்_View_வரி(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }போது( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_மரம்_View_எடு(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
வெறுமை sqlite3_மரம்_View_Trigger(
  மரம்_View *pView,
  மாறிலி Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  முழு cnt = 0;
  எனில்( pTrigger==0 ) திரும்பு;
  sqlite3_மரம்_View_தள்ளு(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  செய்{
    எனில்( cnt++ && pTrigger->pNext==0 ){
      sqlite3_மரம்_View_எடு(&pView);
      sqlite3_மரம்_View_தள்ளு(&pView, 0);
    }
    sqlite3_மரம்_View_வரி(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_மரம்_View_தள்ளு(&pView, 0);
    sqlite3_மரம்_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_மரம்_View_எடு(&pView);
  }போது( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_மரம்_View_எடு(&pView);
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
வெறுமை sqlite3_Show_கோவை(மாறிலி கோவை *p){ sqlite3_மரம்_View_கோவை(0,p,0); }
வெறுமை sqlite3_Show_கோவை_List(மாறிலி கோவை_List *p){ sqlite3_மரம்_View_கோவை_List(0,p,0,0);}
வெறுமை sqlite3ShowIdList(மாறிலி IdList *p){ sqlite3_மரம்_View_Id_List(0,p,0,0); }
வெறுமை sqlite3ShowSrcList(மாறிலி SrcList *p){
  மரம்_View *pView = 0;
  sqlite3_மரம்_View_தள்ளு(&pView, 0);
  sqlite3_மரம்_View_வரி(pView, "SRCLIST");
  sqlite3_மரம்_View_Src_List(pView,p);
  sqlite3_மரம்_View_எடு(&pView);
}
வெறுமை sqlite3ShowSelect(மாறிலி Select *p){ sqlite3_மரம்_View_Select(0,p,0); }
வெறுமை sqlite3ShowWith(மாறிலி With *p){ sqlite3_மரம்_View_With(0,p,0); }
வெறுமை sqlite3ShowUpsert(மாறிலி Upsert *p){ sqlite3_மரம்_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
வெறுமை sqlite3ShowTriggerStep(மாறிலி TriggerStep *p){
  sqlite3_மரம்_View_Trigger_Step(0,p,0,0);
}
வெறுமை sqlite3ShowTriggerStepList(மாறிலி TriggerStep *p){
  sqlite3_மரம்_View_Trigger_Step(0,p,0,1);
}
வெறுமை sqlite3ShowTrigger(மாறிலி Trigger *p){ sqlite3_மரம்_View_Trigger(0,p,0,0); }
வெறுமை sqlite3ShowTriggerList(மாறிலி Trigger *p){ sqlite3_மரம்_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
வெறுமை sqlite3ShowWindow(மாறிலி Window *p){ sqlite3_மரம்_View_Window(0,p,0); }
வெறுமை sqlite3ShowWinFunc(மாறிலி Window *p){ sqlite3_மரம்_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
