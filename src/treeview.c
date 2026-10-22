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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SQLITE_DEBUG

/*
** Add a new subitem to the tree.  The moreToFollow flag indicates that this
** is not the last item in the tree.
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(ವೃಕ್ಷ_View **pp, u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow){
  ವೃಕ್ಷ_View *p = *pp;
  ಆದರೆ( p==0 ){
    *pp = p = sqlite3_malloc64( ಗಾತ್ರ(*p) );
    ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
    memset(p, 0, ಗಾತ್ರ(*p));
  }ಇಲ್ಲದಿದ್ದರೆ{
    p->iLevel++;
  }
  ಪ್ರತಿಜ್ಞಾ( ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow==0 || ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow==1 );
  ಆದರೆ( p->iLevel<(ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(p->b_ಸಾಲು) ) p->b_ಸಾಲು[p->iLevel] = ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow;
}

/*
** Finished with one layer of the tree
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(ವೃಕ್ಷ_View **pp){
  ವೃಕ್ಷ_View *p = *pp;
  ಆದರೆ( p==0 ) ಹಿಂತಿರುಗಿ;
  p->iLevel--;
  ಆದರೆ( p->iLevel<0 ){
    sqlite3_free(p);
    *pp = 0;
  }
}

/*
** Generate a single line of output for the tree, with a prefix that contains
** all the appropriate tree lines
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(ವೃಕ್ಷ_View *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFormat, ...){
  va_list ap;
  ಪೂರ್ಣಾಂಕ i;
  StrAccum acc;
  ಅಕ್ಷರ zBuf[1000];
  sqlite3StrAccumInit(&acc, 0, zBuf, ಗಾತ್ರ(zBuf), 0);
  ಆದರೆ( p ){
    ಗಾಗಿ(i=0; i<p->iLevel && i<(ಪೂರ್ಣಾಂಕ)ಗಾತ್ರ(p->b_ಸಾಲು)-1; i++){
      sqlite3_str_append(&acc, p->b_ಸಾಲು[i] ? "|   " : "    ", 4);
    }
    sqlite3_str_append(&acc, p->b_ಸಾಲು[i] ? "|-- " : "'-- ", 4);
  }
  ಆದರೆ( zFormat!=0 ){
    va_start(ap, zFormat);
    sqlite3_str_vappendf(&acc, zFormat, ap);
    va_end(ap);
    ಪ್ರತಿಜ್ಞಾ( acc.nChar>0 || acc.accError );
    sqlite3_str_append(&acc, "\n", 1);
  }
  sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&acc);
  fprintf(stdout,"%s", zBuf);
  fflush(stdout);
}

/*
** Shorthand for starting a new tree item that consists of a single label
*/
ಸ್ಥಿರ ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(ವೃಕ್ಷ_View *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLabel,u8 ಇನ್ನಷ್ಟು_Follows){
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&p, ಇನ್ನಷ್ಟು_Follows);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(p, "%s", zLabel);
}

