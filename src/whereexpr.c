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
** This module contains C code that generates VDBE code used to process
** the WHERE clause of SQL statements.
**
** This file was originally part of where.c but was split out to improve
** readability and editability.  This file contains utility routines for
** analyzing Expr objects in the WHERE clause.
*/
#ضمن "sqliteInt.h"
#ضمن "whereInt.h"

/* Forward declarations */
ساكن فراغ exprAnalyze(SrcList*, WhereClause*, صحيح);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
ساكن فراغ whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
ساكن فراغ whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Add a single new WhereTerm entry to the WhereClause object pWC.
** The new WhereTerm object is constructed from Expr p and with wtFlags.
** The index in pWC->a[] of the new WhereTerm is returned on success.
** 0 is returned if the new WhereTerm could not be added due to a memory
** allocation error.  The memory allocation failure will be recorded in
** the db->mallocFailed flag so that higher-level functions can detect it.
**
** This routine will increase the size of the pWC->a[] array as necessary.
**
** If the wtFlags argument includes TERM_DYNAMIC, then responsibility
** for freeing the expression p is assumed by the WhereClause object pWC.
** This is true even if this routine fails to allocate a new WhereTerm.
**
** WARNING:  This routine might reallocate the space used to store
** WhereTerms.  All pointers to WhereTerms should be invalidated after
** calling this routine.  Such pointers may be reinitialized by referencing
** the pWC->a[] array.
*/
ساكن صحيح whereClauseInsert(WhereClause *pWC, Expr *p, u16 wtFlags){
  WhereTerm *pTerm;
  صحيح idx;
  testcase( wtFlags & TERM_VIRTUAL );
  ܐܢ( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, حجم(pWC->a[0])*pWC->nSlot*2 );
    ܐܢ( pWC->a==0 ){
      ܐܢ( wtFlags & TERM_DYNAMIC ){
        sqlite3ExprDelete(db, p);
      }
      pWC->a = pOld;
      ܦܢܐ 0;
    }
    memcpy(pWC->a, pOld, حجم(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  ܐܢ( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  ܐܢ( p && ExprHasProperty(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }ܐܚܪܢܐ{
    pTerm->truthProb = 1;
  }
  pTerm->pExpr = sqlite3ExprSkipCollateAndLikely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         حجم(WhereTerm) - offsetof(WhereTerm,eOperator));
  ܦܢܐ idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
ساكن صحيح allowedOp(صحيح op){
  أكد( TK_GT>TK_EQ && TK_GT<TK_GE );
  أكد( TK_LT>TK_EQ && TK_LT<TK_GE );
  أكد( TK_LE>TK_EQ && TK_LE<TK_GE );
  أكد( TK_GE==TK_EQ+4 );
  أكد( TK_IN<TK_EQ );
  أكد( TK_IS<TK_EQ );
  أكد( TK_ISNULL<TK_EQ );
  ܐܢ( op>TK_GE ) ܦܢܐ 0;
  ܐܢ( op>=TK_EQ ) ܦܢܐ 1;
  ܦܢܐ op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ساكن u16 exprCommute(Parse *pParse, Expr *pExpr){
  ܐܢ( pExpr->pLeft->op==TK_VECTOR
   || pExpr->pRight->op==TK_VECTOR
   || sqlite3BinaryCompareCollSeq(pParse, pExpr->pLeft, pExpr->pRight) !=
      sqlite3BinaryCompareCollSeq(pParse, pExpr->pRight, pExpr->pLeft)
  ){
    pExpr->flags ^= EP_Commuted;
  }
  SWAP(Expr*,pExpr->pRight,pExpr->pLeft);
  ܐܢ( pExpr->op>=TK_GT ){
    أكد( TK_LT==TK_GT+2 );
    أكد( TK_GE==TK_LE+2 );
    أكد( TK_GT>TK_EQ );
    أكد( TK_GT<TK_LE );
    أكد( pExpr->op>=TK_GT && pExpr->op<=TK_GE );
    pExpr->op = ((pExpr->op-TK_GT)^2)+TK_GT;
  }
  ܦܢܐ 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ساكن u16 operatorMask(صحيح op){
  u16 c;
  أكد( allowedOp(op) );
  ܐܢ( op>=TK_EQ ){
    أكد( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }ܐܚܪܢܐ ܐܢ( op==TK_IN ){
    c = WO_IN;
  }ܐܚܪܢܐ ܐܢ( op==TK_ISNULL ){
    c = WO_ISNULL;
  }ܐܚܪܢܐ{
    أكد( op==TK_IS );
    c = WO_IS;
  }
  أكد( op!=TK_ISNULL || c==WO_ISNULL );
  أكد( op!=TK_IN || c==WO_IN );
  أكد( op!=TK_EQ || c==WO_EQ );
  أكد( op!=TK_LT || c==WO_LT );
  أكد( op!=TK_LE || c==WO_LE );
  أكد( op!=TK_GT || c==WO_GT );
  أكد( op!=TK_GE || c==WO_GE );
  أكد( op!=TK_IS || c==WO_IS );
  ܦܢܐ c;
}


#ifndef SQLITE_OMIT_LIKE_OPTIMIZATION
/*
** Check to see if the given expression is a LIKE or GLOB operator that
** can be optimized using inequality constraints.  Return TRUE if it is
** so and false if not.
**
** In order for the operator to be optimizible, the RHS must be a string
** literal that does not begin with a wildcard.  The LHS must be a column
** that may only be NULL, a string, or a BLOB, never a number. (This means
** that virtual tables cannot participate in the LIKE optimization.)  The
** collating sequence for the column on the LHS must be appropriate for
** the operator.
*/
ساكن صحيح isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  Expr *pExpr,      /* Test this expression */
  Expr **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  صحيح *pisComplete, /* True if the only wildcard is % in the last character */
  صحيح *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ثابت u8 *z = 0;           /* String on RHS of LIKE operator */
  Expr *pRight, *pLeft;      /* Right and left size of LIKE operator */
  ExprList *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  صحيح cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_value *pVal = 0;
  صحيح op;                    /* Opcode of pRight */
  صحيح rc;                    /* Result code to return */

  ܐܢ( !sqlite3IsLikeFunction(db, pExpr, pnoCase, (حرف*)wc) ){
    ܦܢܐ 0;
  }
#ifdef SQLITE_EBCDIC
  ܐܢ( *pnoCase ) ܦܢܐ 0;
#endif
  أكد( ExprUseXList(pExpr) );
  pList = pExpr->x.pList;
  pLeft = pList->a[1].pExpr;

  pRight = sqlite3ExprSkipCollate(pList->a[0].pExpr);
  op = pRight->op;
  ܐܢ( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    صحيح iCol = pRight->iColumn;
    pVal = sqlite3VdbeGetBoundValue(pReprepare, iCol, SQLITE_AFF_BLOB);
    ܐܢ( pVal && sqlite3_value_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_value_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    أكد( pRight->op==TK_VARIABLE || pRight->op==TK_REGISTER );
  }ܐܚܪܢܐ ܐܢ( op==TK_STRING ){
    أكد( !ExprHasProperty(pRight, EP_IntValue) );
     z = (u8*)pRight->u.zToken;
  }
  ܐܢ( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    ܟܡܐ( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      ܐܢ( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }ܐܚܪܢܐ ܐܢ( c>=0x80 ){
        ثابت u8 *z2 = z+cnt-1;
        ܐܢ( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          اكسر;
        }ܐܚܪܢܐ{
          cnt = (صحيح)(z2-z);
        }
      }
    }

    /* The optimization is possible only if (1) the pattern does not begin
    ** with a wildcard and if (2) the non-wildcard prefix does not end with
    ** an (illegal 0xff) character, or (3) the pattern does not consist of
    ** a single escape character. The second condition is necessary so
    ** that we can increment the prefix key to find an upper bound for the
    ** range search. The third is because the caller assumes that the pattern
    ** consists of at least one character after all escapes have been
    ** removed.  */
    ܐܢ( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      Expr *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3Expr(db, TK_STRING, (حرف*)z);
      ܐܢ( pPrefix ){
        صحيح iFrom, iTo;
        حرف *zNew;
        أكد( !ExprHasProperty(pPrefix, EP_IntValue) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        لكل(iFrom=iTo=0; iFrom<cnt; iFrom++){
          ܐܢ( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[iTo++] = zNew[iFrom];
        }
        zNew[iTo] = 0;
        أكد( iTo>0 );

        /* If the LHS is not an ordinary column with TEXT affinity, then the
        ** pattern prefix boundaries (both the start and end boundaries) must
        ** not look like a number.  Otherwise the pattern might be treated as
        ** a number, which will invalidate the LIKE optimization.
        **
        ** Getting this right has been a persistent source of bugs in the
        ** LIKE optimization.  See, for example:
        **    2018-09-10 https://sqlite.org/src/info/c94369cae9b561b1
        **    2019-05-02 https://sqlite.org/src/info/b043a54c3de54b28
        **    2019-06-10 https://sqlite.org/src/info/fd76310a5e843e07
        **    2019-06-14 https://sqlite.org/src/info/ce8717f0885af975
        **    2019-09-03 https://sqlite.org/src/info/0f0428096f17252a
        */
        ܐܢ( pLeft->op!=TK_COLUMN
         || sqlite3ExprAffinity(pLeft)!=SQLITE_AFF_TEXT
         || (ALWAYS( ExprUseYTab(pLeft) )
             && ALWAYS(pLeft->y.pTab)
             && IsVirtual(pLeft->y.pTab))  /* Might be numeric */
        ){
          صحيح isNum;
          مزدوج rDummy;
          أكد( zNew[iTo]==0 );
          isNum = sqlite3AtoF(zNew, &rDummy);
          ܐܢ( isNum<=0 ){
            ܐܢ( iTo==1 && zNew[0]=='-' ){
              isNum = +1;
            }ܐܚܪܢܐ{
              zNew[iTo-1]++;
              isNum = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          ܐܢ( isNum>0 ){
            sqlite3ExprDelete(db, pPrefix);
            sqlite3ValueFree(pVal);
            ܦܢܐ 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      ܐܢ( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, pRight->iColumn, 0);
        أكد( !ExprHasProperty(pRight, EP_IntValue) );
        ܐܢ( *pisComplete && pRight->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          صحيح r1 = sqlite3GetTempReg(pParse);
          sqlite3ExprCodeTarget(pParse, pRight, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }ܐܚܪܢܐ{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3ValueFree(pVal);
  ܦܢܐ rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
صحيح sqlite3ExprIsLikeOperator(ثابت Expr *pExpr){
  ساكن ثابت بنية {
    ثابت حرف *zOp;
    unsigned حرف eOp;
  } aOp[] = {
    { "match",  SQLITE_INDEX_CONSTRAINT_MATCH },
    { "glob",   SQLITE_INDEX_CONSTRAINT_GLOB },
    { "like",   SQLITE_INDEX_CONSTRAINT_LIKE },
    { "regexp", SQLITE_INDEX_CONSTRAINT_REGEXP }
  };
  صحيح i;
  أكد( pExpr->op==TK_FUNCTION );
  أكد( !ExprHasProperty(pExpr, EP_IntValue) );
  لكل(i=0; i<ArraySize(aOp); i++){
    ܐܢ( sqlite3StrICmp(pExpr->u.zToken, aOp[i].zOp)==0 ){
      ܦܢܐ aOp[i].eOp;
    }
  }
  ܦܢܐ 0;
}


#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Check to see if the pExpr expression is a form that needs to be passed
** to the xBestIndex method of virtual tables.  Forms of interest include:
**
**          Expression                   Virtual Table Operator
**          -----------------------      ---------------------------------
**      1.  column MATCH expr            SQLITE_INDEX_CONSTRAINT_MATCH
**      2.  column GLOB expr             SQLITE_INDEX_CONSTRAINT_GLOB
**      3.  column LIKE expr             SQLITE_INDEX_CONSTRAINT_LIKE
**      4.  column REGEXP expr           SQLITE_INDEX_CONSTRAINT_REGEXP
**      5.  column != expr               SQLITE_INDEX_CONSTRAINT_NE
**      6.  expr != column               SQLITE_INDEX_CONSTRAINT_NE
**      7.  column IS NOT expr           SQLITE_INDEX_CONSTRAINT_ISNOT
**      8.  expr IS NOT column           SQLITE_INDEX_CONSTRAINT_ISNOT
**      9.  column IS NOT NULL           SQLITE_INDEX_CONSTRAINT_ISNOTNULL
**
** In every case, "column" must be a column of a virtual table.  If there
** is a match, set *ppLeft to the "column" expression, set *ppRight to the
** "expr" expression (even though in forms (6) and (8) the column is on the
** right and the expression is on the left).  Also set *peOp2 to the
** appropriate virtual table operator.  The return value is 1 or 2 if there
** is a match.  The usual return is 1, but if the RHS is also a column
** of virtual table in forms (5) or (7) then return 2.
**
** If the expression matches none of the patterns above, return 0.
*/
ساكن صحيح isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  Expr *pExpr,                    /* Test this expression */
  unsigned حرف *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  Expr **ppLeft,                  /* Column expression to left of MATCH/op2 */
  Expr **ppRight                  /* Expression to left of MATCH/op2 */
){
  ܐܢ( pExpr->op==TK_FUNCTION ){
    ExprList *pList;
    Expr *pCol;                     /* Column reference */
    صحيح i;

    أكد( ExprUseXList(pExpr) );
    pList = pExpr->x.pList;
    ܐܢ( pList==0 || pList->nExpr!=2 ){
      ܦܢܐ 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].pExpr;
    أكد( pCol->op!=TK_COLUMN || (ExprUseYTab(pCol) && pCol->y.pTab!=0) );
    ܐܢ( ExprIsVtab(pCol) && (i = sqlite3ExprIsLikeOperator(pExpr))!=0 ){
      *peOp2 = i;
      *ppRight = pList->a[0].pExpr;
      *ppLeft = pCol;
      ܦܢܐ 1;
    }

    /* We can also match against the first column of overloaded
    ** functions where xFindFunction returns a value of at least
    ** SQLITE_INDEX_CONSTRAINT_FUNCTION.
    **
    **      OVERLOADED(vtab_column,expression)
    **
    ** Historically, xFindFunction expected to see lower-case function
    ** names.  But for this use case, xFindFunction is expected to deal
    ** with function names in an arbitrary case.
    */
    pCol = pList->a[0].pExpr;
    أكد( pCol->op!=TK_COLUMN || ExprUseYTab(pCol) );
    أكد( pCol->op!=TK_COLUMN || (ExprUseYTab(pCol) && pCol->y.pTab!=0) );
    ܐܢ( ExprIsVtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      فراغ (*xNotUsed)(sqlite3_context*,صحيح,sqlite3_value**);
      فراغ *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      أكد( pVtab!=0 );
      أكد( pVtab->pModule!=0 );
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      pMod = (sqlite3_module *)pVtab->pModule;
      ܐܢ( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, pExpr->u.zToken, &xNotUsed, &pNotUsed);
        ܐܢ( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *ppRight = pList->a[1].pExpr;
          *ppLeft = pCol;
          ܦܢܐ 1;
        }
      }
    }
  }ܐܚܪܢܐ ܐܢ( pExpr->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    أكد( TK_NE < TK_EQ );
    أكد( TK_ISNOT < TK_EQ );
    أكد( TK_NOTNULL < TK_EQ );
    ܦܢܐ 0;
  }ܐܚܪܢܐ ܐܢ( pExpr->op==TK_NE || pExpr->op==TK_ISNOT || pExpr->op==TK_NOTNULL ){
    صحيح res = 0;
    Expr *pLeft = pExpr->pLeft;
    Expr *pRight = pExpr->pRight;
    أكد( pLeft->op!=TK_COLUMN || (ExprUseYTab(pLeft) && pLeft->y.pTab!=0) );
    ܐܢ( ExprIsVtab(pLeft) ){
      res++;
    }
    أكد( pRight==0 || pRight->op!=TK_COLUMN
            || (ExprUseYTab(pRight) && pRight->y.pTab!=0) );
    ܐܢ( pRight && ExprIsVtab(pRight) ){
      res++;
      SWAP(Expr*, pLeft, pRight);
    }
    *ppLeft = pLeft;
    *ppRight = pRight;
    ܐܢ( pExpr->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    ܐܢ( pExpr->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    ܐܢ( pExpr->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    ܦܢܐ res;
  }
  ܦܢܐ 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ساكن فراغ transferJoinMarkings(Expr *pDerived, Expr *pBase){
  ܐܢ( pDerived && ExprHasProperty(pBase, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= pBase->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ساكن فراغ markTermAsChild(WhereClause *pWC, صحيح iChild, صحيح iParent){
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;
  أكد(   pWC->a[iParent].nChild <  UMXV(pWC->a[0].nChild) );
  pWC->a[iParent].nChild++;
  testcase( pWC->a[iParent].nChild == UMXV(pWC->a[0].nChild) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
ساكن WhereTerm *whereNthSubterm(WhereTerm *pTerm, صحيح N){
  ܐܢ( pTerm->eOperator!=WO_AND ){
    ܦܢܐ N==0 ? pTerm : 0;
  }
  ܐܢ( N<pTerm->u.pAndInfo->wc.nTerm ){
    ܦܢܐ &pTerm->u.pAndInfo->wc.a[N];
  }
  ܦܢܐ 0;
}

/*
** Subterms pOne and pTwo are contained within WHERE clause pWC.  The
** two subterms are in disjunction - they are OR-ed together.
**
** If these two terms are both of the form:  "A op B" with the same
** A and B values but different operators and if the operators are
** compatible (if one is = and the other is <, for example) then
** add a new virtual AND term to pWC that is the combination of the
** two.
**
** Some examples:
**
**    x<y OR x=y    -->     x<=y
**    x=y OR x=y    -->     x=y
**    x<=y OR x<y   -->     x<=y
**
** The following is NOT generated:
**
**    x<y OR x>y    -->     x!=y    
*/
ساكن فراغ whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  Expr *pNew;            /* New virtual expression */
  صحيح op;                /* Operator for the combined expression */
  صحيح idxNew;            /* Index in pWC of the next virtual term */
  Expr *pA, *pB;         /* Expressions associated with pOne and pTwo */

  ܐܢ( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) ܦܢܐ;
  ܐܢ( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ܦܢܐ;
  ܐܢ( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) ܦܢܐ;
  ܐܢ( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) ܦܢܐ;
  pA = pOne->pExpr;
  pB = pTwo->pExpr;
  أكد( pA->pLeft!=0 && pA->pRight!=0 );
  أكد( pB->pLeft!=0 && pB->pRight!=0 );
  ܐܢ( sqlite3ExprCompare(0,pA->pLeft, pB->pLeft, -1) )  ܦܢܐ;
  ܐܢ( sqlite3ExprCompare(0,pA->pRight, pB->pRight,-1) ) ܦܢܐ;
  ܐܢ( ExprHasProperty(pA,EP_Commuted)!=ExprHasProperty(pB,EP_Commuted) ){
    ܦܢܐ;
  }
  /* If we reach this point, it means the two subterms can be combined */
  ܐܢ( (eOp & (eOp-1))!=0 ){
    ܐܢ( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }ܐܚܪܢܐ{
      أكد( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3ExprDup(db, pA, 0);
  ܐܢ( pNew==0 ) ܦܢܐ;
  لكل(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ أكد( op<TK_GE ); }
  pNew->op = op;
  idxNew = whereClauseInsert(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  exprAnalyze(pSrc, pWC, idxNew);
}

#ܐܢ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Analyze a term that consists of two or more OR-connected
** subterms.  So in:
**
**     ... WHERE  (a=5) AND (b=7 OR c=9 OR d=13) AND (d=13)
**                          ^^^^^^^^^^^^^^^^^^^^
**
** This routine analyzes terms such as the middle term in the above example.
** A WhereOrTerm object is computed and attached to the term under
** analysis, regardless of the outcome of the analysis.  Hence:
**
**     WhereTerm.wtFlags   |=  TERM_ORINFO
**     WhereTerm.u.pOrInfo  =  a dynamically allocated WhereOrTerm object
**
** The term being analyzed must have two or more of OR-connected subterms.
** A single subterm might be a set of AND-connected sub-subterms.
** Examples of terms under analysis:
**
**     (A)     t1.x=t2.y OR t1.x=t2.z OR t1.y=15 OR t1.z=t3.a+5
**     (B)     x=expr1 OR expr2=x OR x=expr3
**     (C)     t1.x=t2.y OR (t1.x=t2.z AND t1.y=15)
**     (D)     x=expr1 OR (y>11 AND y<22 AND z LIKE '*hello*')
**     (E)     (p.a=1 AND q.b=2 AND r.c=3) OR (p.x=4 AND q.y=5 AND r.z=6)
**     (F)     x>A OR (x=A AND y>=B)
**
** CASE 1:
**
** If all subterms are of the form T.C=expr for some single column of C and
** a single table T (as shown in example B above) then create a new virtual
** term that is an equivalent IN expression.  In other words, if the term
** being analyzed is:
**
**      x = expr1  OR  expr2 = x  OR  x = expr3
**
** then create a new virtual term like this:
**
**      x IN (expr1,expr2,expr3)
**
** CASE 2:
**
** If there are exactly two disjuncts and one side has x>A and the other side
** has x=A (for the same x and A) then add a new virtual conjunct term to the
** WHERE clause of the form "x>=A".  Example:
**
**      x>A OR (x=A AND y>B)    adds:    x>=A
**
** The added conjunct can sometimes be helpful in query planning.
**
** CASE 3:
**
** If all subterms are indexable by a single table T, then set
**
**     WhereTerm.eOperator              =  WO_OR
**     WhereTerm.u.pOrInfo->indexable  |=  the cursor number for table T
**
** A subterm is "indexable" if it is of the form
** "T.C <op> <expr>" where C is any column of table T and
** <op> is one of "=", "<", "<=", ">", ">=", "IS NULL", or "IN".
** A subterm is also indexable if it is an AND of two or more
** subsubterms at least one of which is indexable.  Indexable AND
** subterms have their eOperator set to WO_AND and they have
** u.pAndInfo set to a dynamically allocated WhereAndTerm object.
**
** From another point of view, "indexable" means that the subterm could
** potentially be used with an index if an appropriate index exists.
** This analysis does not consider whether or not the index exists; that
** is decided elsewhere.  This analysis only looks at whether subterms
** appropriate for indexing exist.
**
** All examples A through E above satisfy case 3.  But if a term
** also satisfies case 1 (such as B) we know that the optimizer will
** always prefer case 1, so in that case we pretend that case 3 is not
** satisfied.
**
** It might be the case that multiple tables are indexable.  For example,
** (E) above is indexable on tables P, Q, and R.
**
** Terms that satisfy case 3 are candidates for lookup by using
** separate indices to find rowids for each subterm and composing
** the union of all rowids using a RowSet object.  This is similar
** to "bitmap indices" in other database engines.
**
** OTHERWISE:
**
** If none of cases 1, 2, or 3 apply, then leave the eOperator set to
** zero.  This term is not useful for search.
*/
ساكن فراغ exprAnalyzeOrTerm(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  صحيح idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  Expr *pExpr = pTerm->pExpr;             /* The expression of the term */
  صحيح i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  WhereTerm *pOrTerm;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chngToIN;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  أكد( (pTerm->wtFlags & (TERM_DYNAMIC|TERM_ORINFO|TERM_ANDINFO))==0 );
  أكد( pExpr->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, حجم(*pOrInfo));
  ܐܢ( pOrInfo==0 ) ܦܢܐ;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, حجم(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, pExpr, TK_OR);
  sqlite3WhereExprAnalyze(pSrc, pOrWc);
  ܐܢ( db->mallocFailed ) ܦܢܐ;
  أكد( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  لكل(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    ܐܢ( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      أكد( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, حجم(*pAndInfo));
      ܐܢ( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        صحيح j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->leftCursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, حجم(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->pExpr, TK_AND);
        sqlite3WhereExprAnalyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        ܐܢ( !db->mallocFailed ){
          لكل(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            أكد( pAndTerm->pExpr );
            ܐܢ( allowedOp(pAndTerm->pExpr->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->leftCursor);
            }
          }
        }
        indexable &= b;
      }
    }ܐܚܪܢܐ ܐܢ( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }ܐܚܪܢܐ{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->leftCursor);
      ܐܢ( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->leftCursor);
      }
      indexable &= b;
      ܐܢ( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }ܐܚܪܢܐ{
        chngToIN &= b;
      }
    }
  }

  /*
  ** Record the set of tables that satisfy case 3.  The set might be
  ** empty.
  */
  pOrInfo->indexable = indexable;
  pTerm->eOperator = WO_OR;
  pTerm->leftCursor = -1;
  ܐܢ( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  ܐܢ( indexable && pOrWc->nTerm==2 ){
    صحيح iOne = 0;
    WhereTerm *pOne;
    ܟܡܐ( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      صحيح iTwo = 0;
      WhereTerm *pTwo;
      ܟܡܐ( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
        whereCombineDisjuncts(pSrc, pWC, pOne, pTwo);
      }
    }
  }

  /*
  ** chngToIN holds a set of tables that *might* satisfy case 1.  But
  ** we have to do some additional checking to see if case 1 really
  ** is satisfied.
  **
  ** chngToIN will hold either 0, 1, or 2 bits.  The 0-bit case means
  ** that there is no possibility of transforming the OR clause into an
  ** IN operator because one or more terms in the OR clause contain
  ** something other than == on a column in the single table.  The 1-bit
  ** case means that every term of the OR clause is of the form
  ** "table.column=expr" for some single table.  The one bit that is set
  ** will correspond to the common table.  We still need to check to make
  ** sure the same column is used on all terms.  The 2-bit case is when
  ** the all terms are of the form "table1.column=table2.column".  It
  ** might be possible to form an IN operator with either table1.column
  ** or table2.column as the LHS if either is common to every term of
  ** the OR clause.
  **
  ** Note that terms of the form "table.column1=table.column2" (the
  ** same table on both sizes of the ==) cannot be optimized.
  */
  ܐܢ( chngToIN ){
    صحيح okToChngToIN = 0;     /* True if the conversion to IN is valid */
    صحيح iColumn = -1;         /* Column index on lhs of IN operator */
    صحيح iCursor = -1;         /* Table cursor common to all terms */
    صحيح j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    لكل(j=0; j<2 && !okToChngToIN; j++){
      Expr *pLeft = 0;
      pOrTerm = pOrWc->a;
      لكل(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        أكد( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        ܐܢ( pOrTerm->leftCursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          أكد( j==1 );
          استمر;
        }
        ܐܢ( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->leftCursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          أكد( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          استمر;
        }
        أكد( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        iColumn = pOrTerm->u.x.leftColumn;
        iCursor = pOrTerm->leftCursor;
        pLeft = pOrTerm->pExpr->pLeft;
        اكسر;
      }
      ܐܢ( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        أكد( j==1 );
        أكد( IsPowerOfTwo(chngToIN) );
        أكد( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        اكسر;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      لكل(; i>=0 && okToChngToIN; i--, pOrTerm++){
        أكد( pOrTerm->eOperator & WO_EQ );
        أكد( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        ܐܢ( pOrTerm->leftCursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }ܐܚܪܢܐ ܐܢ( pOrTerm->u.x.leftColumn!=iColumn || (iColumn==XN_EXPR
               && sqlite3ExprCompare(pParse, pOrTerm->pExpr->pLeft, pLeft, -1)
        )){
          okToChngToIN = 0;
        }ܐܚܪܢܐ{
          صحيح affLeft, affRight;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          affRight = sqlite3ExprAffinity(pOrTerm->pExpr->pRight);
          affLeft = sqlite3ExprAffinity(pOrTerm->pExpr->pLeft);
          ܐܢ( affRight!=0 && affRight!=affLeft ){
            okToChngToIN = 0;
          }ܐܚܪܢܐ{
            pOrTerm->wtFlags |= TERM_OK;
          }
        }
      }
    }

    /* At this point, okToChngToIN is true if original pTerm is a
    ** candidate to satisfy case 1, though we are not yet certain that
    ** the collating sequences are all compatible.  Try to construct a
    ** new virtual term that is pTerm converted from an OR operator
    ** into an IN operator.
    **
    ** During construction, verify that the collating sequences on all
    ** subterms of the OR are compatible.  Omit the construction of the
    ** new IN operator if there are any collating sequence mismatches.
    */
    ܐܢ( okToChngToIN ){
      Expr *pDup;            /* A transient duplicate expression */
      ExprList *pList = 0;   /* The RHS of the IN operator */
      Expr *pLeft = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      Expr *pNew;            /* The complete IN operator */

      لكل(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        Expr *pThis;
        ܐܢ( (pOrTerm->wtFlags & TERM_OK)==0 ) استمر;
        أكد( pOrTerm->eOperator & WO_EQ );
        أكد( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        أكد( pOrTerm->leftCursor==iCursor );
        أكد( pOrTerm->u.x.leftColumn==iColumn );
        pThis = pOrTerm->pExpr;
        pDup = sqlite3ExprDup(db, pThis->pRight, 0);
        pList = sqlite3ExprListAppend(pWInfo->pParse, pList, pDup);
        ܐܢ( pLeft==0 ){
          pLeft = pThis->pLeft;
          pCollSeq = sqlite3ExprCompareCollSeq(pParse, pThis);
        }ܐܚܪܢܐ{
          أكد( 0==sqlite3ExprCompare(pParse, 
                        sqlite3ExprSkipCollate(pThis->pLeft),
                        sqlite3ExprSkipCollate(pLeft), -1) );
          ܐܢ( pCollSeq!=sqlite3ExprCompareCollSeq(pParse, pThis) ){
            pLeft = 0;  /* Collating sequence mismatch */
            اكسر;
          }
        }
      }
      ܐܢ( pLeft==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }ܐܚܪܢܐ{
        pDup = sqlite3ExprDup(db, pLeft, 0);
        ܐܢ( sqlite3ExprCollSeq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          أكد( pCollSeq->zName!=0 );
          pDup = sqlite3ExprAddCollateString(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      ܐܢ( pNew ){
        صحيح idxNew;
        transferJoinMarkings(pNew, pExpr);
        أكد( ExprUseXList(pNew) );
        pNew->x.pList = pList;
        idxNew = whereClauseInsert(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        exprAnalyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }ܐܚܪܢܐ{
        sqlite3ExprListDelete(db, pList);
      }
    }
  }
}
#endif /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

/*
** We already know that pExpr is a binary operator where both operands are
** column references.  This routine checks to see if pExpr is an equivalence
** relation:
**   1.  The SQLITE_Transitive optimization must be enabled
**   2.  Must be either an == or an IS operator
**   3.  Not originating in the ON clause of an OUTER JOIN
**   4.  The operator is not IS or else the query does not contain RIGHT JOIN
**   5.  The affinities of A and B must be compatible
**   6.  Both operands use the same collating sequence, and they must not
**       use explicit COLLATE clauses.
** If this routine returns TRUE, that means that the RHS can be substituted
** for the LHS anyplace else in the WHERE clause where the LHS column occurs.
** This is an optimization.  No harm comes from returning 0.  But if 1 is
** returned when it should not be, then incorrect answers might result.
*/
ساكن صحيح termIsEquivalence(Parse *pParse, Expr *pExpr, SrcList *pSrc){
  حرف aff1, aff2;
  ܐܢ( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) ܦܢܐ 0;  /* (1) */
  ܐܢ( pExpr->op!=TK_EQ && pExpr->op!=TK_IS ) ܦܢܐ 0;                 /* (2) */
  ܐܢ( ExprHasProperty(pExpr, EP_OuterON|EP_Collate) ) ܦܢܐ 0;        /* (3) */
  أكد( pSrc!=0 );
  ܐܢ( pExpr->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    ܦܢܐ 0;                                                          /* (4) */
  }
  aff1 = sqlite3ExprAffinity(pExpr->pLeft);
  aff2 = sqlite3ExprAffinity(pExpr->pRight);
  ܐܢ( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    ܦܢܐ 0;                                                          /* (5) */
  }
  ܐܢ( !sqlite3ExprCollSeqMatch(pParse, pExpr->pLeft, pExpr->pRight) ){
    ܦܢܐ 0;                                                          /* (6) */
  }
  ܦܢܐ 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ساكن Bitmask exprSelectUsage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  ܟܡܐ( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pEList);
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3WhereExprUsage(pMaskSet, pS->pWhere);
    mask |= sqlite3WhereExprUsage(pMaskSet, pS->pHaving);
    ܐܢ( ALWAYS(pSrc!=0) ){
      صحيح i;
      لكل(i=0; i<pSrc->nSrc; i++){
        ܐܢ( pSrc->a[i].fg.isSubquery ){
          mask |= exprSelectUsage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        ܐܢ( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3WhereExprUsage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        ܐܢ( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3WhereExprListUsage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  ܦܢܐ mask;
}

/*
** Expression pExpr is one operand of a comparison operator that might
** be useful for indexing.  This routine checks to see if pExpr appears
** in any index.  Return TRUE (1) if pExpr is an indexed term and return
** FALSE (0) if not.  If TRUE is returned, also set aiCurCol[0] to the cursor
** number of the table that is indexed and aiCurCol[1] to the column number
** of the column that is indexed, or XN_EXPR (-2) if an expression is being
** indexed.
**
** If pExpr is a TK_COLUMN column reference, then this routine always returns
** true even if that particular column is not indexed, because the column
** might be added to an automatic index later.
*/
ساكن SQLITE_NOINLINE صحيح exprMightBeIndexed2(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *aiCurCol,         /* Write the referenced table cursor and column here */
  Expr *pExpr,           /* An operand of a comparison operator */
  صحيح j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  صحيح i;
  صحيح iCur;
  ܥܒܕ{
    iCur = pFrom->a[j].iCursor;
    لكل(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ܐܢ( pIdx->aColExpr==0 ) استمر;
      لكل(i=0; i<pIdx->nKeyCol; i++){
        ܐܢ( pIdx->aiColumn[i]!=XN_EXPR ) استمر;
        أكد( pIdx->bHasExpr );
        ܐܢ( sqlite3ExprCompareSkip(pExpr,pIdx->aColExpr->a[i].pExpr,iCur)==0
         && !sqlite3ExprIsConstant(0,pIdx->aColExpr->a[i].pExpr)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_EXPR;
          ܦܢܐ 1;
        }
      }
    }
  }ܟܡܐ( ++j < pFrom->nSrc );
  ܦܢܐ 0;
}
ساكن صحيح exprMightBeIndexed(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *aiCurCol,         /* Write the referenced table cursor & column here */
  Expr *pExpr,           /* An operand of a comparison operator */
  صحيح op                 /* The specific comparison operator */
){
  صحيح i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  أكد( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  أكد( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  أكد( op<=TK_GE );
  ܐܢ( pExpr->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    أكد( ExprUseXList(pExpr) );
    pExpr = pExpr->x.pList->a[0].pExpr;
  }

  ܐܢ( pExpr->op==TK_COLUMN ){
    aiCurCol[0] = pExpr->iTable;
    aiCurCol[1] = pExpr->iColumn;
    ܦܢܐ 1;
  }

  لكل(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    لكل(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      ܐܢ( pIdx->aColExpr ){
        ܦܢܐ exprMightBeIndexed2(pFrom,aiCurCol,pExpr,i);
      }
    }
  }
  ܦܢܐ 0;
}


/*
** The input to this routine is an WhereTerm structure with only the
** "pExpr" field filled in.  The job of this routine is to analyze the
** subexpression and populate all the other fields of the WhereTerm
** structure.
**
** If the expression is of the form "<expr> <op> X" it gets commuted
** to the standard form of "X <op> <expr>".
**
** If the expression is of the form "X <op> Y" where both X and Y are
** columns, then the original expression is unchanged and a new virtual
** term of the form "Y <op> X" is added to the WHERE clause and
** analyzed separately.  The original term is marked with TERM_COPIED
** and the new term is marked with TERM_DYNAMIC (because it's pExpr
** needs to be freed with the WhereClause) and TERM_VIRTUAL (because it
** is a commuted copy of a prior term.)  The original term has nChild=1
** and the copy has idxParent set to the index of the original term.
*/
ساكن فراغ exprAnalyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  صحيح idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  Expr *pExpr;                     /* The expression to be analyzed */
  Bitmask prereqLeft;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extraRight = 0;          /* Extra dependencies on LEFT JOIN */
  Expr *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  صحيح isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  صحيح noCase = 0;                  /* uppercase equivalent to lowercase */
  صحيح op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned حرف eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  صحيح nLeft;                       /* Number of elements on left side vector */

  ܐܢ( db->mallocFailed ){
    ܦܢܐ;
  }
  أكد( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  pExpr = pTerm->pExpr;
  أكد( pExpr!=0 ); /* Because malloc() has not failed */
  أكد( pExpr->op!=TK_AS && pExpr->op!=TK_COLLATE );
exprAnalyze_restart:
  pMaskSet->bVarSelect = 0;
  prereqLeft = sqlite3WhereExprUsage(pMaskSet, pExpr->pLeft);
  op = pExpr->op;
  ܐܢ( op==TK_IN ){
    أكد( pExpr->pRight==0 );
    ܐܢ( sqlite3ExprCheckIN(pParse, pExpr) ) ܦܢܐ;
    ܐܢ( ExprUseXSelect(pExpr) ){
      pTerm->prereqRight = exprSelectUsage(pMaskSet, pExpr->x.pSelect);
    }ܐܚܪܢܐ{
      pTerm->prereqRight = sqlite3WhereExprListUsage(pMaskSet, pExpr->x.pList);
    }
    prereqAll = prereqLeft | pTerm->prereqRight;
  }ܐܚܪܢܐ{
    pTerm->prereqRight = sqlite3WhereExprUsage(pMaskSet, pExpr->pRight);
    ܐܢ( pExpr->pLeft==0
     || ExprHasProperty(pExpr, EP_xIsSelect|EP_IfNullRow)
     || pExpr->x.pList!=0
    ){
      prereqAll = sqlite3WhereExprUsageNN(pMaskSet, pExpr);
    }ܐܚܪܢܐ{
      prereqAll = prereqLeft | pTerm->prereqRight;
    }
  }
  ܐܢ( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  ܐܢ( prereqAll!=sqlite3WhereExprUsageNN(pMaskSet, pExpr) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3TreeViewExpr(0,pExpr,0);
    أكد( 0 );
  }
#endif

  ܐܢ( ExprHasProperty(pExpr, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, pExpr->w.iJoin);
    ܐܢ( ExprHasProperty(pExpr, EP_OuterON) ){
      prereqAll |= x;
      extraRight = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }ܐܚܪܢܐ ܐܢ( (prereqAll>>1)>=x ){
      ExprClearProperty(pExpr, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->leftCursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  ܐܢ( allowedOp(op) ){
    صحيح aiCurCol[2];
    Expr *pLeft = sqlite3ExprSkipCollate(pExpr->pLeft);
    Expr *pRight = sqlite3ExprSkipCollate(pExpr->pRight);
    u16 opMask = (pTerm->prereqRight & prereqLeft)==0 ? WO_ALL : WO_EQUIV;

    ܐܢ( pTerm->u.x.iField>0 ){
      أكد( op==TK_IN );
      أكد( pLeft->op==TK_VECTOR );
      أكد( ExprUseXList(pLeft) );
      pLeft = pLeft->x.pList->a[pTerm->u.x.iField-1].pExpr;
    }

    ܐܢ( exprMightBeIndexed(pSrc, aiCurCol, pLeft, op) ){
      pTerm->leftCursor = aiCurCol[0];
      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.leftColumn = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    ܐܢ( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    ܐܢ( pRight
     && exprMightBeIndexed(pSrc, aiCurCol, pRight, op)
     && !ExprHasProperty(pRight, EP_FixedCol)
    ){
      WhereTerm *pNew;
      Expr *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      أكد( pTerm->u.x.iField==0 );
      ܐܢ( pTerm->leftCursor>=0 ){
        صحيح idxNew;
        pDup = sqlite3ExprDup(db, pExpr, 0);
        ܐܢ( db->mallocFailed ){
          sqlite3ExprDelete(db, pDup);
          ܦܢܐ;
        }
        idxNew = whereClauseInsert(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        ܐܢ( idxNew==0 ) ܦܢܐ;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        ܐܢ( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        أكد( pWInfo->pTabList!=0 );
        ܐܢ( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }ܐܚܪܢܐ{
        pDup = pExpr;
        pNew = pTerm;
      }
      pNew->wtFlags |= exprCommute(pParse, pDup);
      pNew->leftCursor = aiCurCol[0];
      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.leftColumn = aiCurCol[1];
      testcase( (prereqLeft | extraRight) != prereqLeft );
      pNew->prereqRight = prereqLeft | extraRight;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }ܐܚܪܢܐ
    ܐܢ( op==TK_ISNULL
     && !ExprHasProperty(pExpr,EP_OuterON)
     && 0==sqlite3ExprCanBeNull(pLeft)
    ){
      أكد( !ExprHasProperty(pExpr, EP_IntValue) );
      pExpr->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      pExpr->u.zToken = "false";
      ExprSetProperty(pExpr, EP_IsFalse);
      pTerm->prereqAll = 0;
      pTerm->eOperator = 0;
    }
  }

#ifndef SQLITE_OMIT_BETWEEN_OPTIMIZATION
  /* If a term is the BETWEEN operator, create two new virtual terms
  ** that define the range that the BETWEEN implements.  For example:
  **
  **      a BETWEEN b AND c
  **
  ** is converted into:
  **
  **      (a BETWEEN b AND c) AND (a>=b) AND (a<=c)
  **
  ** The two new terms are added onto the end of the WhereClause object.
  ** The new terms are "dynamic" and are children of the original BETWEEN
  ** term.  That means that if the BETWEEN term is coded, the children are
  ** skipped.  Or, if the children are satisfied by an index, the original
  ** BETWEEN term is skipped.
  */
  ܐܚܪܢܐ ܐܢ( pExpr->op==TK_BETWEEN && pWC->op==TK_AND ){
    ExprList *pList;
    صحيح i;
    ساكن ثابت u8 ops[] = {TK_GE, TK_LE};
    أكد( ExprUseXList(pExpr) );
    pList = pExpr->x.pList;
    أكد( pList!=0 );
    أكد( pList->nExpr==2 );
    أكد( pWC->a[idxTerm].nChild==0 );
    لكل(i=0; i<2; i++){
      Expr *pNewExpr;
      صحيح idxNew;
      pNewExpr = sqlite3PExpr(pParse, ops[i],
                             sqlite3ExprDup(db, pExpr->pLeft, 0),
                             sqlite3ExprDup(db, pList->a[i].pExpr, 0));
      transferJoinMarkings(pNewExpr, pExpr);
      idxNew = whereClauseInsert(pWC, pNewExpr, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      exprAnalyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#ܐܢ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  ܐܚܪܢܐ ܐܢ( pExpr->op==TK_OR && !ExprHasProperty(pExpr, EP_Collate) ){
    Expr *pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
    ܐܢ( pAlt!=pExpr ){
      pTerm->pExpr = pExpr = sqlite3ExprSkipCollateAndLikely(pAlt);
      اذهب exprAnalyze_restart;
    }
    أكد( pWC->op==TK_AND );
    exprAnalyzeOrTerm(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  ܐܚܪܢܐ ܐܢ( pExpr->op==TK_NOTNULL ){
    ܐܢ( pExpr->pLeft->op==TK_COLUMN
     && pExpr->pLeft->iColumn>=0
     && !ExprHasProperty(pExpr, EP_OuterON)
    ){
      Expr *pNewExpr;
      Expr *pLeft = pExpr->pLeft;
      صحيح idxNew;
      WhereTerm *pNewTerm;
 
      pNewExpr = sqlite3PExpr(pParse, TK_GT,
                              sqlite3ExprDup(db, pLeft, 0),
                              sqlite3ExprAlloc(db, TK_NULL, 0, 0));
 
      idxNew = whereClauseInsert(pWC, pNewExpr,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      ܐܢ( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereqRight = 0;
        pNewTerm->leftCursor = pLeft->iTable;
        pNewTerm->u.x.leftColumn = pLeft->iColumn;
        pNewTerm->eOperator = WO_GT;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
    }
  }


#ifndef SQLITE_OMIT_LIKE_OPTIMIZATION
  /* Add constraints to reduce the search space on a LIKE or GLOB
  ** operator.
  **
  ** A like pattern of the form "x LIKE 'aBc%'" is changed into constraints
  **
  **          x>='ABC' AND x<'abd' AND x LIKE 'aBc%'
  **
  ** The last character of the prefix "abc" is incremented to form the
  ** termination condition "abd".  If case is not significant (the default
  ** for LIKE) then the lower-bound is made all uppercase and the upper-
  ** bound is made all lowercase so that the bounds also work when comparing
  ** BLOBs.
  */
  ܐܚܪܢܐ ܐܢ( pExpr->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, pExpr, &pStr1, &isComplete, &noCase)
  ){
    Expr *pLeft;       /* LHS of LIKE/GLOB operator */
    Expr *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    Expr *pNewExpr1;
    Expr *pNewExpr2;
    صحيح idxNew1;
    صحيح idxNew2;
    ثابت حرف *zCollSeqName;     /* Name of collating sequence */
    ثابت u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    أكد( ExprUseXList(pExpr) );
    pLeft = pExpr->x.pList->a[1].pExpr;
    pStr2 = sqlite3ExprDup(db, pStr1, 0);
    أكد( pStr1==0 || !ExprHasProperty(pStr1, EP_IntValue) );
    أكد( pStr2==0 || !ExprHasProperty(pStr2, EP_IntValue) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    ܐܢ( noCase && !pParse->db->mallocFailed ){
      صحيح i;
      حرف c;
      pTerm->wtFlags |= TERM_LIKE;
      لكل(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    ܐܢ( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      ܐܢ( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        ܐܢ( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      ܟܡܐ( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      أكد( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3StrBINARY;
    pNewExpr1 = sqlite3ExprDup(db, pLeft, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3ExprAddCollateString(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, pExpr);
    idxNew1 = whereClauseInsert(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3ExprDup(db, pLeft, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3ExprAddCollateString(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, pExpr);
    idxNew2 = whereClauseInsert(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    exprAnalyze(pSrc, pWC, idxNew1);
    exprAnalyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    ܐܢ( isComplete ){
      markTermAsChild(pWC, idxNew1, idxTerm);
      markTermAsChild(pWC, idxNew2, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

  /* If there is a vector == or IS term - e.g. "(a, b) == (?, ?)" - create
  ** new terms for each component comparison - "a = ?" and "b = ?".  The
  ** new terms completely replace the original vector comparison, which is
  ** no longer used.
  **
  ** This is only required if at least one side of the comparison operation
  ** is not a sub-select.
  **
  ** tag-20220128a
  */
  ܐܢ( (pExpr->op==TK_EQ || pExpr->op==TK_IS)
   && (nLeft = sqlite3ExprVectorSize(pExpr->pLeft))>1
   && sqlite3ExprVectorSize(pExpr->pRight)==nLeft
   && ( (pExpr->pLeft->flags & EP_xIsSelect)==0
     || (pExpr->pRight->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    صحيح i;
    لكل(i=0; i<nLeft; i++){
      صحيح idxNew;
      Expr *pNew;
      Expr *pLeft = sqlite3ExprForVectorField(pParse, pExpr->pLeft, i, nLeft);
      Expr *pRight = sqlite3ExprForVectorField(pParse, pExpr->pRight, i, nLeft);

      pNew = sqlite3PExpr(pParse, pExpr->op, pLeft, pRight);
      transferJoinMarkings(pNew, pExpr);
      idxNew = whereClauseInsert(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      exprAnalyze(pSrc, pWC, idxNew);
    }
    pTerm = &pWC->a[idxTerm];
    pTerm->wtFlags |= TERM_CODED|TERM_VIRTUAL;  /* Disable the original */
    pTerm->eOperator = WO_ROWVAL;
  }

  /* If there is a vector IN term - e.g. "(a, b) IN (SELECT ...)" - create
  ** a virtual term for each vector component. The expression object
  ** used by each such virtual term is pExpr (the full vector IN(...)
  ** expression). The WhereTerm.u.x.iField variable identifies the index within
  ** the vector on the LHS that the virtual term represents.
  **
  ** This only works if the RHS is a simple SELECT (not a compound) that does
  ** not use window functions.
  */
  ܐܚܪܢܐ ܐܢ( pExpr->op==TK_IN
   && pTerm->u.x.iField==0
   && pExpr->pLeft->op==TK_VECTOR
   && ALWAYS( ExprUseXSelect(pExpr) )
   && (pExpr->x.pSelect->pPrior==0 || (pExpr->x.pSelect->selFlags & SF_Values))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && pExpr->x.pSelect->pWin==0
#endif
   && (pExpr->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && pExpr->x.pSelect->pEList->nExpr <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    صحيح i;
    أكد( pTerm->nChild==0 );
    لكل(i=0; i<sqlite3ExprVectorSize(pExpr->pLeft); i++){
      صحيح idxNew;
      idxNew = whereClauseInsert(pWC, pExpr, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      exprAnalyze(pSrc, pWC, idxNew);
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }

#ifndef SQLITE_OMIT_VIRTUALTABLE
  /* Add a WO_AUX auxiliary term to the constraint set if the
  ** current expression is of the form "column OP expr" where OP
  ** is an operator that gets passed into virtual tables but which is
  ** not normally optimized for ordinary tables.  In other words, OP
  ** is one of MATCH, LIKE, GLOB, REGEXP, !=, IS, IS NOT, or NOT NULL.
  ** This information is used by the xBestIndex methods of
  ** virtual tables.  The native query optimizer does not attempt
  ** to do anything with MATCH functions.
  */
  ܐܚܪܢܐ ܐܢ( pWC->op==TK_AND ){
    Expr *pRight = 0, *pLeft = 0;
    صحيح res = isAuxiliaryVtabOperator(db, pExpr, &eOp2, &pLeft, &pRight);
    ܟܡܐ( res-- > 0 ){
      صحيح idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereqExpr;

      prereqExpr = sqlite3WhereExprUsage(pMaskSet, pRight);
      prereqColumn = sqlite3WhereExprUsage(pMaskSet, pLeft);
      ܐܢ( (prereqExpr & prereqColumn)==0 ){
        Expr *pNewExpr;
        pNewExpr = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3ExprDup(db, pRight, 0));
        ܐܢ( ExprHasProperty(pExpr, EP_OuterON) && pNewExpr ){
          ExprSetProperty(pNewExpr, EP_OuterON);
          pNewExpr->w.iJoin = pExpr->w.iJoin;
        }
        idxNew = whereClauseInsert(pWC, pNewExpr, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereqRight = prereqExpr | extraRight;
        pNewTerm->leftCursor = pLeft->iTable;
        pNewTerm->u.x.leftColumn = pLeft->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(Expr*, pLeft, pRight);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereqRight |= extraRight;
}

/***************************************************************************
** Routines with file scope above.  Interface to the rest of the where.c
** subsystem follows.
***************************************************************************/

/*
** This routine identifies subexpressions in the WHERE clause where
** each subexpression is separated by the AND operator or some other
** operator specified in the op parameter.  The WhereClause structure
** is filled with pointers to subexpressions.  For example:
**
**    WHERE  a=='hello' AND coalesce(b,11)<10 AND (c+12!=d OR c==22)
**           \________/     \_______________/     \________________/
**            slot[0]            slot[1]               slot[2]
**
** The original WHERE clause in pExpr is unaltered.  All this routine
** does is make slot[] entries point to substructure within pExpr.
**
** In the previous sentence and in the diagram, "slot[]" refers to
** the WhereClause.a[] array.  The slot[] array grows as needed to contain
** all terms of the WHERE clause.
*/
فراغ sqlite3WhereSplit(WhereClause *pWC, Expr *pExpr, u8 op){
  Expr *pE2 = sqlite3ExprSkipCollateAndLikely(pExpr);
  pWC->op = op;
  أكد( pE2!=0 || pExpr==0 );
  ܐܢ( pE2==0 ) ܦܢܐ;
  ܐܢ( pE2->op!=op ){
    whereClauseInsert(pWC, pExpr, 0);
  }ܐܚܪܢܐ{
    sqlite3WhereSplit(pWC, pE2->pLeft, op);
    sqlite3WhereSplit(pWC, pE2->pRight, op);
  }
}

/*
** Add either a LIMIT (if eMatchOp==SQLITE_INDEX_CONSTRAINT_LIMIT) or
** OFFSET (if eMatchOp==SQLITE_INDEX_CONSTRAINT_OFFSET) term to the
** where-clause passed as the first argument. The value for the term
** is found in register iReg.
**
** In the common case where the value is a simple integer
** (example: "LIMIT 5 OFFSET 10") then the expression codes as a
** TK_INTEGER so that it will be available to sqlite3_vtab_rhs_value().
** If not, then it codes as a TK_REGISTER expression.
*/
ساكن فراغ whereAddLimitExpr(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  صحيح iReg,           /* Register that will hold value of the limit/offset */
  Expr *pExpr,        /* Expression that defines the limit/offset */
  صحيح iCsr,           /* Cursor to which the constraint applies */
  صحيح eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Expr *pNew;
  صحيح iVal = 0;

  ܐܢ( sqlite3ExprIsInteger(pExpr, &iVal, pParse, 0) && iVal>=0 ){
    Expr *pVal = sqlite3ExprInt32(db, iVal);
    ܐܢ( pVal==0 ) ܦܢܐ;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }ܐܚܪܢܐ{
    Expr *pVal = sqlite3ExprAlloc(db, TK_REGISTER, 0, 0);
    ܐܢ( pVal==0 ) ܦܢܐ;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  ܐܢ( pNew ){
    WhereTerm *pTerm;
    صحيح idx;
    idx = whereClauseInsert(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->leftCursor = iCsr;
    pTerm->eOperator = WO_AUX;
    pTerm->eMatchOp = eMatchOp;
  }
}

/*
** Possibly add terms corresponding to the LIMIT and OFFSET clauses of the
** SELECT statement passed as the second argument. These terms are only
** added if:
**
**   1. The SELECT statement has a LIMIT clause, and
**   2. The SELECT statement is not an aggregate or DISTINCT query, and
**   3. The SELECT statement has exactly one object in its FROM clause, and
**      that object is a virtual table, and
**   4. There are no terms in the WHERE clause that will not be passed
**      to the virtual table xBestIndex method.
**   5. The ORDER BY clause, if any, will be made available to the xBestIndex
**      method.
**
** LIMIT and OFFSET terms are ignored by most of the planner code. They
** exist only so that they may be passed to the xBestIndex method of the
** single virtual table in the FROM clause of the SELECT.
*/
فراغ SQLITE_NOINLINE sqlite3WhereAddLimit(WhereClause *pWC, Select *p){
  أكد( p!=0 && p->pLimit!=0 );                 /* 1 -- checked by caller */
  ܐܢ( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    ExprList *pOrderBy = p->pOrderBy;
    صحيح iCsr = p->pSrc->a[0].iCursor;
    صحيح ii;

    /* Check condition (4). Return early if it is not met. */
    لكل(ii=0; ii<pWC->nTerm; ii++){
      ܐܢ( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        أكد( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        أكد( pWC->a[ii].eOperator==WO_ROWVAL );
        استمر;
      }
      ܐܢ( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        استمر;
      }
      ܐܢ( pWC->a[ii].leftCursor==iCsr && pWC->a[ii].prereqRight==0 ) استمر;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      ܐܢ( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        ܐܢ( pParent->leftCursor==iCsr
         && pParent->prereqRight==0
         && pParent->nChild==1
        ){
          استمر;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      ܦܢܐ;
    }

    /* Check condition (5). Return early if it is not met. */
    ܐܢ( pOrderBy ){
      لكل(ii=0; ii<pOrderBy->nExpr; ii++){
        Expr *pExpr = pOrderBy->a[ii].pExpr;
        ܐܢ( pExpr->op!=TK_COLUMN ) ܦܢܐ;
        ܐܢ( pExpr->iTable!=iCsr ) ܦܢܐ;
        ܐܢ( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) ܦܢܐ;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    أكد( p->pLimit->op==TK_LIMIT );
    ܐܢ( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      whereAddLimitExpr(pWC, p->iOffset, p->pLimit->pRight,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    ܐܢ( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      whereAddLimitExpr(pWC, p->iLimit, p->pLimit->pLeft,
                        iCsr, SQLITE_INDEX_CONSTRAINT_LIMIT);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
فراغ sqlite3WhereClauseInit(
  WhereClause *pWC,        /* The WhereClause to be initialized */
  WhereInfo *pWInfo        /* The WHERE processing context */
){
  pWC->pWInfo = pWInfo;
  pWC->hasOr = 0;
  pWC->pOuter = 0;
  pWC->nTerm = 0;
  pWC->nBase = 0;
  pWC->nSlot = ArraySize(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of
** sqlite3WhereClauseInit().
*/
فراغ sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  أكد( pWC->nTerm>=pWC->nBase );
  ܐܢ( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    صحيح i;
    /* Verify that every term past pWC->nBase is virtual */
    لكل(i=pWC->nBase; i<pWC->nTerm; i++){
      أكد( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    ܟܡܐ(1){
      أكد( a->eMatchOp==0 || a->eOperator==WO_AUX );
      ܐܢ( a->wtFlags & TERM_DYNAMIC ){
        sqlite3ExprDelete(db, a->pExpr);
      }
      ܐܢ( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        ܐܢ( a->wtFlags & TERM_ORINFO ){
          أكد( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }ܐܚܪܢܐ{
          أكد( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      ܐܢ( a==aLast ) اكسر;
      a++;
    }
  }
}


/*
** These routines walk (recursively) an expression tree and generate
** a bitmask indicating which tables are used in that expression
** tree.
**
** sqlite3WhereExprUsage(MaskSet, Expr) ->
**
**       Return a Bitmask of all tables referenced by Expr.  Expr can be
**       be NULL, in which case 0 is returned.
**
** sqlite3WhereExprUsageNN(MaskSet, Expr) ->
**
**       Same as sqlite3WhereExprUsage() except that Expr must not be
**       NULL.  The "NN" suffix on the name stands for "Not Null".
**
** sqlite3WhereExprListUsage(MaskSet, ExprList) ->
**
**       Return a Bitmask of all tables referenced by every expression
**       in the expression list ExprList.  ExprList can be NULL, in which
**       case 0 is returned.
**
** sqlite3WhereExprUsageFull(MaskSet, ExprList) ->
**
**       Internal use only.  Called only by sqlite3WhereExprUsageNN() for
**       complex expressions that require pushing register values onto
**       the stack.  Many calls to sqlite3WhereExprUsageNN() do not need
**       the more complex analysis done by this routine.  Hence, the
**       computations done by this routine are broken out into a separate
**       "no-inline" function to avoid the stack push overhead in the
**       common case where it is not needed.
*/
ساكن SQLITE_NOINLINE Bitmask sqlite3WhereExprUsageFull(
  WhereMaskSet *pMaskSet,
  Expr *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  ܐܢ( p->pLeft ) mask |= sqlite3WhereExprUsageNN(pMaskSet, p->pLeft);
  ܐܢ( p->pRight ){
    mask |= sqlite3WhereExprUsageNN(pMaskSet, p->pRight);
    أكد( p->x.pList==0 );
  }ܐܚܪܢܐ ܐܢ( ExprUseXSelect(p) ){
    ܐܢ( ExprHasProperty(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= exprSelectUsage(pMaskSet, p->x.pSelect);
  }ܐܚܪܢܐ ܐܢ( p->x.pList ){
    mask |= sqlite3WhereExprListUsage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  ܐܢ( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && ExprUseYWin(p) ){
    أكد( p->y.pWin!=0 );
    mask |= sqlite3WhereExprListUsage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3WhereExprListUsage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3WhereExprUsage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  ܦܢܐ mask;
}
Bitmask sqlite3WhereExprUsageNN(WhereMaskSet *pMaskSet, Expr *p){
  ܐܢ( p->op==TK_COLUMN && !ExprHasProperty(p, EP_FixedCol) ){
    ܦܢܐ sqlite3WhereGetMask(pMaskSet, p->iTable);
  }ܐܚܪܢܐ ܐܢ( ExprHasProperty(p, EP_TokenOnly|EP_Leaf) ){
    أكد( p->op!=TK_IF_NULL_ROW );
    ܦܢܐ 0;
  }
  ܦܢܐ sqlite3WhereExprUsageFull(pMaskSet, p);
}
Bitmask sqlite3WhereExprUsage(WhereMaskSet *pMaskSet, Expr *p){
  ܦܢܐ p ? sqlite3WhereExprUsageNN(pMaskSet,p) : 0;
}
Bitmask sqlite3WhereExprListUsage(WhereMaskSet *pMaskSet, ExprList *pList){
  صحيح i;
  Bitmask mask = 0;
  ܐܢ( pList ){
    لكل(i=0; i<pList->nExpr; i++){
      mask |= sqlite3WhereExprUsage(pMaskSet, pList->a[i].pExpr);
    }
  }
  ܦܢܐ mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
فراغ sqlite3WhereExprAnalyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  صحيح i;
  لكل(i=pWC->nTerm-1; i>=0; i--){
    exprAnalyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
فراغ sqlite3WhereTabFuncArgs(
  Parse *pParse,                    /* Parsing context */
  SrcItem *pItem,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  Table *pTab;
  صحيح j, k;
  ExprList *pArgs;
  Expr *pColRef;
  Expr *pTerm;
  ܐܢ( pItem->fg.isTabFunc==0 ) ܦܢܐ;
  pTab = pItem->pSTab;
  أكد( pTab!=0 );
  ܐܢ( !IsVirtual(pTab) ){
    sqlite3ErrorMsg(pParse, "'%s' is not a function", pItem->zName);
    ܦܢܐ;
  }
  pArgs = pItem->u1.pFuncArg;
  ܐܢ( pArgs==0 ) ܦܢܐ;
  لكل(j=k=0; j<pArgs->nExpr; j++){
    Expr *pRhs;
    u32 joinType;
    ܟܡܐ( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    ܐܢ( k>=pTab->nCol ){
      sqlite3ErrorMsg(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      ܦܢܐ;
    }
    pColRef = sqlite3ExprAlloc(pParse->db, TK_COLUMN, 0, 0);
    ܐܢ( pColRef==0 ) ܦܢܐ;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    أكد( ExprUseYTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3ExprColUsed(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3ExprDup(pParse->db, pArgs->a[j].pExpr, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    ܐܢ( pItem->fg.jointype & (JT_LEFT|JT_RIGHT) ){
      testcase( pItem->fg.jointype & JT_LEFT );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_RIGHT ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }ܐܚܪܢܐ{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3SetJoinExpr(pTerm, pItem->iCursor, joinType);
    whereClauseInsert(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
صحيح sqlite3WhereLoopBloomable(ثابت WhereLoop *pLoop){
  ܐܢ( pLoop->wsFlags & WHERE_IPK ) ܦܢܐ 1;
  ܐܢ( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) ܦܢܐ 0;
  ܦܢܐ sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
