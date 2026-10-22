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
#include "sqliteInt.h"
#include "whereInt.h"

/* Forward declarations */
static void exprAnalyze(SrcList*, WhereClause*, ціле);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
static void whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
static void whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
static ціле whereClauseInsert(WhereClause *pWC, Expr *p, u16 wtFlags){
  WhereTerm *pTerm;
  ціле idx;
  testcase( wtFlags & TERM_VIRTUAL );
  якщо( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, sizeof(pWC->a[0])*pWC->nSlot*2 );
    якщо( pWC->a==0 ){
      якщо( wtFlags & TERM_DYNAMIC ){
        sqlite3ExprDelete(db, p);
      }
      pWC->a = pOld;
      повернути 0;
    }
    memcpy(pWC->a, pOld, sizeof(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  якщо( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  якщо( p && ExprHasProperty(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }інакше{
    pTerm->truthProb = 1;
  }
  pTerm->pExpr = sqlite3ExprSkipCollateAndLikely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         sizeof(WhereTerm) - offsetof(WhereTerm,eOperator));
  повернути idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
static ціле allowedOp(ціле op){
  assert( TK_GT>TK_EQ && TK_GT<TK_GE );
  assert( TK_LT>TK_EQ && TK_LT<TK_GE );
  assert( TK_LE>TK_EQ && TK_LE<TK_GE );
  assert( TK_GE==TK_EQ+4 );
  assert( TK_IN<TK_EQ );
  assert( TK_IS<TK_EQ );
  assert( TK_ISNULL<TK_EQ );
  якщо( op>TK_GE ) повернути 0;
  якщо( op>=TK_EQ ) повернути 1;
  повернути op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
static u16 exprCommute(Parse *pParse, Expr *pExpr){
  якщо( pExpr->pLeft->op==TK_VECTOR
   || pExpr->pRight->op==TK_VECTOR
   || sqlite3BinaryCompareCollSeq(pParse, pExpr->pLeft, pExpr->pRight) !=
      sqlite3BinaryCompareCollSeq(pParse, pExpr->pRight, pExpr->pLeft)
  ){
    pExpr->flags ^= EP_Commuted;
  }
  SWAP(Expr*,pExpr->pRight,pExpr->pLeft);
  якщо( pExpr->op>=TK_GT ){
    assert( TK_LT==TK_GT+2 );
    assert( TK_GE==TK_LE+2 );
    assert( TK_GT>TK_EQ );
    assert( TK_GT<TK_LE );
    assert( pExpr->op>=TK_GT && pExpr->op<=TK_GE );
    pExpr->op = ((pExpr->op-TK_GT)^2)+TK_GT;
  }
  повернути 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
static u16 operatorMask(ціле op){
  u16 c;
  assert( allowedOp(op) );
  якщо( op>=TK_EQ ){
    assert( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }інакше якщо( op==TK_IN ){
    c = WO_IN;
  }інакше якщо( op==TK_ISNULL ){
    c = WO_ISNULL;
  }інакше{
    assert( op==TK_IS );
    c = WO_IS;
  }
  assert( op!=TK_ISNULL || c==WO_ISNULL );
  assert( op!=TK_IN || c==WO_IN );
  assert( op!=TK_EQ || c==WO_EQ );
  assert( op!=TK_LT || c==WO_LT );
  assert( op!=TK_LE || c==WO_LE );
  assert( op!=TK_GT || c==WO_GT );
  assert( op!=TK_GE || c==WO_GE );
  assert( op!=TK_IS || c==WO_IS );
  повернути c;
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
static ціле isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  Expr *pExpr,      /* Test this expression */
  Expr **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  ціле *pisComplete, /* True if the only wildcard is % in the last character */
  ціле *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  const u8 *z = 0;           /* String on RHS of LIKE operator */
  Expr *pRight, *pLeft;      /* Right and left size of LIKE operator */
  ExprList *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  ціле cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_value *pVal = 0;
  ціле op;                    /* Opcode of pRight */
  ціле rc;                    /* Result code to return */

  якщо( !sqlite3IsLikeFunction(db, pExpr, pnoCase, (char*)wc) ){
    повернути 0;
  }
#ifdef SQLITE_EBCDIC
  якщо( *pnoCase ) повернути 0;
#endif
  assert( ExprUseXList(pExpr) );
  pList = pExpr->x.pList;
  pLeft = pList->a[1].pExpr;

  pRight = sqlite3ExprSkipCollate(pList->a[0].pExpr);
  op = pRight->op;
  якщо( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    ціле iCol = pRight->iColumn;
    pVal = sqlite3VdbeGetBoundValue(pReprepare, iCol, SQLITE_AFF_BLOB);
    якщо( pVal && sqlite3_value_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_value_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    assert( pRight->op==TK_VARIABLE || pRight->op==TK_REGISTER );
  }інакше якщо( op==TK_STRING ){
    assert( !ExprHasProperty(pRight, EP_IntValue) );
     z = (u8*)pRight->u.zToken;
  }
  якщо( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    поки( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      якщо( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }інакше якщо( c>=0x80 ){
        const u8 *z2 = z+cnt-1;
        якщо( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          break;
        }інакше{
          cnt = (ціле)(z2-z);
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
    якщо( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      Expr *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3Expr(db, TK_STRING, (char*)z);
      якщо( pPrefix ){
        ціле iFrom, iTo;
        char *zNew;
        assert( !ExprHasProperty(pPrefix, EP_IntValue) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        для(iFrom=iTo=0; iFrom<cnt; iFrom++){
          якщо( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[iTo++] = zNew[iFrom];
        }
        zNew[iTo] = 0;
        assert( iTo>0 );

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
        якщо( pLeft->op!=TK_COLUMN
         || sqlite3ExprAffinity(pLeft)!=SQLITE_AFF_TEXT
         || (ALWAYS( ExprUseYTab(pLeft) )
             && ALWAYS(pLeft->y.pTab)
             && IsVirtual(pLeft->y.pTab))  /* Might be numeric */
        ){
          ціле isNum;
          double rDummy;
          assert( zNew[iTo]==0 );
          isNum = sqlite3AtoF(zNew, &rDummy);
          якщо( isNum<=0 ){
            якщо( iTo==1 && zNew[0]=='-' ){
              isNum = +1;
            }інакше{
              zNew[iTo-1]++;
              isNum = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          якщо( isNum>0 ){
            sqlite3ExprDelete(db, pPrefix);
            sqlite3ValueFree(pVal);
            повернути 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      якщо( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, pRight->iColumn, 0);
        assert( !ExprHasProperty(pRight, EP_IntValue) );
        якщо( *pisComplete && pRight->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          ціле r1 = sqlite3GetTempReg(pParse);
          sqlite3ExprCodeTarget(pParse, pRight, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }інакше{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3ValueFree(pVal);
  повернути rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
ціле sqlite3ExprIsLikeOperator(const Expr *pExpr){
  static const struct {
    const char *zOp;
    unsigned char eOp;
  } aOp[] = {
    { "match",  SQLITE_INDEX_CONSTRAINT_MATCH },
    { "glob",   SQLITE_INDEX_CONSTRAINT_GLOB },
    { "like",   SQLITE_INDEX_CONSTRAINT_LIKE },
    { "regexp", SQLITE_INDEX_CONSTRAINT_REGEXP }
  };
  ціле i;
  assert( pExpr->op==TK_FUNCTION );
  assert( !ExprHasProperty(pExpr, EP_IntValue) );
  для(i=0; i<ArraySize(aOp); i++){
    якщо( sqlite3StrICmp(pExpr->u.zToken, aOp[i].zOp)==0 ){
      повернути aOp[i].eOp;
    }
  }
  повернути 0;
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
static ціле isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  Expr *pExpr,                    /* Test this expression */
  unsigned char *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  Expr **ppLeft,                  /* Column expression to left of MATCH/op2 */
  Expr **ppRight                  /* Expression to left of MATCH/op2 */
){
  якщо( pExpr->op==TK_FUNCTION ){
    ExprList *pList;
    Expr *pCol;                     /* Column reference */
    ціле i;

    assert( ExprUseXList(pExpr) );
    pList = pExpr->x.pList;
    якщо( pList==0 || pList->nExpr!=2 ){
      повернути 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].pExpr;
    assert( pCol->op!=TK_COLUMN || (ExprUseYTab(pCol) && pCol->y.pTab!=0) );
    якщо( ExprIsVtab(pCol) && (i = sqlite3ExprIsLikeOperator(pExpr))!=0 ){
      *peOp2 = i;
      *ppRight = pList->a[0].pExpr;
      *ppLeft = pCol;
      повернути 1;
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
    assert( pCol->op!=TK_COLUMN || ExprUseYTab(pCol) );
    assert( pCol->op!=TK_COLUMN || (ExprUseYTab(pCol) && pCol->y.pTab!=0) );
    якщо( ExprIsVtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      void (*xNotUsed)(sqlite3_context*,ціле,sqlite3_value**);
      void *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      assert( pVtab!=0 );
      assert( pVtab->pModule!=0 );
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
      pMod = (sqlite3_module *)pVtab->pModule;
      якщо( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, pExpr->u.zToken, &xNotUsed, &pNotUsed);
        якщо( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *ppRight = pList->a[1].pExpr;
          *ppLeft = pCol;
          повернути 1;
        }
      }
    }
  }інакше якщо( pExpr->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    assert( TK_NE < TK_EQ );
    assert( TK_ISNOT < TK_EQ );
    assert( TK_NOTNULL < TK_EQ );
    повернути 0;
  }інакше якщо( pExpr->op==TK_NE || pExpr->op==TK_ISNOT || pExpr->op==TK_NOTNULL ){
    ціле res = 0;
    Expr *pLeft = pExpr->pLeft;
    Expr *pRight = pExpr->pRight;
    assert( pLeft->op!=TK_COLUMN || (ExprUseYTab(pLeft) && pLeft->y.pTab!=0) );
    якщо( ExprIsVtab(pLeft) ){
      res++;
    }
    assert( pRight==0 || pRight->op!=TK_COLUMN
            || (ExprUseYTab(pRight) && pRight->y.pTab!=0) );
    якщо( pRight && ExprIsVtab(pRight) ){
      res++;
      SWAP(Expr*, pLeft, pRight);
    }
    *ppLeft = pLeft;
    *ppRight = pRight;
    якщо( pExpr->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    якщо( pExpr->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    якщо( pExpr->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    повернути res;
  }
  повернути 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
static void transferJoinMarkings(Expr *pDerived, Expr *pBase){
  якщо( pDerived && ExprHasProperty(pBase, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= pBase->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
static void markTermAsChild(WhereClause *pWC, ціле iChild, ціле iParent){
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;
  assert(   pWC->a[iParent].nChild <  UMXV(pWC->a[0].nChild) );
  pWC->a[iParent].nChild++;
  testcase( pWC->a[iParent].nChild == UMXV(pWC->a[0].nChild) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
static WhereTerm *whereNthSubterm(WhereTerm *pTerm, ціле N){
  якщо( pTerm->eOperator!=WO_AND ){
    повернути N==0 ? pTerm : 0;
  }
  якщо( N<pTerm->u.pAndInfo->wc.nTerm ){
    повернути &pTerm->u.pAndInfo->wc.a[N];
  }
  повернути 0;
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
static void whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  Expr *pNew;            /* New virtual expression */
  ціле op;                /* Operator for the combined expression */
  ціле idxNew;            /* Index in pWC of the next virtual term */
  Expr *pA, *pB;         /* Expressions associated with pOne and pTwo */

  якщо( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) повернути;
  якщо( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) повернути;
  якщо( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) повернути;
  якщо( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) повернути;
  pA = pOne->pExpr;
  pB = pTwo->pExpr;
  assert( pA->pLeft!=0 && pA->pRight!=0 );
  assert( pB->pLeft!=0 && pB->pRight!=0 );
  якщо( sqlite3ExprCompare(0,pA->pLeft, pB->pLeft, -1) )  повернути;
  якщо( sqlite3ExprCompare(0,pA->pRight, pB->pRight,-1) ) повернути;
  якщо( ExprHasProperty(pA,EP_Commuted)!=ExprHasProperty(pB,EP_Commuted) ){
    повернути;
  }
  /* If we reach this point, it means the two subterms can be combined */
  якщо( (eOp & (eOp-1))!=0 ){
    якщо( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }інакше{
      assert( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3ExprDup(db, pA, 0);
  якщо( pNew==0 ) повернути;
  для(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ assert( op<TK_GE ); }
  pNew->op = op;
  idxNew = whereClauseInsert(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  exprAnalyze(pSrc, pWC, idxNew);
}

#якщо !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
static void exprAnalyzeOrTerm(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  ціле idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  Expr *pExpr = pTerm->pExpr;             /* The expression of the term */
  ціле i;                                  /* Loop counters */
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
  assert( (pTerm->wtFlags & (TERM_DYNAMIC|TERM_ORINFO|TERM_ANDINFO))==0 );
  assert( pExpr->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, sizeof(*pOrInfo));
  якщо( pOrInfo==0 ) повернути;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, sizeof(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, pExpr, TK_OR);
  sqlite3WhereExprAnalyze(pSrc, pOrWc);
  якщо( db->mallocFailed ) повернути;
  assert( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  для(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    якщо( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      assert( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, sizeof(*pAndInfo));
      якщо( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        ціле j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->leftCursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, sizeof(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->pExpr, TK_AND);
        sqlite3WhereExprAnalyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        якщо( !db->mallocFailed ){
          для(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            assert( pAndTerm->pExpr );
            якщо( allowedOp(pAndTerm->pExpr->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->leftCursor);
            }
          }
        }
        indexable &= b;
      }
    }інакше якщо( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }інакше{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->leftCursor);
      якщо( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->leftCursor);
      }
      indexable &= b;
      якщо( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }інакше{
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
  якщо( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  якщо( indexable && pOrWc->nTerm==2 ){
    ціле iOne = 0;
    WhereTerm *pOne;
    поки( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      ціле iTwo = 0;
      WhereTerm *pTwo;
      поки( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  якщо( chngToIN ){
    ціле okToChngToIN = 0;     /* True if the conversion to IN is valid */
    ціле iColumn = -1;         /* Column index on lhs of IN operator */
    ціле iCursor = -1;         /* Table cursor common to all terms */
    ціле j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    для(j=0; j<2 && !okToChngToIN; j++){
      Expr *pLeft = 0;
      pOrTerm = pOrWc->a;
      для(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        assert( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        якщо( pOrTerm->leftCursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          assert( j==1 );
          continue;
        }
        якщо( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->leftCursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          assert( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          continue;
        }
        assert( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        iColumn = pOrTerm->u.x.leftColumn;
        iCursor = pOrTerm->leftCursor;
        pLeft = pOrTerm->pExpr->pLeft;
        break;
      }
      якщо( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        assert( j==1 );
        assert( IsPowerOfTwo(chngToIN) );
        assert( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        break;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      для(; i>=0 && okToChngToIN; i--, pOrTerm++){
        assert( pOrTerm->eOperator & WO_EQ );
        assert( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        якщо( pOrTerm->leftCursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }інакше якщо( pOrTerm->u.x.leftColumn!=iColumn || (iColumn==XN_EXPR
               && sqlite3ExprCompare(pParse, pOrTerm->pExpr->pLeft, pLeft, -1)
        )){
          okToChngToIN = 0;
        }інакше{
          ціле affLeft, affRight;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          affRight = sqlite3ExprAffinity(pOrTerm->pExpr->pRight);
          affLeft = sqlite3ExprAffinity(pOrTerm->pExpr->pLeft);
          якщо( affRight!=0 && affRight!=affLeft ){
            okToChngToIN = 0;
          }інакше{
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
    якщо( okToChngToIN ){
      Expr *pDup;            /* A transient duplicate expression */
      ExprList *pList = 0;   /* The RHS of the IN operator */
      Expr *pLeft = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      Expr *pNew;            /* The complete IN operator */

      для(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        Expr *pThis;
        якщо( (pOrTerm->wtFlags & TERM_OK)==0 ) continue;
        assert( pOrTerm->eOperator & WO_EQ );
        assert( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        assert( pOrTerm->leftCursor==iCursor );
        assert( pOrTerm->u.x.leftColumn==iColumn );
        pThis = pOrTerm->pExpr;
        pDup = sqlite3ExprDup(db, pThis->pRight, 0);
        pList = sqlite3ExprListAppend(pWInfo->pParse, pList, pDup);
        якщо( pLeft==0 ){
          pLeft = pThis->pLeft;
          pCollSeq = sqlite3ExprCompareCollSeq(pParse, pThis);
        }інакше{
          assert( 0==sqlite3ExprCompare(pParse, 
                        sqlite3ExprSkipCollate(pThis->pLeft),
                        sqlite3ExprSkipCollate(pLeft), -1) );
          якщо( pCollSeq!=sqlite3ExprCompareCollSeq(pParse, pThis) ){
            pLeft = 0;  /* Collating sequence mismatch */
            break;
          }
        }
      }
      якщо( pLeft==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }інакше{
        pDup = sqlite3ExprDup(db, pLeft, 0);
        якщо( sqlite3ExprCollSeq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          assert( pCollSeq->zName!=0 );
          pDup = sqlite3ExprAddCollateString(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      якщо( pNew ){
        ціле idxNew;
        transferJoinMarkings(pNew, pExpr);
        assert( ExprUseXList(pNew) );
        pNew->x.pList = pList;
        idxNew = whereClauseInsert(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        exprAnalyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }інакше{
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
static ціле termIsEquivalence(Parse *pParse, Expr *pExpr, SrcList *pSrc){
  char aff1, aff2;
  якщо( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) повернути 0;  /* (1) */
  якщо( pExpr->op!=TK_EQ && pExpr->op!=TK_IS ) повернути 0;                 /* (2) */
  якщо( ExprHasProperty(pExpr, EP_OuterON|EP_Collate) ) повернути 0;        /* (3) */
  assert( pSrc!=0 );
  якщо( pExpr->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    повернути 0;                                                          /* (4) */
  }
  aff1 = sqlite3ExprAffinity(pExpr->pLeft);
  aff2 = sqlite3ExprAffinity(pExpr->pRight);
  якщо( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    повернути 0;                                                          /* (5) */
  }
  якщо( !sqlite3ExprCollSeqMatch(pParse, pExpr->pLeft, pExpr->pRight) ){
    повернути 0;                                                          /* (6) */
  }
  повернути 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
static Bitmask exprSelectUsage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  поки( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pEList);
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3WhereExprUsage(pMaskSet, pS->pWhere);
    mask |= sqlite3WhereExprUsage(pMaskSet, pS->pHaving);
    якщо( ALWAYS(pSrc!=0) ){
      ціле i;
      для(i=0; i<pSrc->nSrc; i++){
        якщо( pSrc->a[i].fg.isSubquery ){
          mask |= exprSelectUsage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        якщо( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3WhereExprUsage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        якщо( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3WhereExprListUsage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  повернути mask;
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
static SQLITE_NOINLINE ціле exprMightBeIndexed2(
  SrcList *pFrom,        /* The FROM clause */
  ціле *aiCurCol,         /* Write the referenced table cursor and column here */
  Expr *pExpr,           /* An operand of a comparison operator */
  ціле j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  ціле i;
  ціле iCur;
  do{
    iCur = pFrom->a[j].iCursor;
    для(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      якщо( pIdx->aColExpr==0 ) continue;
      для(i=0; i<pIdx->nKeyCol; i++){
        якщо( pIdx->aiColumn[i]!=XN_EXPR ) continue;
        assert( pIdx->bHasExpr );
        якщо( sqlite3ExprCompareSkip(pExpr,pIdx->aColExpr->a[i].pExpr,iCur)==0
         && !sqlite3ExprIsConstant(0,pIdx->aColExpr->a[i].pExpr)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_EXPR;
          повернути 1;
        }
      }
    }
  }поки( ++j < pFrom->nSrc );
  повернути 0;
}
static ціле exprMightBeIndexed(
  SrcList *pFrom,        /* The FROM clause */
  ціле *aiCurCol,         /* Write the referenced table cursor & column here */
  Expr *pExpr,           /* An operand of a comparison operator */
  ціле op                 /* The specific comparison operator */
){
  ціле i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  assert( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  assert( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  assert( op<=TK_GE );
  якщо( pExpr->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    assert( ExprUseXList(pExpr) );
    pExpr = pExpr->x.pList->a[0].pExpr;
  }

  якщо( pExpr->op==TK_COLUMN ){
    aiCurCol[0] = pExpr->iTable;
    aiCurCol[1] = pExpr->iColumn;
    повернути 1;
  }

  для(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    для(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      якщо( pIdx->aColExpr ){
        повернути exprMightBeIndexed2(pFrom,aiCurCol,pExpr,i);
      }
    }
  }
  повернути 0;
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
static void exprAnalyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  ціле idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  Expr *pExpr;                     /* The expression to be analyzed */
  Bitmask prereqLeft;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extraRight = 0;          /* Extra dependencies on LEFT JOIN */
  Expr *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  ціле isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  ціле noCase = 0;                  /* uppercase equivalent to lowercase */
  ціле op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned char eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  ціле nLeft;                       /* Number of elements on left side vector */

  якщо( db->mallocFailed ){
    повернути;
  }
  assert( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  pExpr = pTerm->pExpr;
  assert( pExpr!=0 ); /* Because malloc() has not failed */
  assert( pExpr->op!=TK_AS && pExpr->op!=TK_COLLATE );
exprAnalyze_restart:
  pMaskSet->bVarSelect = 0;
  prereqLeft = sqlite3WhereExprUsage(pMaskSet, pExpr->pLeft);
  op = pExpr->op;
  якщо( op==TK_IN ){
    assert( pExpr->pRight==0 );
    якщо( sqlite3ExprCheckIN(pParse, pExpr) ) повернути;
    якщо( ExprUseXSelect(pExpr) ){
      pTerm->prereqRight = exprSelectUsage(pMaskSet, pExpr->x.pSelect);
    }інакше{
      pTerm->prereqRight = sqlite3WhereExprListUsage(pMaskSet, pExpr->x.pList);
    }
    prereqAll = prereqLeft | pTerm->prereqRight;
  }інакше{
    pTerm->prereqRight = sqlite3WhereExprUsage(pMaskSet, pExpr->pRight);
    якщо( pExpr->pLeft==0
     || ExprHasProperty(pExpr, EP_xIsSelect|EP_IfNullRow)
     || pExpr->x.pList!=0
    ){
      prereqAll = sqlite3WhereExprUsageNN(pMaskSet, pExpr);
    }інакше{
      prereqAll = prereqLeft | pTerm->prereqRight;
    }
  }
  якщо( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  якщо( prereqAll!=sqlite3WhereExprUsageNN(pMaskSet, pExpr) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3TreeViewExpr(0,pExpr,0);
    assert( 0 );
  }
#endif

  якщо( ExprHasProperty(pExpr, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, pExpr->w.iJoin);
    якщо( ExprHasProperty(pExpr, EP_OuterON) ){
      prereqAll |= x;
      extraRight = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }інакше якщо( (prereqAll>>1)>=x ){
      ExprClearProperty(pExpr, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->leftCursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  якщо( allowedOp(op) ){
    ціле aiCurCol[2];
    Expr *pLeft = sqlite3ExprSkipCollate(pExpr->pLeft);
    Expr *pRight = sqlite3ExprSkipCollate(pExpr->pRight);
    u16 opMask = (pTerm->prereqRight & prereqLeft)==0 ? WO_ALL : WO_EQUIV;

    якщо( pTerm->u.x.iField>0 ){
      assert( op==TK_IN );
      assert( pLeft->op==TK_VECTOR );
      assert( ExprUseXList(pLeft) );
      pLeft = pLeft->x.pList->a[pTerm->u.x.iField-1].pExpr;
    }

    якщо( exprMightBeIndexed(pSrc, aiCurCol, pLeft, op) ){
      pTerm->leftCursor = aiCurCol[0];
      assert( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.leftColumn = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    якщо( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    якщо( pRight
     && exprMightBeIndexed(pSrc, aiCurCol, pRight, op)
     && !ExprHasProperty(pRight, EP_FixedCol)
    ){
      WhereTerm *pNew;
      Expr *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      assert( pTerm->u.x.iField==0 );
      якщо( pTerm->leftCursor>=0 ){
        ціле idxNew;
        pDup = sqlite3ExprDup(db, pExpr, 0);
        якщо( db->mallocFailed ){
          sqlite3ExprDelete(db, pDup);
          повернути;
        }
        idxNew = whereClauseInsert(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        якщо( idxNew==0 ) повернути;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        якщо( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        assert( pWInfo->pTabList!=0 );
        якщо( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }інакше{
        pDup = pExpr;
        pNew = pTerm;
      }
      pNew->wtFlags |= exprCommute(pParse, pDup);
      pNew->leftCursor = aiCurCol[0];
      assert( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.leftColumn = aiCurCol[1];
      testcase( (prereqLeft | extraRight) != prereqLeft );
      pNew->prereqRight = prereqLeft | extraRight;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }інакше
    якщо( op==TK_ISNULL
     && !ExprHasProperty(pExpr,EP_OuterON)
     && 0==sqlite3ExprCanBeNull(pLeft)
    ){
      assert( !ExprHasProperty(pExpr, EP_IntValue) );
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
  інакше якщо( pExpr->op==TK_BETWEEN && pWC->op==TK_AND ){
    ExprList *pList;
    ціле i;
    static const u8 ops[] = {TK_GE, TK_LE};
    assert( ExprUseXList(pExpr) );
    pList = pExpr->x.pList;
    assert( pList!=0 );
    assert( pList->nExpr==2 );
    assert( pWC->a[idxTerm].nChild==0 );
    для(i=0; i<2; i++){
      Expr *pNewExpr;
      ціле idxNew;
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

#якщо !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  інакше якщо( pExpr->op==TK_OR && !ExprHasProperty(pExpr, EP_Collate) ){
    Expr *pAlt = sqlite3ExprSimplifiedAndOr(pExpr);
    якщо( pAlt!=pExpr ){
      pTerm->pExpr = pExpr = sqlite3ExprSkipCollateAndLikely(pAlt);
      goto exprAnalyze_restart;
    }
    assert( pWC->op==TK_AND );
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
  інакше якщо( pExpr->op==TK_NOTNULL ){
    якщо( pExpr->pLeft->op==TK_COLUMN
     && pExpr->pLeft->iColumn>=0
     && !ExprHasProperty(pExpr, EP_OuterON)
    ){
      Expr *pNewExpr;
      Expr *pLeft = pExpr->pLeft;
      ціле idxNew;
      WhereTerm *pNewTerm;
 
      pNewExpr = sqlite3PExpr(pParse, TK_GT,
                              sqlite3ExprDup(db, pLeft, 0),
                              sqlite3ExprAlloc(db, TK_NULL, 0, 0));
 
      idxNew = whereClauseInsert(pWC, pNewExpr,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      якщо( idxNew ){
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
  інакше якщо( pExpr->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, pExpr, &pStr1, &isComplete, &noCase)
  ){
    Expr *pLeft;       /* LHS of LIKE/GLOB operator */
    Expr *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    Expr *pNewExpr1;
    Expr *pNewExpr2;
    ціле idxNew1;
    ціле idxNew2;
    const char *zCollSeqName;     /* Name of collating sequence */
    const u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    assert( ExprUseXList(pExpr) );
    pLeft = pExpr->x.pList->a[1].pExpr;
    pStr2 = sqlite3ExprDup(db, pStr1, 0);
    assert( pStr1==0 || !ExprHasProperty(pStr1, EP_IntValue) );
    assert( pStr2==0 || !ExprHasProperty(pStr2, EP_IntValue) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    якщо( noCase && !pParse->db->mallocFailed ){
      ціле i;
      char c;
      pTerm->wtFlags |= TERM_LIKE;
      для(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    якщо( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      якщо( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        якщо( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      поки( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      assert( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
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
    якщо( isComplete ){
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
  якщо( (pExpr->op==TK_EQ || pExpr->op==TK_IS)
   && (nLeft = sqlite3ExprVectorSize(pExpr->pLeft))>1
   && sqlite3ExprVectorSize(pExpr->pRight)==nLeft
   && ( (pExpr->pLeft->flags & EP_xIsSelect)==0
     || (pExpr->pRight->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    ціле i;
    для(i=0; i<nLeft; i++){
      ціле idxNew;
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
  інакше якщо( pExpr->op==TK_IN
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
    ціле i;
    assert( pTerm->nChild==0 );
    для(i=0; i<sqlite3ExprVectorSize(pExpr->pLeft); i++){
      ціле idxNew;
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
  інакше якщо( pWC->op==TK_AND ){
    Expr *pRight = 0, *pLeft = 0;
    ціле res = isAuxiliaryVtabOperator(db, pExpr, &eOp2, &pLeft, &pRight);
    поки( res-- > 0 ){
      ціле idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereqExpr;

      prereqExpr = sqlite3WhereExprUsage(pMaskSet, pRight);
      prereqColumn = sqlite3WhereExprUsage(pMaskSet, pLeft);
      якщо( (prereqExpr & prereqColumn)==0 ){
        Expr *pNewExpr;
        pNewExpr = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3ExprDup(db, pRight, 0));
        якщо( ExprHasProperty(pExpr, EP_OuterON) && pNewExpr ){
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
void sqlite3WhereSplit(WhereClause *pWC, Expr *pExpr, u8 op){
  Expr *pE2 = sqlite3ExprSkipCollateAndLikely(pExpr);
  pWC->op = op;
  assert( pE2!=0 || pExpr==0 );
  якщо( pE2==0 ) повернути;
  якщо( pE2->op!=op ){
    whereClauseInsert(pWC, pExpr, 0);
  }інакше{
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
static void whereAddLimitExpr(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  ціле iReg,           /* Register that will hold value of the limit/offset */
  Expr *pExpr,        /* Expression that defines the limit/offset */
  ціле iCsr,           /* Cursor to which the constraint applies */
  ціле eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  Expr *pNew;
  ціле iVal = 0;

  якщо( sqlite3ExprIsInteger(pExpr, &iVal, pParse, 0) && iVal>=0 ){
    Expr *pVal = sqlite3ExprInt32(db, iVal);
    якщо( pVal==0 ) повернути;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }інакше{
    Expr *pVal = sqlite3ExprAlloc(db, TK_REGISTER, 0, 0);
    якщо( pVal==0 ) повернути;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  якщо( pNew ){
    WhereTerm *pTerm;
    ціле idx;
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
void SQLITE_NOINLINE sqlite3WhereAddLimit(WhereClause *pWC, Select *p){
  assert( p!=0 && p->pLimit!=0 );                 /* 1 -- checked by caller */
  якщо( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    ExprList *pOrderBy = p->pOrderBy;
    ціле iCsr = p->pSrc->a[0].iCursor;
    ціле ii;

    /* Check condition (4). Return early if it is not met. */
    для(ii=0; ii<pWC->nTerm; ii++){
      якщо( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        assert( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        assert( pWC->a[ii].eOperator==WO_ROWVAL );
        continue;
      }
      якщо( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        continue;
      }
      якщо( pWC->a[ii].leftCursor==iCsr && pWC->a[ii].prereqRight==0 ) continue;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      якщо( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        якщо( pParent->leftCursor==iCsr
         && pParent->prereqRight==0
         && pParent->nChild==1
        ){
          continue;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      повернути;
    }

    /* Check condition (5). Return early if it is not met. */
    якщо( pOrderBy ){
      для(ii=0; ii<pOrderBy->nExpr; ii++){
        Expr *pExpr = pOrderBy->a[ii].pExpr;
        якщо( pExpr->op!=TK_COLUMN ) повернути;
        якщо( pExpr->iTable!=iCsr ) повернути;
        якщо( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) повернути;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    assert( p->pLimit->op==TK_LIMIT );
    якщо( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      whereAddLimitExpr(pWC, p->iOffset, p->pLimit->pRight,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    якщо( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      whereAddLimitExpr(pWC, p->iLimit, p->pLimit->pLeft,
                        iCsr, SQLITE_INDEX_CONSTRAINT_LIMIT);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
void sqlite3WhereClauseInit(
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
void sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  assert( pWC->nTerm>=pWC->nBase );
  якщо( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    ціле i;
    /* Verify that every term past pWC->nBase is virtual */
    для(i=pWC->nBase; i<pWC->nTerm; i++){
      assert( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    поки(1){
      assert( a->eMatchOp==0 || a->eOperator==WO_AUX );
      якщо( a->wtFlags & TERM_DYNAMIC ){
        sqlite3ExprDelete(db, a->pExpr);
      }
      якщо( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        якщо( a->wtFlags & TERM_ORINFO ){
          assert( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }інакше{
          assert( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      якщо( a==aLast ) break;
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
static SQLITE_NOINLINE Bitmask sqlite3WhereExprUsageFull(
  WhereMaskSet *pMaskSet,
  Expr *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  якщо( p->pLeft ) mask |= sqlite3WhereExprUsageNN(pMaskSet, p->pLeft);
  якщо( p->pRight ){
    mask |= sqlite3WhereExprUsageNN(pMaskSet, p->pRight);
    assert( p->x.pList==0 );
  }інакше якщо( ExprUseXSelect(p) ){
    якщо( ExprHasProperty(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= exprSelectUsage(pMaskSet, p->x.pSelect);
  }інакше якщо( p->x.pList ){
    mask |= sqlite3WhereExprListUsage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  якщо( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && ExprUseYWin(p) ){
    assert( p->y.pWin!=0 );
    mask |= sqlite3WhereExprListUsage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3WhereExprListUsage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3WhereExprUsage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  повернути mask;
}
Bitmask sqlite3WhereExprUsageNN(WhereMaskSet *pMaskSet, Expr *p){
  якщо( p->op==TK_COLUMN && !ExprHasProperty(p, EP_FixedCol) ){
    повернути sqlite3WhereGetMask(pMaskSet, p->iTable);
  }інакше якщо( ExprHasProperty(p, EP_TokenOnly|EP_Leaf) ){
    assert( p->op!=TK_IF_NULL_ROW );
    повернути 0;
  }
  повернути sqlite3WhereExprUsageFull(pMaskSet, p);
}
Bitmask sqlite3WhereExprUsage(WhereMaskSet *pMaskSet, Expr *p){
  повернути p ? sqlite3WhereExprUsageNN(pMaskSet,p) : 0;
}
Bitmask sqlite3WhereExprListUsage(WhereMaskSet *pMaskSet, ExprList *pList){
  ціле i;
  Bitmask mask = 0;
  якщо( pList ){
    для(i=0; i<pList->nExpr; i++){
      mask |= sqlite3WhereExprUsage(pMaskSet, pList->a[i].pExpr);
    }
  }
  повернути mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
void sqlite3WhereExprAnalyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  ціле i;
  для(i=pWC->nTerm-1; i>=0; i--){
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
void sqlite3WhereTabFuncArgs(
  Parse *pParse,                    /* Parsing context */
  SrcItem *pItem,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  Table *pTab;
  ціле j, k;
  ExprList *pArgs;
  Expr *pColRef;
  Expr *pTerm;
  якщо( pItem->fg.isTabFunc==0 ) повернути;
  pTab = pItem->pSTab;
  assert( pTab!=0 );
  якщо( !IsVirtual(pTab) ){
    sqlite3ErrorMsg(pParse, "'%s' is not a function", pItem->zName);
    повернути;
  }
  pArgs = pItem->u1.pFuncArg;
  якщо( pArgs==0 ) повернути;
  для(j=k=0; j<pArgs->nExpr; j++){
    Expr *pRhs;
    u32 joinType;
    поки( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    якщо( k>=pTab->nCol ){
      sqlite3ErrorMsg(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      повернути;
    }
    pColRef = sqlite3ExprAlloc(pParse->db, TK_COLUMN, 0, 0);
    якщо( pColRef==0 ) повернути;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    assert( ExprUseYTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3ExprColUsed(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3ExprDup(pParse->db, pArgs->a[j].pExpr, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    якщо( pItem->fg.jointype & (JT_LEFT|JT_RIGHT) ){
      testcase( pItem->fg.jointype & JT_LEFT );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_RIGHT ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }інакше{
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
ціле sqlite3WhereLoopBloomable(const WhereLoop *pLoop){
  якщо( pLoop->wsFlags & WHERE_IPK ) повернути 1;
  якщо( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) повернути 0;
  повернути sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
