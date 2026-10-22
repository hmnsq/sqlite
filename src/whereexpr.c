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
#অন্তর্ভাব "sqliteInt.h"
#অন্তর্ভাব "whereInt.h"

/* Forward declarations */
স্থির শূন্য রাশিমালা_Analyze(SrcList*, WhereClause*, পূর্ণাঙ্ক);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
স্থির শূন্য whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
স্থির শূন্য whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
স্থির পূর্ণাঙ্ক where_Clause_ঢোকাও(WhereClause *pWC, রাশিমালা *p, u16 wtFlags){
  WhereTerm *pTerm;
  পূর্ণাঙ্ক idx;
  testcase( wtFlags & TERM_VIRTUAL );
  যদি( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, পরিমাণ(pWC->a[0])*pWC->nSlot*2 );
    যদি( pWC->a==0 ){
      যদি( wtFlags & TERM_DYNAMIC ){
        sqlite3_রাশিমালা_Delete(db, p);
      }
      pWC->a = pOld;
      প্রত্যাগম 0;
    }
    memcpy(pWC->a, pOld, পরিমাণ(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  যদি( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  যদি( p && রাশিমালা_Has_Property(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }অন্যথা{
    pTerm->truthProb = 1;
  }
  pTerm->p_রাশিমালা = sqlite3_রাশিমালা_Skip_Collate_And_Likely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         পরিমাণ(WhereTerm) - offsetof(WhereTerm,eOperator));
  প্রত্যাগম idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
স্থির পূর্ণাঙ্ক allowedOp(পূর্ণাঙ্ক op){
  প্রতিজ্ঞা( TK_GT>TK_EQ && TK_GT<TK_GE );
  প্রতিজ্ঞা( TK_LT>TK_EQ && TK_LT<TK_GE );
  প্রতিজ্ঞা( TK_LE>TK_EQ && TK_LE<TK_GE );
  প্রতিজ্ঞা( TK_GE==TK_EQ+4 );
  প্রতিজ্ঞা( TK_IN<TK_EQ );
  প্রতিজ্ঞা( TK_IS<TK_EQ );
  প্রতিজ্ঞা( TK_ISNULL<TK_EQ );
  যদি( op>TK_GE ) প্রত্যাগম 0;
  যদি( op>=TK_EQ ) প্রত্যাগম 1;
  প্রত্যাগম op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
স্থির u16 রাশিমালা_Commute(Parse *pParse, রাশিমালা *p_রাশিমালা){
  যদি( p_রাশিমালা->p_বাম->op==TK_VECTOR
   || p_রাশিমালা->p_ডান->op==TK_VECTOR
   || sqlite3_দ্বিমিক_Compare_Coll_Seq(pParse, p_রাশিমালা->p_বাম, p_রাশিমালা->p_ডান) !=
      sqlite3_দ্বিমিক_Compare_Coll_Seq(pParse, p_রাশিমালা->p_ডান, p_রাশিমালা->p_বাম)
  ){
    p_রাশিমালা->flags ^= EP_Commuted;
  }
  SWAP(রাশিমালা*,p_রাশিমালা->p_ডান,p_রাশিমালা->p_বাম);
  যদি( p_রাশিমালা->op>=TK_GT ){
    প্রতিজ্ঞা( TK_LT==TK_GT+2 );
    প্রতিজ্ঞা( TK_GE==TK_LE+2 );
    প্রতিজ্ঞা( TK_GT>TK_EQ );
    প্রতিজ্ঞা( TK_GT<TK_LE );
    প্রতিজ্ঞা( p_রাশিমালা->op>=TK_GT && p_রাশিমালা->op<=TK_GE );
    p_রাশিমালা->op = ((p_রাশিমালা->op-TK_GT)^2)+TK_GT;
  }
  প্রত্যাগম 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
স্থির u16 operatorMask(পূর্ণাঙ্ক op){
  u16 c;
  প্রতিজ্ঞা( allowedOp(op) );
  যদি( op>=TK_EQ ){
    প্রতিজ্ঞা( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }অন্যথা যদি( op==TK_IN ){
    c = WO_IN;
  }অন্যথা যদি( op==TK_ISNULL ){
    c = WO_ISNULL;
  }অন্যথা{
    প্রতিজ্ঞা( op==TK_IS );
    c = WO_IS;
  }
  প্রতিজ্ঞা( op!=TK_ISNULL || c==WO_ISNULL );
  প্রতিজ্ঞা( op!=TK_IN || c==WO_IN );
  প্রতিজ্ঞা( op!=TK_EQ || c==WO_EQ );
  প্রতিজ্ঞা( op!=TK_LT || c==WO_LT );
  প্রতিজ্ঞা( op!=TK_LE || c==WO_LE );
  প্রতিজ্ঞা( op!=TK_GT || c==WO_GT );
  প্রতিজ্ঞা( op!=TK_GE || c==WO_GE );
  প্রতিজ্ঞা( op!=TK_IS || c==WO_IS );
  প্রত্যাগম c;
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
স্থির পূর্ণাঙ্ক isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  রাশিমালা *p_রাশিমালা,      /* Test this expression */
  রাশিমালা **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  পূর্ণাঙ্ক *pisComplete, /* True if the only wildcard is % in the last character */
  পূর্ণাঙ্ক *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  ধ্রুব u8 *z = 0;           /* String on RHS of LIKE operator */
  রাশিমালা *p_ডান, *p_বাম;      /* Right and left size of LIKE operator */
  রাশিমালা_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  পূর্ণাঙ্ক cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_মান *pVal = 0;
  পূর্ণাঙ্ক op;                    /* Opcode of pRight */
  পূর্ণাঙ্ক rc;                    /* Result code to return */

  যদি( !sqlite3IsLikeFunction(db, p_রাশিমালা, pnoCase, (বর্ণ*)wc) ){
    প্রত্যাগম 0;
  }
#ifdef SQLITE_EBCDIC
  যদি( *pnoCase ) প্রত্যাগম 0;
#endif
  প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
  pList = p_রাশিমালা->x.pList;
  p_বাম = pList->a[1].p_রাশিমালা;

  p_ডান = sqlite3_রাশিমালা_Skip_Collate(pList->a[0].p_রাশিমালা);
  op = p_ডান->op;
  যদি( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    পূর্ণাঙ্ক iCol = p_ডান->iColumn;
    pVal = sqlite3_Vdbe_Get_Bound_মান(pReprepare, iCol, SQLITE_AFF_BLOB);
    যদি( pVal && sqlite3_মান_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_মান_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    প্রতিজ্ঞা( p_ডান->op==TK_VARIABLE || p_ডান->op==TK_REGISTER );
  }অন্যথা যদি( op==TK_STRING ){
    প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_ডান, EP_Int_মান) );
     z = (u8*)p_ডান->u.zToken;
  }
  যদি( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    যাবত্( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      যদি( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }অন্যথা যদি( c>=0x80 ){
        ধ্রুব u8 *z2 = z+cnt-1;
        যদি( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          ভঙ্গ;
        }অন্যথা{
          cnt = (পূর্ণাঙ্ক)(z2-z);
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
    যদি( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      রাশিমালা *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3_রাশিমালা(db, TK_STRING, (বর্ণ*)z);
      যদি( pPrefix ){
        পূর্ণাঙ্ক iFrom, iTo;
        বর্ণ *zNew;
        প্রতিজ্ঞা( !রাশিমালা_Has_Property(pPrefix, EP_Int_মান) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        প্রতি(iFrom=iTo=0; iFrom<cnt; iFrom++){
          যদি( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[iTo++] = zNew[iFrom];
        }
        zNew[iTo] = 0;
        প্রতিজ্ঞা( iTo>0 );

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
        যদি( p_বাম->op!=TK_COLUMN
         || sqlite3_রাশিমালা_Affinity(p_বাম)!=SQLITE_AFF_TEXT
         || (ALWAYS( রাশিমালা_Use_YTab(p_বাম) )
             && ALWAYS(p_বাম->y.pTab)
             && IsVirtual(p_বাম->y.pTab))  /* Might be numeric */
        ){
          পূর্ণাঙ্ক is_অঙ্ক;
          দ্বিগুণ rDummy;
          প্রতিজ্ঞা( zNew[iTo]==0 );
          is_অঙ্ক = sqlite3AtoF(zNew, &rDummy);
          যদি( is_অঙ্ক<=0 ){
            যদি( iTo==1 && zNew[0]=='-' ){
              is_অঙ্ক = +1;
            }অন্যথা{
              zNew[iTo-1]++;
              is_অঙ্ক = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          যদি( is_অঙ্ক>0 ){
            sqlite3_রাশিমালা_Delete(db, pPrefix);
            sqlite3_মান_Free(pVal);
            প্রত্যাগম 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      যদি( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_ডান->iColumn, 0);
        প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_ডান, EP_Int_মান) );
        যদি( *pisComplete && p_ডান->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          পূর্ণাঙ্ক r1 = sqlite3GetTempReg(pParse);
          sqlite3_রাশিমালা_Code_Target(pParse, p_ডান, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }অন্যথা{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_মান_Free(pVal);
  প্রত্যাগম rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
পূর্ণাঙ্ক sqlite3_রাশিমালা_Is_Like_Operator(ধ্রুব রাশিমালা *p_রাশিমালা){
  স্থির ধ্রুব সংরচনা {
    ধ্রুব বর্ণ *zOp;
    unsigned বর্ণ eOp;
  } aOp[] = {
    { "match",  SQLITE_INDEX_CONSTRAINT_MATCH },
    { "glob",   SQLITE_INDEX_CONSTRAINT_GLOB },
    { "like",   SQLITE_INDEX_CONSTRAINT_LIKE },
    { "regexp", SQLITE_INDEX_CONSTRAINT_REGEXP }
  };
  পূর্ণাঙ্ক i;
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_FUNCTION );
  প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
  প্রতি(i=0; i<ArraySize(aOp); i++){
    যদি( sqlite3StrICmp(p_রাশিমালা->u.zToken, aOp[i].zOp)==0 ){
      প্রত্যাগম aOp[i].eOp;
    }
  }
  প্রত্যাগম 0;
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
স্থির পূর্ণাঙ্ক isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  রাশিমালা *p_রাশিমালা,                    /* Test this expression */
  unsigned বর্ণ *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  রাশিমালা **pp_বাম,                  /* Column expression to left of MATCH/op2 */
  রাশিমালা **pp_ডান                  /* Expression to left of MATCH/op2 */
){
  যদি( p_রাশিমালা->op==TK_FUNCTION ){
    রাশিমালা_List *pList;
    রাশিমালা *pCol;                     /* Column reference */
    পূর্ণাঙ্ক i;

    প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
    pList = p_রাশিমালা->x.pList;
    যদি( pList==0 || pList->n_রাশিমালা!=2 ){
      প্রত্যাগম 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].p_রাশিমালা;
    প্রতিজ্ঞা( pCol->op!=TK_COLUMN || (রাশিমালা_Use_YTab(pCol) && pCol->y.pTab!=0) );
    যদি( রাশিমালা_Is_Vtab(pCol) && (i = sqlite3_রাশিমালা_Is_Like_Operator(p_রাশিমালা))!=0 ){
      *peOp2 = i;
      *pp_ডান = pList->a[0].p_রাশিমালা;
      *pp_বাম = pCol;
      প্রত্যাগম 1;
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
    pCol = pList->a[0].p_রাশিমালা;
    প্রতিজ্ঞা( pCol->op!=TK_COLUMN || রাশিমালা_Use_YTab(pCol) );
    প্রতিজ্ঞা( pCol->op!=TK_COLUMN || (রাশিমালা_Use_YTab(pCol) && pCol->y.pTab!=0) );
    যদি( রাশিমালা_Is_Vtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      শূন্য (*xNotUsed)(sqlite3_context*,পূর্ণাঙ্ক,sqlite3_মান**);
      শূন্য *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      প্রতিজ্ঞা( pVtab!=0 );
      প্রতিজ্ঞা( pVtab->pModule!=0 );
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      pMod = (sqlite3_module *)pVtab->pModule;
      যদি( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, p_রাশিমালা->u.zToken, &xNotUsed, &pNotUsed);
        যদি( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_ডান = pList->a[1].p_রাশিমালা;
          *pp_বাম = pCol;
          প্রত্যাগম 1;
        }
      }
    }
  }অন্যথা যদি( p_রাশিমালা->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    প্রতিজ্ঞা( TK_NE < TK_EQ );
    প্রতিজ্ঞা( TK_ISNOT < TK_EQ );
    প্রতিজ্ঞা( TK_NOTNULL < TK_EQ );
    প্রত্যাগম 0;
  }অন্যথা যদি( p_রাশিমালা->op==TK_NE || p_রাশিমালা->op==TK_ISNOT || p_রাশিমালা->op==TK_NOTNULL ){
    পূর্ণাঙ্ক res = 0;
    রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
    রাশিমালা *p_ডান = p_রাশিমালা->p_ডান;
    প্রতিজ্ঞা( p_বাম->op!=TK_COLUMN || (রাশিমালা_Use_YTab(p_বাম) && p_বাম->y.pTab!=0) );
    যদি( রাশিমালা_Is_Vtab(p_বাম) ){
      res++;
    }
    প্রতিজ্ঞা( p_ডান==0 || p_ডান->op!=TK_COLUMN
            || (রাশিমালা_Use_YTab(p_ডান) && p_ডান->y.pTab!=0) );
    যদি( p_ডান && রাশিমালা_Is_Vtab(p_ডান) ){
      res++;
      SWAP(রাশিমালা*, p_বাম, p_ডান);
    }
    *pp_বাম = p_বাম;
    *pp_ডান = p_ডান;
    যদি( p_রাশিমালা->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    যদি( p_রাশিমালা->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    যদি( p_রাশিমালা->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    প্রত্যাগম res;
  }
  প্রত্যাগম 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
স্থির শূন্য transferJoinMarkings(রাশিমালা *pDerived, রাশিমালা *pBase){
  যদি( pDerived && রাশিমালা_Has_Property(pBase, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= pBase->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
স্থির শূন্য markTermAsChild(WhereClause *pWC, পূর্ণাঙ্ক iChild, পূর্ণাঙ্ক iParent){
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;
  প্রতিজ্ঞা(   pWC->a[iParent].nChild <  UMXV(pWC->a[0].nChild) );
  pWC->a[iParent].nChild++;
  testcase( pWC->a[iParent].nChild == UMXV(pWC->a[0].nChild) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
স্থির WhereTerm *whereNthSubterm(WhereTerm *pTerm, পূর্ণাঙ্ক N){
  যদি( pTerm->eOperator!=WO_AND ){
    প্রত্যাগম N==0 ? pTerm : 0;
  }
  যদি( N<pTerm->u.pAndInfo->wc.nTerm ){
    প্রত্যাগম &pTerm->u.pAndInfo->wc.a[N];
  }
  প্রত্যাগম 0;
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
স্থির শূন্য whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  রাশিমালা *pNew;            /* New virtual expression */
  পূর্ণাঙ্ক op;                /* Operator for the combined expression */
  পূর্ণাঙ্ক idxNew;            /* Index in pWC of the next virtual term */
  রাশিমালা *pA, *pB;         /* Expressions associated with pOne and pTwo */

  যদি( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) প্রত্যাগম;
  যদি( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) প্রত্যাগম;
  যদি( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) প্রত্যাগম;
  যদি( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) প্রত্যাগম;
  pA = pOne->p_রাশিমালা;
  pB = pTwo->p_রাশিমালা;
  প্রতিজ্ঞা( pA->p_বাম!=0 && pA->p_ডান!=0 );
  প্রতিজ্ঞা( pB->p_বাম!=0 && pB->p_ডান!=0 );
  যদি( sqlite3_রাশিমালা_Compare(0,pA->p_বাম, pB->p_বাম, -1) )  প্রত্যাগম;
  যদি( sqlite3_রাশিমালা_Compare(0,pA->p_ডান, pB->p_ডান,-1) ) প্রত্যাগম;
  যদি( রাশিমালা_Has_Property(pA,EP_Commuted)!=রাশিমালা_Has_Property(pB,EP_Commuted) ){
    প্রত্যাগম;
  }
  /* If we reach this point, it means the two subterms can be combined */
  যদি( (eOp & (eOp-1))!=0 ){
    যদি( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }অন্যথা{
      প্রতিজ্ঞা( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_রাশিমালা_Dup(db, pA, 0);
  যদি( pNew==0 ) প্রত্যাগম;
  প্রতি(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ প্রতিজ্ঞা( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_ঢোকাও(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  রাশিমালা_Analyze(pSrc, pWC, idxNew);
}

#যদি !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
স্থির শূন্য রাশিমালা_Analyze_Or_Term(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  পূর্ণাঙ্ক idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  রাশিমালা *p_রাশিমালা = pTerm->p_রাশিমালা;             /* The expression of the term */
  পূর্ণাঙ্ক i;                                  /* Loop counters */
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
  প্রতিজ্ঞা( (pTerm->wtFlags & (TERM_DYNAMIC|TERM_ORINFO|TERM_ANDINFO))==0 );
  প্রতিজ্ঞা( p_রাশিমালা->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, পরিমাণ(*pOrInfo));
  যদি( pOrInfo==0 ) প্রত্যাগম;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, পরিমাণ(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, p_রাশিমালা, TK_OR);
  sqlite3_Where_রাশিমালা_Analyze(pSrc, pOrWc);
  যদি( db->mallocFailed ) প্রত্যাগম;
  প্রতিজ্ঞা( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  প্রতি(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    যদি( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      প্রতিজ্ঞা( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, পরিমাণ(*pAndInfo));
      যদি( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        পূর্ণাঙ্ক j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->বাম_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, পরিমাণ(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->p_রাশিমালা, TK_AND);
        sqlite3_Where_রাশিমালা_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        যদি( !db->mallocFailed ){
          প্রতি(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            প্রতিজ্ঞা( pAndTerm->p_রাশিমালা );
            যদি( allowedOp(pAndTerm->p_রাশিমালা->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->বাম_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }অন্যথা যদি( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }অন্যথা{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->বাম_Cursor);
      যদি( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->বাম_Cursor);
      }
      indexable &= b;
      যদি( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }অন্যথা{
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
  pTerm->বাম_Cursor = -1;
  যদি( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  যদি( indexable && pOrWc->nTerm==2 ){
    পূর্ণাঙ্ক iOne = 0;
    WhereTerm *pOne;
    যাবত্( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      পূর্ণাঙ্ক iTwo = 0;
      WhereTerm *pTwo;
      যাবত্( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  যদি( chngToIN ){
    পূর্ণাঙ্ক okToChngToIN = 0;     /* True if the conversion to IN is valid */
    পূর্ণাঙ্ক iColumn = -1;         /* Column index on lhs of IN operator */
    পূর্ণাঙ্ক iCursor = -1;         /* Table cursor common to all terms */
    পূর্ণাঙ্ক j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    প্রতি(j=0; j<2 && !okToChngToIN; j++){
      রাশিমালা *p_বাম = 0;
      pOrTerm = pOrWc->a;
      প্রতি(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        প্রতিজ্ঞা( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        যদি( pOrTerm->বাম_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          প্রতিজ্ঞা( j==1 );
          অনুবর্ত;
        }
        যদি( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->বাম_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          প্রতিজ্ঞা( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          অনুবর্ত;
        }
        প্রতিজ্ঞা( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        iColumn = pOrTerm->u.x.বাম_Column;
        iCursor = pOrTerm->বাম_Cursor;
        p_বাম = pOrTerm->p_রাশিমালা->p_বাম;
        ভঙ্গ;
      }
      যদি( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        প্রতিজ্ঞা( j==1 );
        প্রতিজ্ঞা( IsPowerOfTwo(chngToIN) );
        প্রতিজ্ঞা( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        ভঙ্গ;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      প্রতি(; i>=0 && okToChngToIN; i--, pOrTerm++){
        প্রতিজ্ঞা( pOrTerm->eOperator & WO_EQ );
        প্রতিজ্ঞা( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        যদি( pOrTerm->বাম_Cursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }অন্যথা যদি( pOrTerm->u.x.বাম_Column!=iColumn || (iColumn==XN_রাশিমালা
               && sqlite3_রাশিমালা_Compare(pParse, pOrTerm->p_রাশিমালা->p_বাম, p_বাম, -1)
        )){
          okToChngToIN = 0;
        }অন্যথা{
          পূর্ণাঙ্ক aff_বাম, aff_ডান;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_ডান = sqlite3_রাশিমালা_Affinity(pOrTerm->p_রাশিমালা->p_ডান);
          aff_বাম = sqlite3_রাশিমালা_Affinity(pOrTerm->p_রাশিমালা->p_বাম);
          যদি( aff_ডান!=0 && aff_ডান!=aff_বাম ){
            okToChngToIN = 0;
          }অন্যথা{
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
    যদি( okToChngToIN ){
      রাশিমালা *pDup;            /* A transient duplicate expression */
      রাশিমালা_List *pList = 0;   /* The RHS of the IN operator */
      রাশিমালা *p_বাম = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      রাশিমালা *pNew;            /* The complete IN operator */

      প্রতি(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        রাশিমালা *pThis;
        যদি( (pOrTerm->wtFlags & TERM_OK)==0 ) অনুবর্ত;
        প্রতিজ্ঞা( pOrTerm->eOperator & WO_EQ );
        প্রতিজ্ঞা( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        প্রতিজ্ঞা( pOrTerm->বাম_Cursor==iCursor );
        প্রতিজ্ঞা( pOrTerm->u.x.বাম_Column==iColumn );
        pThis = pOrTerm->p_রাশিমালা;
        pDup = sqlite3_রাশিমালা_Dup(db, pThis->p_ডান, 0);
        pList = sqlite3_রাশিমালা_List_Append(pWInfo->pParse, pList, pDup);
        যদি( p_বাম==0 ){
          p_বাম = pThis->p_বাম;
          pCollSeq = sqlite3_রাশিমালা_Compare_Coll_Seq(pParse, pThis);
        }অন্যথা{
          প্রতিজ্ঞা( 0==sqlite3_রাশিমালা_Compare(pParse, 
                        sqlite3_রাশিমালা_Skip_Collate(pThis->p_বাম),
                        sqlite3_রাশিমালা_Skip_Collate(p_বাম), -1) );
          যদি( pCollSeq!=sqlite3_রাশিমালা_Compare_Coll_Seq(pParse, pThis) ){
            p_বাম = 0;  /* Collating sequence mismatch */
            ভঙ্গ;
          }
        }
      }
      যদি( p_বাম==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }অন্যথা{
        pDup = sqlite3_রাশিমালা_Dup(db, p_বাম, 0);
        যদি( sqlite3_রাশিমালা_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          প্রতিজ্ঞা( pCollSeq->zName!=0 );
          pDup = sqlite3_রাশিমালা_Add_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      যদি( pNew ){
        পূর্ণাঙ্ক idxNew;
        transferJoinMarkings(pNew, p_রাশিমালা);
        প্রতিজ্ঞা( রাশিমালা_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_ঢোকাও(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        রাশিমালা_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }অন্যথা{
        sqlite3_রাশিমালা_List_Delete(db, pList);
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
স্থির পূর্ণাঙ্ক termIsEquivalence(Parse *pParse, রাশিমালা *p_রাশিমালা, SrcList *pSrc){
  বর্ণ aff1, aff2;
  যদি( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) প্রত্যাগম 0;  /* (1) */
  যদি( p_রাশিমালা->op!=TK_EQ && p_রাশিমালা->op!=TK_IS ) প্রত্যাগম 0;                 /* (2) */
  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON|EP_Collate) ) প্রত্যাগম 0;        /* (3) */
  প্রতিজ্ঞা( pSrc!=0 );
  যদি( p_রাশিমালা->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    প্রত্যাগম 0;                                                          /* (4) */
  }
  aff1 = sqlite3_রাশিমালা_Affinity(p_রাশিমালা->p_বাম);
  aff2 = sqlite3_রাশিমালা_Affinity(p_রাশিমালা->p_ডান);
  যদি( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    প্রত্যাগম 0;                                                          /* (5) */
  }
  যদি( !sqlite3_রাশিমালা_Coll_Seq_Match(pParse, p_রাশিমালা->p_বাম, p_রাশিমালা->p_ডান) ){
    প্রত্যাগম 0;                                                          /* (6) */
  }
  প্রত্যাগম 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
স্থির Bitmask রাশিমালা_Select_Usage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  যাবত্( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3_Where_রাশিমালা_List_Usage(pMaskSet, pS->pEList);
    mask |= sqlite3_Where_রাশিমালা_List_Usage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3_Where_রাশিমালা_List_Usage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3_Where_রাশিমালা_Usage(pMaskSet, pS->pWhere);
    mask |= sqlite3_Where_রাশিমালা_Usage(pMaskSet, pS->pHaving);
    যদি( ALWAYS(pSrc!=0) ){
      পূর্ণাঙ্ক i;
      প্রতি(i=0; i<pSrc->nSrc; i++){
        যদি( pSrc->a[i].fg.isSubquery ){
          mask |= রাশিমালা_Select_Usage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        যদি( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3_Where_রাশিমালা_Usage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        যদি( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3_Where_রাশিমালা_List_Usage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  প্রত্যাগম mask;
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
স্থির SQLITE_NOINLINE পূর্ণাঙ্ক রাশিমালা_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  পূর্ণাঙ্ক *aiCurCol,         /* Write the referenced table cursor and column here */
  রাশিমালা *p_রাশিমালা,           /* An operand of a comparison operator */
  পূর্ণাঙ্ক j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  পূর্ণাঙ্ক i;
  পূর্ণাঙ্ক iCur;
  কুরু{
    iCur = pFrom->a[j].iCursor;
    প্রতি(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      যদি( pIdx->a_Col_রাশিমালা==0 ) অনুবর্ত;
      প্রতি(i=0; i<pIdx->n_চাবি_Col; i++){
        যদি( pIdx->aiColumn[i]!=XN_রাশিমালা ) অনুবর্ত;
        প্রতিজ্ঞা( pIdx->b_Has_রাশিমালা );
        যদি( sqlite3_রাশিমালা_Compare_Skip(p_রাশিমালা,pIdx->a_Col_রাশিমালা->a[i].p_রাশিমালা,iCur)==0
         && !sqlite3_রাশিমালা_Is_Constant(0,pIdx->a_Col_রাশিমালা->a[i].p_রাশিমালা)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_রাশিমালা;
          প্রত্যাগম 1;
        }
      }
    }
  }যাবত্( ++j < pFrom->nSrc );
  প্রত্যাগম 0;
}
স্থির পূর্ণাঙ্ক রাশিমালা_Might_Be_Indexed(
  SrcList *pFrom,        /* The FROM clause */
  পূর্ণাঙ্ক *aiCurCol,         /* Write the referenced table cursor & column here */
  রাশিমালা *p_রাশিমালা,           /* An operand of a comparison operator */
  পূর্ণাঙ্ক op                 /* The specific comparison operator */
){
  পূর্ণাঙ্ক i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  প্রতিজ্ঞা( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  প্রতিজ্ঞা( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  প্রতিজ্ঞা( op<=TK_GE );
  যদি( p_রাশিমালা->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
    p_রাশিমালা = p_রাশিমালা->x.pList->a[0].p_রাশিমালা;
  }

  যদি( p_রাশিমালা->op==TK_COLUMN ){
    aiCurCol[0] = p_রাশিমালা->iTable;
    aiCurCol[1] = p_রাশিমালা->iColumn;
    প্রত্যাগম 1;
  }

  প্রতি(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    প্রতি(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      যদি( pIdx->a_Col_রাশিমালা ){
        প্রত্যাগম রাশিমালা_Might_Be_Indexed2(pFrom,aiCurCol,p_রাশিমালা,i);
      }
    }
  }
  প্রত্যাগম 0;
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
স্থির শূন্য রাশিমালা_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  পূর্ণাঙ্ক idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  রাশিমালা *p_রাশিমালা;                     /* The expression to be analyzed */
  Bitmask prereq_বাম;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_ডান = 0;          /* Extra dependencies on LEFT JOIN */
  রাশিমালা *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  পূর্ণাঙ্ক isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  পূর্ণাঙ্ক noCase = 0;                  /* uppercase equivalent to lowercase */
  পূর্ণাঙ্ক op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned বর্ণ eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  পূর্ণাঙ্ক n_বাম;                       /* Number of elements on left side vector */

  যদি( db->mallocFailed ){
    প্রত্যাগম;
  }
  প্রতিজ্ঞা( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  p_রাশিমালা = pTerm->p_রাশিমালা;
  প্রতিজ্ঞা( p_রাশিমালা!=0 ); /* Because malloc() has not failed */
  প্রতিজ্ঞা( p_রাশিমালা->op!=TK_AS && p_রাশিমালা->op!=TK_COLLATE );
রাশিমালা_Analyze_restart:
  pMaskSet->bVarSelect = 0;
  prereq_বাম = sqlite3_Where_রাশিমালা_Usage(pMaskSet, p_রাশিমালা->p_বাম);
  op = p_রাশিমালা->op;
  যদি( op==TK_IN ){
    প্রতিজ্ঞা( p_রাশিমালা->p_ডান==0 );
    যদি( sqlite3_রাশিমালা_Check_IN(pParse, p_রাশিমালা) ) প্রত্যাগম;
    যদি( রাশিমালা_Use_XSelect(p_রাশিমালা) ){
      pTerm->prereq_ডান = রাশিমালা_Select_Usage(pMaskSet, p_রাশিমালা->x.pSelect);
    }অন্যথা{
      pTerm->prereq_ডান = sqlite3_Where_রাশিমালা_List_Usage(pMaskSet, p_রাশিমালা->x.pList);
    }
    prereqAll = prereq_বাম | pTerm->prereq_ডান;
  }অন্যথা{
    pTerm->prereq_ডান = sqlite3_Where_রাশিমালা_Usage(pMaskSet, p_রাশিমালা->p_ডান);
    যদি( p_রাশিমালা->p_বাম==0
     || রাশিমালা_Has_Property(p_রাশিমালা, EP_xIsSelect|EP_IfNullRow)
     || p_রাশিমালা->x.pList!=0
    ){
      prereqAll = sqlite3_Where_রাশিমালা_Usage_NN(pMaskSet, p_রাশিমালা);
    }অন্যথা{
      prereqAll = prereq_বাম | pTerm->prereq_ডান;
    }
  }
  যদি( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  যদি( prereqAll!=sqlite3_Where_রাশিমালা_Usage_NN(pMaskSet, p_রাশিমালা) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_বৃক্ষ_View_রাশিমালা(0,p_রাশিমালা,0);
    প্রতিজ্ঞা( 0 );
  }
#endif

  যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, p_রাশিমালা->w.iJoin);
    যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON) ){
      prereqAll |= x;
      extra_ডান = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }অন্যথা যদি( (prereqAll>>1)>=x ){
      রাশিমালা_Clear_Property(p_রাশিমালা, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->বাম_Cursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  যদি( allowedOp(op) ){
    পূর্ণাঙ্ক aiCurCol[2];
    রাশিমালা *p_বাম = sqlite3_রাশিমালা_Skip_Collate(p_রাশিমালা->p_বাম);
    রাশিমালা *p_ডান = sqlite3_রাশিমালা_Skip_Collate(p_রাশিমালা->p_ডান);
    u16 opMask = (pTerm->prereq_ডান & prereq_বাম)==0 ? WO_ALL : WO_EQUIV;

    যদি( pTerm->u.x.iField>0 ){
      প্রতিজ্ঞা( op==TK_IN );
      প্রতিজ্ঞা( p_বাম->op==TK_VECTOR );
      প্রতিজ্ঞা( রাশিমালা_Use_XList(p_বাম) );
      p_বাম = p_বাম->x.pList->a[pTerm->u.x.iField-1].p_রাশিমালা;
    }

    যদি( রাশিমালা_Might_Be_Indexed(pSrc, aiCurCol, p_বাম, op) ){
      pTerm->বাম_Cursor = aiCurCol[0];
      প্রতিজ্ঞা( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.বাম_Column = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    যদি( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    যদি( p_ডান
     && রাশিমালা_Might_Be_Indexed(pSrc, aiCurCol, p_ডান, op)
     && !রাশিমালা_Has_Property(p_ডান, EP_FixedCol)
    ){
      WhereTerm *pNew;
      রাশিমালা *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      প্রতিজ্ঞা( pTerm->u.x.iField==0 );
      যদি( pTerm->বাম_Cursor>=0 ){
        পূর্ণাঙ্ক idxNew;
        pDup = sqlite3_রাশিমালা_Dup(db, p_রাশিমালা, 0);
        যদি( db->mallocFailed ){
          sqlite3_রাশিমালা_Delete(db, pDup);
          প্রত্যাগম;
        }
        idxNew = where_Clause_ঢোকাও(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        যদি( idxNew==0 ) প্রত্যাগম;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        যদি( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        প্রতিজ্ঞা( pWInfo->pTabList!=0 );
        যদি( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }অন্যথা{
        pDup = p_রাশিমালা;
        pNew = pTerm;
      }
      pNew->wtFlags |= রাশিমালা_Commute(pParse, pDup);
      pNew->বাম_Cursor = aiCurCol[0];
      প্রতিজ্ঞা( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.বাম_Column = aiCurCol[1];
      testcase( (prereq_বাম | extra_ডান) != prereq_বাম );
      pNew->prereq_ডান = prereq_বাম | extra_ডান;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }অন্যথা
    যদি( op==TK_ISNULL
     && !রাশিমালা_Has_Property(p_রাশিমালা,EP_OuterON)
     && 0==sqlite3_রাশিমালা_Can_Be_Null(p_বাম)
    ){
      প্রতিজ্ঞা( !রাশিমালা_Has_Property(p_রাশিমালা, EP_Int_মান) );
      p_রাশিমালা->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_রাশিমালা->u.zToken = "false";
      রাশিমালা_Set_Property(p_রাশিমালা, EP_IsFalse);
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
  অন্যথা যদি( p_রাশিমালা->op==TK_BETWEEN && pWC->op==TK_AND ){
    রাশিমালা_List *pList;
    পূর্ণাঙ্ক i;
    স্থির ধ্রুব u8 ops[] = {TK_GE, TK_LE};
    প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
    pList = p_রাশিমালা->x.pList;
    প্রতিজ্ঞা( pList!=0 );
    প্রতিজ্ঞা( pList->n_রাশিমালা==2 );
    প্রতিজ্ঞা( pWC->a[idxTerm].nChild==0 );
    প্রতি(i=0; i<2; i++){
      রাশিমালা *p_New_রাশিমালা;
      পূর্ণাঙ্ক idxNew;
      p_New_রাশিমালা = sqlite3PExpr(pParse, ops[i],
                             sqlite3_রাশিমালা_Dup(db, p_রাশিমালা->p_বাম, 0),
                             sqlite3_রাশিমালা_Dup(db, pList->a[i].p_রাশিমালা, 0));
      transferJoinMarkings(p_New_রাশিমালা, p_রাশিমালা);
      idxNew = where_Clause_ঢোকাও(pWC, p_New_রাশিমালা, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      রাশিমালা_Analyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#যদি !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  অন্যথা যদি( p_রাশিমালা->op==TK_OR && !রাশিমালা_Has_Property(p_রাশিমালা, EP_Collate) ){
    রাশিমালা *pAlt = sqlite3_রাশিমালা_Simplified_And_Or(p_রাশিমালা);
    যদি( pAlt!=p_রাশিমালা ){
      pTerm->p_রাশিমালা = p_রাশিমালা = sqlite3_রাশিমালা_Skip_Collate_And_Likely(pAlt);
      গচ্ছ রাশিমালা_Analyze_restart;
    }
    প্রতিজ্ঞা( pWC->op==TK_AND );
    রাশিমালা_Analyze_Or_Term(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  অন্যথা যদি( p_রাশিমালা->op==TK_NOTNULL ){
    যদি( p_রাশিমালা->p_বাম->op==TK_COLUMN
     && p_রাশিমালা->p_বাম->iColumn>=0
     && !রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON)
    ){
      রাশিমালা *p_New_রাশিমালা;
      রাশিমালা *p_বাম = p_রাশিমালা->p_বাম;
      পূর্ণাঙ্ক idxNew;
      WhereTerm *pNewTerm;
 
      p_New_রাশিমালা = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_রাশিমালা_Dup(db, p_বাম, 0),
                              sqlite3_রাশিমালা_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_ঢোকাও(pWC, p_New_রাশিমালা,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      যদি( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_ডান = 0;
        pNewTerm->বাম_Cursor = p_বাম->iTable;
        pNewTerm->u.x.বাম_Column = p_বাম->iColumn;
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
  অন্যথা যদি( p_রাশিমালা->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_রাশিমালা, &pStr1, &isComplete, &noCase)
  ){
    রাশিমালা *p_বাম;       /* LHS of LIKE/GLOB operator */
    রাশিমালা *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    রাশিমালা *pNewExpr1;
    রাশিমালা *pNewExpr2;
    পূর্ণাঙ্ক idxNew1;
    পূর্ণাঙ্ক idxNew2;
    ধ্রুব বর্ণ *zCollSeqName;     /* Name of collating sequence */
    ধ্রুব u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    প্রতিজ্ঞা( রাশিমালা_Use_XList(p_রাশিমালা) );
    p_বাম = p_রাশিমালা->x.pList->a[1].p_রাশিমালা;
    pStr2 = sqlite3_রাশিমালা_Dup(db, pStr1, 0);
    প্রতিজ্ঞা( pStr1==0 || !রাশিমালা_Has_Property(pStr1, EP_Int_মান) );
    প্রতিজ্ঞা( pStr2==0 || !রাশিমালা_Has_Property(pStr2, EP_Int_মান) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    যদি( noCase && !pParse->db->mallocFailed ){
      পূর্ণাঙ্ক i;
      বর্ণ c;
      pTerm->wtFlags |= TERM_LIKE;
      প্রতি(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    যদি( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      যদি( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        যদি( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      যাবত্( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      প্রতিজ্ঞা( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_দ্বিমিক;
    pNewExpr1 = sqlite3_রাশিমালা_Dup(db, p_বাম, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_রাশিমালা_Add_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_রাশিমালা);
    idxNew1 = where_Clause_ঢোকাও(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_রাশিমালা_Dup(db, p_বাম, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_রাশিমালা_Add_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_রাশিমালা);
    idxNew2 = where_Clause_ঢোকাও(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    রাশিমালা_Analyze(pSrc, pWC, idxNew1);
    রাশিমালা_Analyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    যদি( isComplete ){
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
  যদি( (p_রাশিমালা->op==TK_EQ || p_রাশিমালা->op==TK_IS)
   && (n_বাম = sqlite3_রাশিমালা_Vector_Size(p_রাশিমালা->p_বাম))>1
   && sqlite3_রাশিমালা_Vector_Size(p_রাশিমালা->p_ডান)==n_বাম
   && ( (p_রাশিমালা->p_বাম->flags & EP_xIsSelect)==0
     || (p_রাশিমালা->p_ডান->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    পূর্ণাঙ্ক i;
    প্রতি(i=0; i<n_বাম; i++){
      পূর্ণাঙ্ক idxNew;
      রাশিমালা *pNew;
      রাশিমালা *p_বাম = sqlite3_রাশিমালা_For_Vector_Field(pParse, p_রাশিমালা->p_বাম, i, n_বাম);
      রাশিমালা *p_ডান = sqlite3_রাশিমালা_For_Vector_Field(pParse, p_রাশিমালা->p_ডান, i, n_বাম);

      pNew = sqlite3PExpr(pParse, p_রাশিমালা->op, p_বাম, p_ডান);
      transferJoinMarkings(pNew, p_রাশিমালা);
      idxNew = where_Clause_ঢোকাও(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      রাশিমালা_Analyze(pSrc, pWC, idxNew);
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
  অন্যথা যদি( p_রাশিমালা->op==TK_IN
   && pTerm->u.x.iField==0
   && p_রাশিমালা->p_বাম->op==TK_VECTOR
   && ALWAYS( রাশিমালা_Use_XSelect(p_রাশিমালা) )
   && (p_রাশিমালা->x.pSelect->pPrior==0 || (p_রাশিমালা->x.pSelect->selFlags & SF_মানসমূহ))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p_রাশিমালা->x.pSelect->pWin==0
#endif
   && (p_রাশিমালা->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_রাশিমালা->x.pSelect->pEList->n_রাশিমালা <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    পূর্ণাঙ্ক i;
    প্রতিজ্ঞা( pTerm->nChild==0 );
    প্রতি(i=0; i<sqlite3_রাশিমালা_Vector_Size(p_রাশিমালা->p_বাম); i++){
      পূর্ণাঙ্ক idxNew;
      idxNew = where_Clause_ঢোকাও(pWC, p_রাশিমালা, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      রাশিমালা_Analyze(pSrc, pWC, idxNew);
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
  অন্যথা যদি( pWC->op==TK_AND ){
    রাশিমালা *p_ডান = 0, *p_বাম = 0;
    পূর্ণাঙ্ক res = isAuxiliaryVtabOperator(db, p_রাশিমালা, &eOp2, &p_বাম, &p_ডান);
    যাবত্( res-- > 0 ){
      পূর্ণাঙ্ক idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereq_রাশিমালা;

      prereq_রাশিমালা = sqlite3_Where_রাশিমালা_Usage(pMaskSet, p_ডান);
      prereqColumn = sqlite3_Where_রাশিমালা_Usage(pMaskSet, p_বাম);
      যদি( (prereq_রাশিমালা & prereqColumn)==0 ){
        রাশিমালা *p_New_রাশিমালা;
        p_New_রাশিমালা = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3_রাশিমালা_Dup(db, p_ডান, 0));
        যদি( রাশিমালা_Has_Property(p_রাশিমালা, EP_OuterON) && p_New_রাশিমালা ){
          রাশিমালা_Set_Property(p_New_রাশিমালা, EP_OuterON);
          p_New_রাশিমালা->w.iJoin = p_রাশিমালা->w.iJoin;
        }
        idxNew = where_Clause_ঢোকাও(pWC, p_New_রাশিমালা, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_ডান = prereq_রাশিমালা | extra_ডান;
        pNewTerm->বাম_Cursor = p_বাম->iTable;
        pNewTerm->u.x.বাম_Column = p_বাম->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(রাশিমালা*, p_বাম, p_ডান);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereq_ডান |= extra_ডান;
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
শূন্য sqlite3WhereSplit(WhereClause *pWC, রাশিমালা *p_রাশিমালা, u8 op){
  রাশিমালা *pE2 = sqlite3_রাশিমালা_Skip_Collate_And_Likely(p_রাশিমালা);
  pWC->op = op;
  প্রতিজ্ঞা( pE2!=0 || p_রাশিমালা==0 );
  যদি( pE2==0 ) প্রত্যাগম;
  যদি( pE2->op!=op ){
    where_Clause_ঢোকাও(pWC, p_রাশিমালা, 0);
  }অন্যথা{
    sqlite3WhereSplit(pWC, pE2->p_বাম, op);
    sqlite3WhereSplit(pWC, pE2->p_ডান, op);
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
স্থির শূন্য where_Add_Limit_রাশিমালা(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  পূর্ণাঙ্ক iReg,           /* Register that will hold value of the limit/offset */
  রাশিমালা *p_রাশিমালা,        /* Expression that defines the limit/offset */
  পূর্ণাঙ্ক iCsr,           /* Cursor to which the constraint applies */
  পূর্ণাঙ্ক eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  রাশিমালা *pNew;
  পূর্ণাঙ্ক iVal = 0;

  যদি( sqlite3_রাশিমালা_Is_Integer(p_রাশিমালা, &iVal, pParse, 0) && iVal>=0 ){
    রাশিমালা *pVal = sqlite3_রাশিমালা_Int32(db, iVal);
    যদি( pVal==0 ) প্রত্যাগম;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }অন্যথা{
    রাশিমালা *pVal = sqlite3_রাশিমালা_Alloc(db, TK_REGISTER, 0, 0);
    যদি( pVal==0 ) প্রত্যাগম;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  যদি( pNew ){
    WhereTerm *pTerm;
    পূর্ণাঙ্ক idx;
    idx = where_Clause_ঢোকাও(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->বাম_Cursor = iCsr;
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
শূন্য SQLITE_NOINLINE sqlite3WhereAddLimit(WhereClause *pWC, Select *p){
  প্রতিজ্ঞা( p!=0 && p->pLimit!=0 );                 /* 1 -- checked by caller */
  যদি( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    রাশিমালা_List *pOrderBy = p->pOrderBy;
    পূর্ণাঙ্ক iCsr = p->pSrc->a[0].iCursor;
    পূর্ণাঙ্ক ii;

    /* Check condition (4). Return early if it is not met. */
    প্রতি(ii=0; ii<pWC->nTerm; ii++){
      যদি( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        প্রতিজ্ঞা( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        প্রতিজ্ঞা( pWC->a[ii].eOperator==WO_ROWVAL );
        অনুবর্ত;
      }
      যদি( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        অনুবর্ত;
      }
      যদি( pWC->a[ii].বাম_Cursor==iCsr && pWC->a[ii].prereq_ডান==0 ) অনুবর্ত;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      যদি( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        যদি( pParent->বাম_Cursor==iCsr
         && pParent->prereq_ডান==0
         && pParent->nChild==1
        ){
          অনুবর্ত;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      প্রত্যাগম;
    }

    /* Check condition (5). Return early if it is not met. */
    যদি( pOrderBy ){
      প্রতি(ii=0; ii<pOrderBy->n_রাশিমালা; ii++){
        রাশিমালা *p_রাশিমালা = pOrderBy->a[ii].p_রাশিমালা;
        যদি( p_রাশিমালা->op!=TK_COLUMN ) প্রত্যাগম;
        যদি( p_রাশিমালা->iTable!=iCsr ) প্রত্যাগম;
        যদি( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) প্রত্যাগম;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    প্রতিজ্ঞা( p->pLimit->op==TK_LIMIT );
    যদি( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_রাশিমালা(pWC, p->iOffset, p->pLimit->p_ডান,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    যদি( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_রাশিমালা(pWC, p->iLimit, p->pLimit->p_বাম,
                        iCsr, SQLITE_INDEX_CONSTRAINT_LIMIT);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
শূন্য sqlite3WhereClauseInit(
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
শূন্য sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  প্রতিজ্ঞা( pWC->nTerm>=pWC->nBase );
  যদি( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    পূর্ণাঙ্ক i;
    /* Verify that every term past pWC->nBase is virtual */
    প্রতি(i=pWC->nBase; i<pWC->nTerm; i++){
      প্রতিজ্ঞা( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    যাবত্(1){
      প্রতিজ্ঞা( a->eMatchOp==0 || a->eOperator==WO_AUX );
      যদি( a->wtFlags & TERM_DYNAMIC ){
        sqlite3_রাশিমালা_Delete(db, a->p_রাশিমালা);
      }
      যদি( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        যদি( a->wtFlags & TERM_ORINFO ){
          প্রতিজ্ঞা( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }অন্যথা{
          প্রতিজ্ঞা( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      যদি( a==aLast ) ভঙ্গ;
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
স্থির SQLITE_NOINLINE Bitmask sqlite3_Where_রাশিমালা_Usage_Full(
  WhereMaskSet *pMaskSet,
  রাশিমালা *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  যদি( p->p_বাম ) mask |= sqlite3_Where_রাশিমালা_Usage_NN(pMaskSet, p->p_বাম);
  যদি( p->p_ডান ){
    mask |= sqlite3_Where_রাশিমালা_Usage_NN(pMaskSet, p->p_ডান);
    প্রতিজ্ঞা( p->x.pList==0 );
  }অন্যথা যদি( রাশিমালা_Use_XSelect(p) ){
    যদি( রাশিমালা_Has_Property(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= রাশিমালা_Select_Usage(pMaskSet, p->x.pSelect);
  }অন্যথা যদি( p->x.pList ){
    mask |= sqlite3_Where_রাশিমালা_List_Usage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  যদি( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && রাশিমালা_Use_YWin(p) ){
    প্রতিজ্ঞা( p->y.pWin!=0 );
    mask |= sqlite3_Where_রাশিমালা_List_Usage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3_Where_রাশিমালা_List_Usage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3_Where_রাশিমালা_Usage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  প্রত্যাগম mask;
}
Bitmask sqlite3_Where_রাশিমালা_Usage_NN(WhereMaskSet *pMaskSet, রাশিমালা *p){
  যদি( p->op==TK_COLUMN && !রাশিমালা_Has_Property(p, EP_FixedCol) ){
    প্রত্যাগম sqlite3WhereGetMask(pMaskSet, p->iTable);
  }অন্যথা যদি( রাশিমালা_Has_Property(p, EP_TokenOnly|EP_Leaf) ){
    প্রতিজ্ঞা( p->op!=TK_IF_NULL_ROW );
    প্রত্যাগম 0;
  }
  প্রত্যাগম sqlite3_Where_রাশিমালা_Usage_Full(pMaskSet, p);
}
Bitmask sqlite3_Where_রাশিমালা_Usage(WhereMaskSet *pMaskSet, রাশিমালা *p){
  প্রত্যাগম p ? sqlite3_Where_রাশিমালা_Usage_NN(pMaskSet,p) : 0;
}
Bitmask sqlite3_Where_রাশিমালা_List_Usage(WhereMaskSet *pMaskSet, রাশিমালা_List *pList){
  পূর্ণাঙ্ক i;
  Bitmask mask = 0;
  যদি( pList ){
    প্রতি(i=0; i<pList->n_রাশিমালা; i++){
      mask |= sqlite3_Where_রাশিমালা_Usage(pMaskSet, pList->a[i].p_রাশিমালা);
    }
  }
  প্রত্যাগম mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
শূন্য sqlite3_Where_রাশিমালা_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  পূর্ণাঙ্ক i;
  প্রতি(i=pWC->nTerm-1; i>=0; i--){
    রাশিমালা_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
শূন্য sqlite3WhereTabFuncArgs(
  Parse *pParse,                    /* Parsing context */
  SrcItem *pItem,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  Table *pTab;
  পূর্ণাঙ্ক j, k;
  রাশিমালা_List *pArgs;
  রাশিমালা *pColRef;
  রাশিমালা *pTerm;
  যদি( pItem->fg.isTabFunc==0 ) প্রত্যাগম;
  pTab = pItem->pSTab;
  প্রতিজ্ঞা( pTab!=0 );
  যদি( !IsVirtual(pTab) ){
    sqlite3_Error_বার্তা(pParse, "'%s' is not a function", pItem->zName);
    প্রত্যাগম;
  }
  pArgs = pItem->u1.pFuncArg;
  যদি( pArgs==0 ) প্রত্যাগম;
  প্রতি(j=k=0; j<pArgs->n_রাশিমালা; j++){
    রাশিমালা *pRhs;
    u32 joinType;
    যাবত্( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    যদি( k>=pTab->nCol ){
      sqlite3_Error_বার্তা(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      প্রত্যাগম;
    }
    pColRef = sqlite3_রাশিমালা_Alloc(pParse->db, TK_COLUMN, 0, 0);
    যদি( pColRef==0 ) প্রত্যাগম;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    প্রতিজ্ঞা( রাশিমালা_Use_YTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3_রাশিমালা_Col_Used(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_রাশিমালা_Dup(pParse->db, pArgs->a[j].p_রাশিমালা, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    যদি( pItem->fg.jointype & (JT_বাম|JT_ডান) ){
      testcase( pItem->fg.jointype & JT_বাম );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_ডান ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }অন্যথা{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_Set_Join_রাশিমালা(pTerm, pItem->iCursor, joinType);
    where_Clause_ঢোকাও(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
পূর্ণাঙ্ক sqlite3WhereLoopBloomable(ধ্রুব WhereLoop *pLoop){
  যদি( pLoop->wsFlags & WHERE_IPK ) প্রত্যাগম 1;
  যদি( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) প্রত্যাগম 0;
  প্রত্যাগম sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