/*
** Show a list of Column objects in tree format.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಸ್ತಂಭ_List(
  ವೃಕ್ಷ_View *pView,
  ಸ್ಥಿರಾಂಕ ಸ್ತಂಭ *a_ಲಂಬಸಾಲು,
  ಪೂರ್ಣಾಂಕ n_ಲಂಬಸಾಲು,
  u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow
){
  ಪೂರ್ಣಾಂಕ i;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "COLUMNS");
  ಗಾಗಿ(i=0; i<n_ಲಂಬಸಾಲು; i++){
    u16 flg = a_ಲಂಬಸಾಲು[i].ಲಂಬಸಾಲು_Flags;
    ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು_ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow = i<(n_ಲಂಬಸಾಲು - 1);
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಲಂಬಸಾಲು_ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, 0);
    ಬರೆ(" %s", a_ಲಂಬಸಾಲು[i].zCnName);
    ಆಯ್ಕೆ( a_ಲಂಬಸಾಲು[i].eCType ){
      ಸ್ಥಿತಿ COLTYPE_ANY:      ಬರೆ(" ANY");        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ COLTYPE_BLOB:     ಬರೆ(" BLOB");       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ COLTYPE_INT:      ಬರೆ(" INT");        ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ COLTYPE_INTEGER:  ಬರೆ(" INTEGER");    ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ COLTYPE_REAL:     ಬರೆ(" REAL");       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ COLTYPE_ಪಠ್ಯ:     ಬರೆ(" TEXT");       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ COLTYPE_CUSTOM: {
        ಆದರೆ( flg & COLFLAG_HASTYPE ){
          ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *z = a_ಲಂಬಸಾಲು[i].zCnName;
          z += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(z)+1;
          ಬರೆ(" X-%s", z);
          ನಿಲ್ಲಿಸು;
        }
      }
    }
    ಆದರೆ( flg & COLFLAG_PRIMKEY ) ಬರೆ(" PRIMARY KEY");
    ಆದರೆ( flg & COLFLAG_HIDDEN ) ಬರೆ(" HIDDEN");
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ COLFLAG_NOEXPAND
    ಆದರೆ( flg & COLFLAG_NOEXPAND ) ಬರೆ(" NO-EXPAND");
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( flg ) ಬರೆ(" flags=%04x", flg);
    ಬರೆ("\n");      
    fflush(stdout);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

/*
** Generate a human-readable description of a WITH clause.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_With(ವೃಕ್ಷ_View *pView, ಸ್ಥಿರಾಂಕ With *pWith, u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pWith==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( pWith->nCte==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( pWith->pOuter ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WITH (0x%p, pOuter=0x%p)",pWith,pWith->pOuter);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WITH (0x%p)", pWith);
  }
  ಆದರೆ( pWith->nCte>0 ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
    ಗಾಗಿ(i=0; i<pWith->nCte; i++){
      StrAccum x;
      ಅಕ್ಷರ z_ಸಾಲು[1000];
      ಸ್ಥಿರಾಂಕ ರಚನೆ Cte *pCte = &pWith->a[i];
      sqlite3StrAccumInit(&x, 0, z_ಸಾಲು, ಗಾತ್ರ(z_ಸಾಲು), 0);
      sqlite3_str_appendf(&x, "%s", pCte->zName);
      ಆದರೆ( pCte->p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ && pCte->p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ->n_ಅಭಿವ್ಯಕ್ತಿ>0 ){
        ಅಕ್ಷರ cSep = '(';
        ಪೂರ್ಣಾಂಕ j;
        ಗಾಗಿ(j=0; j<pCte->p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ->n_ಅಭಿವ್ಯಕ್ತಿ; j++){
          sqlite3_str_appendf(&x, "%c%s", cSep, pCte->p_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ->a[j].zEName);
          cSep = ',';
        }
        sqlite3_str_appendf(&x, ")");
      }
      ಆದರೆ( pCte->eM10d!=M10d_Any ){
        sqlite3_str_appendf(&x, " %sMATERIALIZED", 
           pCte->eM10d==M10d_No ? "NOT " : "");
      }
      ಆದರೆ( pCte->pUse ){
        sqlite3_str_appendf(&x, " (pUse=0x%p, nUse=%d)", pCte->pUse,
                 pCte->pUse->nUse);
      }
      sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&x);
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, z_ಸಾಲು, i<pWith->nCte-1);
      sqlite3_ವೃಕ್ಷ_View_Select(pView, pCte->pSelect, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
}

/*
** Generate a human-readable description of a SrcList object.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Src_List(ವೃಕ್ಷ_View *pView, ಸ್ಥಿರಾಂಕ SrcList *pSrc){
  ಪೂರ್ಣಾಂಕ i;
  ಆದರೆ( pSrc==0 ) ಹಿಂತಿರುಗಿ;
  ಗಾಗಿ(i=0; i<pSrc->nSrc; i++){
    ಸ್ಥಿರಾಂಕ Src_ಅಂಶ *p_ಅಂಶ = &pSrc->a[i];
    StrAccum x;
    ಪೂರ್ಣಾಂಕ n = 0;
    ಅಕ್ಷರ z_ಸಾಲು[1000];
    sqlite3StrAccumInit(&x, 0, z_ಸಾಲು, ಗಾತ್ರ(z_ಸಾಲು), 0);
    x.printfFlags |= SQLITE_PRINTF_ಆಂತರಿಕ;
    sqlite3_str_appendf(&x, "{%d:*} %!S", p_ಅಂಶ->iCursor, p_ಅಂಶ);
    ಆದರೆ( p_ಅಂಶ->pSTab ){
      sqlite3_str_appendf(&x, " tab=%Q nCol=%d ptr=%p used=%llx%s",
           p_ಅಂಶ->pSTab->zName, p_ಅಂಶ->pSTab->n_ಲಂಬಸಾಲು, p_ಅಂಶ->pSTab, 
           p_ಅಂಶ->ಲಂಬಸಾಲು_ಬಳಸಿದ,
           p_ಅಂಶ->fg.rowid_ಬಳಸಿದ ? "+rowid" : "");
    }
    ಆದರೆ( (p_ಅಂಶ->fg.jointype & (JT_ಎಡ|JT_ಬಲ))==(JT_ಎಡ|JT_ಬಲ) ){
      sqlite3_str_appendf(&x, " FULL-OUTER-JOIN");
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಂಶ->fg.jointype & JT_ಎಡ ){
      sqlite3_str_appendf(&x, " LEFT-JOIN");
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಂಶ->fg.jointype & JT_ಬಲ ){
      sqlite3_str_appendf(&x, " RIGHT-JOIN");
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಂಶ->fg.jointype & JT_CROSS ){
      sqlite3_str_appendf(&x, " CROSS-JOIN");
    }
    ಆದರೆ( p_ಅಂಶ->fg.jointype & JT_LTORJ ){
      sqlite3_str_appendf(&x, " LTORJ");
    }
    ಆದರೆ( p_ಅಂಶ->fg.fromDDL ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ಆದರೆ( p_ಅಂಶ->fg.isCte ){
      ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *aMat[] = {",MAT", "", ",NO-MAT"};
      sqlite3_str_appendf(&x, " CteUse=%d%s",
                          p_ಅಂಶ->u2.pCteUse->nUse,
                          aMat[p_ಅಂಶ->u2.pCteUse->eM10d]);
    }
    ಆದರೆ( p_ಅಂಶ->fg.isOn || (p_ಅಂಶ->fg.isUsing==0 && p_ಅಂಶ->u3.pOn!=0) ){
      sqlite3_str_appendf(&x, " isOn");
    }
    ಆದರೆ( p_ಅಂಶ->fg.isTabFunc )      sqlite3_str_appendf(&x, " isTabFunc");
    ಆದರೆ( p_ಅಂಶ->fg.isCorrelated )   sqlite3_str_appendf(&x, " isCorrelated");
    ಆದರೆ( p_ಅಂಶ->fg.isMaterialized ) sqlite3_str_appendf(&x, " isMaterialized");
    ಆದರೆ( p_ಅಂಶ->fg.ಮೂಲಕ_Coroutine )   sqlite3_str_appendf(&x, " viaCoroutine");
    ಆದರೆ( p_ಅಂಶ->fg.notCte )         sqlite3_str_appendf(&x, " notCte");
    ಆದರೆ( p_ಅಂಶ->fg.isNestedFrom )   sqlite3_str_appendf(&x, " isNestedFrom");
    ಆದರೆ( p_ಅಂಶ->fg.fixedSchema )    sqlite3_str_appendf(&x, " fixedSchema");
    ಆದರೆ( p_ಅಂಶ->fg.hadSchema )      sqlite3_str_appendf(&x, " hadSchema");
    ಆದರೆ( p_ಅಂಶ->fg.isSubquery )     sqlite3_str_appendf(&x, " isSubquery");

    sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&x);
    sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, z_ಸಾಲು, i<pSrc->nSrc-1);
    n = 0;
    ಆದರೆ( p_ಅಂಶ->fg.isSubquery ) n++;
    ಆದರೆ( p_ಅಂಶ->fg.isTabFunc ) n++;
    ಆದರೆ( p_ಅಂಶ->fg.isUsing || p_ಅಂಶ->u3.pOn!=0 ) n++;
    ಆದರೆ( p_ಅಂಶ->fg.isUsing ){
      sqlite3_ವೃಕ್ಷ_View_Id_List(pView, p_ಅಂಶ->u3.pUsing, (--n)>0, "USING");
    }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಂಶ->u3.pOn!=0 ){
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "ON", (--n)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಂಶ->u3.pOn, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    ಆದರೆ( p_ಅಂಶ->fg.isSubquery ){
      ಪ್ರತಿಜ್ಞಾ( n==1 );
      ಆದರೆ( p_ಅಂಶ->pSTab ){
        ಕೋಷ್ಟಕ *pTab = p_ಅಂಶ->pSTab;
        sqlite3_ವೃಕ್ಷ_View_ಸ್ತಂಭ_List(pView, pTab->a_ಲಂಬಸಾಲು, pTab->n_ಲಂಬಸಾಲು, 1);
      }
      ಪ್ರತಿಜ್ಞಾ( (ಪೂರ್ಣಾಂಕ)p_ಅಂಶ->fg.isNestedFrom == IsNestedFrom(p_ಅಂಶ) );
      sqlite3_ವೃಕ್ಷ_View_Select(pView, p_ಅಂಶ->u4.pSubq->pSelect, 0);
    }
    ಆದರೆ( p_ಅಂಶ->fg.isTabFunc ){
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಂಶ->u1.pFuncArg, 0, "func-args:");
    }
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
}

/*
** Generate a human-readable description of a Select object.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Select(ವೃಕ್ಷ_View *pView, ಸ್ಥಿರಾಂಕ Select *p, u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow){
  ಪೂರ್ಣಾಂಕ n = 0;
  ಪೂರ್ಣಾಂಕ cnt = 0;
  ಆದರೆ( p==0 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "nil-SELECT");
    ಹಿಂತಿರುಗಿ;
  } 
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
  ಆದರೆ( p->pWith ){
    sqlite3_ವೃಕ್ಷ_View_With(pView, p->pWith, 1);
    cnt = 1;
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 1);
  }
  ಮಾಡು{
    ಆದರೆ( p->selFlags & SF_WhereBegin ){
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "sqlite3WhereBegin()");
    }ಇಲ್ಲದಿದ್ದರೆ{
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,
        "SELECT%s%s (%u/%p) selFlags=0x%x nSelectRow=%d",
        ((p->selFlags & SF_Distinct) ? " DISTINCT" : ""),
        ((p->selFlags & SF_Aggregate) ? " agg_flag" : ""),
        p->selId, p, p->selFlags,
        (ಪೂರ್ಣಾಂಕ)p->n_Select_ಅಡ್ಡಸಾಲು
      );
    }
    ಆದರೆ( cnt++ ) sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    ಆದರೆ( p->p_ಪೂರ್ವ ){
      n = 1000;
    }ಇಲ್ಲದಿದ್ದರೆ{
      n = 0;
      ಆದರೆ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ) n++;
      ಆದರೆ( p->pWhere ) n++;
      ಆದರೆ( p->p_ಗುಂಪು_By ) n++;
      ಆದರೆ( p->pHaving ) n++;
      ಆದರೆ( p->p_ಕ್ರಮ_By ) n++;
      ಆದರೆ( p->p_ಎಲ್ಲೆ ) n++;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಆದರೆ( p->pWin ) n++;
      ಆದರೆ( p->pWinDefn ) n++;
#ಆದರೆ_ಅಂತ್ಯ
    }
    ಆದರೆ( p->pEList ){
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p->pEList, n>0, "result-set");
    }
    n--;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( p->pWin ){
      ಕಿಟಕಿ *pX;
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "window-functions");
      ಗಾಗಿ(pX=p->pWin; pX; pX=pX->p_ಮುಂದಿನ_Win){
        sqlite3_ವೃಕ್ಷ_View_Win_Func(pView, pX, pX->p_ಮುಂದಿನ_Win!=0);
      }
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p->pSrc && p->pSrc->nSrc && p->pSrc->nAlloc ){
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FROM");
      sqlite3_ವೃಕ್ಷ_View_Src_List(pView, p->pSrc);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    ಆದರೆ( p->pWhere ){
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "WHERE", (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p->pWhere, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    ಆದರೆ( p->p_ಗುಂಪು_By ){
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p->p_ಗುಂಪು_By, (n--)>0, "GROUPBY");
    }
    ಆದರೆ( p->pHaving ){
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "HAVING", (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p->pHaving, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
    ಆದರೆ( p->pWinDefn ){
      ಕಿಟಕಿ *pX;
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "WINDOW", (n--)>0);
      ಗಾಗಿ(pX=p->pWinDefn; pX; pX=pX->p_ಮುಂದಿನ_Win){
        sqlite3_ವೃಕ್ಷ_View_ಕಿಟಕಿ(pView, pX, pX->p_ಮುಂದಿನ_Win!=0);
      }
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಆದರೆ( p->p_ಕ್ರಮ_By ){
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p->p_ಕ್ರಮ_By, (n--)>0, "ORDERBY");
    }
    ಆದರೆ( p->p_ಎಲ್ಲೆ ){
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "LIMIT", (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p->p_ಎಲ್ಲೆ->p_ಎಡ, p->p_ಎಲ್ಲೆ->p_ಬಲ!=0);
      ಆದರೆ( p->p_ಎಲ್ಲೆ->p_ಬಲ ){
        sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "OFFSET", 0);
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p->p_ಎಲ್ಲೆ->p_ಬಲ, 0);
        sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      }
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    ಆದರೆ( p->p_ಪೂರ್ವ ){
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOp = "UNION";
      ಆಯ್ಕೆ( p->op ){
        ಸ್ಥಿತಿ TK_ALL:         zOp = "UNION ALL";  ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ TK_INTERSECT:   zOp = "INTERSECT";  ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ TK_EXCEPT:      zOp = "EXCEPT";     ನಿಲ್ಲಿಸು;
      }
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, zOp, 1);
    }
    p = p->p_ಪೂರ್ವ;
  }ಆಗ( p!=0 );
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
/*
** Generate a description of starting or stopping bounds
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Bound(
  ವೃಕ್ಷ_View *pView,        /* View context */
  u8 eBound,              /* UNBOUNDED, CURRENT, PRECEDING, FOLLOWING */
  ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ,            /* Value for PRECEDING or FOLLOWING */
  u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow         /* True if more to follow */
){
  ಆಯ್ಕೆ( eBound ){
    ಸ್ಥಿತಿ TK_UNBOUNDED: {
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "UNBOUNDED", ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_CURRENT: {
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "CURRENT", ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_PRECEDING: {
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "PRECEDING", ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_FOLLOWING: {
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "FOLLOWING", ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      ನಿಲ್ಲಿಸು;
    }
  }
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window object
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಕಿಟಕಿ(ವೃಕ್ಷ_View *pView, ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ *pWin, u8 ಇನ್ನಷ್ಟು){
  ಪೂರ್ಣಾಂಕ nElement = 0;
  ಆದರೆ( pWin==0 ) ಹಿಂತಿರುಗಿ;
  ಆದರೆ( pWin->pFilter ){
    sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "FILTER", 1);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pWin->pFilter, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    ಆದರೆ( pWin->eFrmType==TK_FILTER ) ಹಿಂತಿರುಗಿ;
  }
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು);
  ಆದರೆ( pWin->zName ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "OVER %s (%p)", pWin->zName, pWin);
  }ಇಲ್ಲದಿದ್ದರೆ{
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "OVER (%p)", pWin);
  }
  ಆದರೆ( pWin->z_ತಳ )    nElement++;
  ಆದರೆ( pWin->p_ಕ್ರಮ_By ) nElement++;
  ಆದರೆ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ) nElement++;
  ಆದರೆ( pWin->eExclude ) nElement++;
  ಆದರೆ( pWin->z_ತಳ ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--nElement)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "window: %s", pWin->z_ತಳ);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pWin->p_ವಿಭಜನೆ ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pWin->p_ವಿಭಜನೆ, nElement>0,"PARTITION-BY");
  }
  ಆದರೆ( pWin->p_ಕ್ರಮ_By ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pWin->p_ಕ್ರಮ_By, (--nElement)>0, "ORDER-BY");
  }
  ಆದರೆ( pWin->eFrmType!=0 && pWin->eFrmType!=TK_FILTER ){
    ಅಕ್ಷರ zBuf[30];
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zFrmType = "ROWS";
    ಆದರೆ( pWin->eFrmType==TK_RANGE ) zFrmType = "RANGE";
    ಆದರೆ( pWin->eFrmType==TK_GROUPS ) zFrmType = "GROUPS";
    sqlite3_snprintf(ಗಾತ್ರ(zBuf),zBuf,"%s%s",zFrmType,
        pWin->b_Implicit_ಚೌಕಟ್ಟು ? " (implied)" : "");
    sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, zBuf, (--nElement)>0);
    sqlite3_ವೃಕ್ಷ_View_Bound(pView, pWin->e_ಆರಂಭ, pWin->p_ಆರಂಭ, 1);
    sqlite3_ವೃಕ್ಷ_View_Bound(pView, pWin->e_ಕೊನೆ, pWin->p_ಕೊನೆ, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pWin->eExclude ){
    ಅಕ್ಷರ zBuf[30];
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zExclude;
    ಆಯ್ಕೆ( pWin->eExclude ){
      ಸ್ಥಿತಿ TK_NO:      zExclude = "NO OTHERS";   ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ TK_CURRENT: zExclude = "CURRENT ROW"; ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ TK_ಗುಂಪು:   zExclude = "GROUP";       ನಿಲ್ಲಿಸು;
      ಸ್ಥಿತಿ TK_TIES:    zExclude = "TIES";        ನಿಲ್ಲಿಸು;
      ಪೂರ್ವನಿಯೋಜಿತ:
        sqlite3_snprintf(ಗಾತ್ರ(zBuf),zBuf,"invalid(%d)", pWin->eExclude);
        zExclude = zBuf;
        ನಿಲ್ಲಿಸು;
    }
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "EXCLUDE %s", zExclude);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
/*
** Generate a human-readable explanation for a Window Function object
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Win_Func(ವೃಕ್ಷ_View *pView, ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ *pWin, u8 ಇನ್ನಷ್ಟು){
  ಆದರೆ( pWin==0 ) ಹಿಂತಿರುಗಿ;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WINFUNC %s(%d)",
                       pWin->pWFunc->zName, pWin->pWFunc->nArg);
  sqlite3_ವೃಕ್ಷ_View_ಕಿಟಕಿ(pView, pWin, 0);
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_WINDOWFUNC */

