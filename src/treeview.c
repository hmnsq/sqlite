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
#અન્તર્ભાવ "sqliteInt.h"
#ifdef SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
સ્થિર શૂન્ય sqlite3_વૃક્ષ_View_ધકેલો(વૃક્ષ_View **pp, u8 moreToFollow){
  વૃક્ષ_View *p = *pp;
  જો( p==0 ){
    *pp = p = sqlite3_malloc64( કદ(*p) );
    જો( p==0 ) પાછા;
    memset(p, 0, કદ(*p));
  }નહીંતર{
    p->iLevel++;
  }
  પ્રતિજ્ઞા( moreToFollow==0 || moreToFollow==1 );
  જો( p->iLevel<(પૂર્ણાંક)કદ(p->b_લીટી) ) p->b_લીટી[p->iLevel] = moreToFollow;
}

/*
** Finished with one layer of the tree
*/
સ્થિર શૂન્ય sqlite3_વૃક્ષ_View_કાઢો(વૃક્ષ_View **pp){
  વૃક્ષ_View *p = *pp;
  જો( p==0 ) પાછા;
  p->iLevel--;
  જો( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
શૂન્ય sqlite3_વૃક્ષ_View_લીટી(વૃક્ષ_View *p, અચલ અક્ષર *zFormat, ...){
  va_list ap;
  પૂર્ણાંક i;
  StrAccum acc;
  અક્ષર zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, કદ(zBuf), 0);
  જો( p ){
    માટે(i=0; i<p->iLevel && i<(પૂર્ણાંક)કદ(p->b_લીટી)-1; i++){
      sqlite3_str_append(&acc, p->b_લીટી[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_લીટી[i] ? "|-- " : "'-- ", 4);
  }
  જો( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    પ્રતિજ્ઞા( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3StrAccumFinish(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
સ્થિર શૂન્ય sqlite3_વૃક્ષ_View_Item(વૃક્ષ_View *p, અચલ અક્ષર *zLabel,u8 moreFollows){
  sqlite3_વૃક્ષ_View_ધકેલો(&p, moreFollows);
  sqlite3_વૃક્ષ_View_લીટી(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
શૂન્ય sqlite3_વૃક્ષ_View_Column_List(
  વૃક્ષ_View *pView,
  અચલ Column *aCol,
  પૂર્ણાંક nCol,
  u8 moreToFollow
){
  પૂર્ણાંક i;
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, moreToFollow);
  sqlite3_વૃક્ષ_View_લીટી(pView, "COLUMNS");
  માટે(i=0; i<nCol; i++){
    u16 flg = aCol[i].colFlags;
    પૂર્ણાંક colMoreToFollow = i<(nCol - 1);
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, colMoreToFollow);
    sqlite3_વૃક્ષ_View_લીટી(pView, 0);
    printf(" %s", aCol[i].zCnName);
    પસંદગી( aCol[i].eCType ){
      સ્થિતિ COLTYPE_ANY:      printf(" ANY");        તોડો;
      સ્થિતિ COLTYPE_BLOB:     printf(" BLOB");       તોડો;
      સ્થિતિ COLTYPE_INT:      printf(" INT");        તોડો;
      સ્થિતિ COLTYPE_INTEGER:  printf(" INTEGER");    તોડો;
      સ્થિતિ COLTYPE_REAL:     printf(" REAL");       તોડો;
      સ્થિતિ COLTYPE_TEXT:     printf(" TEXT");       તોડો;
      સ્થિતિ COLTYPE_CUSTOM: {
        જો( flg & COLFLAG_HASTYPE ){
          અચલ અક્ષર *z = aCol[i].zCnName;
          z += strlen(z)+1;
          printf(" X-%s", z);
          તોડો;
        }
      }
    }
    જો( flg & COLFLAG_PRIMKEY ) printf(" PRIMARY KEY");
    જો( flg & COLFLAG_HIDDEN ) printf(" HIDDEN");
#ifdef COLFLAG_NOEXPAND
    જો( flg & COLFLAG_NOEXPAND ) printf(" NO-EXPAND");
#endif
    જો( flg ) printf(" flags=%04x", flg);
    printf("\n");      
    fflush(stdout);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
શૂન્ય sqlite3_વૃક્ષ_View_With(વૃક્ષ_View *pView, અચલ With *pWith, u8 moreToFollow){
  પૂર્ણાંક i;
  જો( pWith==0 ) પાછા;
  જો( pWith->nCte==0 ) પાછા;
  જો( pWith->pOuter ){
    sqlite3_વૃક્ષ_View_લીટી(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }નહીંતર{
    sqlite3_વૃક્ષ_View_લીટી(pView, "WITH (0x%p)", pWith);
  }
  જો( pWith->nCte>0 ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, moreToFollow);
    માટે(i=0; i<pWith->nCte; i++){
      StrAccum x;
      અક્ષર z_લીટી[1000];
      અચલ સંરચના Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_લીટી, કદ(z_લીટી), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      જો( pCte->pCols && pCte->pCols->n_પદાવલિ>0 ){
        અક્ષર cSep = '(';
        પૂર્ણાંક j;
        માટે(j=0; j<pCte->pCols->n_પદાવલિ; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->pCols->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      જો( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      જો( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3StrAccumFinish(&x);
      sqlite3_વૃક્ષ_View_Item(pView, z_લીટી, i<pWith->nCte-1);
      sqlite3_વૃક્ષ_View_Select(pView, pCte->pSelect, 0);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
શૂન્ય sqlite3_વૃક્ષ_View_Src_List(વૃક્ષ_View *pView, અચલ SrcList *pSrc){
  પૂર્ણાંક i;
  જો( pSrc==0 ) પાછા;
  માટે(i=0; i<pSrc->nSrc; i++){
    અચલ SrcItem *pItem = &pSrc->a[i];
    StrAccum x;
    પૂર્ણાંક n = 0;
    અક્ષર z_લીટી[1000];
    sqlite3StrAccumInit(&x, 0, z_લીટી, કદ(z_લીટી), 0);
    x.printfFlags |= SQLITE_PRINTF_INTERNAL;
    sqlite3_str_appendf(&x, "{%d:*} %!S", pItem->iCursor, pItem);
    જો( pItem->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           pItem->pSTab->zName, pItem->pSTab->nCol, pItem->pSTab, 
           pItem->colUsed,
           pItem->fg.rowidUsed ? "+rowid" : "");
    }
    જો( (pItem->fg.jointype & (JT_ડાબું|JT_જમણું))==(JT_ડાબું|JT_જમણું) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }નહીંતર જો( pItem->fg.jointype & JT_ડાબું ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }નહીંતર જો( pItem->fg.jointype & JT_જમણું ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }નહીંતર જો( pItem->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    જો( pItem->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    જો( pItem->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    જો( pItem->fg.isCte ){
      સ્થિર અચલ અક્ષર *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          pItem->u2.pCteUse->nUse,
                          aMat[pItem->u2.pCteUse->eM10d]);
    }
    જો( pItem->fg.isOn || (pItem->fg.isUsing==0 && pItem->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    જો( pItem->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    જો( pItem->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    જો( pItem->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    જો( pItem->fg.viaCoroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    જો( pItem->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    જો( pItem->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    જો( pItem->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    જો( pItem->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    જો( pItem->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3StrAccumFinish(&x);
    sqlite3_વૃક્ષ_View_Item(pView, z_લીટી, i<pSrc->nSrc-1);
    n = 0;
    જો( pItem->fg.isSubquery ) n++;
    જો( pItem->fg.isTabFunc ) n++;
    જો( pItem->fg.isUsing || pItem->u3.pOn!=0 ) n++;
    જો( pItem->fg.isUsing ){
      sqlite3_વૃક્ષ_View_Id_List(pView, pItem->u3.pUsing, (--n)>0, "USING");
    }નહીંતર જો( pItem->u3.pOn!=0 ){
      sqlite3_વૃક્ષ_View_Item(pView, "ON", (--n)>0);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, pItem->u3.pOn, 0);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
    જો( pItem->fg.isSubquery ){
      પ્રતિજ્ઞા( n==1 );
      જો( pItem->pSTab ){
        Table *pTab = pItem->pSTab;
        sqlite3_વૃક્ષ_View_Column_List(pView, pTab->aCol, pTab->nCol, 1);
      }
      પ્રતિજ્ઞા( (પૂર્ણાંક)pItem->fg.isNestedFrom == IsNestedFrom(pItem) );
      sqlite3_વૃક્ષ_View_Select(pView, pItem->u4.pSubq->pSelect, 0);
    }
    જો( pItem->fg.isTabFunc ){
      sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pItem->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
શૂન્ય sqlite3_વૃક્ષ_View_Select(વૃક્ષ_View *pView, અચલ Select *p, u8 moreToFollow){
  પૂર્ણાંક n = 0;
  પૂર્ણાંક cnt = 0;
  જો( p==0 ){
    sqlite3_વૃક્ષ_View_લીટી(pView, "nil-SELECT");
    પાછા;
  } 
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, moreToFollow);
  જો( p->pWith ){
    sqlite3_વૃક્ષ_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, 1);
  }
  કરો{
    જો( p->selFlags & SF_WhereBegin ){
      sqlite3_વૃક્ષ_View_લીટી(pView, "sqlite3WhereBegin()");
    }નહીંતર{
      sqlite3_વૃક્ષ_View_લીટી(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (પૂર્ણાંક)p->nSelectRow
      );
    }
    જો( cnt++ ) sqlite3_વૃક્ષ_View_કાઢો(&pView);
    જો( p->pPrior ){
      n = 1000;
    }નહીંતર{
      n = 0;
      જો( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      જો( p->pWhere ) n++;
      જો( p->pGroupBy ) n++;
      જો( p->pHaving ) n++;
      જો( p->pOrderBy ) n++;
      જો( p->pLimit ) n++;
#ifndef SQLITE_OMIT_WINDOWFUNC
      જો( p->pWin ) n++;
      જો( p->pWinDefn ) n++;
#endif
    }
    જો( p->pEList ){
      sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ifndef SQLITE_OMIT_WINDOWFUNC
    જો( p->pWin ){
      Window *pX;
      sqlite3_વૃક્ષ_View_ધકેલો(&pView, (n--)>0);
      sqlite3_વૃક્ષ_View_લીટી(pView, "window-functions");
      માટે(pX=p->pWin; pX; pX=pX->pNextWin){
        sqlite3_વૃક્ષ_View_Win_Func(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
#endif
    જો( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_વૃક્ષ_View_ધકેલો(&pView, (n--)>0);
      sqlite3_વૃક્ષ_View_લીટી(pView, "FROM");
      sqlite3_વૃક્ષ_View_Src_List(pView, p->pSrc);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
    જો( p->pWhere ){
      sqlite3_વૃક્ષ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p->pWhere, 0);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
    જો( p->pGroupBy ){
      sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, p->pGroupBy, (n--)>0, "GROUPBY");
    }
    જો( p->pHaving ){
      sqlite3_વૃક્ષ_View_Item(pView, "HAVING", (n--)>0);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p->pHaving, 0);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
#ifndef SQLITE_OMIT_WINDOWFUNC
    જો( p->pWinDefn ){
      Window *pX;
      sqlite3_વૃક્ષ_View_Item(pView, "WINDOW", (n--)>0);
      માટે(pX=p->pWinDefn; pX; pX=pX->pNextWin){
        sqlite3_વૃક્ષ_View_Window(pView, pX, pX->pNextWin!=0);
      }
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
#endif
    જો( p->pOrderBy ){
      sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, p->pOrderBy, (n--)>0, "ORDERBY");
    }
    જો( p->pLimit ){
      sqlite3_વૃક્ષ_View_Item(pView, "LIMIT", (n--)>0);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p->pLimit->p_ડાબું, p->pLimit->p_જમણું!=0);
      જો( p->pLimit->p_જમણું ){
        sqlite3_વૃક્ષ_View_Item(pView, "OFFSET", 0);
        sqlite3_વૃક્ષ_View_પદાવલિ(pView, p->pLimit->p_જમણું, 0);
        sqlite3_વૃક્ષ_View_કાઢો(&pView);
      }
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
    જો( p->pPrior ){
      અચલ અક્ષર *zOp = "UNION";
      પસંદગી( p->op ){
        સ્થિતિ TK_ALL:         zOp = "UNION ALL";  તોડો;
        સ્થિતિ TK_INTERSECT:   zOp = "INTERSECT";  તોડો;
        સ્થિતિ TK_EXCEPT:      zOp = "EXCEPT";     તોડો;
      }
      sqlite3_વૃક્ષ_View_Item(pView, zOp, 1);
    }
    p = p->pPrior;
  }જ્યારે( p!=0 );
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
શૂન્ય sqlite3_વૃક્ષ_View_Bound(
  વૃક્ષ_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  પદાવલિ *p_પદાવલિ,            /* Value for PRECEDING or FOLLOWING */
  u8 moreToFollow         /* True if more to follow */
){
  પસંદગી( eBound ){
    સ્થિતિ TK_UNBOUNDED: {
      sqlite3_વૃક્ષ_View_Item(pView, "UNBOUNDED", moreToFollow);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
      તોડો;
    }
    સ્થિતિ TK_CURRENT: {
      sqlite3_વૃક્ષ_View_Item(pView, "CURRENT", moreToFollow);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
      તોડો;
    }
    સ્થિતિ TK_PRECEDING: {
      sqlite3_વૃક્ષ_View_Item(pView, "PRECEDING", moreToFollow);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ, 0);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
      તોડો;
    }
    સ્થિતિ TK_FOLLOWING: {
      sqlite3_વૃક્ષ_View_Item(pView, "FOLLOWING", moreToFollow);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ, 0);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
      તોડો;
    }
  }
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
શૂન્ય sqlite3_વૃક્ષ_View_Window(વૃક્ષ_View *pView, અચલ Window *pWin, u8 more){
  પૂર્ણાંક nElement = 0;
  જો( pWin==0 ) પાછા;
  જો( pWin->pFilter ){
    sqlite3_વૃક્ષ_View_Item(pView, "FILTER", 1);
    sqlite3_વૃક્ષ_View_પદાવલિ(pView, pWin->pFilter, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
    જો( pWin->eFrmType==TK_FILTER ) પાછા;
  }
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, more);
  જો( pWin->zName ){
    sqlite3_વૃક્ષ_View_લીટી(pView, "OVER %s (%p)", pWin->zName, pWin);
  }નહીંતર{
    sqlite3_વૃક્ષ_View_લીટી(pView, "OVER (%p)", pWin);
  }
  જો( pWin->zBase )    nElement++;
  જો( pWin->pOrderBy ) nElement++;
  જો( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  જો( pWin->eExclude ) nElement++;
  જો( pWin->zBase ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--nElement)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "window: %s", pWin->zBase);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pWin->pPartition ){
    sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pWin->pPartition, nElement>0,"PARTITION-BY");
  }
  જો( pWin->pOrderBy ){
    sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pWin->pOrderBy, (--nElement)>0, "ORDER-BY");
  }
  જો( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    અક્ષર zBuf[30];
    અચલ અક્ષર *zFrmType = "ROWS";
    જો( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    જો( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(કદ(zBuf),zBuf,"%s%s",zFrmType,
        pWin->bImplicitFrame ? " (implied)" : "");
    sqlite3_વૃક્ષ_View_Item(pView, zBuf, (--nElement)>0);
    sqlite3_વૃક્ષ_View_Bound(pView, pWin->eStart, pWin->pStart, 1);
    sqlite3_વૃક્ષ_View_Bound(pView, pWin->eEnd, pWin->pEnd, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pWin->eExclude ){
    અક્ષર zBuf[30];
    અચલ અક્ષર *zExclude;
    પસંદગી( pWin->eExclude ){
      સ્થિતિ TK_NO:      zExclude = "NO OTHERS";   તોડો;
      સ્થિતિ TK_CURRENT: zExclude = "CURRENT ROW"; તોડો;
      સ્થિતિ TK_GROUP:   zExclude = "GROUP";       તોડો;
      સ્થિતિ TK_TIES:    zExclude = "TIES";        તોડો;
      મૂળભૂત:
        sqlite3_snprintf(કદ(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        તોડો;
    }
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, 0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "EXCLUDE %s", zExclude);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

#ifndef SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
શૂન્ય sqlite3_વૃક્ષ_View_Win_Func(વૃક્ષ_View *pView, અચલ Window *pWin, u8 more){
  જો( pWin==0 ) પાછા;
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, more);
  sqlite3_વૃક્ષ_View_લીટી(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_વૃક્ષ_View_Window(pView, pWin, 0);
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}
#endif /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
શૂન્ય sqlite3_વૃક્ષ_View_પદાવલિ(વૃક્ષ_View *pView, અચલ પદાવલિ *p_પદાવલિ, u8 moreToFollow){
  અચલ અક્ષર *zBinOp = 0;   /* Binary operator */
  અચલ અક્ષર *zUniOp = 0;   /* Unary operator */
  અક્ષર zFlgs[200];
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, moreToFollow);
  જો( p_પદાવલિ==0 ){
    sqlite3_વૃક્ષ_View_લીટી(pView, "nil");
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
    પાછા;
  }
  જો( p_પદાવલિ->flags || p_પદાવલિ->aff_પદાવલિ || p_પદાવલિ->vvaFlags || p_પદાવલિ->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, કદ(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_પદાવલિ->flags, p_પદાવલિ->aff_પદાવલિ ? p_પદાવલિ->aff_પદાવલિ : 'n');
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_પદાવલિ->w.iJoin);
    }
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_પદાવલિ->w.iJoin);
    }
    જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    જો( પદાવલિ_Has_VVAProperty(p_પદાવલિ, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    જો( p_પદાવલિ->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_પદાવલિ->iAgg);
    }
    sqlite3StrAccumFinish(&x);
  }નહીંતર{
    zFlgs[0] = 0;
  }
  પસંદગી( p_પદાવલિ->op ){
    સ્થિતિ TK_AGG_COLUMN: {
      sqlite3_વૃક્ષ_View_લીટી(pView, "AGG{%d:%d}%s",
            p_પદાવલિ->iTable, p_પદાવલિ->iColumn, zFlgs);
      તોડો;
    }
    સ્થિતિ TK_COLUMN: {
      જો( p_પદાવલિ->iTable<0 ){
        /* This only happens when coding check constraints */
        અક્ષર zOp2[16];
        જો( p_પદાવલિ->op2 ){
          sqlite3_snprintf(કદ(zOp2),zOp2," op2=0x%02x",p_પદાવલિ->op2);
        }નહીંતર{
          zOp2[0] = 0;
        }
        sqlite3_વૃક્ષ_View_લીટી(pView, "COLUMN(%d)%s%s",
                                    p_પદાવલિ->iColumn, zFlgs, zOp2);
      }નહીંતર{
        પ્રતિજ્ઞા( પદાવલિ_Use_YTab(p_પદાવલિ) );
        sqlite3_વૃક્ષ_View_લીટી(pView, "{%d:%d} pTab=%p%s",
                        p_પદાવલિ->iTable, p_પદાવલિ->iColumn,
                        p_પદાવલિ->y.pTab, zFlgs);
      }
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_FixedCol) ){
        sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 0);
      }
      તોડો;
    }
    સ્થિતિ TK_INTEGER: {
      જો( p_પદાવલિ->flags & EP_Int_કિંમત ){
        sqlite3_વૃક્ષ_View_લીટી(pView, "%d", p_પદાવલિ->u.i_કિંમત);
      }નહીંતર{
        sqlite3_વૃક્ષ_View_લીટી(pView, "%s", p_પદાવલિ->u.zToken);
      }
      તોડો;
    }
#ifndef SQLITE_OMIT_FLOATING_POINT
    સ્થિતિ TK_FLOAT: {
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3_વૃક્ષ_View_લીટી(pView,"%s", p_પદાવલિ->u.zToken);
      તોડો;
    }
#endif
    સ્થિતિ TK_STRING: {
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3_વૃક્ષ_View_લીટી(pView,"%Q", p_પદાવલિ->u.zToken);
      તોડો;
    }
    સ્થિતિ TK_NULL: {
      sqlite3_વૃક્ષ_View_લીટી(pView,"NULL");
      તોડો;
    }
    સ્થિતિ TK_TRUEFALSE: {
      sqlite3_વૃક્ષ_View_લીટી(pView,"%s%s",
         sqlite3_પદાવલિ_Truth_કિંમત(p_પદાવલિ) ? "TRUE" : "FALSE", zFlgs);
      તોડો;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    સ્થિતિ TK_BLOB: {
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3_વૃક્ષ_View_લીટી(pView,"%s", p_પદાવલિ->u.zToken);
      તોડો;
    }
#endif
    સ્થિતિ TK_VARIABLE: {
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3_વૃક્ષ_View_લીટી(pView,"VARIABLE(%s,%d)",
                          p_પદાવલિ->u.zToken, p_પદાવલિ->iColumn);
      તોડો;
    }
    સ્થિતિ TK_REGISTER: {
      sqlite3_વૃક્ષ_View_લીટી(pView,"REGISTER(%d)", p_પદાવલિ->iTable);
      તોડો;
    }
    સ્થિતિ TK_ID: {
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3_વૃક્ષ_View_લીટી(pView,"ID \"%w\"", p_પદાવલિ->u.zToken);
      તોડો;
    }
    સ્થિતિ TK_ASTERISK: {
      sqlite3_વૃક્ષ_View_લીટી(pView,"ASKTERISK");
      તોડો;
    }
#ifndef SQLITE_OMIT_CAST
    સ્થિતિ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3_વૃક્ષ_View_લીટી(pView,"CAST %Q", p_પદાવલિ->u.zToken);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 0);
      તોડો;
    }
#endif /* SQLITE_OMIT_CAST */
    સ્થિતિ TK_LT:      zBinOp = "LT";     તોડો;
    સ્થિતિ TK_LE:      zBinOp = "LE";     તોડો;
    સ્થિતિ TK_GT:      zBinOp = "GT";     તોડો;
    સ્થિતિ TK_GE:      zBinOp = "GE";     તોડો;
    સ્થિતિ TK_NE:      zBinOp = "NE";     તોડો;
    સ્થિતિ TK_EQ:      zBinOp = "EQ";     તોડો;
    સ્થિતિ TK_IS:      zBinOp = "IS";     તોડો;
    સ્થિતિ TK_ISNOT:   zBinOp = "ISNOT";  તોડો;
    સ્થિતિ TK_AND:     zBinOp = "AND";    તોડો;
    સ્થિતિ TK_OR:      zBinOp = "OR";     તોડો;
    સ્થિતિ TK_PLUS:    zBinOp = "ADD";    તોડો;
    સ્થિતિ TK_STAR:    zBinOp = "MUL";    તોડો;
    સ્થિતિ TK_MINUS:   zBinOp = "SUB";    તોડો;
    સ્થિતિ TK_REM:     zBinOp = "REM";    તોડો;
    સ્થિતિ TK_BITAND:  zBinOp = "BITAND"; તોડો;
    સ્થિતિ TK_BITOR:   zBinOp = "BITOR";  તોડો;
    સ્થિતિ TK_SLASH:   zBinOp = "DIV";    તોડો;
    સ્થિતિ TK_LSHIFT:  zBinOp = "LSHIFT"; તોડો;
    સ્થિતિ TK_RSHIFT:  zBinOp = "RSHIFT"; તોડો;
    સ્થિતિ TK_CONCAT:  zBinOp = "CONCAT"; તોડો;
    સ્થિતિ TK_DOT:     zBinOp = "DOT";    તોડો;
    સ્થિતિ TK_LIMIT:   zBinOp = "LIMIT";  તોડો;

    સ્થિતિ TK_UMINUS:  zUniOp = "UMINUS"; તોડો;
    સ્થિતિ TK_UPLUS:   zUniOp = "UPLUS";  તોડો;
    સ્થિતિ TK_BITNOT:  zUniOp = "BITNOT"; તોડો;
    સ્થિતિ TK_NOT:     zUniOp = "NOT";    તોડો;
    સ્થિતિ TK_ISNULL:  zUniOp = "ISNULL"; તોડો;
    સ્થિતિ TK_NOTNULL: zUniOp = "NOTNULL"; તોડો;

    સ્થિતિ TK_TRUTH: {
      પૂર્ણાંક x;
      અચલ અક્ષર *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      પ્રતિજ્ઞા( p_પદાવલિ->op2==TK_IS || p_પદાવલિ->op2==TK_ISNOT );
      પ્રતિજ્ઞા( p_પદાવલિ->p_જમણું );
      પ્રતિજ્ઞા( sqlite3_પદાવલિ_Skip_Collate_And_Likely(p_પદાવલિ->p_જમણું)->op
                  == TK_TRUEFALSE );
      x = (p_પદાવલિ->op2==TK_ISNOT)*2 + sqlite3_પદાવલિ_Truth_કિંમત(p_પદાવલિ->p_જમણું);
      zUniOp = azOp[x];
      તોડો;
    }

    સ્થિતિ TK_SPAN: {
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3_વૃક્ષ_View_લીટી(pView, "SPAN %Q", p_પદાવલિ->u.zToken);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 0);
      તોડો;
    }

    સ્થિતિ TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3_વૃક્ષ_View_લીટી(pView, "%sCOLLATE %Q%s",
        !પદાવલિ_Has_Property(p_પદાવલિ, EP_Collate) ? "SOFT-" : "",
        p_પદાવલિ->u.zToken, zFlgs);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 0);
      તોડો;
    }

    સ્થિતિ TK_AGG_FUNCTION:
    સ્થિતિ TK_FUNCTION: {
      પદાવલિ_List *pFarg;       /* List of function arguments */
      Window *pWin;
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }નહીંતર{
        પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
        pFarg = p_પદાવલિ->x.pList;
#ifndef SQLITE_OMIT_WINDOWFUNC
        pWin = IsWindowFunc(p_પદાવલિ) ? p_પદાવલિ->y.pWin : 0;
#નહીંતર
        pWin = 0;
#endif 
      }
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      જો( p_પદાવલિ->op==TK_AGG_FUNCTION ){
        sqlite3_વૃક્ષ_View_લીટી(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_પદાવલિ->op2, p_પદાવલિ->u.zToken, zFlgs,
                             p_પદાવલિ->pAggInfo ? p_પદાવલિ->pAggInfo->selId : 0,
                             p_પદાવલિ->iAgg, p_પદાવલિ->pAggInfo);
      }નહીંતર જો( p_પદાવલિ->op2!=0 ){
        અચલ અક્ષર *zOp2;
        અક્ષર zBuf[8];
        sqlite3_snprintf(કદ(zBuf),zBuf,"0x%02x",p_પદાવલિ->op2);
        zOp2 = zBuf;
        જો( p_પદાવલિ->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        જો( p_પદાવલિ->op2==NC_Idx_પદાવલિ ) zOp2 = "NC_IdxExpr";
        જો( p_પદાવલિ->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        જો( p_પદાવલિ->op2==NC_GenCol ) zOp2 = "NC_GenCol";
        sqlite3_વૃક્ષ_View_લીટી(pView, "FUNCTION %Q%s op2=%s",
                            p_પદાવલિ->u.zToken, zFlgs, zOp2);
      }નહીંતર{
        sqlite3_વૃક્ષ_View_લીટી(pView, "FUNCTION %Q%s", p_પદાવલિ->u.zToken, zFlgs);
      }
      જો( pFarg ){
        sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pFarg, pWin!=0 || p_પદાવલિ->p_ડાબું, 0);
        જો( p_પદાવલિ->p_ડાબું ){
          પદાવલિ *pOB = p_પદાવલિ->p_ડાબું;
          પ્રતિજ્ઞા( pOB->op==TK_ORDER );
          પ્રતિજ્ઞા( પદાવલિ_Use_XList(pOB) );
          sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ifndef SQLITE_OMIT_WINDOWFUNC
      જો( pWin ){
        sqlite3_વૃક્ષ_View_Window(pView, pWin, 0);
      }
#endif
      તોડો;
    }
    સ્થિતિ TK_ORDER: {
      sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, p_પદાવલિ->x.pList, 0, "ORDERBY");
      તોડો;
    }
#ifndef SQLITE_OMIT_SUBQUERY
    સ્થિતિ TK_EXISTS: {
      પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ) );
      sqlite3_વૃક્ષ_View_લીટી(pView, "EXISTS-expr flags=0x%x", p_પદાવલિ->flags);
      sqlite3_વૃક્ષ_View_Select(pView, p_પદાવલિ->x.pSelect, 0);
      તોડો;
    }
    સ્થિતિ TK_SELECT: {
      પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ) );
      sqlite3_વૃક્ષ_View_લીટી(pView, "subquery-expr flags=0x%x", p_પદાવલિ->flags);
      sqlite3_વૃક્ષ_View_Select(pView, p_પદાવલિ->x.pSelect, 0);
      તોડો;
    }
    સ્થિતિ TK_IN: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      અક્ષર *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_પદાવલિ->flags);
      જો( p_પદાવલિ->iTable ) sqlite3_str_appendf(pStr, " iTable=%d",p_પદાવલિ->iTable);
      જો( પદાવલિ_Has_Property(p_પદાવલિ, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_પદાવલિ->y.sub.regReturn, p_પદાવલિ->y.sub.iAddr);
      }
      z = sqlite3_str_finish(pStr);
      sqlite3_વૃક્ષ_View_લીટી(pView, z);
      sqlite3_free(z);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 1);
      જો( પદાવલિ_Use_XSelect(p_પદાવલિ) ){
        sqlite3_વૃક્ષ_View_Select(pView, p_પદાવલિ->x.pSelect, 0);
      }નહીંતર{
        sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, p_પદાવલિ->x.pList, 0, 0);
      }
      તોડો;
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
    સ્થિતિ TK_BETWEEN: {
      અચલ પદાવલિ *pX, *pY, *pZ;
      pX = p_પદાવલિ->p_ડાબું;
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
      પ્રતિજ્ઞા( p_પદાવલિ->x.pList->n_પદાવલિ==2 );
      pY = p_પદાવલિ->x.pList->a[0].p_પદાવલિ;
      pZ = p_પદાવલિ->x.pList->a[1].p_પદાવલિ;
      sqlite3_વૃક્ષ_View_લીટી(pView, "BETWEEN%s", zFlgs);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, pX, 1);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, pY, 1);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, pZ, 0);
      તોડો;
    }
    સ્થિતિ TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_વૃક્ષ_View_લીટી(pView, "%s(%d)", 
          p_પદાવલિ->iTable ? "NEW" : "OLD", p_પદાવલિ->iColumn);
      તોડો;
    }
    સ્થિતિ TK_CASE: {
      sqlite3_વૃક્ષ_View_લીટી(pView, "CASE");
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 1);
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
      sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, p_પદાવલિ->x.pList, 0, 0);
      તોડો;
    }
#ifndef SQLITE_OMIT_TRIGGER
    સ્થિતિ TK_RAISE: {
      અચલ અક્ષર *zType = "unk";
      પસંદગી( p_પદાવલિ->aff_પદાવલિ ){
        સ્થિતિ OE_Rollback:   zType = "rollback";  તોડો;
        સ્થિતિ OE_Abort:      zType = "abort";     તોડો;
        સ્થિતિ OE_Fail:       zType = "fail";      તોડો;
        સ્થિતિ OE_Ignore:     zType = "ignore";    તોડો;
      }
      પ્રતિજ્ઞા( !પદાવલિ_Has_Property(p_પદાવલિ, EP_Int_કિંમત) );
      sqlite3_વૃક્ષ_View_લીટી(pView, "RAISE %s", zType);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 0);
      તોડો;
    }
#endif
    સ્થિતિ TK_MATCH: {
      sqlite3_વૃક્ષ_View_લીટી(pView, "MATCH {%d:%d}%s",
                          p_પદાવલિ->iTable, p_પદાવલિ->iColumn, zFlgs);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_જમણું, 0);
      તોડો;
    }
    સ્થિતિ TK_VECTOR: {
      અક્ષર *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      પ્રતિજ્ઞા( પદાવલિ_Use_XList(p_પદાવલિ) );
      sqlite3_વૃક્ષ_View_Bare_પદાવલિ_List(pView, p_પદાવલિ->x.pList, z);
      sqlite3_free(z);
      તોડો;
    }
    સ્થિતિ TK_SELECT_COLUMN: {
      sqlite3_વૃક્ષ_View_લીટી(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_પદાવલિ->iColumn, p_પદાવલિ->iTable-1,
              p_પદાવલિ->p_જમણું==p_પદાવલિ->p_ડાબું ? " (SELECT-owner)" : "");
      પ્રતિજ્ઞા( પદાવલિ_Use_XSelect(p_પદાવલિ->p_ડાબું) );
      sqlite3_વૃક્ષ_View_Select(pView, p_પદાવલિ->p_ડાબું->x.pSelect, 0);
      તોડો;
    }
    સ્થિતિ TK_IF_NULL_ROW: {
      sqlite3_વૃક્ષ_View_લીટી(pView, "IF-NULL-ROW %d", p_પદાવલિ->iTable);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 0);
      તોડો;
    }
    સ્થિતિ TK_ERROR: {
      પદાવલિ tmp;
      sqlite3_વૃક્ષ_View_લીટી(pView, "ERROR");
      tmp = *p_પદાવલિ;
      tmp.op = p_પદાવલિ->op2;
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, &tmp, 0);
      તોડો;
    }
    સ્થિતિ TK_ROW: {
      જો( p_પદાવલિ->iColumn<=0 ){
        sqlite3_વૃક્ષ_View_લીટી(pView, "First FROM table rowid");
      }નહીંતર{
        sqlite3_વૃક્ષ_View_લીટી(pView, "First FROM table column %d",
            p_પદાવલિ->iColumn-1);
      }
      તોડો;
    }
    મૂળભૂત: {
      sqlite3_વૃક્ષ_View_લીટી(pView, "op=%d", p_પદાવલિ->op);
      તોડો;
    }
  }
  જો( zBinOp ){
    sqlite3_વૃક્ષ_View_લીટી(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 1);
    sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_જમણું, 0);
  }નહીંતર જો( zUniOp ){
    sqlite3_વૃક્ષ_View_લીટી(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_વૃક્ષ_View_પદાવલિ(pView, p_પદાવલિ->p_ડાબું, 0);
  }
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
શૂન્ય sqlite3_વૃક્ષ_View_Bare_પદાવલિ_List(
  વૃક્ષ_View *pView,
  અચલ પદાવલિ_List *pList,
  અચલ અક્ષર *zLabel
){
  જો( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  જો( pList==0 ){
    sqlite3_વૃક્ષ_View_લીટી(pView, "%s (empty)", zLabel);
  }નહીંતર{
    પૂર્ણાંક i;
    sqlite3_વૃક્ષ_View_લીટી(pView, "%s", zLabel);
    માટે(i=0; i<pList->n_પદાવલિ; i++){
      પૂર્ણાંક j = pList->a[i].u.x.iOrderByCol;
      u8 sortFlags = pList->a[i].fg.sortFlags;
      અક્ષર *zName = pList->a[i].zEName;
      પૂર્ણાંક moreToFollow = i<pList->n_પદાવલિ - 1;
      જો( j || zName || sortFlags ){
        sqlite3_વૃક્ષ_View_ધકેલો(&pView, moreToFollow);
        moreToFollow = 0;
        sqlite3_વૃક્ષ_View_લીટી(pView, 0);
        જો( zName ){
          પસંદગી( pList->a[i].fg.eEName ){
            મૂળભૂત:
              fprintf(stdout, "AS %s ", zName);
              તોડો;
            સ્થિતિ ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              જો( pList->a[i].fg.bUsed ) fprintf(stdout, "(used) ");
              જો( pList->a[i].fg.bUsingTerm ) fprintf(stdout, "(USING-term) ");
              જો( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              તોડો;
            સ્થિતિ ENAME_SPAN:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              તોડો;
          }
        }
        જો( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        જો( sortFlags & KEYINFO_ORDER_DESC ){
          fprintf(stdout, "DESC ");
        }નહીંતર જો( sortFlags & KEYINFO_ORDER_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, pList->a[i].p_પદાવલિ, moreToFollow);
      જો( j || zName || sortFlags ){
        sqlite3_વૃક્ષ_View_કાઢો(&pView);
      }
    }
  }
}
શૂન્ય sqlite3_વૃક્ષ_View_પદાવલિ_List(
  વૃક્ષ_View *pView,
  અચલ પદાવલિ_List *pList,
  u8 moreToFollow,
  અચલ અક્ષર *zLabel
){
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, moreToFollow);
  sqlite3_વૃક્ષ_View_Bare_પદાવલિ_List(pView, pList, zLabel);
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
શૂન્ય sqlite3_વૃક્ષ_View_Bare_Id_List(
  વૃક્ષ_View *pView,
  અચલ IdList *pList,
  અચલ અક્ષર *zLabel
){
  જો( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  જો( pList==0 ){
    sqlite3_વૃક્ષ_View_લીટી(pView, "%s (empty)", zLabel);
  }નહીંતર{
    પૂર્ણાંક i;
    sqlite3_વૃક્ષ_View_લીટી(pView, "%s", zLabel);
    માટે(i=0; i<pList->nId; i++){
      અક્ષર *zName = pList->a[i].zName;
      પૂર્ણાંક moreToFollow = i<pList->nId - 1;
      જો( zName==0 ) zName = "(null)";
      sqlite3_વૃક્ષ_View_ધકેલો(&pView, moreToFollow);
      sqlite3_વૃક્ષ_View_લીટી(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
  }
}
શૂન્ય sqlite3_વૃક્ષ_View_Id_List(
  વૃક્ષ_View *pView,
  અચલ IdList *pList,
  u8 moreToFollow,
  અચલ અક્ષર *zLabel
){
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, moreToFollow);
  sqlite3_વૃક્ષ_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
શૂન્ય sqlite3_વૃક્ષ_View_Upsert(
  વૃક્ષ_View *pView,
  અચલ Upsert *pUpsert,
  u8 moreToFollow
){
  જો( pUpsert==0 ) પાછા;
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, moreToFollow);
  જ્યારે( pUpsert ){
    પૂર્ણાંક n;
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, pUpsert->pNextUpsert!=0 || moreToFollow);
    sqlite3_વૃક્ષ_View_લીટી(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->pUpsertSet!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pUpsert->pUpsertTarget, (n--)>0, "TARGET");
    sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pUpsert->pUpsertSet, (n--)>0, "SET");
    જો( pUpsert->pUpsertWhere ){
      sqlite3_વૃક્ષ_View_Item(pView, "WHERE", (n--)>0);
      sqlite3_વૃક્ષ_View_પદાવલિ(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
    }
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
    pUpsert = pUpsert->pNextUpsert;
  }
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}

#જો TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
શૂન્ય sqlite3_વૃક્ષ_View_Delete(
  અચલ With *pWith,
  અચલ SrcList *pTabList,
  અચલ પદાવલિ *pWhere,
  અચલ પદાવલિ_List *pOrderBy,
  અચલ પદાવલિ *pLimit,
  અચલ Trigger *pTrigger
){
  પૂર્ણાંક n = 0;
  વૃક્ષ_View *pView = 0;
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, 0);
  sqlite3_વૃક્ષ_View_લીટી(pView, "DELETE");
  જો( pWith ) n++;
  જો( pTabList ) n++;
  જો( pWhere ) n++;
  જો( pOrderBy ) n++;
  જો( pLimit ) n++;
  જો( pTrigger ) n++;
  જો( pWith ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_With(pView, pWith, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pTabList ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "FROM");
    sqlite3_વૃક્ષ_View_Src_List(pView, pTabList);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pWhere ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "WHERE");
    sqlite3_વૃક્ષ_View_પદાવલિ(pView, pWhere, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pOrderBy ){
    sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  જો( pLimit ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "LIMIT");
    sqlite3_વૃક્ષ_View_પદાવલિ(pView, pLimit, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pTrigger ){
    sqlite3_વૃક્ષ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}
#endif /* TREETRACE_ENABLED */

#જો TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
શૂન્ય sqlite3_વૃક્ષ_View_ઉમેરો(
  અચલ With *pWith,
  અચલ SrcList *pTabList,
  અચલ IdList *pColumnList,
  અચલ Select *pSelect,
  અચલ પદાવલિ_List *p_પદાવલિ_List,
  પૂર્ણાંક onError,
  અચલ Upsert *pUpsert,
  અચલ Trigger *pTrigger
){
  વૃક્ષ_View *pView = 0;
  પૂર્ણાંક n = 0;
  અચલ અક્ષર *zLabel = "INSERT";
  પસંદગી( onError ){
    સ્થિતિ OE_Replace:  zLabel = "REPLACE";             તોડો;
    સ્થિતિ OE_Ignore:   zLabel = "INSERT OR IGNORE";    તોડો;
    સ્થિતિ OE_Rollback: zLabel = "INSERT OR ROLLBACK";  તોડો;
    સ્થિતિ OE_Abort:    zLabel = "INSERT OR ABORT";     તોડો;
    સ્થિતિ OE_Fail:     zLabel = "INSERT OR FAIL";      તોડો;
  }
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, 0);
  sqlite3_વૃક્ષ_View_લીટી(pView, zLabel);
  જો( pWith ) n++;
  જો( pTabList ) n++;
  જો( pColumnList ) n++;
  જો( pSelect ) n++;
  જો( p_પદાવલિ_List ) n++;
  જો( pUpsert ) n++;
  જો( pTrigger ) n++;
  જો( pWith ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_With(pView, pWith, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pTabList ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "INTO");
    sqlite3_વૃક્ષ_View_Src_List(pView, pTabList);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pColumnList ){
    sqlite3_વૃક્ષ_View_Id_List(pView, pColumnList, (--n)>0, "COLUMNS");
  }
  જો( pSelect ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "DATA-SOURCE");
    sqlite3_વૃક્ષ_View_Select(pView, pSelect, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( p_પદાવલિ_List ){
    sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, p_પદાવલિ_List, (--n)>0, "VALUES");
  }
  જો( pUpsert ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "UPSERT");
    sqlite3_વૃક્ષ_View_Upsert(pView, pUpsert, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pTrigger ){
    sqlite3_વૃક્ષ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}
#endif /* TREETRACE_ENABLED */

#જો TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
શૂન્ય sqlite3_વૃક્ષ_View_Update(
  અચલ With *pWith,
  અચલ SrcList *pTabList,
  અચલ પદાવલિ_List *pChanges,
  અચલ પદાવલિ *pWhere,
  પૂર્ણાંક onError,
  અચલ પદાવલિ_List *pOrderBy,
  અચલ પદાવલિ *pLimit,
  અચલ Upsert *pUpsert,
  અચલ Trigger *pTrigger
){
  પૂર્ણાંક n = 0;
  વૃક્ષ_View *pView = 0;
  અચલ અક્ષર *zLabel = "UPDATE";
  પસંદગી( onError ){
    સ્થિતિ OE_Replace:  zLabel = "UPDATE OR REPLACE";   તોડો;
    સ્થિતિ OE_Ignore:   zLabel = "UPDATE OR IGNORE";    તોડો;
    સ્થિતિ OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  તોડો;
    સ્થિતિ OE_Abort:    zLabel = "UPDATE OR ABORT";     તોડો;
    સ્થિતિ OE_Fail:     zLabel = "UPDATE OR FAIL";      તોડો;
  }
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, 0);
  sqlite3_વૃક્ષ_View_લીટી(pView, zLabel);
  જો( pWith ) n++;
  જો( pTabList ) n++;
  જો( pChanges ) n++;
  જો( pWhere ) n++;
  જો( pOrderBy ) n++;
  જો( pLimit ) n++;
  જો( pUpsert ) n++;
  જો( pTrigger ) n++;
  જો( pWith ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_With(pView, pWith, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pTabList ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "FROM");
    sqlite3_વૃક્ષ_View_Src_List(pView, pTabList);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pChanges ){
    sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pChanges, (--n)>0, "SET");
  }
  જો( pWhere ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "WHERE");
    sqlite3_વૃક્ષ_View_પદાવલિ(pView, pWhere, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pOrderBy ){
    sqlite3_વૃક્ષ_View_પદાવલિ_List(pView, pOrderBy, (--n)>0, "ORDER-BY");
  }
  જો( pLimit ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "LIMIT");
    sqlite3_વૃક્ષ_View_પદાવલિ(pView, pLimit, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pUpsert ){
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, (--n)>0);
    sqlite3_વૃક્ષ_View_લીટી(pView, "UPSERT");
    sqlite3_વૃક્ષ_View_Upsert(pView, pUpsert, 0);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }
  જો( pTrigger ){
    sqlite3_વૃક્ષ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}
#endif /* TREETRACE_ENABLED */

#ifndef SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
શૂન્ય sqlite3_વૃક્ષ_View_Trigger_Step(
  વૃક્ષ_View *pView,
  અચલ TriggerStep *pStep,
  u8 moreToFollow,
  u8 showFullList
){
  પૂર્ણાંક cnt = 0;
  જો( pStep==0 ) પાછા;
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, 
      moreToFollow || (showFullList && pStep->pNext!=0));
  કરો{
    જો( cnt++ && pStep->pNext==0 ){
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
      sqlite3_વૃક્ષ_View_ધકેલો(&pView, 0);
    }
    sqlite3_વૃક્ષ_View_લીટી(pView, "%s", pStep->zSpan ? pStep->zSpan : "RETURNING");
  }જ્યારે( showFullList && (pStep = pStep->pNext)!=0 );
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
શૂન્ય sqlite3_વૃક્ષ_View_Trigger(
  વૃક્ષ_View *pView,
  અચલ Trigger *pTrigger,
  u8 moreToFollow,
  u8 showFullList
){
  પૂર્ણાંક cnt = 0;
  જો( pTrigger==0 ) પાછા;
  sqlite3_વૃક્ષ_View_ધકેલો(&pView,
     moreToFollow || (showFullList && pTrigger->pNext!=0));
  કરો{
    જો( cnt++ && pTrigger->pNext==0 ){
      sqlite3_વૃક્ષ_View_કાઢો(&pView);
      sqlite3_વૃક્ષ_View_ધકેલો(&pView, 0);
    }
    sqlite3_વૃક્ષ_View_લીટી(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_વૃક્ષ_View_ધકેલો(&pView, 0);
    sqlite3_વૃક્ષ_View_Trigger_Step(pView, pTrigger->step_list, 0, 1);
    sqlite3_વૃક્ષ_View_કાઢો(&pView);
  }જ્યારે( showFullList && (pTrigger = pTrigger->pNext)!=0 );
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
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
શૂન્ય sqlite3_Show_પદાવલિ(અચલ પદાવલિ *p){ sqlite3_વૃક્ષ_View_પદાવલિ(0,p,0); }
શૂન્ય sqlite3_Show_પદાવલિ_List(અચલ પદાવલિ_List *p){ sqlite3_વૃક્ષ_View_પદાવલિ_List(0,p,0,0);}
શૂન્ય sqlite3ShowIdList(અચલ IdList *p){ sqlite3_વૃક્ષ_View_Id_List(0,p,0,0); }
શૂન્ય sqlite3ShowSrcList(અચલ SrcList *p){
  વૃક્ષ_View *pView = 0;
  sqlite3_વૃક્ષ_View_ધકેલો(&pView, 0);
  sqlite3_વૃક્ષ_View_લીટી(pView, "SRCLIST");
  sqlite3_વૃક્ષ_View_Src_List(pView,p);
  sqlite3_વૃક્ષ_View_કાઢો(&pView);
}
શૂન્ય sqlite3ShowSelect(અચલ Select *p){ sqlite3_વૃક્ષ_View_Select(0,p,0); }
શૂન્ય sqlite3ShowWith(અચલ With *p){ sqlite3_વૃક્ષ_View_With(0,p,0); }
શૂન્ય sqlite3ShowUpsert(અચલ Upsert *p){ sqlite3_વૃક્ષ_View_Upsert(0,p,0); }
#ifndef SQLITE_OMIT_TRIGGER
શૂન્ય sqlite3ShowTriggerStep(અચલ TriggerStep *p){
  sqlite3_વૃક્ષ_View_Trigger_Step(0,p,0,0);
}
શૂન્ય sqlite3ShowTriggerStepList(અચલ TriggerStep *p){
  sqlite3_વૃક્ષ_View_Trigger_Step(0,p,0,1);
}
શૂન્ય sqlite3ShowTrigger(અચલ Trigger *p){ sqlite3_વૃક્ષ_View_Trigger(0,p,0,0); }
શૂન્ય sqlite3ShowTriggerList(અચલ Trigger *p){ sqlite3_વૃક્ષ_View_Trigger(0,p,0,1);}
#endif
#ifndef SQLITE_OMIT_WINDOWFUNC
શૂન્ય sqlite3ShowWindow(અચલ Window *p){ sqlite3_વૃક્ષ_View_Window(0,p,0); }
શૂન્ય sqlite3ShowWinFunc(અચલ Window *p){ sqlite3_વૃક્ષ_View_Win_Func(0,p,0); }
#endif

#endif /* SQLITE_DEBUG */
