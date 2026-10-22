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
ساكن فراغ تعبير_Analyze(SrcList*, WhereClause*, صحيح);

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
ساكن صحيح where_Clause_أدرج(WhereClause *pWC, تعبير *p, u16 wtFlags){
  WhereTerm *pTerm;
  صحيح idx;
  testcase( wtFlags & TERM_VIRTUAL );
  إذا( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, حجم(pWC->a[0])*pWC->nSlot*2 );
    إذا( pWC->a==0 ){
      إذا( wtFlags & TERM_DYNAMIC ){
        sqlite3_تعبير_Delete(db, p);
      }
      pWC->a = pOld;
      إرجاع 0;
    }
    memcpy(pWC->a, pOld, حجم(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  إذا( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  إذا( p && تعبير_Has_Property(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }وإلا{
    pTerm->truthProb = 1;
  }
  pTerm->p_تعبير = sqlite3_تعبير_Skip_Collate_And_Likely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         حجم(WhereTerm) - offsetof(WhereTerm,eOperator));
  إرجاع idx;
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
  إذا( op>TK_GE ) إرجاع 0;
  إذا( op>=TK_EQ ) إرجاع 1;
  إرجاع op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
ساكن u16 تعبير_Commute(Parse *pParse, تعبير *p_تعبير){
  إذا( p_تعبير->p_يسار->op==TK_VECTOR
   || p_تعبير->p_يمين->op==TK_VECTOR
   || sqlite3_ثنائي_Compare_Coll_Seq(pParse, p_تعبير->p_يسار, p_تعبير->p_يمين) !=
      sqlite3_ثنائي_Compare_Coll_Seq(pParse, p_تعبير->p_يمين, p_تعبير->p_يسار)
  ){
    p_تعبير->flags ^= EP_Commuted;
  }
  SWAP(تعبير*,p_تعبير->p_يمين,p_تعبير->p_يسار);
  إذا( p_تعبير->op>=TK_GT ){
    أكد( TK_LT==TK_GT+2 );
    أكد( TK_GE==TK_LE+2 );
    أكد( TK_GT>TK_EQ );
    أكد( TK_GT<TK_LE );
    أكد( p_تعبير->op>=TK_GT && p_تعبير->op<=TK_GE );
    p_تعبير->op = ((p_تعبير->op-TK_GT)^2)+TK_GT;
  }
  إرجاع 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
ساكن u16 operatorMask(صحيح op){
  u16 c;
  أكد( allowedOp(op) );
  إذا( op>=TK_EQ ){
    أكد( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }وإلا إذا( op==TK_IN ){
    c = WO_IN;
  }وإلا إذا( op==TK_ISNULL ){
    c = WO_ISNULL;
  }وإلا{
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
  إرجاع c;
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
  تعبير *p_تعبير,      /* Test this expression */
  تعبير **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  صحيح *pisComplete, /* True if the only wildcard is % in the last character */
  صحيح *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ثابت u8 *z = 0;           /* String on RHS of LIKE operator */
  تعبير *p_يمين, *p_يسار;      /* Right and left size of LIKE operator */
  تعبير_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  صحيح cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_قيمة *pVal = 0;
  صحيح op;                    /* Opcode of pRight */
  صحيح rc;                    /* Result code to return */

  إذا( !sqlite3IsLikeFunction(db, p_تعبير, pnoCase, (حرف*)wc) ){
    إرجاع 0;
  }
#ifdef SQLITE_EBCDIC
  إذا( *pnoCase ) إرجاع 0;
#endif
  أكد( تعبير_Use_XList(p_تعبير) );
  pList = p_تعبير->x.pList;
  p_يسار = pList->a[1].p_تعبير;

  p_يمين = sqlite3_تعبير_Skip_Collate(pList->a[0].p_تعبير);
  op = p_يمين->op;
  إذا( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    صحيح iCol = p_يمين->iColumn;
    pVal = sqlite3_Vdbe_Get_Bound_قيمة(pReprepare, iCol, SQLITE_AFF_BLOB);
    إذا( pVal && sqlite3_قيمة_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_قيمة_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    أكد( p_يمين->op==TK_VARIABLE || p_يمين->op==TK_REGISTER );
  }وإلا إذا( op==TK_STRING ){
    أكد( !تعبير_Has_Property(p_يمين, EP_Int_قيمة) );
     z = (u8*)p_يمين->u.zToken;
  }
  إذا( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    بينما( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      إذا( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }وإلا إذا( c>=0x80 ){
        ثابت u8 *z2 = z+cnt-1;
        إذا( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          اكسر;
        }وإلا{
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
    إذا( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      تعبير *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3_تعبير(db, TK_STRING, (حرف*)z);
      إذا( pPrefix ){
        صحيح iFrom, iTo;
        حرف *zNew;
        أكد( !تعبير_Has_Property(pPrefix, EP_Int_قيمة) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        لكل(iFrom=iTo=0; iFrom<cnt; iFrom++){
          إذا( zNew[iFrom]==wc[3] ) iFrom++;
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
        إذا( p_يسار->op!=TK_COLUMN
         || sqlite3_تعبير_Affinity(p_يسار)!=SQLITE_AFF_TEXT
         || (ALWAYS( تعبير_Use_YTab(p_يسار) )
             && ALWAYS(p_يسار->y.pTab)
             && IsVirtual(p_يسار->y.pTab))  /* Might be numeric */
        ){
          صحيح is_رقم;
          مزدوج rDummy;
          أكد( zNew[iTo]==0 );
          is_رقم = sqlite3AtoF(zNew, &rDummy);
          إذا( is_رقم<=0 ){
            إذا( iTo==1 && zNew[0]=='-' ){
              is_رقم = +1;
            }وإلا{
              zNew[iTo-1]++;
              is_رقم = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          إذا( is_رقم>0 ){
            sqlite3_تعبير_Delete(db, pPrefix);
            sqlite3_قيمة_Free(pVal);
            إرجاع 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      إذا( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_يمين->iColumn, 0);
        أكد( !تعبير_Has_Property(p_يمين, EP_Int_قيمة) );
        إذا( *pisComplete && p_يمين->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          صحيح r1 = sqlite3GetTempReg(pParse);
          sqlite3_تعبير_Code_Target(pParse, p_يمين, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }وإلا{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_قيمة_Free(pVal);
  إرجاع rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
صحيح sqlite3_تعبير_Is_Like_Operator(ثابت تعبير *p_تعبير){
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
  أكد( p_تعبير->op==TK_FUNCTION );
  أكد( !تعبير_Has_Property(p_تعبير, EP_Int_قيمة) );
  لكل(i=0; i<ArraySize(aOp); i++){
    إذا( sqlite3StrICmp(p_تعبير->u.zToken, aOp[i].zOp)==0 ){
      إرجاع aOp[i].eOp;
    }
  }
  إرجاع 0;
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
  تعبير *p_تعبير,                    /* Test this expression */
  unsigned حرف *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  تعبير **pp_يسار,                  /* Column expression to left of MATCH/op2 */
  تعبير **pp_يمين                  /* Expression to left of MATCH/op2 */
){
  إذا( p_تعبير->op==TK_FUNCTION ){
    تعبير_List *pList;
    تعبير *pCol;                     /* Column reference */
    صحيح i;

    أكد( تعبير_Use_XList(p_تعبير) );
    pList = p_تعبير->x.pList;
    إذا( pList==0 || pList->n_تعبير!=2 ){
      إرجاع 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].p_تعبير;
    أكد( pCol->op!=TK_COLUMN || (تعبير_Use_YTab(pCol) && pCol->y.pTab!=0) );
    إذا( تعبير_Is_Vtab(pCol) && (i = sqlite3_تعبير_Is_Like_Operator(p_تعبير))!=0 ){
      *peOp2 = i;
      *pp_يمين = pList->a[0].p_تعبير;
      *pp_يسار = pCol;
      إرجاع 1;
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
    pCol = pList->a[0].p_تعبير;
    أكد( pCol->op!=TK_COLUMN || تعبير_Use_YTab(pCol) );
    أكد( pCol->op!=TK_COLUMN || (تعبير_Use_YTab(pCol) && pCol->y.pTab!=0) );
    إذا( تعبير_Is_Vtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      فراغ (*xNotUsed)(sqlite3_context*,صحيح,sqlite3_قيمة**);
      فراغ *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      أكد( pVtab!=0 );
      أكد( pVtab->pModule!=0 );
      أكد( !تعبير_Has_Property(p_تعبير, EP_Int_قيمة) );
      pMod = (sqlite3_module *)pVtab->pModule;
      إذا( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, p_تعبير->u.zToken, &xNotUsed, &pNotUsed);
        إذا( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_يمين = pList->a[1].p_تعبير;
          *pp_يسار = pCol;
          إرجاع 1;
        }
      }
    }
  }وإلا إذا( p_تعبير->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    أكد( TK_NE < TK_EQ );
    أكد( TK_ISNOT < TK_EQ );
    أكد( TK_NOTNULL < TK_EQ );
    إرجاع 0;
  }وإلا إذا( p_تعبير->op==TK_NE || p_تعبير->op==TK_ISNOT || p_تعبير->op==TK_NOTNULL ){
    صحيح res = 0;
    تعبير *p_يسار = p_تعبير->p_يسار;
    تعبير *p_يمين = p_تعبير->p_يمين;
    أكد( p_يسار->op!=TK_COLUMN || (تعبير_Use_YTab(p_يسار) && p_يسار->y.pTab!=0) );
    إذا( تعبير_Is_Vtab(p_يسار) ){
      res++;
    }
    أكد( p_يمين==0 || p_يمين->op!=TK_COLUMN
            || (تعبير_Use_YTab(p_يمين) && p_يمين->y.pTab!=0) );
    إذا( p_يمين && تعبير_Is_Vtab(p_يمين) ){
      res++;
      SWAP(تعبير*, p_يسار, p_يمين);
    }
    *pp_يسار = p_يسار;
    *pp_يمين = p_يمين;
    إذا( p_تعبير->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    إذا( p_تعبير->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    إذا( p_تعبير->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    إرجاع res;
  }
  إرجاع 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
ساكن فراغ transferJoinMarkings(تعبير *pDerived, تعبير *pBase){
  إذا( pDerived && تعبير_Has_Property(pBase, EP_OuterON|EP_InnerON) ){
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
  إذا( pTerm->eOperator!=WO_AND ){
    إرجاع N==0 ? pTerm : 0;
  }
  إذا( N<pTerm->u.pAndInfo->wc.nTerm ){
    إرجاع &pTerm->u.pAndInfo->wc.a[N];
  }
  إرجاع 0;
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
  تعبير *pNew;            /* New virtual expression */
  صحيح op;                /* Operator for the combined expression */
  صحيح idxNew;            /* Index in pWC of the next virtual term */
  تعبير *pA, *pB;         /* Expressions associated with pOne and pTwo */

  إذا( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) إرجاع;
  إذا( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) إرجاع;
  إذا( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) إرجاع;
  إذا( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) إرجاع;
  pA = pOne->p_تعبير;
  pB = pTwo->p_تعبير;
  أكد( pA->p_يسار!=0 && pA->p_يمين!=0 );
  أكد( pB->p_يسار!=0 && pB->p_يمين!=0 );
  إذا( sqlite3_تعبير_Compare(0,pA->p_يسار, pB->p_يسار, -1) )  إرجاع;
  إذا( sqlite3_تعبير_Compare(0,pA->p_يمين, pB->p_يمين,-1) ) إرجاع;
  إذا( تعبير_Has_Property(pA,EP_Commuted)!=تعبير_Has_Property(pB,EP_Commuted) ){
    إرجاع;
  }
  /* If we reach this point, it means the two subterms can be combined */
  إذا( (eOp & (eOp-1))!=0 ){
    إذا( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }وإلا{
      أكد( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_تعبير_Dup(db, pA, 0);
  إذا( pNew==0 ) إرجاع;
  لكل(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ أكد( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_أدرج(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  تعبير_Analyze(pSrc, pWC, idxNew);
}

#إذا !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
ساكن فراغ تعبير_Analyze_Or_Term(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  صحيح idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  تعبير *p_تعبير = pTerm->p_تعبير;             /* The expression of the term */
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
  أكد( p_تعبير->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, حجم(*pOrInfo));
  إذا( pOrInfo==0 ) إرجاع;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, حجم(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, p_تعبير, TK_OR);
  sqlite3_Where_تعبير_Analyze(pSrc, pOrWc);
  إذا( db->mallocFailed ) إرجاع;
  أكد( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  لكل(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    إذا( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      أكد( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, حجم(*pAndInfo));
      إذا( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        صحيح j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->يسار_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, حجم(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->p_تعبير, TK_AND);
        sqlite3_Where_تعبير_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        إذا( !db->mallocFailed ){
          لكل(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            أكد( pAndTerm->p_تعبير );
            إذا( allowedOp(pAndTerm->p_تعبير->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->يسار_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }وإلا إذا( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }وإلا{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->يسار_Cursor);
      إذا( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->يسار_Cursor);
      }
      indexable &= b;
      إذا( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }وإلا{
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
  pTerm->يسار_Cursor = -1;
  إذا( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  إذا( indexable && pOrWc->nTerm==2 ){
    صحيح iOne = 0;
    WhereTerm *pOne;
    بينما( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      صحيح iTwo = 0;
      WhereTerm *pTwo;
      بينما( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  إذا( chngToIN ){
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
      تعبير *p_يسار = 0;
      pOrTerm = pOrWc->a;
      لكل(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        أكد( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        إذا( pOrTerm->يسار_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          أكد( j==1 );
          استمر;
        }
        إذا( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->يسار_Cursor))==0 ){
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
        iColumn = pOrTerm->u.x.يسار_Column;
        iCursor = pOrTerm->يسار_Cursor;
        p_يسار = pOrTerm->p_تعبير->p_يسار;
        اكسر;
      }
      إذا( i<0 ){
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
        إذا( pOrTerm->يسار_Cursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }وإلا إذا( pOrTerm->u.x.يسار_Column!=iColumn || (iColumn==XN_تعبير
               && sqlite3_تعبير_Compare(pParse, pOrTerm->p_تعبير->p_يسار, p_يسار, -1)
        )){
          okToChngToIN = 0;
        }وإلا{
          صحيح aff_يسار, aff_يمين;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_يمين = sqlite3_تعبير_Affinity(pOrTerm->p_تعبير->p_يمين);
          aff_يسار = sqlite3_تعبير_Affinity(pOrTerm->p_تعبير->p_يسار);
          إذا( aff_يمين!=0 && aff_يمين!=aff_يسار ){
            okToChngToIN = 0;
          }وإلا{
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
    إذا( okToChngToIN ){
      تعبير *pDup;            /* A transient duplicate expression */
      تعبير_List *pList = 0;   /* The RHS of the IN operator */
      تعبير *p_يسار = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      تعبير *pNew;            /* The complete IN operator */

      لكل(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        تعبير *pThis;
        إذا( (pOrTerm->wtFlags & TERM_OK)==0 ) استمر;
        أكد( pOrTerm->eOperator & WO_EQ );
        أكد( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        أكد( pOrTerm->يسار_Cursor==iCursor );
        أكد( pOrTerm->u.x.يسار_Column==iColumn );
        pThis = pOrTerm->p_تعبير;
        pDup = sqlite3_تعبير_Dup(db, pThis->p_يمين, 0);
        pList = sqlite3_تعبير_List_Append(pWInfo->pParse, pList, pDup);
        إذا( p_يسار==0 ){
          p_يسار = pThis->p_يسار;
          pCollSeq = sqlite3_تعبير_Compare_Coll_Seq(pParse, pThis);
        }وإلا{
          أكد( 0==sqlite3_تعبير_Compare(pParse, 
                        sqlite3_تعبير_Skip_Collate(pThis->p_يسار),
                        sqlite3_تعبير_Skip_Collate(p_يسار), -1) );
          إذا( pCollSeq!=sqlite3_تعبير_Compare_Coll_Seq(pParse, pThis) ){
            p_يسار = 0;  /* Collating sequence mismatch */
            اكسر;
          }
        }
      }
      إذا( p_يسار==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }وإلا{
        pDup = sqlite3_تعبير_Dup(db, p_يسار, 0);
        إذا( sqlite3_تعبير_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          أكد( pCollSeq->zName!=0 );
          pDup = sqlite3_تعبير_Add_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      إذا( pNew ){
        صحيح idxNew;
        transferJoinMarkings(pNew, p_تعبير);
        أكد( تعبير_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_أدرج(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        تعبير_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }وإلا{
        sqlite3_تعبير_List_Delete(db, pList);
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
ساكن صحيح termIsEquivalence(Parse *pParse, تعبير *p_تعبير, SrcList *pSrc){
  حرف aff1, aff2;
  إذا( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) إرجاع 0;  /* (1) */
  إذا( p_تعبير->op!=TK_EQ && p_تعبير->op!=TK_IS ) إرجاع 0;                 /* (2) */
  إذا( تعبير_Has_Property(p_تعبير, EP_OuterON|EP_Collate) ) إرجاع 0;        /* (3) */
  أكد( pSrc!=0 );
  إذا( p_تعبير->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    إرجاع 0;                                                          /* (4) */
  }
  aff1 = sqlite3_تعبير_Affinity(p_تعبير->p_يسار);
  aff2 = sqlite3_تعبير_Affinity(p_تعبير->p_يمين);
  إذا( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    إرجاع 0;                                                          /* (5) */
  }
  إذا( !sqlite3_تعبير_Coll_Seq_Match(pParse, p_تعبير->p_يسار, p_تعبير->p_يمين) ){
    إرجاع 0;                                                          /* (6) */
  }
  إرجاع 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
ساكن Bitmask تعبير_Select_Usage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  بينما( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3_Where_تعبير_List_Usage(pMaskSet, pS->pEList);
    mask |= sqlite3_Where_تعبير_List_Usage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3_Where_تعبير_List_Usage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3_Where_تعبير_Usage(pMaskSet, pS->pWhere);
    mask |= sqlite3_Where_تعبير_Usage(pMaskSet, pS->pHaving);
    إذا( ALWAYS(pSrc!=0) ){
      صحيح i;
      لكل(i=0; i<pSrc->nSrc; i++){
        إذا( pSrc->a[i].fg.isSubquery ){
          mask |= تعبير_Select_Usage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        إذا( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3_Where_تعبير_Usage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        إذا( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3_Where_تعبير_List_Usage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  إرجاع mask;
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
ساكن SQLITE_NOINLINE صحيح تعبير_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *aiCurCol,         /* Write the referenced table cursor and column here */
  تعبير *p_تعبير,           /* An operand of a comparison operator */
  صحيح j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  صحيح i;
  صحيح iCur;
  افعل{
    iCur = pFrom->a[j].iCursor;
    لكل(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      إذا( pIdx->a_Col_تعبير==0 ) استمر;
      لكل(i=0; i<pIdx->n_مفتاح_Col; i++){
        إذا( pIdx->aiColumn[i]!=XN_تعبير ) استمر;
        أكد( pIdx->b_Has_تعبير );
        إذا( sqlite3_تعبير_Compare_Skip(p_تعبير,pIdx->a_Col_تعبير->a[i].p_تعبير,iCur)==0
         && !sqlite3_تعبير_Is_Constant(0,pIdx->a_Col_تعبير->a[i].p_تعبير)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_تعبير;
          إرجاع 1;
        }
      }
    }
  }بينما( ++j < pFrom->nSrc );
  إرجاع 0;
}
ساكن صحيح تعبير_Might_Be_Indexed(
  SrcList *pFrom,        /* The FROM clause */
  صحيح *aiCurCol,         /* Write the referenced table cursor & column here */
  تعبير *p_تعبير,           /* An operand of a comparison operator */
  صحيح op                 /* The specific comparison operator */
){
  صحيح i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  أكد( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  أكد( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  أكد( op<=TK_GE );
  إذا( p_تعبير->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    أكد( تعبير_Use_XList(p_تعبير) );
    p_تعبير = p_تعبير->x.pList->a[0].p_تعبير;
  }

  إذا( p_تعبير->op==TK_COLUMN ){
    aiCurCol[0] = p_تعبير->iTable;
    aiCurCol[1] = p_تعبير->iColumn;
    إرجاع 1;
  }

  لكل(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    لكل(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      إذا( pIdx->a_Col_تعبير ){
        إرجاع تعبير_Might_Be_Indexed2(pFrom,aiCurCol,p_تعبير,i);
      }
    }
  }
  إرجاع 0;
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
ساكن فراغ تعبير_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  صحيح idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  تعبير *p_تعبير;                     /* The expression to be analyzed */
  Bitmask prereq_يسار;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_يمين = 0;          /* Extra dependencies on LEFT JOIN */
  تعبير *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  صحيح isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  صحيح noCase = 0;                  /* uppercase equivalent to lowercase */
  صحيح op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned حرف eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  صحيح n_يسار;                       /* Number of elements on left side vector */

  إذا( db->mallocFailed ){
    إرجاع;
  }
  أكد( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  p_تعبير = pTerm->p_تعبير;
  أكد( p_تعبير!=0 ); /* Because malloc() has not failed */
  أكد( p_تعبير->op!=TK_AS && p_تعبير->op!=TK_COLLATE );
تعبير_Analyze_restart:
  pMaskSet->bVarSelect = 0;
  prereq_يسار = sqlite3_Where_تعبير_Usage(pMaskSet, p_تعبير->p_يسار);
  op = p_تعبير->op;
  إذا( op==TK_IN ){
    أكد( p_تعبير->p_يمين==0 );
    إذا( sqlite3_تعبير_Check_IN(pParse, p_تعبير) ) إرجاع;
    إذا( تعبير_Use_XSelect(p_تعبير) ){
      pTerm->prereq_يمين = تعبير_Select_Usage(pMaskSet, p_تعبير->x.pSelect);
    }وإلا{
      pTerm->prereq_يمين = sqlite3_Where_تعبير_List_Usage(pMaskSet, p_تعبير->x.pList);
    }
    prereqAll = prereq_يسار | pTerm->prereq_يمين;
  }وإلا{
    pTerm->prereq_يمين = sqlite3_Where_تعبير_Usage(pMaskSet, p_تعبير->p_يمين);
    إذا( p_تعبير->p_يسار==0
     || تعبير_Has_Property(p_تعبير, EP_xIsSelect|EP_IfNullRow)
     || p_تعبير->x.pList!=0
    ){
      prereqAll = sqlite3_Where_تعبير_Usage_NN(pMaskSet, p_تعبير);
    }وإلا{
      prereqAll = prereq_يسار | pTerm->prereq_يمين;
    }
  }
  إذا( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  إذا( prereqAll!=sqlite3_Where_تعبير_Usage_NN(pMaskSet, p_تعبير) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_شجرة_View_تعبير(0,p_تعبير,0);
    أكد( 0 );
  }
#endif

  إذا( تعبير_Has_Property(p_تعبير, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, p_تعبير->w.iJoin);
    إذا( تعبير_Has_Property(p_تعبير, EP_OuterON) ){
      prereqAll |= x;
      extra_يمين = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }وإلا إذا( (prereqAll>>1)>=x ){
      تعبير_Clear_Property(p_تعبير, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->يسار_Cursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  إذا( allowedOp(op) ){
    صحيح aiCurCol[2];
    تعبير *p_يسار = sqlite3_تعبير_Skip_Collate(p_تعبير->p_يسار);
    تعبير *p_يمين = sqlite3_تعبير_Skip_Collate(p_تعبير->p_يمين);
    u16 opMask = (pTerm->prereq_يمين & prereq_يسار)==0 ? WO_ALL : WO_EQUIV;

    إذا( pTerm->u.x.iField>0 ){
      أكد( op==TK_IN );
      أكد( p_يسار->op==TK_VECTOR );
      أكد( تعبير_Use_XList(p_يسار) );
      p_يسار = p_يسار->x.pList->a[pTerm->u.x.iField-1].p_تعبير;
    }

    إذا( تعبير_Might_Be_Indexed(pSrc, aiCurCol, p_يسار, op) ){
      pTerm->يسار_Cursor = aiCurCol[0];
      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.يسار_Column = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    إذا( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    إذا( p_يمين
     && تعبير_Might_Be_Indexed(pSrc, aiCurCol, p_يمين, op)
     && !تعبير_Has_Property(p_يمين, EP_FixedCol)
    ){
      WhereTerm *pNew;
      تعبير *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      أكد( pTerm->u.x.iField==0 );
      إذا( pTerm->يسار_Cursor>=0 ){
        صحيح idxNew;
        pDup = sqlite3_تعبير_Dup(db, p_تعبير, 0);
        إذا( db->mallocFailed ){
          sqlite3_تعبير_Delete(db, pDup);
          إرجاع;
        }
        idxNew = where_Clause_أدرج(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        إذا( idxNew==0 ) إرجاع;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        إذا( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        أكد( pWInfo->pTabList!=0 );
        إذا( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }وإلا{
        pDup = p_تعبير;
        pNew = pTerm;
      }
      pNew->wtFlags |= تعبير_Commute(pParse, pDup);
      pNew->يسار_Cursor = aiCurCol[0];
      أكد( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.يسار_Column = aiCurCol[1];
      testcase( (prereq_يسار | extra_يمين) != prereq_يسار );
      pNew->prereq_يمين = prereq_يسار | extra_يمين;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }وإلا
    إذا( op==TK_ISNULL
     && !تعبير_Has_Property(p_تعبير,EP_OuterON)
     && 0==sqlite3_تعبير_Can_Be_Null(p_يسار)
    ){
      أكد( !تعبير_Has_Property(p_تعبير, EP_Int_قيمة) );
      p_تعبير->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_تعبير->u.zToken = "false";
      تعبير_Set_Property(p_تعبير, EP_IsFalse);
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
  وإلا إذا( p_تعبير->op==TK_BETWEEN && pWC->op==TK_AND ){
    تعبير_List *pList;
    صحيح i;
    ساكن ثابت u8 ops[] = {TK_GE, TK_LE};
    أكد( تعبير_Use_XList(p_تعبير) );
    pList = p_تعبير->x.pList;
    أكد( pList!=0 );
    أكد( pList->n_تعبير==2 );
    أكد( pWC->a[idxTerm].nChild==0 );
    لكل(i=0; i<2; i++){
      تعبير *p_New_تعبير;
      صحيح idxNew;
      p_New_تعبير = sqlite3PExpr(pParse, ops[i],
                             sqlite3_تعبير_Dup(db, p_تعبير->p_يسار, 0),
                             sqlite3_تعبير_Dup(db, pList->a[i].p_تعبير, 0));
      transferJoinMarkings(p_New_تعبير, p_تعبير);
      idxNew = where_Clause_أدرج(pWC, p_New_تعبير, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      تعبير_Analyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#إذا !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  وإلا إذا( p_تعبير->op==TK_OR && !تعبير_Has_Property(p_تعبير, EP_Collate) ){
    تعبير *pAlt = sqlite3_تعبير_Simplified_And_Or(p_تعبير);
    إذا( pAlt!=p_تعبير ){
      pTerm->p_تعبير = p_تعبير = sqlite3_تعبير_Skip_Collate_And_Likely(pAlt);
      اذهب تعبير_Analyze_restart;
    }
    أكد( pWC->op==TK_AND );
    تعبير_Analyze_Or_Term(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  وإلا إذا( p_تعبير->op==TK_NOTNULL ){
    إذا( p_تعبير->p_يسار->op==TK_COLUMN
     && p_تعبير->p_يسار->iColumn>=0
     && !تعبير_Has_Property(p_تعبير, EP_OuterON)
    ){
      تعبير *p_New_تعبير;
      تعبير *p_يسار = p_تعبير->p_يسار;
      صحيح idxNew;
      WhereTerm *pNewTerm;
 
      p_New_تعبير = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_تعبير_Dup(db, p_يسار, 0),
                              sqlite3_تعبير_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_أدرج(pWC, p_New_تعبير,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      إذا( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_يمين = 0;
        pNewTerm->يسار_Cursor = p_يسار->iTable;
        pNewTerm->u.x.يسار_Column = p_يسار->iColumn;
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
  وإلا إذا( p_تعبير->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_تعبير, &pStr1, &isComplete, &noCase)
  ){
    تعبير *p_يسار;       /* LHS of LIKE/GLOB operator */
    تعبير *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    تعبير *pNewExpr1;
    تعبير *pNewExpr2;
    صحيح idxNew1;
    صحيح idxNew2;
    ثابت حرف *zCollSeqName;     /* Name of collating sequence */
    ثابت u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    أكد( تعبير_Use_XList(p_تعبير) );
    p_يسار = p_تعبير->x.pList->a[1].p_تعبير;
    pStr2 = sqlite3_تعبير_Dup(db, pStr1, 0);
    أكد( pStr1==0 || !تعبير_Has_Property(pStr1, EP_Int_قيمة) );
    أكد( pStr2==0 || !تعبير_Has_Property(pStr2, EP_Int_قيمة) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    إذا( noCase && !pParse->db->mallocFailed ){
      صحيح i;
      حرف c;
      pTerm->wtFlags |= TERM_LIKE;
      لكل(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    إذا( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      إذا( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        إذا( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      بينما( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      أكد( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_ثنائي;
    pNewExpr1 = sqlite3_تعبير_Dup(db, p_يسار, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_تعبير_Add_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_تعبير);
    idxNew1 = where_Clause_أدرج(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_تعبير_Dup(db, p_يسار, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_تعبير_Add_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_تعبير);
    idxNew2 = where_Clause_أدرج(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    تعبير_Analyze(pSrc, pWC, idxNew1);
    تعبير_Analyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    إذا( isComplete ){
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
  إذا( (p_تعبير->op==TK_EQ || p_تعبير->op==TK_IS)
   && (n_يسار = sqlite3_تعبير_Vector_Size(p_تعبير->p_يسار))>1
   && sqlite3_تعبير_Vector_Size(p_تعبير->p_يمين)==n_يسار
   && ( (p_تعبير->p_يسار->flags & EP_xIsSelect)==0
     || (p_تعبير->p_يمين->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    صحيح i;
    لكل(i=0; i<n_يسار; i++){
      صحيح idxNew;
      تعبير *pNew;
      تعبير *p_يسار = sqlite3_تعبير_For_Vector_Field(pParse, p_تعبير->p_يسار, i, n_يسار);
      تعبير *p_يمين = sqlite3_تعبير_For_Vector_Field(pParse, p_تعبير->p_يمين, i, n_يسار);

      pNew = sqlite3PExpr(pParse, p_تعبير->op, p_يسار, p_يمين);
      transferJoinMarkings(pNew, p_تعبير);
      idxNew = where_Clause_أدرج(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      تعبير_Analyze(pSrc, pWC, idxNew);
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
  وإلا إذا( p_تعبير->op==TK_IN
   && pTerm->u.x.iField==0
   && p_تعبير->p_يسار->op==TK_VECTOR
   && ALWAYS( تعبير_Use_XSelect(p_تعبير) )
   && (p_تعبير->x.pSelect->pPrior==0 || (p_تعبير->x.pSelect->selFlags & SF_قيم))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p_تعبير->x.pSelect->pWin==0
#endif
   && (p_تعبير->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_تعبير->x.pSelect->pEList->n_تعبير <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    صحيح i;
    أكد( pTerm->nChild==0 );
    لكل(i=0; i<sqlite3_تعبير_Vector_Size(p_تعبير->p_يسار); i++){
      صحيح idxNew;
      idxNew = where_Clause_أدرج(pWC, p_تعبير, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      تعبير_Analyze(pSrc, pWC, idxNew);
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
  وإلا إذا( pWC->op==TK_AND ){
    تعبير *p_يمين = 0, *p_يسار = 0;
    صحيح res = isAuxiliaryVtabOperator(db, p_تعبير, &eOp2, &p_يسار, &p_يمين);
    بينما( res-- > 0 ){
      صحيح idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereq_تعبير;

      prereq_تعبير = sqlite3_Where_تعبير_Usage(pMaskSet, p_يمين);
      prereqColumn = sqlite3_Where_تعبير_Usage(pMaskSet, p_يسار);
      إذا( (prereq_تعبير & prereqColumn)==0 ){
        تعبير *p_New_تعبير;
        p_New_تعبير = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3_تعبير_Dup(db, p_يمين, 0));
        إذا( تعبير_Has_Property(p_تعبير, EP_OuterON) && p_New_تعبير ){
          تعبير_Set_Property(p_New_تعبير, EP_OuterON);
          p_New_تعبير->w.iJoin = p_تعبير->w.iJoin;
        }
        idxNew = where_Clause_أدرج(pWC, p_New_تعبير, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_يمين = prereq_تعبير | extra_يمين;
        pNewTerm->يسار_Cursor = p_يسار->iTable;
        pNewTerm->u.x.يسار_Column = p_يسار->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(تعبير*, p_يسار, p_يمين);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereq_يمين |= extra_يمين;
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
فراغ sqlite3WhereSplit(WhereClause *pWC, تعبير *p_تعبير, u8 op){
  تعبير *pE2 = sqlite3_تعبير_Skip_Collate_And_Likely(p_تعبير);
  pWC->op = op;
  أكد( pE2!=0 || p_تعبير==0 );
  إذا( pE2==0 ) إرجاع;
  إذا( pE2->op!=op ){
    where_Clause_أدرج(pWC, p_تعبير, 0);
  }وإلا{
    sqlite3WhereSplit(pWC, pE2->p_يسار, op);
    sqlite3WhereSplit(pWC, pE2->p_يمين, op);
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
ساكن فراغ where_Add_Limit_تعبير(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  صحيح iReg,           /* Register that will hold value of the limit/offset */
  تعبير *p_تعبير,        /* Expression that defines the limit/offset */
  صحيح iCsr,           /* Cursor to which the constraint applies */
  صحيح eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  تعبير *pNew;
  صحيح iVal = 0;

  إذا( sqlite3_تعبير_Is_Integer(p_تعبير, &iVal, pParse, 0) && iVal>=0 ){
    تعبير *pVal = sqlite3_تعبير_Int32(db, iVal);
    إذا( pVal==0 ) إرجاع;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }وإلا{
    تعبير *pVal = sqlite3_تعبير_Alloc(db, TK_REGISTER, 0, 0);
    إذا( pVal==0 ) إرجاع;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  إذا( pNew ){
    WhereTerm *pTerm;
    صحيح idx;
    idx = where_Clause_أدرج(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->يسار_Cursor = iCsr;
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
  إذا( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    تعبير_List *pOrderBy = p->pOrderBy;
    صحيح iCsr = p->pSrc->a[0].iCursor;
    صحيح ii;

    /* Check condition (4). Return early if it is not met. */
    لكل(ii=0; ii<pWC->nTerm; ii++){
      إذا( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        أكد( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        أكد( pWC->a[ii].eOperator==WO_ROWVAL );
        استمر;
      }
      إذا( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        استمر;
      }
      إذا( pWC->a[ii].يسار_Cursor==iCsr && pWC->a[ii].prereq_يمين==0 ) استمر;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      إذا( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        إذا( pParent->يسار_Cursor==iCsr
         && pParent->prereq_يمين==0
         && pParent->nChild==1
        ){
          استمر;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      إرجاع;
    }

    /* Check condition (5). Return early if it is not met. */
    إذا( pOrderBy ){
      لكل(ii=0; ii<pOrderBy->n_تعبير; ii++){
        تعبير *p_تعبير = pOrderBy->a[ii].p_تعبير;
        إذا( p_تعبير->op!=TK_COLUMN ) إرجاع;
        إذا( p_تعبير->iTable!=iCsr ) إرجاع;
        إذا( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) إرجاع;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    أكد( p->pLimit->op==TK_LIMIT );
    إذا( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_تعبير(pWC, p->iOffset, p->pLimit->p_يمين,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    إذا( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_تعبير(pWC, p->iLimit, p->pLimit->p_يسار,
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
  إذا( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    صحيح i;
    /* Verify that every term past pWC->nBase is virtual */
    لكل(i=pWC->nBase; i<pWC->nTerm; i++){
      أكد( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    بينما(1){
      أكد( a->eMatchOp==0 || a->eOperator==WO_AUX );
      إذا( a->wtFlags & TERM_DYNAMIC ){
        sqlite3_تعبير_Delete(db, a->p_تعبير);
      }
      إذا( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        إذا( a->wtFlags & TERM_ORINFO ){
          أكد( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }وإلا{
          أكد( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      إذا( a==aLast ) اكسر;
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
ساكن SQLITE_NOINLINE Bitmask sqlite3_Where_تعبير_Usage_Full(
  WhereMaskSet *pMaskSet,
  تعبير *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  إذا( p->p_يسار ) mask |= sqlite3_Where_تعبير_Usage_NN(pMaskSet, p->p_يسار);
  إذا( p->p_يمين ){
    mask |= sqlite3_Where_تعبير_Usage_NN(pMaskSet, p->p_يمين);
    أكد( p->x.pList==0 );
  }وإلا إذا( تعبير_Use_XSelect(p) ){
    إذا( تعبير_Has_Property(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= تعبير_Select_Usage(pMaskSet, p->x.pSelect);
  }وإلا إذا( p->x.pList ){
    mask |= sqlite3_Where_تعبير_List_Usage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  إذا( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && تعبير_Use_YWin(p) ){
    أكد( p->y.pWin!=0 );
    mask |= sqlite3_Where_تعبير_List_Usage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3_Where_تعبير_List_Usage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3_Where_تعبير_Usage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  إرجاع mask;
}
Bitmask sqlite3_Where_تعبير_Usage_NN(WhereMaskSet *pMaskSet, تعبير *p){
  إذا( p->op==TK_COLUMN && !تعبير_Has_Property(p, EP_FixedCol) ){
    إرجاع sqlite3WhereGetMask(pMaskSet, p->iTable);
  }وإلا إذا( تعبير_Has_Property(p, EP_TokenOnly|EP_Leaf) ){
    أكد( p->op!=TK_IF_NULL_ROW );
    إرجاع 0;
  }
  إرجاع sqlite3_Where_تعبير_Usage_Full(pMaskSet, p);
}
Bitmask sqlite3_Where_تعبير_Usage(WhereMaskSet *pMaskSet, تعبير *p){
  إرجاع p ? sqlite3_Where_تعبير_Usage_NN(pMaskSet,p) : 0;
}
Bitmask sqlite3_Where_تعبير_List_Usage(WhereMaskSet *pMaskSet, تعبير_List *pList){
  صحيح i;
  Bitmask mask = 0;
  إذا( pList ){
    لكل(i=0; i<pList->n_تعبير; i++){
      mask |= sqlite3_Where_تعبير_Usage(pMaskSet, pList->a[i].p_تعبير);
    }
  }
  إرجاع mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
فراغ sqlite3_Where_تعبير_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  صحيح i;
  لكل(i=pWC->nTerm-1; i>=0; i--){
    تعبير_Analyze(pTabList, pWC, i);
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
  تعبير_List *pArgs;
  تعبير *pColRef;
  تعبير *pTerm;
  إذا( pItem->fg.isTabFunc==0 ) إرجاع;
  pTab = pItem->pSTab;
  أكد( pTab!=0 );
  إذا( !IsVirtual(pTab) ){
    sqlite3_Error_رسالة(pParse, "'%s' is not a function", pItem->zName);
    إرجاع;
  }
  pArgs = pItem->u1.pFuncArg;
  إذا( pArgs==0 ) إرجاع;
  لكل(j=k=0; j<pArgs->n_تعبير; j++){
    تعبير *pRhs;
    u32 joinType;
    بينما( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    إذا( k>=pTab->nCol ){
      sqlite3_Error_رسالة(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      إرجاع;
    }
    pColRef = sqlite3_تعبير_Alloc(pParse->db, TK_COLUMN, 0, 0);
    إذا( pColRef==0 ) إرجاع;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    أكد( تعبير_Use_YTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3_تعبير_Col_Used(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_تعبير_Dup(pParse->db, pArgs->a[j].p_تعبير, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    إذا( pItem->fg.jointype & (JT_يسار|JT_يمين) ){
      testcase( pItem->fg.jointype & JT_يسار );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_يمين ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }وإلا{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_Set_Join_تعبير(pTerm, pItem->iCursor, joinType);
    where_Clause_أدرج(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
صحيح sqlite3WhereLoopBloomable(ثابت WhereLoop *pLoop){
  إذا( pLoop->wsFlags & WHERE_IPK ) إرجاع 1;
  إذا( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) إرجاع 0;
  إرجاع sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