/*
** Generate a human-readable explanation of an expression tree.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(ವೃಕ್ಷ_View *pView, ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಅಭಿವ್ಯಕ್ತಿ, u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow){
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zBinOp = 0;   /* Binary operator */
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zUniOp = 0;   /* Unary operator */
  ಅಕ್ಷರ zFlgs[200];
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ==0 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "nil");
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    ಹಿಂತಿರುಗಿ;
  }
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->flags || p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ || p_ಅಭಿವ್ಯಕ್ತಿ->vvaFlags || p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo ){
    StrAccum x;
    sqlite3StrAccumInit(&x, 0, zFlgs, ಗಾತ್ರ(zFlgs), 0);
    sqlite3_str_appendf(&x, " fg.af=%x.%c",
      p_ಅಭಿವ್ಯಕ್ತಿ->flags, p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ ? p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ : 'n');
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_OuterON) ){
      sqlite3_str_appendf(&x, " outer.iJoin=%d", p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin);
    }
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_InnerON) ){
      sqlite3_str_appendf(&x, " inner.iJoin=%d", p_ಅಭಿವ್ಯಕ್ತಿ->w.iJoin);
    }
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_FromDDL) ){
      sqlite3_str_appendf(&x, " DDL");
    }
    ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_VVAProperty(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Immutable) ){
      sqlite3_str_appendf(&x, " IMMUTABLE");
    }
    ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo!=0 ){
      sqlite3_str_appendf(&x, " agg-column[%d]", p_ಅಭಿವ್ಯಕ್ತಿ->iAgg);
    }
    sqlite3_Str_Accum_ಮುಕ್ತಾಯ(&x);
  }ಇಲ್ಲದಿದ್ದರೆ{
    zFlgs[0] = 0;
  }
  ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op ){
    ಸ್ಥಿತಿ TK_AGG_ಸ್ತಂಭ: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "AGG{%d:%d}%s",
            p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ, zFlgs);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ಸ್ತಂಭ: {
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ<0 ){
        /* This only happens when coding check constraints */
        ಅಕ್ಷರ zOp2[16];
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op2 ){
          sqlite3_snprintf(ಗಾತ್ರ(zOp2),zOp2," op2=0x%02x",p_ಅಭಿವ್ಯಕ್ತಿ->op2);
        }ಇಲ್ಲದಿದ್ದರೆ{
          zOp2[0] = 0;
        }
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "COLUMN(%d)%s%s",
                                    p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ, zFlgs, zOp2);
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_YTab(p_ಅಭಿವ್ಯಕ್ತಿ) );
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "{%d:%d} pTab=%p%s",
                        p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ,
                        p_ಅಭಿವ್ಯಕ್ತಿ->y.pTab, zFlgs);
      }
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Fixed_ಲಂಬಸಾಲು) ){
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_INTEGER: {
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->flags & EP_Int_ಮೌಲ್ಯ ){
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%d", p_ಅಭಿವ್ಯಕ್ತಿ->u.i_ಮೌಲ್ಯ);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      }
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_FLOATING_ಬಿಂದು
    ಸ್ಥಿತಿ TK_FLOAT: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"%s", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ TK_STRING: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"%Q", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_NULL: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"NULL");
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_TRUEFALSE: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"%s%s",
         sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ) ? "TRUE" : "FALSE", zFlgs);
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_BLOB_LITERAL
    ಸ್ಥಿತಿ TK_BLOB: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"%s", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ TK_VARIABLE: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"VARIABLE(%s,%d)",
                          p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ರಿಜಿಸ್ಟರ್: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"REGISTER(%d)", p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ID: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"ID \"%w\"", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ASTERISK: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"ASKTERISK");
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_CAST
    ಸ್ಥಿತಿ TK_CAST: {
      /* Expressions of the form:   CAST(pLeft AS token) */
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView,"CAST %Q", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_CAST */
    ಸ್ಥಿತಿ TK_LT:      zBinOp = "LT";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_LE:      zBinOp = "LE";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_GT:      zBinOp = "GT";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_GE:      zBinOp = "GE";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_NE:      zBinOp = "NE";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_EQ:      zBinOp = "EQ";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_IS:      zBinOp = "IS";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_ISNOT:   zBinOp = "ISNOT";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_AND:     zBinOp = "AND";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_OR:      zBinOp = "OR";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_PLUS:    zBinOp = "ADD";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_ನಕ್ಷತ್ರ:    zBinOp = "MUL";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_MINUS:   zBinOp = "SUB";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_REM:     zBinOp = "REM";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_BITAND:  zBinOp = "BITAND"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_BITOR:   zBinOp = "BITOR";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_SLASH:   zBinOp = "DIV";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_LSHIFT:  zBinOp = "LSHIFT"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_RSHIFT:  zBinOp = "RSHIFT"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_CONCAT:  zBinOp = "CONCAT"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_DOT:     zBinOp = "DOT";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_ಎಲ್ಲೆ:   zBinOp = "LIMIT";  ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ TK_UMINUS:  zUniOp = "UMINUS"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_UPLUS:   zUniOp = "UPLUS";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_BITNOT:  zUniOp = "BITNOT"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_NOT:     zUniOp = "NOT";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_ISNULL:  zUniOp = "ISNULL"; ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ TK_NOTNULL: zUniOp = "NOTNULL"; ನಿಲ್ಲಿಸು;

    ಸ್ಥಿತಿ TK_TRUTH: {
      ಪೂರ್ಣಾಂಕ x;
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *azOp[] = {
         "IS-FALSE", "IS-TRUE", "IS-NOT-FALSE", "IS-NOT-TRUE"
      };
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_IS || p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_ISNOT );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ );
      ಪ್ರತಿಜ್ಞಾ( sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Skip_Collate_And_Likely(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ)->op
                  == TK_TRUEFALSE );
      x = (p_ಅಭಿವ್ಯಕ್ತಿ->op2==TK_ISNOT)*2 + sqlite3_ಅಭಿವ್ಯಕ್ತಿ_Truth_ಮೌಲ್ಯ(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ);
      zUniOp = azOp[x];
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ TK_ಹರವು: {
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "SPAN %Q", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ TK_COLLATE: {
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
      ನಿಲ್ಲಿಸು;
    }

    ಸ್ಥಿತಿ TK_AGG_FUNCTION:
    ಸ್ಥಿತಿ TK_FUNCTION: {
      ಅಭಿವ್ಯಕ್ತಿ_List *pFarg;       /* List of function arguments */
      ಕಿಟಕಿ *pWin;
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_TokenOnly) ){
        pFarg = 0;
        pWin = 0;
      }ಇಲ್ಲದಿದ್ದರೆ{
        ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
        pFarg = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
        pWin = Is_ಕಿಟಕಿ_Func(p_ಅಭಿವ್ಯಕ್ತಿ) ? p_ಅಭಿವ್ಯಕ್ತಿ->y.pWin : 0;
#ಇಲ್ಲದಿದ್ದರೆ
        pWin = 0;
#ಆದರೆ_ಅಂತ್ಯ 
      }
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op==TK_AGG_FUNCTION ){
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "AGG_FUNCTION%d %Q%s agg=%d[%d]/%p",
                             p_ಅಭಿವ್ಯಕ್ತಿ->op2, p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, zFlgs,
                             p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo ? p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo->selId : 0,
                             p_ಅಭಿವ್ಯಕ್ತಿ->iAgg, p_ಅಭಿವ್ಯಕ್ತಿ->pAggInfo);
      }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op2!=0 ){
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zOp2;
        ಅಕ್ಷರ zBuf[8];
        sqlite3_snprintf(ಗಾತ್ರ(zBuf),zBuf,"0x%02x",p_ಅಭಿವ್ಯಕ್ತಿ->op2);
        zOp2 = zBuf;
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==NC_IsCheck ) zOp2 = "NC_IsCheck";
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==NC_Idx_ಅಭಿವ್ಯಕ್ತಿ ) zOp2 = "NC_IdxExpr";
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==NC_PartIdx ) zOp2 = "NC_PartIdx";
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->op2==NC_Gen_ಲಂಬಸಾಲು ) zOp2 = "NC_GenCol";
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FUNCTION %Q%s op2=%s",
                            p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, zFlgs, zOp2);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FUNCTION %Q%s", p_ಅಭಿವ್ಯಕ್ತಿ->u.zToken, zFlgs);
      }
      ಆದರೆ( pFarg ){
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pFarg, pWin!=0 || p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
        ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ){
          ಅಭಿವ್ಯಕ್ತಿ *pOB = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
          ಪ್ರತಿಜ್ಞಾ( pOB->op==TK_ಕ್ರಮ );
          ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(pOB) );
          sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pOB->x.pList, pWin!=0, "ORDERBY");
        }
      }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
      ಆದರೆ( pWin ){
        sqlite3_ವೃಕ್ಷ_View_ಕಿಟಕಿ(pView, pWin, 0);
      }
