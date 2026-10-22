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
#شامل "sqliteInt.h"
#شامل "whereInt.h"

/* Forward declarations */
ایستا تهی عبارت_Analyze(SrcList*, WhereClause*, صحیح);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
ایستا تهی whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
ایستا تهی whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
ایستا صحیح where_Clause_درج(WhereClause *pWC, عبارت *p, u16 wtFlags){
  WhereTerm *pTerm;
  صحیح idx;
  testcase( wtFlags & TERM_VIRTUAL );
  اگر( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, اندازه(pWC->a[0])*pWC->nSlot*2 );
    اگر( pWC->a==0 ){
      اگر( wtFlags & TERM_DYNAMIC ){
        sqlite3_عبارت_Delete(db, p);
      }
      pWC->a = pOld;
      بازگشت 0;
    }
    memcpy(pWC->a, pOld, اندازه(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  اگر( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  اگر( p && عبارت_Has_Property(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }وگرنه{
    pTerm->truthProb = 1;
  }
  pTerm->p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         اندازه(WhereTerm) - offsetof(WhereTerm,eOperator));
  بازگشت idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
ایستا صحیح allowedOp(صحیح op){
  أكد( TK_GT>TK_EQ && TK_GT<TK_GE );
  أكد( TK_LT>TK_EQ && TK_LT<TK_GE );
  أكد( TK_LE>TK_EQ && TK_LE<TK_GE );
  أكد( TK_GE==TK_EQ+4 );
  أكد( TK_IN<TK_EQ );
  أكد( TK_IS<TK_EQ );
  أكد( TK_ISNULL<TK_EQ );
  اگر( op>TK_GE ) بازگشت 0;
  اگر( op>=TK_EQ ) بازگشت 1;
  بازگشت op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ایستا u16 عبارت_Commute(Parse *pParse, عبارت *p_عبارت){
  اگر( p_عبارت->p_چپ->op==TK_VECTOR
   || p_عبارت->p_راست->op==TK_VECTOR
   || sqlite3_دودویی_Compare_Coll_Seq(pParse, p_عبارت->p_چپ, p_عبارت->p_راست) !=
      sqlite3_دودویی_Compare_Coll_Seq(pParse, p_عبارت->p_راست, p_عبارت->p_چپ)
  ){
    p_عبارت->flags ^= EP_Commuted;
  }
  SWAP(عبارت*,p_عبارت->p_راست,p_عبارت->p_چپ);
  اگر( p_عبارت->op>=TK_GT ){
    أكد( TK_LT==TK_GT+2 );
    أكد( TK_GE==TK_LE+2 );
    أكد( TK_GT>TK_EQ );
    أكد( TK_GT<TK_LE );
    أكد( p_عبارت->op>=TK_GT && p_عبارت->op<=TK_GE );
    p_عبارت->op = ((p_عبارت->op-TK_GT)^2)+TK_GT;
  }
  بازگشت 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ایستا u16 operatorMask(صحیح op){
  u16 c;
  أكد( allowedOp(op) );
  اگر( op>=TK_EQ ){
    أكد( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }وگرنه اگر( op==TK_IN ){
    c = WO_IN;
  }وگرنه اگر( op==TK_ISNULL ){
    c = WO_ISNULL;
  }وگرنه{
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
  بازگشت c;
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
ایستا صحیح isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  عبارت *p_عبارت,      /* Test this expression */
  عبارت **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  صحیح *pisComplete, /* True if the only wildcard is % in the last character */
  صحیح *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ثابت u8 *z = 0;           /* String on RHS of LIKE operator */
  عبارت *p_راست, *p_چپ;      /* Right and left size of LIKE operator */
  عبارت_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  صحیح cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_مقدار *pVal = 0;
  صحیح op;                    /* Opcode of pRight */
  صحیح rc;                    /* Result code to return */

  اگر( !sqlite3IsLikeFunction(db, p_عبارت, pnoCase, (نویسه*)wc) ){
    بازگشت 0;
  }
#ifdef SQLITE_EBCDIC
  اگر( *pnoCase ) بازگشت 0;
#endif
  أكد( عبارت_Use_XList(p_عبارت) );
  pList = p_عبارت->x.pList;
  p_چپ = pList->a[1].p_عبارت;

  p_راست = sqlite3_عبارت_Skip_Collate(pList->a[0].p_عبارت);
  op = p_راست->op;
  اگر( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    صحیح iCol = p_راست->iColumn;
    pVal = sqlite3_Vdbe_Get_Bound_مقدار(pReprepare, iCol, SQLITE_AFF_BLOB);
    اگر( pVal && sqlite3_مقدار_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_مقدار_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    أكد( p_راست->op==TK_VARIABLE || p_راست->op==TK_REGISTER );
  }وگرنه اگر( op==TK_STRING ){
    أكد( !عبارت_Has_Property(p_راست, EP_Int_مقدار) );
     z = (u8*)p_راست->u.zToken;
  }
  اگر( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    وقتی( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      اگر( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }وگرنه اگر( c>=0x80 ){
        ثابت u8 *z2 = z+cnt-1;
        اگر( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          بشکن;
        }وگرنه{
          cnt = (صحیح)(z2-z);
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
    اگر( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      عبارت *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3_عبارت(db, TK_STRING, (نویسه*)z);
      اگر( pPrefix ){
        صحیح iFrom, iTo;
        نویسه *zNew;
        أكد( !عبارت_Has_Property(pPrefix, EP_Int_مقدار) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        برای(iFrom=iTo=0; iFrom<cnt; iFrom++){
          اگر( zNew[iFrom]==wc[3] ) iFrom++;
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
        اگر( p_چپ->op!=TK_COLUMN
         || sqlite3_عبارت_Affinity(p_چپ)!=SQLITE_AFF_TEXT
         || (ALWAYS( عبارت_Use_YTab(p_چپ) )
             && ALWAYS(p_چپ->y.pTab)
             && IsVirtual(p_چپ->y.pTab))  /* Might be numeric */
        ){
          صحیح is_رقم;
          دوگانه rDummy;
          أكد( zNew[iTo]==0 );
          is_رقم = sqlite3AtoF(zNew, &rDummy);
          اگر( is_رقم<=0 ){
            اگر( iTo==1 && zNew[0]=='-' ){
              is_رقم = +1;
            }وگرنه{
              zNew[iTo-1]++;
              is_رقم = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          اگر( is_رقم>0 ){
            sqlite3_عبارت_Delete(db, pPrefix);
            sqlite3_مقدار_Free(pVal);
            بازگشت 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      اگر( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_راست->iColumn, 0);
        أكد( !عبارت_Has_Property(p_راست, EP_Int_مقدار) );
        اگر( *pisComplete && p_راست->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          صحیح r1 = sqlite3GetTempReg(pParse);
          sqlite3_عبارت_Code_Target(pParse, p_راست, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }وگرنه{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_مقدار_Free(pVal);
  بازگشت rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
صحیح sqlite3_عبارت_Is_Like_Operator(ثابت عبارت *p_عبارت){
  ایستا ثابت ساختار {
    ثابت نویسه *zOp;
    unsigned نویسه eOp;
  } aOp[] = {
    { "match",  SQLITE_INDEX_CONSTRAINT_MATCH },
    { "glob",   SQLITE_INDEX_CONSTRAINT_GLOB },
    { "like",   SQLITE_INDEX_CONSTRAINT_LIKE },
    { "regexp", SQLITE_INDEX_CONSTRAINT_REGEXP }
  };
  صحیح i;
  أكد( p_عبارت->op==TK_FUNCTION );
  أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
  برای(i=0; i<ArraySize(aOp); i++){
    اگر( sqlite3StrICmp(p_عبارت->u.zToken, aOp[i].zOp)==0 ){
      بازگشت aOp[i].eOp;
    }
  }
  بازگشت 0;
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
ایستا صحیح isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  عبارت *p_عبارت,                    /* Test this expression */
  unsigned نویسه *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  عبارت **pp_چپ,                  /* Column expression to left of MATCH/op2 */
  عبارت **pp_راست                  /* Expression to left of MATCH/op2 */
){
  اگر( p_عبارت->op==TK_FUNCTION ){
    عبارت_List *pList;
    عبارت *pCol;                     /* Column reference */
    صحیح i;

    أكد( عبارت_Use_XList(p_عبارت) );
    pList = p_عبارت->x.pList;
    اگر( pList==0 || pList->n_عبارت!=2 ){
      بازگشت 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].p_عبارت;
    أكد( pCol->op!=TK_COLUMN || (عبارت_Use_YTab(pCol) && pCol->y.pTab!=0) );
    اگر( عبارت_Is_Vtab(pCol) && (i = sqlite3_عبارت_Is_Like_Operator(p_عبارت))!=0 ){
      *peOp2 = i;
      *pp_راست = pList->a[0].p_عبارت;
      *pp_چپ = pCol;
      بازگشت 1;
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
    pCol = pList->a[0].p_عبارت;
    أكد( pCol->op!=TK_COLUMN || عبارت_Use_YTab(pCol) );
    أكد( pCol->op!=TK_COLUMN || (عبارت_Use_YTab(pCol) && pCol->y.pTab!=0) );
    اگر( عبارت_Is_Vtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      تهی (*xNotUsed)(sqlite3_context*,صحیح,sqlite3_مقدار**);
      تهی *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      أكد( pVtab!=0 );
      أكد( pVtab->pModule!=0 );
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      pMod = (sqlite3_module *)pVtab->pModule;
      اگر( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, p_عبارت->u.zToken, &xNotUsed, &pNotUsed);
        اگر( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_راست = pList->a[1].p_عبارت;
          *pp_چپ = pCol;
          بازگشت 1;
        }
      }
    }
  }وگرنه اگر( p_عبارت->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    أكد( TK_NE < TK_EQ );
    أكد( TK_ISNOT < TK_EQ );
    أكد( TK_NOTNULL < TK_EQ );
    بازگشت 0;
  }وگرنه اگر( p_عبارت->op==TK_NE || p_عبارت->op==TK_ISNOT || p_عبارت->op==TK_NOTNULL ){
    صحیح res = 0;
    عبارت *p_چپ = p_عبارت->p_چپ;
    عبارت *p_راست = p_عبارت->p_راست;
    أكد( p_چپ->op!=TK_COLUMN || (عبارت_Use_YTab(p_چپ) && p_چپ->y.pTab!=0) );
    اگر( عبارت_Is_Vtab(p_چپ) ){
      res++;
    }
    أكد( p_راست==0 || p_راست->op!=TK_COLUMN
            || (عبارت_Use_YTab(p_راست) && p_راست->y.pTab!=0) );
    اگر( p_راست && عبارت_Is_Vtab(p_راست) ){
      res++;
      SWAP(عبارت*, p_چپ, p_راست);
    }
    *pp_چپ = p_چپ;
    *pp_راست = p_راست;
    اگر( p_عبارت->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    اگر( p_عبارت->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    اگر( p_عبارت->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    بازگشت res;
  }
  بازگشت 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ایستا تهی transferJoinMarkings(عبارت *pDerived, عبارت *pBase){
  اگر( pDerived && عبارت_Has_Property(pBase, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= pBase->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
ایستا تهی markTermAsChild(WhereClause *pWC, صحیح iChild, صحیح iParent){
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
ایستا WhereTerm *whereNthSubterm(WhereTerm *pTerm, صحیح N){
  اگر( pTerm->eOperator!=WO_AND ){
    بازگشت N==0 ? pTerm : 0;
  }
  اگر( N<pTerm->u.pAndInfo->wc.nTerm ){
    بازگشت &pTerm->u.pAndInfo->wc.a[N];
  }
  بازگشت 0;
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
ایستا تهی whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  عبارت *pNew;            /* New virtual expression */
  صحیح op;                /* Operator for the combined expression */
  صحیح idxNew;            /* Index in pWC of the next virtual term */
  عبارت *pA, *pB;         /* Expressions associated with pOne and pTwo */

  اگر( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) بازگشت;
  اگر( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) بازگشت;
  اگر( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) بازگشت;
  اگر( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) بازگشت;
  pA = pOne->p_عبارت;
  pB = pTwo->p_عبارت;
  أكد( pA->p_چپ!=0 && pA->p_راست!=0 );
  أكد( pB->p_چپ!=0 && pB->p_راست!=0 );
  اگر( sqlite3_عبارت_Compare(0,pA->p_چپ, pB->p_چپ, -1) )  بازگشت;
  اگر( sqlite3_عبارت_Compare(0,pA->p_راست, pB->p_راست,-1) ) بازگشت;
  اگر( عبارت_Has_Property(pA,EP_Commuted)!=عبارت_Has_Property(pB,EP_Commuted) ){
    بازگشت;
  }
  /* If we reach this point, it means the two subterms can be combined */
  اگر( (eOp & (eOp-1))!=0 ){
    اگر( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }وگرنه{
      أكد( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_عبارت_Dup(db, pA, 0);
  اگر( pNew==0 ) بازگشت;
  برای(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ أكد( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_درج(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  عبارت_Analyze(pSrc, pWC, idxNew);
}

#اگر !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
ایستا تهی عبارت_Analyze_Or_Term(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  صحیح idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  عبارت *p_عبارت = pTerm->p_عبارت;             /* The expression of the term */
  صحیح i;                                  /* Loop counters */
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
  أكد( p_عبارت->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, اندازه(*pOrInfo));
  اگر( pOrInfo==0 ) بازگشت;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, اندازه(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, p_عبارت, TK_OR);
  sqlite3_Where_عبارت_Analyze(pSrc, pOrWc);
  اگر( db->mallocFailed ) بازگشت;
  أكد( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  برای(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    اگر( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      أكد( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, اندازه(*pAndInfo));
      اگر( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        صحیح j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->چپ_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, اندازه(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->p_عبارت, TK_AND);
        sqlite3_Where_عبارت_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        اگر( !db->mallocFailed ){
          برای(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            أكد( pAndTerm->p_عبارت );
            اگر( allowedOp(pAndTerm->p_عبارت->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->چپ_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }وگرنه اگر( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }وگرنه{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->چپ_Cursor);
      اگر( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->چپ_Cursor);
      }
      indexable &= b;
      اگر( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }وگرنه{
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
  pTerm->چپ_Cursor = -1;
  اگر( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  اگر( indexable && pOrWc->nTerm==2 ){
    صحیح iOne = 0;
    WhereTerm *pOne;
    وقتی( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      صحیح iTwo = 0;
      WhereTerm *pTwo;
      وقتی( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  اگر( chngToIN ){
    صحیح okToChngToIN = 0;     /* True if the conversion to IN is valid */
    صحیح iColumn = -1;         /* Column index on lhs of IN operator */
    صحیح iCursor = -1;         /* Table cursor common to all terms */
    صحیح j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    برای(j=0; j<2 && !okToChngToIN; j++){
      عبارت *p_چپ = 0;
      pOrTerm = pOrWc->a;
      برای(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        أكد( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        اگر( pOrTerm->چپ_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          أكد( j==1 );
          ادامه;
        }
        اگر( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->چپ_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          أكد( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          ادامه;
        }
        أكد( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        iColumn = pOrTerm->u.x.چپ_Column;
        iCursor = pOrTerm->چپ_Cursor;
        p_چپ = pOrTerm->p_عبارت->p_چپ;
        بشکن;
      }
      اگر( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        أكد( j==1 );
        أكد( IsPowerOfTwo(chngToIN) );
        أكد( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        بشکن;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      برای(; i>=0 && okToChngToIN; i--, pOrTerm++){
        أكد( pOrTerm->eOperator & WO_EQ );
        أكد( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        اگر( pOrTerm->چپ_Cursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }وگرنه اگر( pOrTerm->u.x.چپ_Column!=iColumn || (iColumn==XN_عبارت
               && sqlite3_عبارت_Compare(pParse, pOrTerm->p_عبارت->p_چپ, p_چپ, -1)
        )){
          okToChngToIN = 0;
        }وگرنه{
          صحیح aff_چپ, aff_راست;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_راست = sqlite3_عبارت_Affinity(pOrTerm->p_عبارت->p_راست);
          aff_چپ = sqlite3_عبارت_Affinity(pOrTerm->p_عبارت->p_چپ);
          اگر( aff_راست!=0 && aff_راست!=aff_چپ ){
            okToChngToIN = 0;
          }وگرنه{
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
    اگر( okToChngToIN ){
      عبارت *pDup;            /* A transient duplicate expression */
      عبارت_List *pList = 0;   /* The RHS of the IN operator */
      عبارت *p_چپ = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      عبارت *pNew;            /* The complete IN operator */

      برای(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        عبارت *pThis;
        اگر( (pOrTerm->wtFlags & TERM_OK)==0 ) ادامه;
        أكد( pOrTerm->eOperator & WO_EQ );
        أكد( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        أكد( pOrTerm->چپ_Cursor==iCursor );
        أكد( pOrTerm->u.x.چپ_Column==iColumn );
        pThis = pOrTerm->p_عبارت;
        pDup = sqlite3_عبارت_Dup(db, pThis->p_راست, 0);
        pList = sqlite3_عبارت_List_Append(pWInfo->pParse, pList, pDup);
        اگر( p_چپ==0 ){
          p_چپ = pThis->p_چپ;
          pCollSeq = sqlite3_عبارت_Compare_Coll_Seq(pParse, pThis);
        }وگرنه{
          أكد( 0==sqlite3_عبارت_Compare(pParse, 
                        sqlite3_عبارت_Skip_Collate(pThis->p_چپ),
                        sqlite3_عبارت_Skip_Collate(p_چپ), -1) );
          اگر( pCollSeq!=sqlite3_عبارت_Compare_Coll_Seq(pParse, pThis) ){
            p_چپ = 0;  /* Collating sequence mismatch */
            بشکن;
          }
        }
      }
      اگر( p_چپ==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }وگرنه{
        pDup = sqlite3_عبارت_Dup(db, p_چپ, 0);
        اگر( sqlite3_عبارت_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          أكد( pCollSeq->zName!=0 );
          pDup = sqlite3_عبارت_Add_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      اگر( pNew ){
        صحیح idxNew;
        transferJoinMarkings(pNew, p_عبارت);
        أكد( عبارت_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_درج(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        عبارت_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }وگرنه{
        sqlite3_عبارت_List_Delete(db, pList);
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
ایستا صحیح termIsEquivalence(Parse *pParse, عبارت *p_عبارت, SrcList *pSrc){
  نویسه aff1, aff2;
  اگر( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) بازگشت 0;  /* (1) */
  اگر( p_عبارت->op!=TK_EQ && p_عبارت->op!=TK_IS ) بازگشت 0;                 /* (2) */
  اگر( عبارت_Has_Property(p_عبارت, EP_OuterON|EP_Collate) ) بازگشت 0;        /* (3) */
  أكد( pSrc!=0 );
  اگر( p_عبارت->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    بازگشت 0;                                                          /* (4) */
  }
  aff1 = sqlite3_عبارت_Affinity(p_عبارت->p_چپ);
  aff2 = sqlite3_عبارت_Affinity(p_عبارت->p_راست);
  اگر( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    بازگشت 0;                                                          /* (5) */
  }
  اگر( !sqlite3_عبارت_Coll_Seq_Match(pParse, p_عبارت->p_چپ, p_عبارت->p_راست) ){
    بازگشت 0;                                                          /* (6) */
  }
  بازگشت 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ایستا Bitmask عبارت_Select_Usage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  وقتی( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3_Where_عبارت_List_Usage(pMaskSet, pS->pEList);
    mask |= sqlite3_Where_عبارت_List_Usage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3_Where_عبارت_List_Usage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3_Where_عبارت_Usage(pMaskSet, pS->pWhere);
    mask |= sqlite3_Where_عبارت_Usage(pMaskSet, pS->pHaving);
    اگر( ALWAYS(pSrc!=0) ){
      صحیح i;
      برای(i=0; i<pSrc->nSrc; i++){
        اگر( pSrc->a[i].fg.isSubquery ){
          mask |= عبارت_Select_Usage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        اگر( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3_Where_عبارت_Usage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        اگر( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3_Where_عبارت_List_Usage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  بازگشت mask;
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
ایستا SQLITE_NOINLINE صحیح عبارت_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  صحیح *aiCurCol,         /* Write the referenced table cursor and column here */
  عبارت *p_عبارت,           /* An operand of a comparison operator */
  صحیح j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  صحیح i;
  صحیح iCur;
  انجامبده{
    iCur = pFrom->a[j].iCursor;
    برای(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      اگر( pIdx->a_Col_عبارت==0 ) ادامه;
      برای(i=0; i<pIdx->n_کلید_Col; i++){
        اگر( pIdx->aiColumn[i]!=XN_عبارت ) ادامه;
        أكد( pIdx->b_Has_عبارت );
        اگر( sqlite3_عبارت_Compare_Skip(p_عبارت,pIdx->a_Col_عبارت->a[i].p_عبارت,iCur)==0
         && !sqlite3_عبارت_Is_Constant(0,pIdx->a_Col_عبارت->a[i].p_عبارت)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_عبارت;
          بازگشت 1;
        }
      }
    }
  }وقتی( ++j < pFrom->nSrc );
  بازگشت 0;
}
ایستا صحیح عبارت_Might_Be_Indexed(
  SrcList *pFrom,        /* The FROM clause */
  صحیح *aiCurCol,         /* Write the referenced table cursor & column here */
  عبارت *p_عبارت,           /* An operand of a comparison operator */
  صحیح op                 /* The specific comparison operator */
){
  صحیح i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  أكد( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  أكد( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  أكد( op<=TK_GE );
  اگر( p_عبارت->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    أكد( عبارت_Use_XList(p_عبارت) );
    p_عبارت = p_عبارت->x.pList->a[0].p_عبارت;
  }

  اگر( p_عبارت->op==TK_COLUMN ){
    aiCurCol[0] = p_عبارت->iTable;
    aiCurCol[1] = p_عبارت->iColumn;
    بازگشت 1;
  }

  برای(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    برای(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      اگر( pIdx->a_Col_عبارت ){
        بازگشت عبارت_Might_Be_Indexed2(pFrom,aiCurCol,p_عبارت,i);
      }
    }
  }
  بازگشت 0;
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
ایستا تهی عبارت_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  صحیح idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  عبارت *p_عبارت;                     /* The expression to be analyzed */
  Bitmask prereq_چپ;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_راست = 0;          /* Extra dependencies on LEFT JOIN */
  عبارت *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  صحیح isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  صحیح noCase = 0;                  /* uppercase equivalent to lowercase */
  صحیح op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned نویسه eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  صحیح n_چپ;                       /* Number of elements on left side vector */

  اگر( db->mallocFailed ){
    بازگشت;
  }
  أكد( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  p_عبارت = pTerm->p_عبارت;
  أكد( p_عبارت!=0 ); /* Because malloc() has not failed */
  أكد( p_عبارت->op!=TK_AS && p_عبارت->op!=TK_COLLATE );
عبارت_Analyze_restart:
  pMaskSet->bVarSelect = 0;
  prereq_چپ = sqlite3_Where_عبارت_Usage(pMaskSet, p_عبارت->p_چپ);
  op = p_عبارت->op;
  اگر( op==TK_IN ){
    أكد( p_عبارت->p_راست==0 );
    اگر( sqlite3_عبارت_Check_IN(pParse, p_عبارت) ) بازگشت;
    اگر( عبارت_Use_XSelect(p_عبارت) ){
      pTerm->prereq_راست = عبارت_Select_Usage(pMaskSet, p_عبارت->x.pSelect);
    }وگرنه{
      pTerm->prereq_راست = sqlite3_Where_عبارت_List_Usage(pMaskSet, p_عبارت->x.pList);
    }
    prereqAll = prereq_چپ | pTerm->prereq_راست;
  }وگرنه{
    pTerm->prereq_راست = sqlite3_Where_عبارت_Usage(pMaskSet, p_عبارت->p_راست);
    اگر( p_عبارت->p_چپ==0
     || عبارت_Has_Property(p_عبارت, EP_xIsSelect|EP_IfNullRow)
     || p_عبارت->x.pList!=0
    ){
      prereqAll = sqlite3_Where_عبارت_Usage_NN(pMaskSet, p_عبارت);
    }وگرنه{
      prereqAll = prereq_چپ | pTerm->prereq_راست;
    }
  }
  اگر( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  اگر( prereqAll!=sqlite3_Where_عبارت_Usage_NN(pMaskSet, p_عبارت) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_درخت_View_عبارت(0,p_عبارت,0);
    أكد( 0 );
  }
#endif

  اگر( عبارت_Has_Property(p_عبارت, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, p_عبارت->w.iJoin);
    اگر( عبارت_Has_Property(p_عبارت, EP_OuterON) ){
      prereqAll |= x;
      extra_راست = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }وگرنه اگر( (prereqAll>>1)>=x ){
      عبارت_Clear_Property(p_عبارت, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->چپ_Cursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  اگر( allowedOp(op) ){
    صحیح aiCurCol[2];
    عبارت *p_چپ = sqlite3_عبارت_Skip_Collate(p_عبارت->p_چپ);
    عبارت *p_راست = sqlite3_عبارت_Skip_Collate(p_عبارت->p_راست);
    u16 opMask = (pTerm->prereq_راست & prereq_چپ)==0 ? WO_ALL : WO_EQUIV;

    اگر( pTerm->u.x.iField>0 ){
      أكد( op==TK_IN );
      أكد( p_چپ->op==TK_VECTOR );
      أكد( عبارت_Use_XList(p_چپ) );
      p_چپ = p_چپ->x.pList->a[pTerm->u.x.iField-1].p_عبارت;
    }

    اگر( عبارت_Might_Be_Indexed(pSrc, aiCurCol, p_چپ, op) ){
      pTerm->چپ_Cursor = aiCurCol[0];
      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.چپ_Column = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    اگر( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    اگر( p_راست
     && عبارت_Might_Be_Indexed(pSrc, aiCurCol, p_راست, op)
     && !عبارت_Has_Property(p_راست, EP_FixedCol)
    ){
      WhereTerm *pNew;
      عبارت *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      أكد( pTerm->u.x.iField==0 );
      اگر( pTerm->چپ_Cursor>=0 ){
        صحیح idxNew;
        pDup = sqlite3_عبارت_Dup(db, p_عبارت, 0);
        اگر( db->mallocFailed ){
          sqlite3_عبارت_Delete(db, pDup);
          بازگشت;
        }
        idxNew = where_Clause_درج(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        اگر( idxNew==0 ) بازگشت;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        اگر( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        أكد( pWInfo->pTabList!=0 );
        اگر( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }وگرنه{
        pDup = p_عبارت;
        pNew = pTerm;
      }
      pNew->wtFlags |= عبارت_Commute(pParse, pDup);
      pNew->چپ_Cursor = aiCurCol[0];
      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.چپ_Column = aiCurCol[1];
      testcase( (prereq_چپ | extra_راست) != prereq_چپ );
      pNew->prereq_راست = prereq_چپ | extra_راست;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }وگرنه
    اگر( op==TK_ISNULL
     && !عبارت_Has_Property(p_عبارت,EP_OuterON)
     && 0==sqlite3_عبارت_Can_Be_Null(p_چپ)
    ){
      أكد( !عبارت_Has_Property(p_عبارت, EP_Int_مقدار) );
      p_عبارت->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_عبارت->u.zToken = "false";
      عبارت_Set_Property(p_عبارت, EP_IsFalse);
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
  وگرنه اگر( p_عبارت->op==TK_BETWEEN && pWC->op==TK_AND ){
    عبارت_List *pList;
    صحیح i;
    ایستا ثابت u8 ops[] = {TK_GE, TK_LE};
    أكد( عبارت_Use_XList(p_عبارت) );
    pList = p_عبارت->x.pList;
    أكد( pList!=0 );
    أكد( pList->n_عبارت==2 );
    أكد( pWC->a[idxTerm].nChild==0 );
    برای(i=0; i<2; i++){
      عبارت *p_New_عبارت;
      صحیح idxNew;
      p_New_عبارت = sqlite3PExpr(pParse, ops[i],
                             sqlite3_عبارت_Dup(db, p_عبارت->p_چپ, 0),
                             sqlite3_عبارت_Dup(db, pList->a[i].p_عبارت, 0));
      transferJoinMarkings(p_New_عبارت, p_عبارت);
      idxNew = where_Clause_درج(pWC, p_New_عبارت, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      عبارت_Analyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#اگر !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  وگرنه اگر( p_عبارت->op==TK_OR && !عبارت_Has_Property(p_عبارت, EP_Collate) ){
    عبارت *pAlt = sqlite3_عبارت_Simplified_And_Or(p_عبارت);
    اگر( pAlt!=p_عبارت ){
      pTerm->p_عبارت = p_عبارت = sqlite3_عبارت_Skip_Collate_And_Likely(pAlt);
      برو عبارت_Analyze_restart;
    }
    أكد( pWC->op==TK_AND );
    عبارت_Analyze_Or_Term(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  وگرنه اگر( p_عبارت->op==TK_NOTNULL ){
    اگر( p_عبارت->p_چپ->op==TK_COLUMN
     && p_عبارت->p_چپ->iColumn>=0
     && !عبارت_Has_Property(p_عبارت, EP_OuterON)
    ){
      عبارت *p_New_عبارت;
      عبارت *p_چپ = p_عبارت->p_چپ;
      صحیح idxNew;
      WhereTerm *pNewTerm;
 
      p_New_عبارت = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_عبارت_Dup(db, p_چپ, 0),
                              sqlite3_عبارت_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_درج(pWC, p_New_عبارت,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      اگر( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_راست = 0;
        pNewTerm->چپ_Cursor = p_چپ->iTable;
        pNewTerm->u.x.چپ_Column = p_چپ->iColumn;
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
  وگرنه اگر( p_عبارت->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_عبارت, &pStr1, &isComplete, &noCase)
  ){
    عبارت *p_چپ;       /* LHS of LIKE/GLOB operator */
    عبارت *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    عبارت *pNewExpr1;
    عبارت *pNewExpr2;
    صحیح idxNew1;
    صحیح idxNew2;
    ثابت نویسه *zCollSeqName;     /* Name of collating sequence */
    ثابت u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    أكد( عبارت_Use_XList(p_عبارت) );
    p_چپ = p_عبارت->x.pList->a[1].p_عبارت;
    pStr2 = sqlite3_عبارت_Dup(db, pStr1, 0);
    أكد( pStr1==0 || !عبارت_Has_Property(pStr1, EP_Int_مقدار) );
    أكد( pStr2==0 || !عبارت_Has_Property(pStr2, EP_Int_مقدار) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    اگر( noCase && !pParse->db->mallocFailed ){
      صحیح i;
      نویسه c;
      pTerm->wtFlags |= TERM_LIKE;
      برای(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    اگر( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      اگر( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        اگر( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      وقتی( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      أكد( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_دودویی;
    pNewExpr1 = sqlite3_عبارت_Dup(db, p_چپ, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_عبارت_Add_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_عبارت);
    idxNew1 = where_Clause_درج(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_عبارت_Dup(db, p_چپ, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_عبارت_Add_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_عبارت);
    idxNew2 = where_Clause_درج(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    عبارت_Analyze(pSrc, pWC, idxNew1);
    عبارت_Analyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    اگر( isComplete ){
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
  اگر( (p_عبارت->op==TK_EQ || p_عبارت->op==TK_IS)
   && (n_چپ = sqlite3_عبارت_Vector_Size(p_عبارت->p_چپ))>1
   && sqlite3_عبارت_Vector_Size(p_عبارت->p_راست)==n_چپ
   && ( (p_عبارت->p_چپ->flags & EP_xIsSelect)==0
     || (p_عبارت->p_راست->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    صحیح i;
    برای(i=0; i<n_چپ; i++){
      صحیح idxNew;
      عبارت *pNew;
      عبارت *p_چپ = sqlite3_عبارت_For_Vector_Field(pParse, p_عبارت->p_چپ, i, n_چپ);
      عبارت *p_راست = sqlite3_عبارت_For_Vector_Field(pParse, p_عبارت->p_راست, i, n_چپ);

      pNew = sqlite3PExpr(pParse, p_عبارت->op, p_چپ, p_راست);
      transferJoinMarkings(pNew, p_عبارت);
      idxNew = where_Clause_درج(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      عبارت_Analyze(pSrc, pWC, idxNew);
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
  وگرنه اگر( p_عبارت->op==TK_IN
   && pTerm->u.x.iField==0
   && p_عبارت->p_چپ->op==TK_VECTOR
   && ALWAYS( عبارت_Use_XSelect(p_عبارت) )
   && (p_عبارت->x.pSelect->pPrior==0 || (p_عبارت->x.pSelect->selFlags & SF_مقادیر))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p_عبارت->x.pSelect->pWin==0
#endif
   && (p_عبارت->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_عبارت->x.pSelect->pEList->n_عبارت <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    صحیح i;
    أكد( pTerm->nChild==0 );
    برای(i=0; i<sqlite3_عبارت_Vector_Size(p_عبارت->p_چپ); i++){
      صحیح idxNew;
      idxNew = where_Clause_درج(pWC, p_عبارت, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      عبارت_Analyze(pSrc, pWC, idxNew);
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
  وگرنه اگر( pWC->op==TK_AND ){
    عبارت *p_راست = 0, *p_چپ = 0;
    صحیح res = isAuxiliaryVtabOperator(db, p_عبارت, &eOp2, &p_چپ, &p_راست);
    وقتی( res-- > 0 ){
      صحیح idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereq_عبارت;

      prereq_عبارت = sqlite3_Where_عبارت_Usage(pMaskSet, p_راست);
      prereqColumn = sqlite3_Where_عبارت_Usage(pMaskSet, p_چپ);
      اگر( (prereq_عبارت & prereqColumn)==0 ){
        عبارت *p_New_عبارت;
        p_New_عبارت = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3_عبارت_Dup(db, p_راست, 0));
        اگر( عبارت_Has_Property(p_عبارت, EP_OuterON) && p_New_عبارت ){
          عبارت_Set_Property(p_New_عبارت, EP_OuterON);
          p_New_عبارت->w.iJoin = p_عبارت->w.iJoin;
        }
        idxNew = where_Clause_درج(pWC, p_New_عبارت, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_راست = prereq_عبارت | extra_راست;
        pNewTerm->چپ_Cursor = p_چپ->iTable;
        pNewTerm->u.x.چپ_Column = p_چپ->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(عبارت*, p_چپ, p_راست);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereq_راست |= extra_راست;
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
تهی sqlite3WhereSplit(WhereClause *pWC, عبارت *p_عبارت, u8 op){
  عبارت *pE2 = sqlite3_عبارت_Skip_Collate_And_Likely(p_عبارت);
  pWC->op = op;
  أكد( pE2!=0 || p_عبارت==0 );
  اگر( pE2==0 ) بازگشت;
  اگر( pE2->op!=op ){
    where_Clause_درج(pWC, p_عبارت, 0);
  }وگرنه{
    sqlite3WhereSplit(pWC, pE2->p_چپ, op);
    sqlite3WhereSplit(pWC, pE2->p_راست, op);
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
ایستا تهی where_Add_Limit_عبارت(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  صحیح iReg,           /* Register that will hold value of the limit/offset */
  عبارت *p_عبارت,        /* Expression that defines the limit/offset */
  صحیح iCsr,           /* Cursor to which the constraint applies */
  صحیح eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  عبارت *pNew;
  صحیح iVal = 0;

  اگر( sqlite3_عبارت_Is_Integer(p_عبارت, &iVal, pParse, 0) && iVal>=0 ){
    عبارت *pVal = sqlite3_عبارت_Int32(db, iVal);
    اگر( pVal==0 ) بازگشت;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }وگرنه{
    عبارت *pVal = sqlite3_عبارت_Alloc(db, TK_REGISTER, 0, 0);
    اگر( pVal==0 ) بازگشت;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  اگر( pNew ){
    WhereTerm *pTerm;
    صحیح idx;
    idx = where_Clause_درج(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->چپ_Cursor = iCsr;
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
تهی SQLITE_NOINLINE sqlite3WhereAddLimit(WhereClause *pWC, Select *p){
  أكد( p!=0 && p->pLimit!=0 );                 /* 1 -- checked by caller */
  اگر( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    عبارت_List *pOrderBy = p->pOrderBy;
    صحیح iCsr = p->pSrc->a[0].iCursor;
    صحیح ii;

    /* Check condition (4). Return early if it is not met. */
    برای(ii=0; ii<pWC->nTerm; ii++){
      اگر( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        أكد( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        أكد( pWC->a[ii].eOperator==WO_ROWVAL );
        ادامه;
      }
      اگر( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        ادامه;
      }
      اگر( pWC->a[ii].چپ_Cursor==iCsr && pWC->a[ii].prereq_راست==0 ) ادامه;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      اگر( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        اگر( pParent->چپ_Cursor==iCsr
         && pParent->prereq_راست==0
         && pParent->nChild==1
        ){
          ادامه;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      بازگشت;
    }

    /* Check condition (5). Return early if it is not met. */
    اگر( pOrderBy ){
      برای(ii=0; ii<pOrderBy->n_عبارت; ii++){
        عبارت *p_عبارت = pOrderBy->a[ii].p_عبارت;
        اگر( p_عبارت->op!=TK_COLUMN ) بازگشت;
        اگر( p_عبارت->iTable!=iCsr ) بازگشت;
        اگر( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) بازگشت;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    أكد( p->pLimit->op==TK_LIMIT );
    اگر( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_عبارت(pWC, p->iOffset, p->pLimit->p_راست,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    اگر( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_عبارت(pWC, p->iLimit, p->pLimit->p_چپ,
                        iCsr, SQLITE_INDEX_CONSTRAINT_LIMIT);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
تهی sqlite3WhereClauseInit(
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
تهی sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  أكد( pWC->nTerm>=pWC->nBase );
  اگر( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    صحیح i;
    /* Verify that every term past pWC->nBase is virtual */
    برای(i=pWC->nBase; i<pWC->nTerm; i++){
      أكد( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    وقتی(1){
      أكد( a->eMatchOp==0 || a->eOperator==WO_AUX );
      اگر( a->wtFlags & TERM_DYNAMIC ){
        sqlite3_عبارت_Delete(db, a->p_عبارت);
      }
      اگر( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        اگر( a->wtFlags & TERM_ORINFO ){
          أكد( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }وگرنه{
          أكد( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      اگر( a==aLast ) بشکن;
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
ایستا SQLITE_NOINLINE Bitmask sqlite3_Where_عبارت_Usage_Full(
  WhereMaskSet *pMaskSet,
  عبارت *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  اگر( p->p_چپ ) mask |= sqlite3_Where_عبارت_Usage_NN(pMaskSet, p->p_چپ);
  اگر( p->p_راست ){
    mask |= sqlite3_Where_عبارت_Usage_NN(pMaskSet, p->p_راست);
    أكد( p->x.pList==0 );
  }وگرنه اگر( عبارت_Use_XSelect(p) ){
    اگر( عبارت_Has_Property(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= عبارت_Select_Usage(pMaskSet, p->x.pSelect);
  }وگرنه اگر( p->x.pList ){
    mask |= sqlite3_Where_عبارت_List_Usage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  اگر( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && عبارت_Use_YWin(p) ){
    أكد( p->y.pWin!=0 );
    mask |= sqlite3_Where_عبارت_List_Usage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3_Where_عبارت_List_Usage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3_Where_عبارت_Usage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  بازگشت mask;
}
Bitmask sqlite3_Where_عبارت_Usage_NN(WhereMaskSet *pMaskSet, عبارت *p){
  اگر( p->op==TK_COLUMN && !عبارت_Has_Property(p, EP_FixedCol) ){
    بازگشت sqlite3WhereGetMask(pMaskSet, p->iTable);
  }وگرنه اگر( عبارت_Has_Property(p, EP_TokenOnly|EP_Leaf) ){
    أكد( p->op!=TK_IF_NULL_ROW );
    بازگشت 0;
  }
  بازگشت sqlite3_Where_عبارت_Usage_Full(pMaskSet, p);
}
Bitmask sqlite3_Where_عبارت_Usage(WhereMaskSet *pMaskSet, عبارت *p){
  بازگشت p ? sqlite3_Where_عبارت_Usage_NN(pMaskSet,p) : 0;
}
Bitmask sqlite3_Where_عبارت_List_Usage(WhereMaskSet *pMaskSet, عبارت_List *pList){
  صحیح i;
  Bitmask mask = 0;
  اگر( pList ){
    برای(i=0; i<pList->n_عبارت; i++){
      mask |= sqlite3_Where_عبارت_Usage(pMaskSet, pList->a[i].p_عبارت);
    }
  }
  بازگشت mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
تهی sqlite3_Where_عبارت_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  صحیح i;
  برای(i=pWC->nTerm-1; i>=0; i--){
    عبارت_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
تهی sqlite3WhereTabFuncArgs(
  Parse *pParse,                    /* Parsing context */
  SrcItem *pItem,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  Table *pTab;
  صحیح j, k;
  عبارت_List *pArgs;
  عبارت *pColRef;
  عبارت *pTerm;
  اگر( pItem->fg.isTabFunc==0 ) بازگشت;
  pTab = pItem->pSTab;
  أكد( pTab!=0 );
  اگر( !IsVirtual(pTab) ){
    sqlite3_Error_پیام(pParse, "'%s' is not a function", pItem->zName);
    بازگشت;
  }
  pArgs = pItem->u1.pFuncArg;
  اگر( pArgs==0 ) بازگشت;
  برای(j=k=0; j<pArgs->n_عبارت; j++){
    عبارت *pRhs;
    u32 joinType;
    وقتی( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    اگر( k>=pTab->nCol ){
      sqlite3_Error_پیام(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      بازگشت;
    }
    pColRef = sqlite3_عبارت_Alloc(pParse->db, TK_COLUMN, 0, 0);
    اگر( pColRef==0 ) بازگشت;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    أكد( عبارت_Use_YTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3_عبارت_Col_Used(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_عبارت_Dup(pParse->db, pArgs->a[j].p_عبارت, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    اگر( pItem->fg.jointype & (JT_چپ|JT_راست) ){
      testcase( pItem->fg.jointype & JT_چپ );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_راست ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }وگرنه{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_Set_Join_عبارت(pTerm, pItem->iCursor, joinType);
    where_Clause_درج(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
صحیح sqlite3WhereLoopBloomable(ثابت WhereLoop *pLoop){
  اگر( pLoop->wsFlags & WHERE_IPK ) بازگشت 1;
  اگر( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) بازگشت 0;
  بازگشت sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
