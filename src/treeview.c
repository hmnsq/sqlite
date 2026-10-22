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
#అన్తర్భావ "sqliteInt.h"
#నిర్వచితమైతే SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
స్థిర శూన్య sqlite3_వృక్షం_View_నెట్టు(వృక్షం_View **pp, u8 మరిన్ని_గమ్యం_Follow){
  వృక్షం_View *p = *pp;
  ఒకవేళ( p==0 ){
    *pp = p = sqlite3_malloc64( పరిమాణం(*p) );
    ఒకవేళ( p==0 ) తిరిగి;
    memset(p, 0, పరిమాణం(*p));
  }లేకపోతే{
    p->iLevel++;
  }
  ప్రతిజ్ఞా( మరిన్ని_గమ్యం_Follow==0 || మరిన్ని_గమ్యం_Follow==1 );
  ఒకవేళ( p->iLevel<(పూర్ణాంకం)పరిమాణం(p->b_పంక్తి) ) p->b_పంక్తి[p->iLevel] = మరిన్ని_గమ్యం_Follow;
}

/*
** Finished with one layer of the tree
*/
స్థిర శూన్య sqlite3_వృక్షం_View_తీయి(వృక్షం_View **pp){
  వృక్షం_View *p = *pp;
  ఒకవేళ( p==0 ) తిరిగి;
  p->iLevel--;
  ఒకవేళ( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
శూన్య sqlite3_వృక్షం_View_పంక్తి(వృక్షం_View *p, స్థిరరాశి అక్షరం *zFormat, ...){
  va_list ap;
  పూర్ణాంకం i;
  StrAccum acc;
  అక్షరం zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, పరిమాణం(zBuf), 0);
  ఒకవేళ( p ){
    కొరకు(i=0; i<p->iLevel && i<(పూర్ణాంకం)పరిమాణం(p->b_పంక్తి)-1; i++){
      sqlite3_str_append(&acc, p->b_పంక్తి[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_పంక్తి[i] ? "|-- " : "'-- ", 4);
  }
  ఒకవేళ( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    ప్రతిజ్ఞా( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3_Str_Accum_ముగింపు(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
స్థిర శూన్య sqlite3_వృక్షం_View_అంశం(వృక్షం_View *p, స్థిరరాశి అక్షరం *zLabel,u8 మరిన్ని_Follows){
  sqlite3_వృక్షం_View_నెట్టు(&p, మరిన్ని_Follows);
  sqlite3_వృక్షం_View_పంక్తి(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
శూన్య sqlite3_వృక్షం_View_స్తంభం_List(
  వృక్షం_View *pView,
  స్థిరరాశి స్తంభం *a_నిలువు,
  పూర్ణాంకం n_నిలువు,
  u8 మరిన్ని_గమ్యం_Follow
){
  పూర్ణాంకం i;
  sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని_గమ్యం_Follow);
  sqlite3_వృక్షం_View_పంక్తి(pView, "COLUMNS");
  కొరకు(i=0; i<n_నిలువు; i++){
    u16 flg = a_నిలువు[i].నిలువు_Flags;
    పూర్ణాంకం నిలువు_మరిన్ని_గమ్యం_Follow = i<(n_నిలువు - 1);
    sqlite3_వృక్షం_View_నెట్టు(&pView, నిలువు_మరిన్ని_గమ్యం_Follow);
    sqlite3_వృక్షం_View_పంక్తి(pView, 0);
    రాయి(" %s", a_నిలువు[i].zCnName);
    ఎంపిక( a_నిలువు[i].eCType ){
      స్థితి COLTYPE_ANY:      రాయి(" ANY");        ఆపు;
      స్థితి COLTYPE_BLOB:     రాయి(" BLOB");       ఆపు;
      స్థితి COLTYPE_INT:      రాయి(" INT");        ఆపు;
      స్థితి COLTYPE_INTEGER:  రాయి(" INTEGER");    ఆపు;
      స్థితి COLTYPE_REAL:     రాయి(" REAL");       ఆపు;
      స్థితి COLTYPE_పాఠ్యం:     రాయి(" TEXT");       ఆపు;
      స్థితి COLTYPE_CUSTOM: {
        ఒకవేళ( flg & COLFLAG_HASTYPE ){
          స్థిరరాశి అక్షరం *z = a_నిలువు[i].zCnName;
          z += అక్షరశ్రేణి_పొడవు(z)+1;
          రాయి(" X-%s", z);
          ఆపు;
        }
      }
    }
    ఒకవేళ( flg & COLFLAG_PRIMKEY ) రాయి(" PRIMARY KEY");
    ఒకవేళ( flg & COLFLAG_HIDDEN ) రాయి(" HIDDEN");
#నిర్వచితమైతే COLFLAG_NOEXPAND
    ఒకవేళ( flg & COLFLAG_NOEXPAND ) రాయి(" NO-EXPAND");
#అయితే_ముగింపు
    ఒకవేళ( flg ) రాయి(" flags=%04x", flg);
    రాయి("\n");      
    fflush(stdout);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  sqlite3_వృక్షం_View_తీయి(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
శూన్య sqlite3_వృక్షం_View_With(వృక్షం_View *pView, స్థిరరాశి With *pWith, u8 మరిన్ని_గమ్యం_Follow){
  పూర్ణాంకం i;
  ఒకవేళ( pWith==0 ) తిరిగి;
  ఒకవేళ( pWith->nCte==0 ) తిరిగి;
  ఒకవేళ( pWith->pOuter ){
    sqlite3_వృక్షం_View_పంక్తి(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }లేకపోతే{
    sqlite3_వృక్షం_View_పంక్తి(pView, "WITH (0x%p)", pWith);
  }
  ఒకవేళ( pWith->nCte>0 ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని_గమ్యం_Follow);
    కొరకు(i=0; i<pWith->nCte; i++){
      StrAccum x;
      అక్షరం z_పంక్తి[1000];
      స్థిరరాశి నిర్మాణం Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_పంక్తి, పరిమాణం(z_పంక్తి), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      ఒకవేళ( pCte->p_నిలువుల_సంఖ్య && pCte->p_నిలువుల_సంఖ్య->n_సమాసం>0 ){
        అక్షరం cSep = '(';
        పూర్ణాంకం j;
        కొరకు(j=0; j<pCte->p_నిలువుల_సంఖ్య->n_సమాసం; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->p_నిలువుల_సంఖ్య->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      ఒకవేళ( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      ఒకవేళ( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3_Str_Accum_ముగింపు(&x);
      sqlite3_వృక్షం_View_అంశం(pView, z_పంక్తి, i<pWith->nCte-1);
      sqlite3_వృక్షం_View_Select(pView, pCte->pSelect, 0);
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
శూన్య sqlite3_వృక్షం_View_Src_List(వృక్షం_View *pView, స్థిరరాశి SrcList *pSrc){
  పూర్ణాంకం i;
  ఒకవేళ( pSrc==0 ) తిరిగి;
  కొరకు(i=0; i<pSrc->nSrc; i++){
    స్థిరరాశి Src_అంశం *p_అంశం = &pSrc->a[i];
    StrAccum x;
    పూర్ణాంకం n = 0;
    అక్షరం z_పంక్తి[1000];
    sqlite3StrAccumInit(&x, 0, z_పంక్తి, పరిమాణం(z_పంక్తి), 0);
    x.printfFlags |= SQLITE_PRINTF_అంతర్గత;
    sqlite3_str_appendf(&x, "{%d:*} %!S", p_అంశం->iCursor, p_అంశం);
    ఒకవేళ( p_అంశం->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           p_అంశం->pSTab->zName, p_అంశం->pSTab->n_నిలువు, p_అంశం->pSTab, 
           p_అంశం->నిలువు_వాడిన,
           p_అంశం->fg.rowid_వాడిన ? "+rowid" : "");
    }
    ఒకవేళ( (p_అంశం->fg.jointype & (JT_ఎడమ|JT_కుడి))==(JT_ఎడమ|JT_కుడి) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }లేకపోతే ఒకవేళ( p_అంశం->fg.jointype & JT_ఎడమ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }లేకపోతే ఒకవేళ( p_అంశం->fg.jointype & JT_కుడి ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }లేకపోతే ఒకవేళ( p_అంశం->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    ఒకవేళ( p_అంశం->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    ఒకవేళ( p_అంశం->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ఒకవేళ( p_అంశం->fg.isCte ){
      స్థిర స్థిరరాశి అక్షరం *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          p_అంశం->u2.pCteUse->nUse,
                          aMat[p_అంశం->u2.pCteUse->eM10d]);
    }
    ఒకవేళ( p_అంశం->fg.isOn || (p_అంశం->fg.isUsing==0 && p_అంశం->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    ఒకవేళ( p_అంశం->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    ఒకవేళ( p_అంశం->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    ఒకవేళ( p_అంశం->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    ఒకవేళ( p_అంశం->fg.ద్వారా_Coroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    ఒకవేళ( p_అంశం->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    ఒకవేళ( p_అంశం->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    ఒకవేళ( p_అంశం->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    ఒకవేళ( p_అంశం->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    ఒకవేళ( p_అంశం->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3_Str_Accum_ముగింపు(&x);
    sqlite3_వృక్షం_View_అంశం(pView, z_పంక్తి, i<pSrc->nSrc-1);
    n = 0;
    ఒకవేళ( p_అంశం->fg.isSubquery ) n++;
    ఒకవేళ( p_అంశం->fg.isTabFunc ) n++;
    ఒకవేళ( p_అంశం->fg.isUsing || p_అంశం->u3.pOn!=0 ) n++;
    ఒకవేళ( p_అంశం->fg.isUsing ){
      sqlite3_వృక్షం_View_Id_List(pView, p_అంశం->u3.pUsing, (--n)>0, "USING");
    }లేకపోతే ఒకవేళ( p_అంశం->u3.pOn!=0 ){
      sqlite3_వృక్షం_View_అంశం(pView, "ON", (--n)>0);
      sqlite3_వృక్షం_View_సమాసం(pView, p_అంశం->u3.pOn, 0);
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
    ఒకవేళ( p_అంశం->fg.isSubquery ){
      ప్రతిజ్ఞా( n==1 );
      ఒకవేళ( p_అంశం->pSTab ){
        పట్టిక *pTab = p_అంశం->pSTab;
        sqlite3_వృక్షం_View_స్తంభం_List(pView, pTab->a_నిలువు, pTab->n_నిలువు, 1);
      }
      ప్రతిజ్ఞా( (పూర్ణాంకం)p_అంశం->fg.isNestedFrom == IsNestedFrom(p_అంశం) );
      sqlite3_వృక్షం_View_Select(pView, p_అంశం->u4.pSubq->pSelect, 0);
    }
    ఒకవేళ( p_అంశం->fg.isTabFunc ){
      sqlite3_వృక్షం_View_సమాసం_List(pView, p_అంశం->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
శూన్య sqlite3_వృక్షం_View_Select(వృక్షం_View *pView, స్థిరరాశి Select *p, u8 మరిన్ని_గమ్యం_Follow){
  పూర్ణాంకం n = 0;
  పూర్ణాంకం cnt = 0;
  ఒకవేళ( p==0 ){
    sqlite3_వృక్షం_View_పంక్తి(pView, "nil-SELECT");
    తిరిగి;
  } 
  sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని_గమ్యం_Follow);
  ఒకవేళ( p->pWith ){
    sqlite3_వృక్షం_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_వృక్షం_View_నెట్టు(&pView, 1);
  }
  చేయి{
    ఒకవేళ( p->selFlags & SF_WhereBegin ){
      sqlite3_వృక్షం_View_పంక్తి(pView, "sqlite3WhereBegin()");
    }లేకపోతే{
      sqlite3_వృక్షం_View_పంక్తి(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (పూర్ణాంకం)p->n_Select_అడ్డువరుస
      );
    }
    ఒకవేళ( cnt++ ) sqlite3_వృక్షం_View_తీయి(&pView);
    ఒకవేళ( p->p_పూర్వ ){
      n = 1000;
    }లేకపోతే{
      n = 0;
      ఒకవేళ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      ఒకవేళ( p->pWhere ) n++;
      ఒకవేళ( p->p_సమూహం_By ) n++;
      ఒకవేళ( p->pHaving ) n++;
      ఒకవేళ( p->p_క్రమం_By ) n++;
      ఒకవేళ( p->p_హద్దు ) n++;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      ఒకవేళ( p->pWin ) n++;
      ఒకవేళ( p->pWinDefn ) n++;
#అయితే_ముగింపు
    }
    ఒకవేళ( p->pEList ){
      sqlite3_వృక్షం_View_సమాసం_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( p->pWin ){
      కిటికీ *pX;
      sqlite3_వృక్షం_View_నెట్టు(&pView, (n--)>0);
      sqlite3_వృక్షం_View_పంక్తి(pView, "window-functions");
      కొరకు(pX=p->pWin; pX; pX=pX->p_తదుపరి_Win){
        sqlite3_వృక్షం_View_Win_Func(pView, pX, pX->p_తదుపరి_Win!=0);
      }
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
#అయితే_ముగింపు
    ఒకవేళ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_వృక్షం_View_నెట్టు(&pView, (n--)>0);
      sqlite3_వృక్షం_View_పంక్తి(pView, "FROM");
      sqlite3_వృక్షం_View_Src_List(pView, p->pSrc);
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
    ఒకవేళ( p->pWhere ){
      sqlite3_వృక్షం_View_అంశం(pView, "WHERE", (n--)>0);
      sqlite3_వృక్షం_View_సమాసం(pView, p->pWhere, 0);
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
    ఒకవేళ( p->p_సమూహం_By ){
      sqlite3_వృక్షం_View_సమాసం_List(pView, p->p_సమూహం_By, (n--)>0, "GROUPBY");
    }
    ఒకవేళ( p->pHaving ){
      sqlite3_వృక్షం_View_అంశం(pView, "HAVING", (n--)>0);
      sqlite3_వృక్షం_View_సమాసం(pView, p->pHaving, 0);
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
    ఒకవేళ( p->pWinDefn ){
      కిటికీ *pX;
      sqlite3_వృక్షం_View_అంశం(pView, "WINDOW", (n--)>0);
      కొరకు(pX=p->pWinDefn; pX; pX=pX->p_తదుపరి_Win){
        sqlite3_వృక్షం_View_కిటికీ(pView, pX, pX->p_తదుపరి_Win!=0);
      }
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
#అయితే_ముగింపు
    ఒకవేళ( p->p_క్రమం_By ){
      sqlite3_వృక్షం_View_సమాసం_List(pView, p->p_క్రమం_By, (n--)>0, "ORDERBY");
    }
    ఒకవేళ( p->p_హద్దు ){
      sqlite3_వృక్షం_View_అంశం(pView, "LIMIT", (n--)>0);
      sqlite3_వృక్షం_View_సమాసం(pView, p->p_హద్దు->p_ఎడమ, p->p_హద్దు->p_కుడి!=0);
      ఒకవేళ( p->p_హద్దు->p_కుడి ){
        sqlite3_వృక్షం_View_అంశం(pView, "OFFSET", 0);
        sqlite3_వృక్షం_View_సమాసం(pView, p->p_హద్దు->p_కుడి, 0);
        sqlite3_వృక్షం_View_తీయి(&pView);
      }
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
    ఒకవేళ( p->p_పూర్వ ){
      స్థిరరాశి అక్షరం *zOp = "UNION";
      ఎంపిక( p->op ){
        స్థితి TK_ALL:         zOp = "UNION ALL";  ఆపు;
        స్థితి TK_INTERSECT:   zOp = "INTERSECT";  ఆపు;
        స్థితి TK_EXCEPT:      zOp = "EXCEPT";     ఆపు;
      }
      sqlite3_వృక్షం_View_అంశం(pView, zOp, 1);
    }
    p = p->p_పూర్వ;
  }అయితే( p!=0 );
  sqlite3_వృక్షం_View_తీయి(&pView);
}

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
శూన్య sqlite3_వృక్షం_View_Bound(
  వృక్షం_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  సమాసం *p_సమాసం,            /* Value for PRECEDING or FOLLOWING */
  u8 మరిన్ని_గమ్యం_Follow         /* True if more to follow */
){
  ఎంపిక( eBound ){
    స్థితి TK_UNBOUNDED: {
      sqlite3_వృక్షం_View_అంశం(pView, "UNBOUNDED", మరిన్ని_గమ్యం_Follow);
      sqlite3_వృక్షం_View_తీయి(&pView);
      ఆపు;
    }
    స్థితి TK_CURRENT: {
      sqlite3_వృక్షం_View_అంశం(pView, "CURRENT", మరిన్ని_గమ్యం_Follow);
      sqlite3_వృక్షం_View_తీయి(&pView);
      ఆపు;
    }
    స్థితి TK_PRECEDING: {
      sqlite3_వృక్షం_View_అంశం(pView, "PRECEDING", మరిన్ని_గమ్యం_Follow);
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం, 0);
      sqlite3_వృక్షం_View_తీయి(&pView);
      ఆపు;
    }
    స్థితి TK_FOLLOWING: {
      sqlite3_వృక్షం_View_అంశం(pView, "FOLLOWING", మరిన్ని_గమ్యం_Follow);
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం, 0);
      sqlite3_వృక్షం_View_తీయి(&pView);
      ఆపు;
    }
  }
}
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
శూన్య sqlite3_వృక్షం_View_కిటికీ(వృక్షం_View *pView, స్థిరరాశి కిటికీ *pWin, u8 మరిన్ని){
  పూర్ణాంకం nElement = 0;
  ఒకవేళ( pWin==0 ) తిరిగి;
  ఒకవేళ( pWin->pFilter ){
    sqlite3_వృక్షం_View_అంశం(pView, "FILTER", 1);
    sqlite3_వృక్షం_View_సమాసం(pView, pWin->pFilter, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
    ఒకవేళ( pWin->eFrmType==TK_FILTER ) తిరిగి;
  }
  sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని);
  ఒకవేళ( pWin->zName ){
    sqlite3_వృక్షం_View_పంక్తి(pView, "OVER %s (%p)", pWin->zName, pWin);
  }లేకపోతే{
    sqlite3_వృక్షం_View_పంక్తి(pView, "OVER (%p)", pWin);
  }
  ఒకవేళ( pWin->z_పునాది )    nElement++;
  ఒకవేళ( pWin->p_క్రమం_By ) nElement++;
  ఒకవేళ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  ఒకవేళ( pWin->eExclude ) nElement++;
  ఒకవేళ( pWin->z_పునాది ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--nElement)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "window: %s", pWin->z_పునాది);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pWin->p_విభజన ){
    sqlite3_వృక్షం_View_సమాసం_List(pView, pWin->p_విభజన, nElement>0,"PARTITION-BY");
  }
  ఒకవేళ( pWin->p_క్రమం_By ){
    sqlite3_వృక్షం_View_సమాసం_List(pView, pWin->p_క్రమం_By, (--nElement)>0, "ORDER-BY");
  }
  ఒకవేళ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    అక్షరం zBuf[30];
    స్థిరరాశి అక్షరం *zFrmType = "ROWS";
    ఒకవేళ( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    ఒకవేళ( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(పరిమాణం(zBuf),zBuf,"%s%s",zFrmType,
        pWin->b_Implicit_చట్రం ? " (implied)" : "");
    sqlite3_వృక్షం_View_అంశం(pView, zBuf, (--nElement)>0);
    sqlite3_వృక్షం_View_Bound(pView, pWin->e_ప్రారంభం, pWin->p_ప్రారంభం, 1);
    sqlite3_వృక్షం_View_Bound(pView, pWin->e_చివర, pWin->p_చివర, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pWin->eExclude ){
    అక్షరం zBuf[30];
    స్థిరరాశి అక్షరం *zExclude;
    ఎంపిక( pWin->eExclude ){
      స్థితి TK_NO:      zExclude = "NO OTHERS";   ఆపు;
      స్థితి TK_CURRENT: zExclude = "CURRENT ROW"; ఆపు;
      స్థితి TK_సమూహం:   zExclude = "GROUP";       ఆపు;
      స్థితి TK_TIES:    zExclude = "TIES";        ఆపు;
      అప్రమేయ:
        sqlite3_snprintf(పరిమాణం(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        ఆపు;
    }
    sqlite3_వృక్షం_View_నెట్టు(&pView, 0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "EXCLUDE %s", zExclude);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  sqlite3_వృక్షం_View_తీయి(&pView);
}
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
శూన్య sqlite3_వృక్షం_View_Win_Func(వృక్షం_View *pView, స్థిరరాశి కిటికీ *pWin, u8 మరిన్ని){
  ఒకవేళ( pWin==0 ) తిరిగి;
  sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని);
  sqlite3_వృక్షం_View_పంక్తి(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_వృక్షం_View_కిటికీ(pView, pWin, 0);
  sqlite3_వృక్షం_View_తీయి(&pView);
}
#అయితే_ముగింపు /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
శూన్య sqlite3_వృక్షం_View_సమాసం(వృక్షం_View *pView, స్థిరరాశి సమాసం *p_సమాసం, u8 మరిన్ని_గమ్యం_Follow){
  స్థిరరాశి అక్షరం *zBinOp = 0;   /* Binary operator */
  స్థిరరాశి అక్షరం *zUniOp = 0;   /* Unary operator */
  అక్షరం zFlgs[200];
  sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని_గమ్యం_Follow);
  ఒకవేళ( p_సమాసం==0 ){
    sqlite3_వృక్షం_View_పంక్తి(pView, "nil");
    sqlite3_వృక్షం_View_తీయి(&pView);
    తిరిగి;
  }
  ఒకవేళ( p_సమాసం->flags || p_సమాసం->aff_సమాసం || p_సమాసం->vvaFlags || p_సమాసం->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, పరిమాణం(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_సమాసం->flags, p_సమాసం->aff_సమాసం ? p_సమాసం->aff_సమాసం : 'n');
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_సమాసం->w.iJoin);
    }
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_సమాసం->w.iJoin);
    }
    ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ఒకవేళ( సమాసం_Has_VVAProperty(p_సమాసం, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    ఒకవేళ( p_సమాసం->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_సమాసం->iAgg);
    }
    sqlite3_Str_Accum_ముగింపు(&x);
  }లేకపోతే{
    zFlgs[0] = 0;
  }
  ఎంపిక( p_సమాసం->op ){
    స్థితి TK_AGG_స్తంభం: {
      sqlite3_వృక్షం_View_పంక్తి(pView, "AGG{%d:%d}%s",
            p_సమాసం->i_పట్టిక, p_సమాసం->i_స్తంభం, zFlgs);
      ఆపు;
    }
    స్థితి TK_స్తంభం: {
      ఒకవేళ( p_సమాసం->i_పట్టిక<0 ){
        /* This only happens when coding check constraints */
        అక్షరం zOp2[16];
        ఒకవేళ( p_సమాసం->op2 ){
          sqlite3_snprintf(పరిమాణం(zOp2),zOp2," op2=0x%02x",p_సమాసం->op2);
        }లేకపోతే{
          zOp2[0] = 0;
        }
        sqlite3_వృక్షం_View_పంక్తి(pView, "COLUMN(%d)%s%s",
                                    p_సమాసం->i_స్తంభం, zFlgs, zOp2);
      }లేకపోతే{
        ప్రతిజ్ఞా( సమాసం_Use_YTab(p_సమాసం) );
        sqlite3_వృక్షం_View_పంక్తి(pView, "{%d:%d} pTab=%p%s",
                        p_సమాసం->i_పట్టిక, p_సమాసం->i_స్తంభం,
                        p_సమాసం->y.pTab, zFlgs);
      }
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Fixed_నిలువు) ){
        sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 0);
      }
      ఆపు;
    }
    స్థితి TK_INTEGER: {
      ఒకవేళ( p_సమాసం->flags & EP_Int_విలువ ){
        sqlite3_వృక్షం_View_పంక్తి(pView, "%d", p_సమాసం->u.i_విలువ);
      }లేకపోతే{
        sqlite3_వృక్షం_View_పంక్తి(pView, "%s", p_సమాసం->u.zToken);
      }
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_FLOATING_బిందువు
    స్థితి TK_FLOAT: {
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_వృక్షం_View_పంక్తి(pView,"%s", p_సమాసం->u.zToken);
      ఆపు;
    }
#అయితే_ముగింపు
    స్థితి TK_STRING: {
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_వృక్షం_View_పంక్తి(pView,"%Q", p_సమాసం->u.zToken);
      ఆపు;
    }
    స్థితి TK_NULL: {
      sqlite3_వృక్షం_View_పంక్తి(pView,"NULL");
      ఆపు;
    }
    స్థితి TK_TRUEFALSE: {
      sqlite3_వృక్షం_View_పంక్తి(pView,"%s%s",
         sqlite3_సమాసం_Truth_విలువ(p_సమాసం) ? "TRUE" : "FALSE", zFlgs);
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_BLOB_LITERAL
    స్థితి TK_BLOB: {
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_వృక్షం_View_పంక్తి(pView,"%s", p_సమాసం->u.zToken);
      ఆపు;
    }
#అయితే_ముగింపు
    స్థితి TK_VARIABLE: {
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_వృక్షం_View_పంక్తి(pView,"VARIABLE(%s,%d)",
                          p_సమాసం->u.zToken, p_సమాసం->i_స్తంభం);
      ఆపు;
    }
    స్థితి TK_రిజిస్టరు: {
      sqlite3_వృక్షం_View_పంక్తి(pView,"REGISTER(%d)", p_సమాసం->i_పట్టిక);
      ఆపు;
    }
    స్థితి TK_ID: {
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_వృక్షం_View_పంక్తి(pView,"ID \"%w\"", p_సమాసం->u.zToken);
      ఆపు;
    }
    స్థితి TK_ASTERISK: {
      sqlite3_వృక్షం_View_పంక్తి(pView,"ASKTERISK");
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_CAST
    స్థితి TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_వృక్షం_View_పంక్తి(pView,"CAST %Q", p_సమాసం->u.zToken);
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 0);
      ఆపు;
    }
#అయితే_ముగింపు /* SQLITE_OMIT_CAST */
    స్థితి TK_LT:      zBinOp = "LT";     ఆపు;
    స్థితి TK_LE:      zBinOp = "LE";     ఆపు;
    స్థితి TK_GT:      zBinOp = "GT";     ఆపు;
    స్థితి TK_GE:      zBinOp = "GE";     ఆపు;
    స్థితి TK_NE:      zBinOp = "NE";     ఆపు;
    స్థితి TK_EQ:      zBinOp = "EQ";     ఆపు;
    స్థితి TK_IS:      zBinOp = "IS";     ఆపు;
    స్థితి TK_ISNOT:   zBinOp = "ISNOT";  ఆపు;
    స్థితి TK_AND:     zBinOp = "AND";    ఆపు;
    స్థితి TK_OR:      zBinOp = "OR";     ఆపు;
    స్థితి TK_PLUS:    zBinOp = "ADD";    ఆపు;
    స్థితి TK_నక్షత్రం:    zBinOp = "MUL";    ఆపు;
    స్థితి TK_MINUS:   zBinOp = "SUB";    ఆపు;
    స్థితి TK_REM:     zBinOp = "REM";    ఆపు;
    స్థితి TK_BITAND:  zBinOp = "BITAND"; ఆపు;
    స్థితి TK_BITOR:   zBinOp = "BITOR";  ఆపు;
    స్థితి TK_SLASH:   zBinOp = "DIV";    ఆపు;
    స్థితి TK_LSHIFT:  zBinOp = "LSHIFT"; ఆపు;
    స్థితి TK_RSHIFT:  zBinOp = "RSHIFT"; ఆపు;
    స్థితి TK_CONCAT:  zBinOp = "CONCAT"; ఆపు;
    స్థితి TK_DOT:     zBinOp = "DOT";    ఆపు;
    స్థితి TK_హద్దు:   zBinOp = "LIMIT";  ఆపు;

    స్థితి TK_UMINUS:  zUniOp = "UMINUS"; ఆపు;
    స్థితి TK_UPLUS:   zUniOp = "UPLUS";  ఆపు;
    స్థితి TK_BITNOT:  zUniOp = "BITNOT"; ఆపు;
    స్థితి TK_NOT:     zUniOp = "NOT";    ఆపు;
    స్థితి TK_ISNULL:  zUniOp = "ISNULL"; ఆపు;
    స్థితి TK_NOTNULL: zUniOp = "NOTNULL"; ఆపు;

    స్థితి TK_TRUTH: {
      పూర్ణాంకం x;
      స్థిరరాశి అక్షరం *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      ప్రతిజ్ఞా( p_సమాసం->op2==TK_IS || p_సమాసం->op2==TK_ISNOT );
      ప్రతిజ్ఞా( p_సమాసం->p_కుడి );
      ప్రతిజ్ఞా( sqlite3_సమాసం_Skip_Collate_And_Likely(p_సమాసం->p_కుడి)->op
                  == TK_TRUEFALSE );
      x = (p_సమాసం->op2==TK_ISNOT)*2 + sqlite3_సమాసం_Truth_విలువ(p_సమాసం->p_కుడి);
      zUniOp = azOp[x];
      ఆపు;
    }

    స్థితి TK_పరిధి: {
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_వృక్షం_View_పంక్తి(pView, "SPAN %Q", p_సమాసం->u.zToken);
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 0);
      ఆపు;
    }

    స్థితి TK_COLLATE: {
      /* COLLATE operators without the EP_Collate flag are intended to
      ** emulate collation associated with a table column.  These show
      ** up in the treeview output as "SOFT-COLLATE".  Explicit COLLATE
      ** operators that appear in the original SQL always have the
      ** EP_Collate bit set and appear in treeview output as just "COLLATE" */
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_వృక్షం_View_పంక్తి(pView, "%sCOLLATE %Q%s",
        !సమాసం_Has_Property(p_సమాసం, EP_Collate) ? "SOFT-" : "",
        p_సమాసం->u.zToken, zFlgs);
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 0);
      ఆపు;
    }

    స్థితి TK_AGG_FUNCTION:
    స్థితి TK_FUNCTION: {
      సమాసం_List *pFarg;       /* List of function arguments */
      కిటికీ *pWin;
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }లేకపోతే{
        ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
        pFarg = p_సమాసం->x.pList;
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
        pWin = Is_కిటికీ_Func(p_సమాసం) ? p_సమాసం->y.pWin : 0;
#లేకపోతే
        pWin = 0;
#అయితే_ముగింపు 
      }
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      ఒకవేళ( p_సమాసం->op==TK_AGG_FUNCTION ){
        sqlite3_వృక్షం_View_పంక్తి(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_సమాసం->op2, p_సమాసం->u.zToken, zFlgs,
                             p_సమాసం->pAggInfo ? p_సమాసం->pAggInfo->selId : 0,
                             p_సమాసం->iAgg, p_సమాసం->pAggInfo);
      }లేకపోతే ఒకవేళ( p_సమాసం->op2!=0 ){
        స్థిరరాశి అక్షరం *zOp2;
        అక్షరం zBuf[8];
        sqlite3_snprintf(పరిమాణం(zBuf),zBuf,"0x%02x",p_సమాసం->op2);
        zOp2 = zBuf;
        ఒకవేళ( p_సమాసం->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        ఒకవేళ( p_సమాసం->op2==NC_Idx_సమాసం ) zOp2 = "NC_IdxExpr";
        ఒకవేళ( p_సమాసం->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        ఒకవేళ( p_సమాసం->op2==NC_Gen_నిలువు ) zOp2 = "NC_GenCol";
        sqlite3_వృక్షం_View_పంక్తి(pView, "FUNCTION %Q%s op2=%s",
                            p_సమాసం->u.zToken, zFlgs, zOp2);
      }లేకపోతే{
        sqlite3_వృక్షం_View_పంక్తి(pView, "FUNCTION %Q%s", p_సమాసం->u.zToken, zFlgs);
      }
      ఒకవేళ( pFarg ){
        sqlite3_వృక్షం_View_సమాసం_List(pView, pFarg, pWin!=0 || p_సమాసం->p_ఎడమ, 0);
        ఒకవేళ( p_సమాసం->p_ఎడమ ){
          సమాసం *pOB = p_సమాసం->p_ఎడమ;
          ప్రతిజ్ఞా( pOB->op==TK_క్రమం );
          ప్రతిజ్ఞా( సమాసం_Use_XList(pOB) );
          sqlite3_వృక్షం_View_సమాసం_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
      ఒకవేళ( pWin ){
        sqlite3_వృక్షం_View_కిటికీ(pView, pWin, 0);
      }
#అయితే_ముగింపు
      ఆపు;
    }
    స్థితి TK_క్రమం: {
      sqlite3_వృక్షం_View_సమాసం_List(pView, p_సమాసం->x.pList, 0, "ORDERBY");
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_SUBQUERY
    స్థితి TK_EXISTS: {
      ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం) );
      sqlite3_వృక్షం_View_పంక్తి(pView, "EXISTS-expr flags=0x%x", p_సమాసం->flags);
      sqlite3_వృక్షం_View_Select(pView, p_సమాసం->x.pSelect, 0);
      ఆపు;
    }
    స్థితి TK_SELECT: {
      ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం) );
      sqlite3_వృక్షం_View_పంక్తి(pView, "subquery-expr flags=0x%x", p_సమాసం->flags);
      sqlite3_వృక్షం_View_Select(pView, p_సమాసం->x.pSelect, 0);
      ఆపు;
    }
    స్థితి TK_లోపల: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      అక్షరం *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_సమాసం->flags);
      ఒకవేళ( p_సమాసం->i_పట్టిక ) sqlite3_str_appendf(pStr, " iTable=%d",p_సమాసం->i_పట్టిక);
      ఒకవేళ( సమాసం_Has_Property(p_సమాసం, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_సమాసం->y.sub.regReturn, p_సమాసం->y.sub.iAddr);
      }
      z = sqlite3_str_ముగింపు(pStr);
      sqlite3_వృక్షం_View_పంక్తి(pView, z);
      sqlite3_free(z);
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 1);
      ఒకవేళ( సమాసం_Use_XSelect(p_సమాసం) ){
        sqlite3_వృక్షం_View_Select(pView, p_సమాసం->x.pSelect, 0);
      }లేకపోతే{
        sqlite3_వృక్షం_View_సమాసం_List(pView, p_సమాసం->x.pList, 0, 0);
      }
      ఆపు;
    }
#అయితే_ముగింపు /* SQLITE_OMIT_SUBQUERY */

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
    స్థితి TK_BETWEEN: {
      స్థిరరాశి సమాసం *pX, *pY, *pZ;
      pX = p_సమాసం->p_ఎడమ;
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
      ప్రతిజ్ఞా( p_సమాసం->x.pList->n_సమాసం==2 );
      pY = p_సమాసం->x.pList->a[0].p_సమాసం;
      pZ = p_సమాసం->x.pList->a[1].p_సమాసం;
      sqlite3_వృక్షం_View_పంక్తి(pView, "BETWEEN%s", zFlgs);
      sqlite3_వృక్షం_View_సమాసం(pView, pX, 1);
      sqlite3_వృక్షం_View_సమాసం(pView, pY, 1);
      sqlite3_వృక్షం_View_సమాసం(pView, pZ, 0);
      ఆపు;
    }
    స్థితి TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_వృక్షం_View_పంక్తి(pView, "%s(%d)", 
          p_సమాసం->i_పట్టిక ? "NEW" : "OLD", p_సమాసం->i_స్తంభం);
      ఆపు;
    }
    స్థితి TK_CASE: {
      sqlite3_వృక్షం_View_పంక్తి(pView, "CASE");
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 1);
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
      sqlite3_వృక్షం_View_సమాసం_List(pView, p_సమాసం->x.pList, 0, 0);
      ఆపు;
    }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
    స్థితి TK_RAISE: {
      స్థిరరాశి అక్షరం *zType = "unk";
      ఎంపిక( p_సమాసం->aff_సమాసం ){
        స్థితి OE_Rollback:   zType = "rollback";  ఆపు;
        స్థితి OE_Abort:      zType = "abort";     ఆపు;
        స్థితి OE_Fail:       zType = "fail";      ఆపు;
        స్థితి OE_Ignore:     zType = "ignore";    ఆపు;
      }
      ప్రతిజ్ఞా( !సమాసం_Has_Property(p_సమాసం, EP_Int_విలువ) );
      sqlite3_వృక్షం_View_పంక్తి(pView, "RAISE %s", zType);
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 0);
      ఆపు;
    }
#అయితే_ముగింపు
    స్థితి TK_సరిపోలిక: {
      sqlite3_వృక్షం_View_పంక్తి(pView, "MATCH {%d:%d}%s",
                          p_సమాసం->i_పట్టిక, p_సమాసం->i_స్తంభం, zFlgs);
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_కుడి, 0);
      ఆపు;
    }
    స్థితి TK_సదిశ: {
      అక్షరం *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      ప్రతిజ్ఞా( సమాసం_Use_XList(p_సమాసం) );
      sqlite3_వృక్షం_View_Bare_సమాసం_List(pView, p_సమాసం->x.pList, z);
      sqlite3_free(z);
      ఆపు;
    }
    స్థితి TK_SELECT_స్తంభం: {
      sqlite3_వృక్షం_View_పంక్తి(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_సమాసం->i_స్తంభం, p_సమాసం->i_పట్టిక-1,
              p_సమాసం->p_కుడి==p_సమాసం->p_ఎడమ ? " (SELECT-owner)" : "");
      ప్రతిజ్ఞా( సమాసం_Use_XSelect(p_సమాసం->p_ఎడమ) );
      sqlite3_వృక్షం_View_Select(pView, p_సమాసం->p_ఎడమ->x.pSelect, 0);
      ఆపు;
    }
    స్థితి TK_IF_NULL_అడ్డువరుస: {
      sqlite3_వృక్షం_View_పంక్తి(pView, "IF-NULL-ROW %d", p_సమాసం->i_పట్టిక);
      sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 0);
      ఆపు;
    }
    స్థితి TK_ERROR: {
      సమాసం tmp;
      sqlite3_వృక్షం_View_పంక్తి(pView, "ERROR");
      tmp = *p_సమాసం;
      tmp.op = p_సమాసం->op2;
      sqlite3_వృక్షం_View_సమాసం(pView, &tmp, 0);
      ఆపు;
    }
    స్థితి TK_అడ్డువరుస: {
      ఒకవేళ( p_సమాసం->i_స్తంభం<=0 ){
        sqlite3_వృక్షం_View_పంక్తి(pView, "First FROM table rowid");
      }లేకపోతే{
        sqlite3_వృక్షం_View_పంక్తి(pView, "First FROM table column %d",
            p_సమాసం->i_స్తంభం-1);
      }
      ఆపు;
    }
    అప్రమేయ: {
      sqlite3_వృక్షం_View_పంక్తి(pView, "op=%d", p_సమాసం->op);
      ఆపు;
    }
  }
  ఒకవేళ( zBinOp ){
    sqlite3_వృక్షం_View_పంక్తి(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 1);
    sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_కుడి, 0);
  }లేకపోతే ఒకవేళ( zUniOp ){
    sqlite3_వృక్షం_View_పంక్తి(pView, "%s%s", zUniOp, zFlgs);
   sqlite3_వృక్షం_View_సమాసం(pView, p_సమాసం->p_ఎడమ, 0);
  }
  sqlite3_వృక్షం_View_తీయి(&pView);
}


/*
** Generate a human-readable explanation of an expression list.
*/
శూన్య sqlite3_వృక్షం_View_Bare_సమాసం_List(
  వృక్షం_View *pView,
  స్థిరరాశి సమాసం_List *pList,
  స్థిరరాశి అక్షరం *zLabel
){
  ఒకవేళ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ఒకవేళ( pList==0 ){
    sqlite3_వృక్షం_View_పంక్తి(pView, "%s (empty)", zLabel);
  }లేకపోతే{
    పూర్ణాంకం i;
    sqlite3_వృక్షం_View_పంక్తి(pView, "%s", zLabel);
    కొరకు(i=0; i<pList->n_సమాసం; i++){
      పూర్ణాంకం j = pList->a[i].u.x.i_క్రమం_By_నిలువు;
      u8 క్రమీకరించు_Flags = pList->a[i].fg.క్రమీకరించు_Flags;
      అక్షరం *zName = pList->a[i].zEName;
      పూర్ణాంకం మరిన్ని_గమ్యం_Follow = i<pList->n_సమాసం - 1;
      ఒకవేళ( j || zName || క్రమీకరించు_Flags ){
        sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని_గమ్యం_Follow);
        మరిన్ని_గమ్యం_Follow = 0;
        sqlite3_వృక్షం_View_పంక్తి(pView, 0);
        ఒకవేళ( zName ){
          ఎంపిక( pList->a[i].fg.eEName ){
            అప్రమేయ:
              fprintf(stdout, "AS %s ", zName);
              ఆపు;
            స్థితి ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              ఒకవేళ( pList->a[i].fg.b_వాడిన ) fprintf(stdout, "(used) ");
              ఒకవేళ( pList->a[i].fg.b_Using_పదం ) fprintf(stdout, "(USING-term) ");
              ఒకవేళ( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              ఆపు;
            స్థితి ENAME_పరిధి:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              ఆపు;
          }
        }
        ఒకవేళ( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        ఒకవేళ( క్రమీకరించు_Flags & KEYINFO_క్రమం_DESC ){
          fprintf(stdout, "DESC ");
        }లేకపోతే ఒకవేళ( క్రమీకరించు_Flags & KEYINFO_క్రమం_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_వృక్షం_View_సమాసం(pView, pList->a[i].p_సమాసం, మరిన్ని_గమ్యం_Follow);
      ఒకవేళ( j || zName || క్రమీకరించు_Flags ){
        sqlite3_వృక్షం_View_తీయి(&pView);
      }
    }
  }
}
శూన్య sqlite3_వృక్షం_View_సమాసం_List(
  వృక్షం_View *pView,
  స్థిరరాశి సమాసం_List *pList,
  u8 మరిన్ని_గమ్యం_Follow,
  స్థిరరాశి అక్షరం *zLabel
){
  sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని_గమ్యం_Follow);
  sqlite3_వృక్షం_View_Bare_సమాసం_List(pView, pList, zLabel);
  sqlite3_వృక్షం_View_తీయి(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
శూన్య sqlite3_వృక్షం_View_Bare_Id_List(
  వృక్షం_View *pView,
  స్థిరరాశి IdList *pList,
  స్థిరరాశి అక్షరం *zLabel
){
  ఒకవేళ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ఒకవేళ( pList==0 ){
    sqlite3_వృక్షం_View_పంక్తి(pView, "%s (empty)", zLabel);
  }లేకపోతే{
    పూర్ణాంకం i;
    sqlite3_వృక్షం_View_పంక్తి(pView, "%s", zLabel);
    కొరకు(i=0; i<pList->nId; i++){
      అక్షరం *zName = pList->a[i].zName;
      పూర్ణాంకం మరిన్ని_గమ్యం_Follow = i<pList->nId - 1;
      ఒకవేళ( zName==0 ) zName = "(null)";
      sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని_గమ్యం_Follow);
      sqlite3_వృక్షం_View_పంక్తి(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
  }
}
శూన్య sqlite3_వృక్షం_View_Id_List(
  వృక్షం_View *pView,
  స్థిరరాశి IdList *pList,
  u8 మరిన్ని_గమ్యం_Follow,
  స్థిరరాశి అక్షరం *zLabel
){
  sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని_గమ్యం_Follow);
  sqlite3_వృక్షం_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_వృక్షం_View_తీయి(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
శూన్య sqlite3_వృక్షం_View_Upsert(
  వృక్షం_View *pView,
  స్థిరరాశి Upsert *pUpsert,
  u8 మరిన్ని_గమ్యం_Follow
){
  ఒకవేళ( pUpsert==0 ) తిరిగి;
  sqlite3_వృక్షం_View_నెట్టు(&pView, మరిన్ని_గమ్యం_Follow);
  అయితే( pUpsert ){
    పూర్ణాంకం n;
    sqlite3_వృక్షం_View_నెట్టు(&pView, pUpsert->p_తదుపరి_Upsert!=0 || మరిన్ని_గమ్యం_Follow);
    sqlite3_వృక్షం_View_పంక్తి(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->p_Upsert_సమితి!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_వృక్షం_View_సమాసం_List(pView, pUpsert->p_Upsert_లక్ష్యం, (n--)>0, "TARGET");
    sqlite3_వృక్షం_View_సమాసం_List(pView, pUpsert->p_Upsert_సమితి, (n--)>0, "SET");
    ఒకవేళ( pUpsert->pUpsertWhere ){
      sqlite3_వృక్షం_View_అంశం(pView, "WHERE", (n--)>0);
      sqlite3_వృక్షం_View_సమాసం(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_వృక్షం_View_తీయి(&pView);
    }
    sqlite3_వృక్షం_View_తీయి(&pView);
    pUpsert = pUpsert->p_తదుపరి_Upsert;
  }
  sqlite3_వృక్షం_View_తీయి(&pView);
}

#ఒకవేళ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
శూన్య sqlite3_వృక్షం_View_Delete(
  స్థిరరాశి With *pWith,
  స్థిరరాశి SrcList *pTabList,
  స్థిరరాశి సమాసం *pWhere,
  స్థిరరాశి సమాసం_List *p_క్రమం_By,
  స్థిరరాశి సమాసం *p_హద్దు,
  స్థిరరాశి Trigger *pTrigger
){
  పూర్ణాంకం n = 0;
  వృక్షం_View *pView = 0;
  sqlite3_వృక్షం_View_నెట్టు(&pView, 0);
  sqlite3_వృక్షం_View_పంక్తి(pView, "DELETE");
  ఒకవేళ( pWith ) n++;
  ఒకవేళ( pTabList ) n++;
  ఒకవేళ( pWhere ) n++;
  ఒకవేళ( p_క్రమం_By ) n++;
  ఒకవేళ( p_హద్దు ) n++;
  ఒకవేళ( pTrigger ) n++;
  ఒకవేళ( pWith ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_With(pView, pWith, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pTabList ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "FROM");
    sqlite3_వృక్షం_View_Src_List(pView, pTabList);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pWhere ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "WHERE");
    sqlite3_వృక్షం_View_సమాసం(pView, pWhere, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( p_క్రమం_By ){
    sqlite3_వృక్షం_View_సమాసం_List(pView, p_క్రమం_By, (--n)>0, "ORDER-BY");
  }
  ఒకవేళ( p_హద్దు ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "LIMIT");
    sqlite3_వృక్షం_View_సమాసం(pView, p_హద్దు, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pTrigger ){
    sqlite3_వృక్షం_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_వృక్షం_View_తీయి(&pView);
}
#అయితే_ముగింపు /* TREETRACE_ENABLED */

#ఒకవేళ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
శూన్య sqlite3_వృక్షం_View_చొప్పించు(
  స్థిరరాశి With *pWith,
  స్థిరరాశి SrcList *pTabList,
  స్థిరరాశి IdList *p_స్తంభం_List,
  స్థిరరాశి Select *pSelect,
  స్థిరరాశి సమాసం_List *p_సమాసం_List,
  పూర్ణాంకం onError,
  స్థిరరాశి Upsert *pUpsert,
  స్థిరరాశి Trigger *pTrigger
){
  వృక్షం_View *pView = 0;
  పూర్ణాంకం n = 0;
  స్థిరరాశి అక్షరం *zLabel = "INSERT";
  ఎంపిక( onError ){
    స్థితి OE_Replace:  zLabel = "REPLACE";             ఆపు;
    స్థితి OE_Ignore:   zLabel = "INSERT OR IGNORE";    ఆపు;
    స్థితి OE_Rollback: zLabel = "INSERT OR ROLLBACK";  ఆపు;
    స్థితి OE_Abort:    zLabel = "INSERT OR ABORT";     ఆపు;
    స్థితి OE_Fail:     zLabel = "INSERT OR FAIL";      ఆపు;
  }
  sqlite3_వృక్షం_View_నెట్టు(&pView, 0);
  sqlite3_వృక్షం_View_పంక్తి(pView, zLabel);
  ఒకవేళ( pWith ) n++;
  ఒకవేళ( pTabList ) n++;
  ఒకవేళ( p_స్తంభం_List ) n++;
  ఒకవేళ( pSelect ) n++;
  ఒకవేళ( p_సమాసం_List ) n++;
  ఒకవేళ( pUpsert ) n++;
  ఒకవేళ( pTrigger ) n++;
  ఒకవేళ( pWith ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_With(pView, pWith, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pTabList ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "INTO");
    sqlite3_వృక్షం_View_Src_List(pView, pTabList);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( p_స్తంభం_List ){
    sqlite3_వృక్షం_View_Id_List(pView, p_స్తంభం_List, (--n)>0, "COLUMNS");
  }
  ఒకవేళ( pSelect ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "DATA-SOURCE");
    sqlite3_వృక్షం_View_Select(pView, pSelect, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( p_సమాసం_List ){
    sqlite3_వృక్షం_View_సమాసం_List(pView, p_సమాసం_List, (--n)>0, "VALUES");
  }
  ఒకవేళ( pUpsert ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "UPSERT");
    sqlite3_వృక్షం_View_Upsert(pView, pUpsert, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pTrigger ){
    sqlite3_వృక్షం_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_వృక్షం_View_తీయి(&pView);
}
#అయితే_ముగింపు /* TREETRACE_ENABLED */

#ఒకవేళ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
శూన్య sqlite3_వృక్షం_View_Update(
  స్థిరరాశి With *pWith,
  స్థిరరాశి SrcList *pTabList,
  స్థిరరాశి సమాసం_List *pChanges,
  స్థిరరాశి సమాసం *pWhere,
  పూర్ణాంకం onError,
  స్థిరరాశి సమాసం_List *p_క్రమం_By,
  స్థిరరాశి సమాసం *p_హద్దు,
  స్థిరరాశి Upsert *pUpsert,
  స్థిరరాశి Trigger *pTrigger
){
  పూర్ణాంకం n = 0;
  వృక్షం_View *pView = 0;
  స్థిరరాశి అక్షరం *zLabel = "UPDATE";
  ఎంపిక( onError ){
    స్థితి OE_Replace:  zLabel = "UPDATE OR REPLACE";   ఆపు;
    స్థితి OE_Ignore:   zLabel = "UPDATE OR IGNORE";    ఆపు;
    స్థితి OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  ఆపు;
    స్థితి OE_Abort:    zLabel = "UPDATE OR ABORT";     ఆపు;
    స్థితి OE_Fail:     zLabel = "UPDATE OR FAIL";      ఆపు;
  }
  sqlite3_వృక్షం_View_నెట్టు(&pView, 0);
  sqlite3_వృక్షం_View_పంక్తి(pView, zLabel);
  ఒకవేళ( pWith ) n++;
  ఒకవేళ( pTabList ) n++;
  ఒకవేళ( pChanges ) n++;
  ఒకవేళ( pWhere ) n++;
  ఒకవేళ( p_క్రమం_By ) n++;
  ఒకవేళ( p_హద్దు ) n++;
  ఒకవేళ( pUpsert ) n++;
  ఒకవేళ( pTrigger ) n++;
  ఒకవేళ( pWith ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_With(pView, pWith, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pTabList ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "FROM");
    sqlite3_వృక్షం_View_Src_List(pView, pTabList);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pChanges ){
    sqlite3_వృక్షం_View_సమాసం_List(pView, pChanges, (--n)>0, "SET");
  }
  ఒకవేళ( pWhere ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "WHERE");
    sqlite3_వృక్షం_View_సమాసం(pView, pWhere, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( p_క్రమం_By ){
    sqlite3_వృక్షం_View_సమాసం_List(pView, p_క్రమం_By, (--n)>0, "ORDER-BY");
  }
  ఒకవేళ( p_హద్దు ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "LIMIT");
    sqlite3_వృక్షం_View_సమాసం(pView, p_హద్దు, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pUpsert ){
    sqlite3_వృక్షం_View_నెట్టు(&pView, (--n)>0);
    sqlite3_వృక్షం_View_పంక్తి(pView, "UPSERT");
    sqlite3_వృక్షం_View_Upsert(pView, pUpsert, 0);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }
  ఒకవేళ( pTrigger ){
    sqlite3_వృక్షం_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_వృక్షం_View_తీయి(&pView);
}
#అయితే_ముగింపు /* TREETRACE_ENABLED */

#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
శూన్య sqlite3_వృక్షం_View_Trigger_అడుగు(
  వృక్షం_View *pView,
  స్థిరరాశి Trigger_అడుగు *p_అడుగు,
  u8 మరిన్ని_గమ్యం_Follow,
  u8 show_నిండిన_List
){
  పూర్ణాంకం cnt = 0;
  ఒకవేళ( p_అడుగు==0 ) తిరిగి;
  sqlite3_వృక్షం_View_నెట్టు(&pView, 
      మరిన్ని_గమ్యం_Follow || (show_నిండిన_List && p_అడుగు->p_తదుపరి!=0));
  చేయి{
    ఒకవేళ( cnt++ && p_అడుగు->p_తదుపరి==0 ){
      sqlite3_వృక్షం_View_తీయి(&pView);
      sqlite3_వృక్షం_View_నెట్టు(&pView, 0);
    }
    sqlite3_వృక్షం_View_పంక్తి(pView, "%s", p_అడుగు->z_పరిధి ? p_అడుగు->z_పరిధి : "RETURNING");
  }అయితే( show_నిండిన_List && (p_అడుగు = p_అడుగు->p_తదుపరి)!=0 );
  sqlite3_వృక్షం_View_తీయి(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
శూన్య sqlite3_వృక్షం_View_Trigger(
  వృక్షం_View *pView,
  స్థిరరాశి Trigger *pTrigger,
  u8 మరిన్ని_గమ్యం_Follow,
  u8 show_నిండిన_List
){
  పూర్ణాంకం cnt = 0;
  ఒకవేళ( pTrigger==0 ) తిరిగి;
  sqlite3_వృక్షం_View_నెట్టు(&pView,
     మరిన్ని_గమ్యం_Follow || (show_నిండిన_List && pTrigger->p_తదుపరి!=0));
  చేయి{
    ఒకవేళ( cnt++ && pTrigger->p_తదుపరి==0 ){
      sqlite3_వృక్షం_View_తీయి(&pView);
      sqlite3_వృక్షం_View_నెట్టు(&pView, 0);
    }
    sqlite3_వృక్షం_View_పంక్తి(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_వృక్షం_View_నెట్టు(&pView, 0);
    sqlite3_వృక్షం_View_Trigger_అడుగు(pView, pTrigger->అడుగు_list, 0, 1);
    sqlite3_వృక్షం_View_తీయి(&pView);
  }అయితే( show_నిండిన_List && (pTrigger = pTrigger->p_తదుపరి)!=0 );
  sqlite3_వృక్షం_View_తీయి(&pView);
}
#అయితే_ముగింపు /* SQLITE_OMIT_TRIGGER */
  

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
శూన్య sqlite3_Show_సమాసం(స్థిరరాశి సమాసం *p){ sqlite3_వృక్షం_View_సమాసం(0,p,0); }
శూన్య sqlite3_Show_సమాసం_List(స్థిరరాశి సమాసం_List *p){ sqlite3_వృక్షం_View_సమాసం_List(0,p,0,0);}
శూన్య sqlite3ShowIdList(స్థిరరాశి IdList *p){ sqlite3_వృక్షం_View_Id_List(0,p,0,0); }
శూన్య sqlite3ShowSrcList(స్థిరరాశి SrcList *p){
  వృక్షం_View *pView = 0;
  sqlite3_వృక్షం_View_నెట్టు(&pView, 0);
  sqlite3_వృక్షం_View_పంక్తి(pView, "SRCLIST");
  sqlite3_వృక్షం_View_Src_List(pView,p);
  sqlite3_వృక్షం_View_తీయి(&pView);
}
శూన్య sqlite3ShowSelect(స్థిరరాశి Select *p){ sqlite3_వృక్షం_View_Select(0,p,0); }
శూన్య sqlite3ShowWith(స్థిరరాశి With *p){ sqlite3_వృక్షం_View_With(0,p,0); }
శూన్య sqlite3ShowUpsert(స్థిరరాశి Upsert *p){ sqlite3_వృక్షం_View_Upsert(0,p,0); }
#నిర్వచితం_కాకపోతే SQLITE_OMIT_TRIGGER
శూన్య sqlite3_Show_Trigger_అడుగు(స్థిరరాశి Trigger_అడుగు *p){
  sqlite3_వృక్షం_View_Trigger_అడుగు(0,p,0,0);
}
శూన్య sqlite3_Show_Trigger_అడుగు_List(స్థిరరాశి Trigger_అడుగు *p){
  sqlite3_వృక్షం_View_Trigger_అడుగు(0,p,0,1);
}
శూన్య sqlite3ShowTrigger(స్థిరరాశి Trigger *p){ sqlite3_వృక్షం_View_Trigger(0,p,0,0); }
శూన్య sqlite3ShowTriggerList(స్థిరరాశి Trigger *p){ sqlite3_వృక్షం_View_Trigger(0,p,0,1);}
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే SQLITE_OMIT_WINDOWFUNC
శూన్య sqlite3_Show_కిటికీ(స్థిరరాశి కిటికీ *p){ sqlite3_వృక్షం_View_కిటికీ(0,p,0); }
శూన్య sqlite3ShowWinFunc(స్థిరరాశి కిటికీ *p){ sqlite3_వృక్షం_View_Win_Func(0,p,0); }
#అయితే_ముగింపు

#అయితే_ముగింపు /* SQLITE_DEBUG */