#ಆದರೆ_ಅಂತ್ಯ
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ಕ್ರಮ: {
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, 0, "ORDERBY");
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_SUBQUERY
    ಸ್ಥಿತಿ TK_EXISTS: {
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "EXISTS-expr flags=0x%x", p_ಅಭಿವ್ಯಕ್ತಿ->flags);
      sqlite3_ವೃಕ್ಷ_View_Select(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_SELECT: {
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "subquery-expr flags=0x%x", p_ಅಭಿವ್ಯಕ್ತಿ->flags);
      sqlite3_ವೃಕ್ಷ_View_Select(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ಒಳ: {
      sqlite3_str *pStr = sqlite3_str_new(0);
      ಅಕ್ಷರ *z;
      sqlite3_str_appendf(pStr, "IN flags=0x%x", p_ಅಭಿವ್ಯಕ್ತಿ->flags);
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ ) sqlite3_str_appendf(pStr, " iTable=%d",p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ);
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Subrtn) ){
        sqlite3_str_appendf(pStr, " subrtn(%d,%d)",
            p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.regReturn, p_ಅಭಿವ್ಯಕ್ತಿ->y.sub.iAddr);
      }
      z = sqlite3_str_ಮುಕ್ತಾಯ(pStr);
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, z);
      sqlite3_free(z);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 1);
      ಆದರೆ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ) ){
        sqlite3_ವೃಕ್ಷ_View_Select(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pSelect, 0);
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, 0, 0);
      }
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_SUBQUERY */

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
    ಸ್ಥಿತಿ TK_BETWEEN: {
      ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pX, *pY, *pZ;
      pX = p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ;
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      ಪ್ರತಿಜ್ಞಾ( p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->n_ಅಭಿವ್ಯಕ್ತಿ==2 );
      pY = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[0].p_ಅಭಿವ್ಯಕ್ತಿ;
      pZ = p_ಅಭಿವ್ಯಕ್ತಿ->x.pList->a[1].p_ಅಭಿವ್ಯಕ್ತಿ;
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "BETWEEN%s", zFlgs);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pX, 1);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pY, 1);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pZ, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_TRIGGER: {
      /* If the opcode is TK_TRIGGER, then the expression is a reference
      ** to a column in the new.* or old.* pseudo-tables available to
      ** trigger programs. In this case Expr.iTable is set to 1 for the
      ** new.* pseudo-table, or 0 for the old.* pseudo-table. Expr.iColumn
      ** is set to the column of the pseudo-table to read, or to -1 to
      ** read the rowid field.
      */
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s(%d)", 
          p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ ? "NEW" : "OLD", p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_CASE: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "CASE");
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 1);
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, 0, 0);
      ನಿಲ್ಲಿಸು;
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
    ಸ್ಥಿತಿ TK_RAISE: {
      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zType = "unk";
      ಆಯ್ಕೆ( p_ಅಭಿವ್ಯಕ್ತಿ->aff_ಅಭಿವ್ಯಕ್ತಿ ){
        ಸ್ಥಿತಿ OE_Rollback:   zType = "rollback";  ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ OE_Abort:      zType = "abort";     ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ OE_Fail:       zType = "fail";      ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ OE_Ignore:     zType = "ignore";    ನಿಲ್ಲಿಸು;
      }
      ಪ್ರತಿಜ್ಞಾ( !ಅಭಿವ್ಯಕ್ತಿ_Has_Property(p_ಅಭಿವ್ಯಕ್ತಿ, EP_Int_ಮೌಲ್ಯ) );
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "RAISE %s", zType);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ನಿಲ್ಲಿಸು;
    }
#ಆದರೆ_ಅಂತ್ಯ
    ಸ್ಥಿತಿ TK_ಹೊಂದಿಕೆ: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "MATCH {%d:%d}%s",
                          p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ, zFlgs);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ಸದಿಶ: {
      ಅಕ್ಷರ *z = sqlite3_mprintf("VECTOR%s",zFlgs);
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XList(p_ಅಭಿವ್ಯಕ್ತಿ) );
      sqlite3_ವೃಕ್ಷ_View_Bare_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ->x.pList, z);
      sqlite3_free(z);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_SELECT_ಸ್ತಂಭ: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "SELECT-COLUMN %d of [0..%d]%s",
              p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ, p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ-1,
              p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ==p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ ? " (SELECT-owner)" : "");
      ಪ್ರತಿಜ್ಞಾ( ಅಭಿವ್ಯಕ್ತಿ_Use_XSelect(p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ) );
      sqlite3_ವೃಕ್ಷ_View_Select(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ->x.pSelect, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_IF_NULL_ಅಡ್ಡಸಾಲು: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "IF-NULL-ROW %d", p_ಅಭಿವ್ಯಕ್ತಿ->i_ಕೋಷ್ಟಕ);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ERROR: {
      ಅಭಿವ್ಯಕ್ತಿ tmp;
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "ERROR");
      tmp = *p_ಅಭಿವ್ಯಕ್ತಿ;
      tmp.op = p_ಅಭಿವ್ಯಕ್ತಿ->op2;
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, &tmp, 0);
      ನಿಲ್ಲಿಸು;
    }
    ಸ್ಥಿತಿ TK_ಅಡ್ಡಸಾಲು: {
      ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ<=0 ){
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "First FROM table rowid");
      }ಇಲ್ಲದಿದ್ದರೆ{
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "First FROM table column %d",
            p_ಅಭಿವ್ಯಕ್ತಿ->i_ಸ್ತಂಭ-1);
      }
      ನಿಲ್ಲಿಸು;
    }
    ಪೂರ್ವನಿಯೋಜಿತ: {
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "op=%d", p_ಅಭಿವ್ಯಕ್ತಿ->op);
      ನಿಲ್ಲಿಸು;
    }
  }
  ಆದರೆ( zBinOp ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s%s", zBinOp, zFlgs);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಎಡ, 1);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಅಭಿವ್ಯಕ್ತಿ->p_ಬಲ, 0);
  }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( zUniOp ){
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
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pList,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLabel
){
  ಆದರೆ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ಆದರೆ( pList==0 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s (empty)", zLabel);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ i;
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s", zLabel);
    ಗಾಗಿ(i=0; i<pList->n_ಅಭಿವ್ಯಕ್ತಿ; i++){
      ಪೂರ್ಣಾಂಕ j = pList->a[i].u.x.i_ಕ್ರಮ_By_ಲಂಬಸಾಲು;
      u8 ವಿಂಗಡಿಸು_Flags = pList->a[i].fg.ವಿಂಗಡಿಸು_Flags;
      ಅಕ್ಷರ *zName = pList->a[i].zEName;
      ಪೂರ್ಣಾಂಕ ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow = i<pList->n_ಅಭಿವ್ಯಕ್ತಿ - 1;
      ಆದರೆ( j || zName || ವಿಂಗಡಿಸು_Flags ){
        sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
        ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow = 0;
        sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, 0);
        ಆದರೆ( zName ){
          ಆಯ್ಕೆ( pList->a[i].fg.eEName ){
            ಪೂರ್ವನಿಯೋಜಿತ:
              fprintf(stdout, "AS %s ", zName);
              ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ ENAME_TAB:
              fprintf(stdout, "TABLE-ALIAS-NAME(\"%s\") ", zName);
              ಆದರೆ( pList->a[i].fg.b_ಬಳಸಿದ ) fprintf(stdout, "(used) ");
              ಆದರೆ( pList->a[i].fg.b_Using_ಪದ ) fprintf(stdout, "(USING-term) ");
              ಆದರೆ( pList->a[i].fg.bNoExpand ) fprintf(stdout, "(NoExpand) ");
              ನಿಲ್ಲಿಸು;
            ಸ್ಥಿತಿ ENAME_ಹರವು:
              fprintf(stdout, "SPAN(\"%s\") ", zName);
              ನಿಲ್ಲಿಸು;
          }
        }
        ಆದರೆ( j ){
          fprintf(stdout, "iOrderByCol=%d ", j);
        }
        ಆದರೆ( ವಿಂಗಡಿಸು_Flags & KEYINFO_ಕ್ರಮ_DESC ){
          fprintf(stdout, "DESC ");
        }ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ( ವಿಂಗಡಿಸು_Flags & KEYINFO_ಕ್ರಮ_BIGNULL ){
          fprintf(stdout, "NULLS-LAST");
        }
        fprintf(stdout, "\n");
        fflush(stdout);
      }
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pList->a[i].p_ಅಭಿವ್ಯಕ್ತಿ, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
      ಆದರೆ( j || zName || ವಿಂಗಡಿಸು_Flags ){
        sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      }
    }
  }
}
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(
  ವೃಕ್ಷ_View *pView,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pList,
  u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLabel
){
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
  sqlite3_ವೃಕ್ಷ_View_Bare_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pList, zLabel);
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

/*
** Generate a human-readable explanation of an id-list.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Bare_Id_List(
  ವೃಕ್ಷ_View *pView,
  ಸ್ಥಿರಾಂಕ IdList *pList,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLabel
){
  ಆದರೆ( zLabel==0 || zLabel[0]==0 ) zLabel = "LIST";
  ಆದರೆ( pList==0 ){
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s (empty)", zLabel);
  }ಇಲ್ಲದಿದ್ದರೆ{
    ಪೂರ್ಣಾಂಕ i;
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s", zLabel);
    ಗಾಗಿ(i=0; i<pList->nId; i++){
      ಅಕ್ಷರ *zName = pList->a[i].zName;
      ಪೂರ್ಣಾಂಕ ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow = i<pList->nId - 1;
      ಆದರೆ( zName==0 ) zName = "(null)";
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
      sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, 0);
      fprintf(stdout, "%s\n", zName);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
  }
}
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Id_List(
  ವೃಕ್ಷ_View *pView,
  ಸ್ಥಿರಾಂಕ IdList *pList,
  u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow,
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLabel
){
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
  sqlite3_ವೃಕ್ಷ_View_Bare_Id_List(pView, pList, zLabel);
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

/*
** Generate a human-readable explanation of a list of Upsert objects
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Upsert(
  ವೃಕ್ಷ_View *pView,
  ಸ್ಥಿರಾಂಕ Upsert *pUpsert,
  u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow
){
  ಆದರೆ( pUpsert==0 ) ಹಿಂತಿರುಗಿ;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
  ಆಗ( pUpsert ){
    ಪೂರ್ಣಾಂಕ n;
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, pUpsert->p_ಮುಂದಿನ_Upsert!=0 || ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "ON CONFLICT DO %s", 
         pUpsert->isDoUpdate ? "UPDATE" : "NOTHING");
    n = (pUpsert->p_Upsert_ಗಣ!=0) + (pUpsert->pUpsertWhere!=0);
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pUpsert->p_Upsert_ಗುರಿ, (n--)>0, "TARGET");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pUpsert->p_Upsert_ಗಣ, (n--)>0, "SET");
    ಆದರೆ( pUpsert->pUpsertWhere ){
      sqlite3_ವೃಕ್ಷ_View_ಅಂಶ(pView, "WHERE", (n--)>0);
      sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pUpsert->pUpsertWhere, 0);
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    }
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
    pUpsert = pUpsert->p_ಮುಂದಿನ_Upsert;
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}

#ಆದರೆ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an DELETE statement.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Delete(
  ಸ್ಥಿರಾಂಕ With *pWith,
  ಸ್ಥಿರಾಂಕ SrcList *pTabList,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pWhere,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ,
  ಸ್ಥಿರಾಂಕ Trigger *pTrigger
){
  ಪೂರ್ಣಾಂಕ n = 0;
  ವೃಕ್ಷ_View *pView = 0;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "DELETE");
  ಆದರೆ( pWith ) n++;
  ಆದರೆ( pTabList ) n++;
  ಆದರೆ( pWhere ) n++;
  ಆದರೆ( p_ಕ್ರಮ_By ) n++;
  ಆದರೆ( p_ಎಲ್ಲೆ ) n++;
  ಆದರೆ( pTrigger ) n++;
  ಆದರೆ( pWith ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_With(pView, pWith, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pTabList ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FROM");
    sqlite3_ವೃಕ್ಷ_View_Src_List(pView, pTabList);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pWhere ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WHERE");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pWhere, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( p_ಕ್ರಮ_By ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಕ್ರಮ_By, (--n)>0, "ORDER-BY");
  }
  ಆದರೆ( p_ಎಲ್ಲೆ ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "LIMIT");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಎಲ್ಲೆ, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pTrigger ){
    sqlite3_ವೃಕ್ಷ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#ಆದರೆ_ಅಂತ್ಯ /* TREETRACE_ENABLED */

#ಆದರೆ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an INSERT statement.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_ಸೇರಿಸು(
  ಸ್ಥಿರಾಂಕ With *pWith,
  ಸ್ಥಿರಾಂಕ SrcList *pTabList,
  ಸ್ಥಿರಾಂಕ IdList *p_ಸ್ತಂಭ_List,
  ಸ್ಥಿರಾಂಕ Select *pSelect,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *p_ಅಭಿವ್ಯಕ್ತಿ_List,
  ಪೂರ್ಣಾಂಕ onError,
  ಸ್ಥಿರಾಂಕ Upsert *pUpsert,
  ಸ್ಥಿರಾಂಕ Trigger *pTrigger
){
  ವೃಕ್ಷ_View *pView = 0;
  ಪೂರ್ಣಾಂಕ n = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLabel = "INSERT";
  ಆಯ್ಕೆ( onError ){
    ಸ್ಥಿತಿ OE_Replace:  zLabel = "REPLACE";             ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Ignore:   zLabel = "INSERT OR IGNORE";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Rollback: zLabel = "INSERT OR ROLLBACK";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Abort:    zLabel = "INSERT OR ABORT";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Fail:     zLabel = "INSERT OR FAIL";      ನಿಲ್ಲಿಸು;
  }
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, zLabel);
  ಆದರೆ( pWith ) n++;
  ಆದರೆ( pTabList ) n++;
  ಆದರೆ( p_ಸ್ತಂಭ_List ) n++;
  ಆದರೆ( pSelect ) n++;
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ_List ) n++;
  ಆದರೆ( pUpsert ) n++;
  ಆದರೆ( pTrigger ) n++;
  ಆದರೆ( pWith ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_With(pView, pWith, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pTabList ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "INTO");
    sqlite3_ವೃಕ್ಷ_View_Src_List(pView, pTabList);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( p_ಸ್ತಂಭ_List ){
    sqlite3_ವೃಕ್ಷ_View_Id_List(pView, p_ಸ್ತಂಭ_List, (--n)>0, "COLUMNS");
  }
  ಆದರೆ( pSelect ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "DATA-SOURCE");
    sqlite3_ವೃಕ್ಷ_View_Select(pView, pSelect, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( p_ಅಭಿವ್ಯಕ್ತಿ_List ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಅಭಿವ್ಯಕ್ತಿ_List, (--n)>0, "VALUES");
  }
  ಆದರೆ( pUpsert ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "UPSERT");
    sqlite3_ವೃಕ್ಷ_View_Upsert(pView, pUpsert, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pTrigger ){
    sqlite3_ವೃಕ್ಷ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#ಆದರೆ_ಅಂತ್ಯ /* TREETRACE_ENABLED */

#ಆದರೆ TREETRACE_ENABLED
/*
** Generate a human-readable diagram of the data structure that go
** into generating an UPDATE statement.
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Update(
  ಸ್ಥಿರಾಂಕ With *pWith,
  ಸ್ಥಿರಾಂಕ SrcList *pTabList,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *pChanges,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *pWhere,
  ಪೂರ್ಣಾಂಕ onError,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *p_ಕ್ರಮ_By,
  ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p_ಎಲ್ಲೆ,
  ಸ್ಥಿರಾಂಕ Upsert *pUpsert,
  ಸ್ಥಿರಾಂಕ Trigger *pTrigger
){
  ಪೂರ್ಣಾಂಕ n = 0;
  ವೃಕ್ಷ_View *pView = 0;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *zLabel = "UPDATE";
  ಆಯ್ಕೆ( onError ){
    ಸ್ಥಿತಿ OE_Replace:  zLabel = "UPDATE OR REPLACE";   ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Ignore:   zLabel = "UPDATE OR IGNORE";    ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Rollback: zLabel = "UPDATE OR ROLLBACK";  ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Abort:    zLabel = "UPDATE OR ABORT";     ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ OE_Fail:     zLabel = "UPDATE OR FAIL";      ನಿಲ್ಲಿಸು;
  }
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, zLabel);
  ಆದರೆ( pWith ) n++;
  ಆದರೆ( pTabList ) n++;
  ಆದರೆ( pChanges ) n++;
  ಆದರೆ( pWhere ) n++;
  ಆದರೆ( p_ಕ್ರಮ_By ) n++;
  ಆದರೆ( p_ಎಲ್ಲೆ ) n++;
  ಆದರೆ( pUpsert ) n++;
  ಆದರೆ( pTrigger ) n++;
  ಆದರೆ( pWith ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_With(pView, pWith, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pTabList ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "FROM");
    sqlite3_ವೃಕ್ಷ_View_Src_List(pView, pTabList);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pChanges ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, pChanges, (--n)>0, "SET");
  }
  ಆದರೆ( pWhere ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "WHERE");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, pWhere, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( p_ಕ್ರಮ_By ){
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(pView, p_ಕ್ರಮ_By, (--n)>0, "ORDER-BY");
  }
  ಆದರೆ( p_ಎಲ್ಲೆ ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "LIMIT");
    sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(pView, p_ಎಲ್ಲೆ, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pUpsert ){
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, (--n)>0);
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "UPSERT");
    sqlite3_ವೃಕ್ಷ_View_Upsert(pView, pUpsert, 0);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }
  ಆದರೆ( pTrigger ){
    sqlite3_ವೃಕ್ಷ_View_Trigger(pView, pTrigger, (--n)>0, 1);
  }
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#ಆದರೆ_ಅಂತ್ಯ /* TREETRACE_ENABLED */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
/*
** Show a human-readable graph of a TriggerStep
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Trigger_ಹೆಜ್ಜೆ(
  ವೃಕ್ಷ_View *pView,
  ಸ್ಥಿರಾಂಕ Trigger_ಹೆಜ್ಜೆ *p_ಹೆಜ್ಜೆ,
  u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow,
  u8 show_ತುಂಬಿದ_List
){
  ಪೂರ್ಣಾಂಕ cnt = 0;
  ಆದರೆ( p_ಹೆಜ್ಜೆ==0 ) ಹಿಂತಿರುಗಿ;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 
      ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow || (show_ತುಂಬಿದ_List && p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ!=0));
  ಮಾಡು{
    ಆದರೆ( cnt++ && p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ==0 ){
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
    }
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "%s", p_ಹೆಜ್ಜೆ->z_ಹರವು ? p_ಹೆಜ್ಜೆ->z_ಹರವು : "RETURNING");
  }ಆಗ( show_ತುಂಬಿದ_List && (p_ಹೆಜ್ಜೆ = p_ಹೆಜ್ಜೆ->p_ಮುಂದಿನ)!=0 );
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
  
/*
** Show a human-readable graph of a Trigger
*/
ಶೂನ್ಯ sqlite3_ವೃಕ್ಷ_View_Trigger(
  ವೃಕ್ಷ_View *pView,
  ಸ್ಥಿರಾಂಕ Trigger *pTrigger,
  u8 ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow,
  u8 show_ತುಂಬಿದ_List
){
  ಪೂರ್ಣಾಂಕ cnt = 0;
  ಆದರೆ( pTrigger==0 ) ಹಿಂತಿರುಗಿ;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView,
     ಇನ್ನಷ್ಟು_ಗಮ್ಯ_Follow || (show_ತುಂಬಿದ_List && pTrigger->p_ಮುಂದಿನ!=0));
  ಮಾಡು{
    ಆದರೆ( cnt++ && pTrigger->p_ಮುಂದಿನ==0 ){
      sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
      sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
    }
    sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "TRIGGER %s", pTrigger->zName);
    sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
    sqlite3_ವೃಕ್ಷ_View_Trigger_ಹೆಜ್ಜೆ(pView, pTrigger->ಹೆಜ್ಜೆ_list, 0, 1);
    sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
  }ಆಗ( show_ತುಂಬಿದ_List && (pTrigger = pTrigger->p_ಮುಂದಿನ)!=0 );
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_OMIT_TRIGGER */
  

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
ಶೂನ್ಯ sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ *p){ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ(0,p,0); }
ಶೂನ್ಯ sqlite3_Show_ಅಭಿವ್ಯಕ್ತಿ_List(ಸ್ಥಿರಾಂಕ ಅಭಿವ್ಯಕ್ತಿ_List *p){ sqlite3_ವೃಕ್ಷ_View_ಅಭಿವ್ಯಕ್ತಿ_List(0,p,0,0);}
ಶೂನ್ಯ sqlite3ShowIdList(ಸ್ಥಿರಾಂಕ IdList *p){ sqlite3_ವೃಕ್ಷ_View_Id_List(0,p,0,0); }
ಶೂನ್ಯ sqlite3ShowSrcList(ಸ್ಥಿರಾಂಕ SrcList *p){
  ವೃಕ್ಷ_View *pView = 0;
  sqlite3_ವೃಕ್ಷ_View_ತಳ್ಳು(&pView, 0);
  sqlite3_ವೃಕ್ಷ_View_ಸಾಲು(pView, "SRCLIST");
  sqlite3_ವೃಕ್ಷ_View_Src_List(pView,p);
  sqlite3_ವೃಕ್ಷ_View_ತೆಗೆ(&pView);
}
ಶೂನ್ಯ sqlite3ShowSelect(ಸ್ಥಿರಾಂಕ Select *p){ sqlite3_ವೃಕ್ಷ_View_Select(0,p,0); }
ಶೂನ್ಯ sqlite3ShowWith(ಸ್ಥಿರಾಂಕ With *p){ sqlite3_ವೃಕ್ಷ_View_With(0,p,0); }
ಶೂನ್ಯ sqlite3ShowUpsert(ಸ್ಥಿರಾಂಕ Upsert *p){ sqlite3_ವೃಕ್ಷ_View_Upsert(0,p,0); }
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_TRIGGER
ಶೂನ್ಯ sqlite3_Show_Trigger_ಹೆಜ್ಜೆ(ಸ್ಥಿರಾಂಕ Trigger_ಹೆಜ್ಜೆ *p){
  sqlite3_ವೃಕ್ಷ_View_Trigger_ಹೆಜ್ಜೆ(0,p,0,0);
}
ಶೂನ್ಯ sqlite3_Show_Trigger_ಹೆಜ್ಜೆ_List(ಸ್ಥಿರಾಂಕ Trigger_ಹೆಜ್ಜೆ *p){
  sqlite3_ವೃಕ್ಷ_View_Trigger_ಹೆಜ್ಜೆ(0,p,0,1);
}
ಶೂನ್ಯ sqlite3ShowTrigger(ಸ್ಥಿರಾಂಕ Trigger *p){ sqlite3_ವೃಕ್ಷ_View_Trigger(0,p,0,0); }
ಶೂನ್ಯ sqlite3ShowTriggerList(ಸ್ಥಿರಾಂಕ Trigger *p){ sqlite3_ವೃಕ್ಷ_View_Trigger(0,p,0,1);}
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ SQLITE_OMIT_WINDOWFUNC
ಶೂನ್ಯ sqlite3_Show_ಕಿಟಕಿ(ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ *p){ sqlite3_ವೃಕ್ಷ_View_ಕಿಟಕಿ(0,p,0); }
ಶೂನ್ಯ sqlite3ShowWinFunc(ಸ್ಥಿರಾಂಕ ಕಿಟಕಿ *p){ sqlite3_ವೃಕ್ಷ_View_Win_Func(0,p,0); }
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ_ಅಂತ್ಯ /* SQLITE_DEBUG */
