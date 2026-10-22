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
#അന്തര്ഭാവ "sqliteInt.h"
#അന്തര്ഭാവ "whereInt.h"

/* Forward declarations */
സ്ഥിതം ശൂന്യം വ്യഞ്ജകം_Analyze(SrcList*, WhereClause*, പൂർണ്ണസംഖ്യ);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
സ്ഥിതം ശൂന്യം whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
സ്ഥിതം ശൂന്യം whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
സ്ഥിതം പൂർണ്ണസംഖ്യ where_Clause_തിരുകുക(WhereClause *pWC, വ്യഞ്ജകം *p, u16 wtFlags){
  WhereTerm *pTerm;
  പൂർണ്ണസംഖ്യ idx;
  testcase( wtFlags & TERM_VIRTUAL );
  എങ്കിൽ( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, വലുപ്പം(pWC->a[0])*pWC->nSlot*2 );
    എങ്കിൽ( pWC->a==0 ){
      എങ്കിൽ( wtFlags & TERM_DYNAMIC ){
        sqlite3_വ്യഞ്ജകം_Delete(db, p);
      }
      pWC->a = pOld;
      തിരികെ 0;
    }
    memcpy(pWC->a, pOld, വലുപ്പം(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  എങ്കിൽ( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  എങ്കിൽ( p && വ്യഞ്ജകം_Has_Property(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }അല്ലെങ്കിൽ{
    pTerm->truthProb = 1;
  }
  pTerm->p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         വലുപ്പം(WhereTerm) - offsetof(WhereTerm,eOperator));
  തിരികെ idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
സ്ഥിതം പൂർണ്ണസംഖ്യ allowedOp(പൂർണ്ണസംഖ്യ op){
  പ്രതിജ്ഞാ( TK_GT>TK_EQ && TK_GT<TK_GE );
  പ്രതിജ്ഞാ( TK_LT>TK_EQ && TK_LT<TK_GE );
  പ്രതിജ്ഞാ( TK_LE>TK_EQ && TK_LE<TK_GE );
  പ്രതിജ്ഞാ( TK_GE==TK_EQ+4 );
  പ്രതിജ്ഞാ( TK_IN<TK_EQ );
  പ്രതിജ്ഞാ( TK_IS<TK_EQ );
  പ്രതിജ്ഞാ( TK_ISNULL<TK_EQ );
  എങ്കിൽ( op>TK_GE ) തിരികെ 0;
  എങ്കിൽ( op>=TK_EQ ) തിരികെ 1;
  തിരികെ op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
സ്ഥിതം u16 വ്യഞ്ജകം_Commute(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  എങ്കിൽ( p_വ്യഞ്ജകം->p_ഇടത്->op==TK_VECTOR
   || p_വ്യഞ്ജകം->p_വലത്->op==TK_VECTOR
   || sqlite3_ദ്വിമാന_Compare_Coll_Seq(pParse, p_വ്യഞ്ജകം->p_ഇടത്, p_വ്യഞ്ജകം->p_വലത്) !=
      sqlite3_ദ്വിമാന_Compare_Coll_Seq(pParse, p_വ്യഞ്ജകം->p_വലത്, p_വ്യഞ്ജകം->p_ഇടത്)
  ){
    p_വ്യഞ്ജകം->flags ^= EP_Commuted;
  }
  SWAP(വ്യഞ്ജകം*,p_വ്യഞ്ജകം->p_വലത്,p_വ്യഞ്ജകം->p_ഇടത്);
  എങ്കിൽ( p_വ്യഞ്ജകം->op>=TK_GT ){
    പ്രതിജ്ഞാ( TK_LT==TK_GT+2 );
    പ്രതിജ്ഞാ( TK_GE==TK_LE+2 );
    പ്രതിജ്ഞാ( TK_GT>TK_EQ );
    പ്രതിജ്ഞാ( TK_GT<TK_LE );
    പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op>=TK_GT && p_വ്യഞ്ജകം->op<=TK_GE );
    p_വ്യഞ്ജകം->op = ((p_വ്യഞ്ജകം->op-TK_GT)^2)+TK_GT;
  }
  തിരികെ 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
സ്ഥിതം u16 operatorMask(പൂർണ്ണസംഖ്യ op){
  u16 c;
  പ്രതിജ്ഞാ( allowedOp(op) );
  എങ്കിൽ( op>=TK_EQ ){
    പ്രതിജ്ഞാ( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }അല്ലെങ്കിൽ എങ്കിൽ( op==TK_IN ){
    c = WO_IN;
  }അല്ലെങ്കിൽ എങ്കിൽ( op==TK_ISNULL ){
    c = WO_ISNULL;
  }അല്ലെങ്കിൽ{
    പ്രതിജ്ഞാ( op==TK_IS );
    c = WO_IS;
  }
  പ്രതിജ്ഞാ( op!=TK_ISNULL || c==WO_ISNULL );
  പ്രതിജ്ഞാ( op!=TK_IN || c==WO_IN );
  പ്രതിജ്ഞാ( op!=TK_EQ || c==WO_EQ );
  പ്രതിജ്ഞാ( op!=TK_LT || c==WO_LT );
  പ്രതിജ്ഞാ( op!=TK_LE || c==WO_LE );
  പ്രതിജ്ഞാ( op!=TK_GT || c==WO_GT );
  പ്രതിജ്ഞാ( op!=TK_GE || c==WO_GE );
  പ്രതിജ്ഞാ( op!=TK_IS || c==WO_IS );
  തിരികെ c;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,      /* Test this expression */
  വ്യഞ്ജകം **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  പൂർണ്ണസംഖ്യ *pisComplete, /* True if the only wildcard is % in the last character */
  പൂർണ്ണസംഖ്യ *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  സ്ഥിരം u8 *z = 0;           /* String on RHS of LIKE operator */
  വ്യഞ്ജകം *p_വലത്, *p_ഇടത്;      /* Right and left size of LIKE operator */
  വ്യഞ്ജകം_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  പൂർണ്ണസംഖ്യ cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_മൂല്യം *pVal = 0;
  പൂർണ്ണസംഖ്യ op;                    /* Opcode of pRight */
  പൂർണ്ണസംഖ്യ rc;                    /* Result code to return */

  എങ്കിൽ( !sqlite3IsLikeFunction(db, p_വ്യഞ്ജകം, pnoCase, (അക്ഷരം*)wc) ){
    തിരികെ 0;
  }
#ifdef SQLITE_EBCDIC
  എങ്കിൽ( *pnoCase ) തിരികെ 0;
#endif
  പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
  pList = p_വ്യഞ്ജകം->x.pList;
  p_ഇടത് = pList->a[1].p_വ്യഞ്ജകം;

  p_വലത് = sqlite3_വ്യഞ്ജകം_Skip_Collate(pList->a[0].p_വ്യഞ്ജകം);
  op = p_വലത്->op;
  എങ്കിൽ( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    പൂർണ്ണസംഖ്യ iCol = p_വലത്->iColumn;
    pVal = sqlite3_Vdbe_Get_Bound_മൂല്യം(pReprepare, iCol, SQLITE_AFF_BLOB);
    എങ്കിൽ( pVal && sqlite3_മൂല്യം_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_മൂല്യം_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    പ്രതിജ്ഞാ( p_വലത്->op==TK_VARIABLE || p_വലത്->op==TK_REGISTER );
  }അല്ലെങ്കിൽ എങ്കിൽ( op==TK_STRING ){
    പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വലത്, EP_Int_മൂല്യം) );
     z = (u8*)p_വലത്->u.zToken;
  }
  എങ്കിൽ( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    ആകുമ്പോൾ( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      എങ്കിൽ( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }അല്ലെങ്കിൽ എങ്കിൽ( c>=0x80 ){
        സ്ഥിരം u8 *z2 = z+cnt-1;
        എങ്കിൽ( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          നിർത്തുക;
        }അല്ലെങ്കിൽ{
          cnt = (പൂർണ്ണസംഖ്യ)(z2-z);
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
    എങ്കിൽ( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      വ്യഞ്ജകം *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3_വ്യഞ്ജകം(db, TK_STRING, (അക്ഷരം*)z);
      എങ്കിൽ( pPrefix ){
        പൂർണ്ണസംഖ്യ iFrom, iTo;
        അക്ഷരം *zNew;
        പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(pPrefix, EP_Int_മൂല്യം) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        വേണ്ടി(iFrom=iTo=0; iFrom<cnt; iFrom++){
          എങ്കിൽ( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[iTo++] = zNew[iFrom];
        }
        zNew[iTo] = 0;
        പ്രതിജ്ഞാ( iTo>0 );

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
        എങ്കിൽ( p_ഇടത്->op!=TK_COLUMN
         || sqlite3_വ്യഞ്ജകം_Affinity(p_ഇടത്)!=SQLITE_AFF_TEXT
         || (ALWAYS( വ്യഞ്ജകം_Use_YTab(p_ഇടത്) )
             && ALWAYS(p_ഇടത്->y.pTab)
             && IsVirtual(p_ഇടത്->y.pTab))  /* Might be numeric */
        ){
          പൂർണ്ണസംഖ്യ is_അക്കം;
          ഇരട്ട rDummy;
          പ്രതിജ്ഞാ( zNew[iTo]==0 );
          is_അക്കം = sqlite3AtoF(zNew, &rDummy);
          എങ്കിൽ( is_അക്കം<=0 ){
            എങ്കിൽ( iTo==1 && zNew[0]=='-' ){
              is_അക്കം = +1;
            }അല്ലെങ്കിൽ{
              zNew[iTo-1]++;
              is_അക്കം = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          എങ്കിൽ( is_അക്കം>0 ){
            sqlite3_വ്യഞ്ജകം_Delete(db, pPrefix);
            sqlite3_മൂല്യം_Free(pVal);
            തിരികെ 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      എങ്കിൽ( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_വലത്->iColumn, 0);
        പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വലത്, EP_Int_മൂല്യം) );
        എങ്കിൽ( *pisComplete && p_വലത്->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          പൂർണ്ണസംഖ്യ r1 = sqlite3GetTempReg(pParse);
          sqlite3_വ്യഞ്ജകം_Code_Target(pParse, p_വലത്, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }അല്ലെങ്കിൽ{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_മൂല്യം_Free(pVal);
  തിരികെ rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
പൂർണ്ണസംഖ്യ sqlite3_വ്യഞ്ജകം_Is_Like_Operator(സ്ഥിരം വ്യഞ്ജകം *p_വ്യഞ്ജകം){
  സ്ഥിതം സ്ഥിരം ഘടന {
    സ്ഥിരം അക്ഷരം *zOp;
    unsigned അക്ഷരം eOp;
  } aOp[] = {
    { "match",  SQLITE_INDEX_CONSTRAINT_MATCH },
    { "glob",   SQLITE_INDEX_CONSTRAINT_GLOB },
    { "like",   SQLITE_INDEX_CONSTRAINT_LIKE },
    { "regexp", SQLITE_INDEX_CONSTRAINT_REGEXP }
  };
  പൂർണ്ണസംഖ്യ i;
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_FUNCTION );
  പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
  വേണ്ടി(i=0; i<ArraySize(aOp); i++){
    എങ്കിൽ( sqlite3StrICmp(p_വ്യഞ്ജകം->u.zToken, aOp[i].zOp)==0 ){
      തിരികെ aOp[i].eOp;
    }
  }
  തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,                    /* Test this expression */
  unsigned അക്ഷരം *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  വ്യഞ്ജകം **pp_ഇടത്,                  /* Column expression to left of MATCH/op2 */
  വ്യഞ്ജകം **pp_വലത്                  /* Expression to left of MATCH/op2 */
){
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_FUNCTION ){
    വ്യഞ്ജകം_List *pList;
    വ്യഞ്ജകം *pCol;                     /* Column reference */
    പൂർണ്ണസംഖ്യ i;

    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
    pList = p_വ്യഞ്ജകം->x.pList;
    എങ്കിൽ( pList==0 || pList->n_വ്യഞ്ജകം!=2 ){
      തിരികെ 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].p_വ്യഞ്ജകം;
    പ്രതിജ്ഞാ( pCol->op!=TK_COLUMN || (വ്യഞ്ജകം_Use_YTab(pCol) && pCol->y.pTab!=0) );
    എങ്കിൽ( വ്യഞ്ജകം_Is_Vtab(pCol) && (i = sqlite3_വ്യഞ്ജകം_Is_Like_Operator(p_വ്യഞ്ജകം))!=0 ){
      *peOp2 = i;
      *pp_വലത് = pList->a[0].p_വ്യഞ്ജകം;
      *pp_ഇടത് = pCol;
      തിരികെ 1;
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
    pCol = pList->a[0].p_വ്യഞ്ജകം;
    പ്രതിജ്ഞാ( pCol->op!=TK_COLUMN || വ്യഞ്ജകം_Use_YTab(pCol) );
    പ്രതിജ്ഞാ( pCol->op!=TK_COLUMN || (വ്യഞ്ജകം_Use_YTab(pCol) && pCol->y.pTab!=0) );
    എങ്കിൽ( വ്യഞ്ജകം_Is_Vtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      ശൂന്യം (*xNotUsed)(sqlite3_context*,പൂർണ്ണസംഖ്യ,sqlite3_മൂല്യം**);
      ശൂന്യം *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      പ്രതിജ്ഞാ( pVtab!=0 );
      പ്രതിജ്ഞാ( pVtab->pModule!=0 );
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      pMod = (sqlite3_module *)pVtab->pModule;
      എങ്കിൽ( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, p_വ്യഞ്ജകം->u.zToken, &xNotUsed, &pNotUsed);
        എങ്കിൽ( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_വലത് = pList->a[1].p_വ്യഞ്ജകം;
          *pp_ഇടത് = pCol;
          തിരികെ 1;
        }
      }
    }
  }അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    പ്രതിജ്ഞാ( TK_NE < TK_EQ );
    പ്രതിജ്ഞാ( TK_ISNOT < TK_EQ );
    പ്രതിജ്ഞാ( TK_NOTNULL < TK_EQ );
    തിരികെ 0;
  }അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_NE || p_വ്യഞ്ജകം->op==TK_ISNOT || p_വ്യഞ്ജകം->op==TK_NOTNULL ){
    പൂർണ്ണസംഖ്യ res = 0;
    വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
    വ്യഞ്ജകം *p_വലത് = p_വ്യഞ്ജകം->p_വലത്;
    പ്രതിജ്ഞാ( p_ഇടത്->op!=TK_COLUMN || (വ്യഞ്ജകം_Use_YTab(p_ഇടത്) && p_ഇടത്->y.pTab!=0) );
    എങ്കിൽ( വ്യഞ്ജകം_Is_Vtab(p_ഇടത്) ){
      res++;
    }
    പ്രതിജ്ഞാ( p_വലത്==0 || p_വലത്->op!=TK_COLUMN
            || (വ്യഞ്ജകം_Use_YTab(p_വലത്) && p_വലത്->y.pTab!=0) );
    എങ്കിൽ( p_വലത് && വ്യഞ്ജകം_Is_Vtab(p_വലത്) ){
      res++;
      SWAP(വ്യഞ്ജകം*, p_ഇടത്, p_വലത്);
    }
    *pp_ഇടത് = p_ഇടത്;
    *pp_വലത് = p_വലത്;
    എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    തിരികെ res;
  }
  തിരികെ 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
സ്ഥിതം ശൂന്യം transferJoinMarkings(വ്യഞ്ജകം *pDerived, വ്യഞ്ജകം *pBase){
  എങ്കിൽ( pDerived && വ്യഞ്ജകം_Has_Property(pBase, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= pBase->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
സ്ഥിതം ശൂന്യം markTermAsChild(WhereClause *pWC, പൂർണ്ണസംഖ്യ iChild, പൂർണ്ണസംഖ്യ iParent){
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;
  പ്രതിജ്ഞാ(   pWC->a[iParent].nChild <  UMXV(pWC->a[0].nChild) );
  pWC->a[iParent].nChild++;
  testcase( pWC->a[iParent].nChild == UMXV(pWC->a[0].nChild) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
സ്ഥിതം WhereTerm *whereNthSubterm(WhereTerm *pTerm, പൂർണ്ണസംഖ്യ N){
  എങ്കിൽ( pTerm->eOperator!=WO_AND ){
    തിരികെ N==0 ? pTerm : 0;
  }
  എങ്കിൽ( N<pTerm->u.pAndInfo->wc.nTerm ){
    തിരികെ &pTerm->u.pAndInfo->wc.a[N];
  }
  തിരികെ 0;
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
സ്ഥിതം ശൂന്യം whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  വ്യഞ്ജകം *pNew;            /* New virtual expression */
  പൂർണ്ണസംഖ്യ op;                /* Operator for the combined expression */
  പൂർണ്ണസംഖ്യ idxNew;            /* Index in pWC of the next virtual term */
  വ്യഞ്ജകം *pA, *pB;         /* Expressions associated with pOne and pTwo */

  എങ്കിൽ( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) തിരികെ;
  എങ്കിൽ( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) തിരികെ;
  എങ്കിൽ( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) തിരികെ;
  എങ്കിൽ( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) തിരികെ;
  pA = pOne->p_വ്യഞ്ജകം;
  pB = pTwo->p_വ്യഞ്ജകം;
  പ്രതിജ്ഞാ( pA->p_ഇടത്!=0 && pA->p_വലത്!=0 );
  പ്രതിജ്ഞാ( pB->p_ഇടത്!=0 && pB->p_വലത്!=0 );
  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(0,pA->p_ഇടത്, pB->p_ഇടത്, -1) )  തിരികെ;
  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare(0,pA->p_വലത്, pB->p_വലത്,-1) ) തിരികെ;
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(pA,EP_Commuted)!=വ്യഞ്ജകം_Has_Property(pB,EP_Commuted) ){
    തിരികെ;
  }
  /* If we reach this point, it means the two subterms can be combined */
  എങ്കിൽ( (eOp & (eOp-1))!=0 ){
    എങ്കിൽ( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }അല്ലെങ്കിൽ{
      പ്രതിജ്ഞാ( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_വ്യഞ്ജകം_Dup(db, pA, 0);
  എങ്കിൽ( pNew==0 ) തിരികെ;
  വേണ്ടി(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ പ്രതിജ്ഞാ( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_തിരുകുക(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  വ്യഞ്ജകം_Analyze(pSrc, pWC, idxNew);
}

#എങ്കിൽ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
സ്ഥിതം ശൂന്യം വ്യഞ്ജകം_Analyze_Or_Term(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  പൂർണ്ണസംഖ്യ idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം = pTerm->p_വ്യഞ്ജകം;             /* The expression of the term */
  പൂർണ്ണസംഖ്യ i;                                  /* Loop counters */
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
  പ്രതിജ്ഞാ( (pTerm->wtFlags & (TERM_DYNAMIC|TERM_ORINFO|TERM_ANDINFO))==0 );
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, വലുപ്പം(*pOrInfo));
  എങ്കിൽ( pOrInfo==0 ) തിരികെ;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, വലുപ്പം(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, p_വ്യഞ്ജകം, TK_OR);
  sqlite3_Where_വ്യഞ്ജകം_Analyze(pSrc, pOrWc);
  എങ്കിൽ( db->mallocFailed ) തിരികെ;
  പ്രതിജ്ഞാ( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  വേണ്ടി(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    എങ്കിൽ( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      പ്രതിജ്ഞാ( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, വലുപ്പം(*pAndInfo));
      എങ്കിൽ( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        പൂർണ്ണസംഖ്യ j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->ഇടത്_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, വലുപ്പം(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->p_വ്യഞ്ജകം, TK_AND);
        sqlite3_Where_വ്യഞ്ജകം_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        എങ്കിൽ( !db->mallocFailed ){
          വേണ്ടി(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            പ്രതിജ്ഞാ( pAndTerm->p_വ്യഞ്ജകം );
            എങ്കിൽ( allowedOp(pAndTerm->p_വ്യഞ്ജകം->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->ഇടത്_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }അല്ലെങ്കിൽ എങ്കിൽ( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }അല്ലെങ്കിൽ{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->ഇടത്_Cursor);
      എങ്കിൽ( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->ഇടത്_Cursor);
      }
      indexable &= b;
      എങ്കിൽ( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }അല്ലെങ്കിൽ{
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
  pTerm->ഇടത്_Cursor = -1;
  എങ്കിൽ( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  എങ്കിൽ( indexable && pOrWc->nTerm==2 ){
    പൂർണ്ണസംഖ്യ iOne = 0;
    WhereTerm *pOne;
    ആകുമ്പോൾ( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      പൂർണ്ണസംഖ്യ iTwo = 0;
      WhereTerm *pTwo;
      ആകുമ്പോൾ( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  എങ്കിൽ( chngToIN ){
    പൂർണ്ണസംഖ്യ okToChngToIN = 0;     /* True if the conversion to IN is valid */
    പൂർണ്ണസംഖ്യ iColumn = -1;         /* Column index on lhs of IN operator */
    പൂർണ്ണസംഖ്യ iCursor = -1;         /* Table cursor common to all terms */
    പൂർണ്ണസംഖ്യ j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    വേണ്ടി(j=0; j<2 && !okToChngToIN; j++){
      വ്യഞ്ജകം *p_ഇടത് = 0;
      pOrTerm = pOrWc->a;
      വേണ്ടി(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        പ്രതിജ്ഞാ( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        എങ്കിൽ( pOrTerm->ഇടത്_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          പ്രതിജ്ഞാ( j==1 );
          തുടരുക;
        }
        എങ്കിൽ( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->ഇടത്_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          പ്രതിജ്ഞാ( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          തുടരുക;
        }
        പ്രതിജ്ഞാ( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        iColumn = pOrTerm->u.x.ഇടത്_Column;
        iCursor = pOrTerm->ഇടത്_Cursor;
        p_ഇടത് = pOrTerm->p_വ്യഞ്ജകം->p_ഇടത്;
        നിർത്തുക;
      }
      എങ്കിൽ( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        പ്രതിജ്ഞാ( j==1 );
        പ്രതിജ്ഞാ( IsPowerOfTwo(chngToIN) );
        പ്രതിജ്ഞാ( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        നിർത്തുക;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      വേണ്ടി(; i>=0 && okToChngToIN; i--, pOrTerm++){
        പ്രതിജ്ഞാ( pOrTerm->eOperator & WO_EQ );
        പ്രതിജ്ഞാ( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        എങ്കിൽ( pOrTerm->ഇടത്_Cursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }അല്ലെങ്കിൽ എങ്കിൽ( pOrTerm->u.x.ഇടത്_Column!=iColumn || (iColumn==XN_വ്യഞ്ജകം
               && sqlite3_വ്യഞ്ജകം_Compare(pParse, pOrTerm->p_വ്യഞ്ജകം->p_ഇടത്, p_ഇടത്, -1)
        )){
          okToChngToIN = 0;
        }അല്ലെങ്കിൽ{
          പൂർണ്ണസംഖ്യ aff_ഇടത്, aff_വലത്;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_വലത് = sqlite3_വ്യഞ്ജകം_Affinity(pOrTerm->p_വ്യഞ്ജകം->p_വലത്);
          aff_ഇടത് = sqlite3_വ്യഞ്ജകം_Affinity(pOrTerm->p_വ്യഞ്ജകം->p_ഇടത്);
          എങ്കിൽ( aff_വലത്!=0 && aff_വലത്!=aff_ഇടത് ){
            okToChngToIN = 0;
          }അല്ലെങ്കിൽ{
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
    എങ്കിൽ( okToChngToIN ){
      വ്യഞ്ജകം *pDup;            /* A transient duplicate expression */
      വ്യഞ്ജകം_List *pList = 0;   /* The RHS of the IN operator */
      വ്യഞ്ജകം *p_ഇടത് = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      വ്യഞ്ജകം *pNew;            /* The complete IN operator */

      വേണ്ടി(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        വ്യഞ്ജകം *pThis;
        എങ്കിൽ( (pOrTerm->wtFlags & TERM_OK)==0 ) തുടരുക;
        പ്രതിജ്ഞാ( pOrTerm->eOperator & WO_EQ );
        പ്രതിജ്ഞാ( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        പ്രതിജ്ഞാ( pOrTerm->ഇടത്_Cursor==iCursor );
        പ്രതിജ്ഞാ( pOrTerm->u.x.ഇടത്_Column==iColumn );
        pThis = pOrTerm->p_വ്യഞ്ജകം;
        pDup = sqlite3_വ്യഞ്ജകം_Dup(db, pThis->p_വലത്, 0);
        pList = sqlite3_വ്യഞ്ജകം_List_Append(pWInfo->pParse, pList, pDup);
        എങ്കിൽ( p_ഇടത്==0 ){
          p_ഇടത് = pThis->p_ഇടത്;
          pCollSeq = sqlite3_വ്യഞ്ജകം_Compare_Coll_Seq(pParse, pThis);
        }അല്ലെങ്കിൽ{
          പ്രതിജ്ഞാ( 0==sqlite3_വ്യഞ്ജകം_Compare(pParse, 
                        sqlite3_വ്യഞ്ജകം_Skip_Collate(pThis->p_ഇടത്),
                        sqlite3_വ്യഞ്ജകം_Skip_Collate(p_ഇടത്), -1) );
          എങ്കിൽ( pCollSeq!=sqlite3_വ്യഞ്ജകം_Compare_Coll_Seq(pParse, pThis) ){
            p_ഇടത് = 0;  /* Collating sequence mismatch */
            നിർത്തുക;
          }
        }
      }
      എങ്കിൽ( p_ഇടത്==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }അല്ലെങ്കിൽ{
        pDup = sqlite3_വ്യഞ്ജകം_Dup(db, p_ഇടത്, 0);
        എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          പ്രതിജ്ഞാ( pCollSeq->zName!=0 );
          pDup = sqlite3_വ്യഞ്ജകം_Add_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      എങ്കിൽ( pNew ){
        പൂർണ്ണസംഖ്യ idxNew;
        transferJoinMarkings(pNew, p_വ്യഞ്ജകം);
        പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_തിരുകുക(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        വ്യഞ്ജകം_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }അല്ലെങ്കിൽ{
        sqlite3_വ്യഞ്ജകം_List_Delete(db, pList);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ termIsEquivalence(Parse *pParse, വ്യഞ്ജകം *p_വ്യഞ്ജകം, SrcList *pSrc){
  അക്ഷരം aff1, aff2;
  എങ്കിൽ( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) തിരികെ 0;  /* (1) */
  എങ്കിൽ( p_വ്യഞ്ജകം->op!=TK_EQ && p_വ്യഞ്ജകം->op!=TK_IS ) തിരികെ 0;                 /* (2) */
  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON|EP_Collate) ) തിരികെ 0;        /* (3) */
  പ്രതിജ്ഞാ( pSrc!=0 );
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    തിരികെ 0;                                                          /* (4) */
  }
  aff1 = sqlite3_വ്യഞ്ജകം_Affinity(p_വ്യഞ്ജകം->p_ഇടത്);
  aff2 = sqlite3_വ്യഞ്ജകം_Affinity(p_വ്യഞ്ജകം->p_വലത്);
  എങ്കിൽ( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    തിരികെ 0;                                                          /* (5) */
  }
  എങ്കിൽ( !sqlite3_വ്യഞ്ജകം_Coll_Seq_Match(pParse, p_വ്യഞ്ജകം->p_ഇടത്, p_വ്യഞ്ജകം->p_വലത്) ){
    തിരികെ 0;                                                          /* (6) */
  }
  തിരികെ 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
സ്ഥിതം Bitmask വ്യഞ്ജകം_Select_Usage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  ആകുമ്പോൾ( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3_Where_വ്യഞ്ജകം_List_Usage(pMaskSet, pS->pEList);
    mask |= sqlite3_Where_വ്യഞ്ജകം_List_Usage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3_Where_വ്യഞ്ജകം_List_Usage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3_Where_വ്യഞ്ജകം_Usage(pMaskSet, pS->pWhere);
    mask |= sqlite3_Where_വ്യഞ്ജകം_Usage(pMaskSet, pS->pHaving);
    എങ്കിൽ( ALWAYS(pSrc!=0) ){
      പൂർണ്ണസംഖ്യ i;
      വേണ്ടി(i=0; i<pSrc->nSrc; i++){
        എങ്കിൽ( pSrc->a[i].fg.isSubquery ){
          mask |= വ്യഞ്ജകം_Select_Usage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        എങ്കിൽ( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3_Where_വ്യഞ്ജകം_Usage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        എങ്കിൽ( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3_Where_വ്യഞ്ജകം_List_Usage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  തിരികെ mask;
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
സ്ഥിതം SQLITE_NOINLINE പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  പൂർണ്ണസംഖ്യ *aiCurCol,         /* Write the referenced table cursor and column here */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,           /* An operand of a comparison operator */
  പൂർണ്ണസംഖ്യ j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  പൂർണ്ണസംഖ്യ i;
  പൂർണ്ണസംഖ്യ iCur;
  ചെയ്യുക{
    iCur = pFrom->a[j].iCursor;
    വേണ്ടി(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      എങ്കിൽ( pIdx->a_Col_വ്യഞ്ജകം==0 ) തുടരുക;
      വേണ്ടി(i=0; i<pIdx->n_താക്കോൽ_Col; i++){
        എങ്കിൽ( pIdx->aiColumn[i]!=XN_വ്യഞ്ജകം ) തുടരുക;
        പ്രതിജ്ഞാ( pIdx->b_Has_വ്യഞ്ജകം );
        എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Compare_Skip(p_വ്യഞ്ജകം,pIdx->a_Col_വ്യഞ്ജകം->a[i].p_വ്യഞ്ജകം,iCur)==0
         && !sqlite3_വ്യഞ്ജകം_Is_Constant(0,pIdx->a_Col_വ്യഞ്ജകം->a[i].p_വ്യഞ്ജകം)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_വ്യഞ്ജകം;
          തിരികെ 1;
        }
      }
    }
  }ആകുമ്പോൾ( ++j < pFrom->nSrc );
  തിരികെ 0;
}
സ്ഥിതം പൂർണ്ണസംഖ്യ വ്യഞ്ജകം_Might_Be_Indexed(
  SrcList *pFrom,        /* The FROM clause */
  പൂർണ്ണസംഖ്യ *aiCurCol,         /* Write the referenced table cursor & column here */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,           /* An operand of a comparison operator */
  പൂർണ്ണസംഖ്യ op                 /* The specific comparison operator */
){
  പൂർണ്ണസംഖ്യ i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  പ്രതിജ്ഞാ( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  പ്രതിജ്ഞാ( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  പ്രതിജ്ഞാ( op<=TK_GE );
  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
    p_വ്യഞ്ജകം = p_വ്യഞ്ജകം->x.pList->a[0].p_വ്യഞ്ജകം;
  }

  എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_COLUMN ){
    aiCurCol[0] = p_വ്യഞ്ജകം->iTable;
    aiCurCol[1] = p_വ്യഞ്ജകം->iColumn;
    തിരികെ 1;
  }

  വേണ്ടി(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    വേണ്ടി(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      എങ്കിൽ( pIdx->a_Col_വ്യഞ്ജകം ){
        തിരികെ വ്യഞ്ജകം_Might_Be_Indexed2(pFrom,aiCurCol,p_വ്യഞ്ജകം,i);
      }
    }
  }
  തിരികെ 0;
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
സ്ഥിതം ശൂന്യം വ്യഞ്ജകം_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  പൂർണ്ണസംഖ്യ idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം;                     /* The expression to be analyzed */
  Bitmask prereq_ഇടത്;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_വലത് = 0;          /* Extra dependencies on LEFT JOIN */
  വ്യഞ്ജകം *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  പൂർണ്ണസംഖ്യ isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  പൂർണ്ണസംഖ്യ noCase = 0;                  /* uppercase equivalent to lowercase */
  പൂർണ്ണസംഖ്യ op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned അക്ഷരം eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  പൂർണ്ണസംഖ്യ n_ഇടത്;                       /* Number of elements on left side vector */

  എങ്കിൽ( db->mallocFailed ){
    തിരികെ;
  }
  പ്രതിജ്ഞാ( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  p_വ്യഞ്ജകം = pTerm->p_വ്യഞ്ജകം;
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം!=0 ); /* Because malloc() has not failed */
  പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->op!=TK_AS && p_വ്യഞ്ജകം->op!=TK_COLLATE );
വ്യഞ്ജകം_Analyze_restart:
  pMaskSet->bVarSelect = 0;
  prereq_ഇടത് = sqlite3_Where_വ്യഞ്ജകം_Usage(pMaskSet, p_വ്യഞ്ജകം->p_ഇടത്);
  op = p_വ്യഞ്ജകം->op;
  എങ്കിൽ( op==TK_IN ){
    പ്രതിജ്ഞാ( p_വ്യഞ്ജകം->p_വലത്==0 );
    എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Check_IN(pParse, p_വ്യഞ്ജകം) ) തിരികെ;
    എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) ){
      pTerm->prereq_വലത് = വ്യഞ്ജകം_Select_Usage(pMaskSet, p_വ്യഞ്ജകം->x.pSelect);
    }അല്ലെങ്കിൽ{
      pTerm->prereq_വലത് = sqlite3_Where_വ്യഞ്ജകം_List_Usage(pMaskSet, p_വ്യഞ്ജകം->x.pList);
    }
    prereqAll = prereq_ഇടത് | pTerm->prereq_വലത്;
  }അല്ലെങ്കിൽ{
    pTerm->prereq_വലത് = sqlite3_Where_വ്യഞ്ജകം_Usage(pMaskSet, p_വ്യഞ്ജകം->p_വലത്);
    എങ്കിൽ( p_വ്യഞ്ജകം->p_ഇടത്==0
     || വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_xIsSelect|EP_IfNullRow)
     || p_വ്യഞ്ജകം->x.pList!=0
    ){
      prereqAll = sqlite3_Where_വ്യഞ്ജകം_Usage_NN(pMaskSet, p_വ്യഞ്ജകം);
    }അല്ലെങ്കിൽ{
      prereqAll = prereq_ഇടത് | pTerm->prereq_വലത്;
    }
  }
  എങ്കിൽ( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  എങ്കിൽ( prereqAll!=sqlite3_Where_വ്യഞ്ജകം_Usage_NN(pMaskSet, p_വ്യഞ്ജകം) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_വൃക്ഷം_View_വ്യഞ്ജകം(0,p_വ്യഞ്ജകം,0);
    പ്രതിജ്ഞാ( 0 );
  }
#endif

  എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, p_വ്യഞ്ജകം->w.iJoin);
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON) ){
      prereqAll |= x;
      extra_വലത് = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }അല്ലെങ്കിൽ എങ്കിൽ( (prereqAll>>1)>=x ){
      വ്യഞ്ജകം_Clear_Property(p_വ്യഞ്ജകം, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->ഇടത്_Cursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  എങ്കിൽ( allowedOp(op) ){
    പൂർണ്ണസംഖ്യ aiCurCol[2];
    വ്യഞ്ജകം *p_ഇടത് = sqlite3_വ്യഞ്ജകം_Skip_Collate(p_വ്യഞ്ജകം->p_ഇടത്);
    വ്യഞ്ജകം *p_വലത് = sqlite3_വ്യഞ്ജകം_Skip_Collate(p_വ്യഞ്ജകം->p_വലത്);
    u16 opMask = (pTerm->prereq_വലത് & prereq_ഇടത്)==0 ? WO_ALL : WO_EQUIV;

    എങ്കിൽ( pTerm->u.x.iField>0 ){
      പ്രതിജ്ഞാ( op==TK_IN );
      പ്രതിജ്ഞാ( p_ഇടത്->op==TK_VECTOR );
      പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_ഇടത്) );
      p_ഇടത് = p_ഇടത്->x.pList->a[pTerm->u.x.iField-1].p_വ്യഞ്ജകം;
    }

    എങ്കിൽ( വ്യഞ്ജകം_Might_Be_Indexed(pSrc, aiCurCol, p_ഇടത്, op) ){
      pTerm->ഇടത്_Cursor = aiCurCol[0];
      പ്രതിജ്ഞാ( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.ഇടത്_Column = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    എങ്കിൽ( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    എങ്കിൽ( p_വലത്
     && വ്യഞ്ജകം_Might_Be_Indexed(pSrc, aiCurCol, p_വലത്, op)
     && !വ്യഞ്ജകം_Has_Property(p_വലത്, EP_FixedCol)
    ){
      WhereTerm *pNew;
      വ്യഞ്ജകം *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      പ്രതിജ്ഞാ( pTerm->u.x.iField==0 );
      എങ്കിൽ( pTerm->ഇടത്_Cursor>=0 ){
        പൂർണ്ണസംഖ്യ idxNew;
        pDup = sqlite3_വ്യഞ്ജകം_Dup(db, p_വ്യഞ്ജകം, 0);
        എങ്കിൽ( db->mallocFailed ){
          sqlite3_വ്യഞ്ജകം_Delete(db, pDup);
          തിരികെ;
        }
        idxNew = where_Clause_തിരുകുക(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        എങ്കിൽ( idxNew==0 ) തിരികെ;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        എങ്കിൽ( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        പ്രതിജ്ഞാ( pWInfo->pTabList!=0 );
        എങ്കിൽ( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }അല്ലെങ്കിൽ{
        pDup = p_വ്യഞ്ജകം;
        pNew = pTerm;
      }
      pNew->wtFlags |= വ്യഞ്ജകം_Commute(pParse, pDup);
      pNew->ഇടത്_Cursor = aiCurCol[0];
      പ്രതിജ്ഞാ( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.ഇടത്_Column = aiCurCol[1];
      testcase( (prereq_ഇടത് | extra_വലത്) != prereq_ഇടത് );
      pNew->prereq_വലത് = prereq_ഇടത് | extra_വലത്;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }അല്ലെങ്കിൽ
    എങ്കിൽ( op==TK_ISNULL
     && !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം,EP_OuterON)
     && 0==sqlite3_വ്യഞ്ജകം_Can_Be_Null(p_ഇടത്)
    ){
      പ്രതിജ്ഞാ( !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Int_മൂല്യം) );
      p_വ്യഞ്ജകം->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_വ്യഞ്ജകം->u.zToken = "false";
      വ്യഞ്ജകം_Set_Property(p_വ്യഞ്ജകം, EP_IsFalse);
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
  അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_BETWEEN && pWC->op==TK_AND ){
    വ്യഞ്ജകം_List *pList;
    പൂർണ്ണസംഖ്യ i;
    സ്ഥിതം സ്ഥിരം u8 ops[] = {TK_GE, TK_LE};
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
    pList = p_വ്യഞ്ജകം->x.pList;
    പ്രതിജ്ഞാ( pList!=0 );
    പ്രതിജ്ഞാ( pList->n_വ്യഞ്ജകം==2 );
    പ്രതിജ്ഞാ( pWC->a[idxTerm].nChild==0 );
    വേണ്ടി(i=0; i<2; i++){
      വ്യഞ്ജകം *p_New_വ്യഞ്ജകം;
      പൂർണ്ണസംഖ്യ idxNew;
      p_New_വ്യഞ്ജകം = sqlite3PExpr(pParse, ops[i],
                             sqlite3_വ്യഞ്ജകം_Dup(db, p_വ്യഞ്ജകം->p_ഇടത്, 0),
                             sqlite3_വ്യഞ്ജകം_Dup(db, pList->a[i].p_വ്യഞ്ജകം, 0));
      transferJoinMarkings(p_New_വ്യഞ്ജകം, p_വ്യഞ്ജകം);
      idxNew = where_Clause_തിരുകുക(pWC, p_New_വ്യഞ്ജകം, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      വ്യഞ്ജകം_Analyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#എങ്കിൽ !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_OR && !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_Collate) ){
    വ്യഞ്ജകം *pAlt = sqlite3_വ്യഞ്ജകം_Simplified_And_Or(p_വ്യഞ്ജകം);
    എങ്കിൽ( pAlt!=p_വ്യഞ്ജകം ){
      pTerm->p_വ്യഞ്ജകം = p_വ്യഞ്ജകം = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(pAlt);
      പോകുക വ്യഞ്ജകം_Analyze_restart;
    }
    പ്രതിജ്ഞാ( pWC->op==TK_AND );
    വ്യഞ്ജകം_Analyze_Or_Term(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_NOTNULL ){
    എങ്കിൽ( p_വ്യഞ്ജകം->p_ഇടത്->op==TK_COLUMN
     && p_വ്യഞ്ജകം->p_ഇടത്->iColumn>=0
     && !വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON)
    ){
      വ്യഞ്ജകം *p_New_വ്യഞ്ജകം;
      വ്യഞ്ജകം *p_ഇടത് = p_വ്യഞ്ജകം->p_ഇടത്;
      പൂർണ്ണസംഖ്യ idxNew;
      WhereTerm *pNewTerm;
 
      p_New_വ്യഞ്ജകം = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_വ്യഞ്ജകം_Dup(db, p_ഇടത്, 0),
                              sqlite3_വ്യഞ്ജകം_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_തിരുകുക(pWC, p_New_വ്യഞ്ജകം,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      എങ്കിൽ( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_വലത് = 0;
        pNewTerm->ഇടത്_Cursor = p_ഇടത്->iTable;
        pNewTerm->u.x.ഇടത്_Column = p_ഇടത്->iColumn;
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
  അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_വ്യഞ്ജകം, &pStr1, &isComplete, &noCase)
  ){
    വ്യഞ്ജകം *p_ഇടത്;       /* LHS of LIKE/GLOB operator */
    വ്യഞ്ജകം *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    വ്യഞ്ജകം *pNewExpr1;
    വ്യഞ്ജകം *pNewExpr2;
    പൂർണ്ണസംഖ്യ idxNew1;
    പൂർണ്ണസംഖ്യ idxNew2;
    സ്ഥിരം അക്ഷരം *zCollSeqName;     /* Name of collating sequence */
    സ്ഥിരം u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_XList(p_വ്യഞ്ജകം) );
    p_ഇടത് = p_വ്യഞ്ജകം->x.pList->a[1].p_വ്യഞ്ജകം;
    pStr2 = sqlite3_വ്യഞ്ജകം_Dup(db, pStr1, 0);
    പ്രതിജ്ഞാ( pStr1==0 || !വ്യഞ്ജകം_Has_Property(pStr1, EP_Int_മൂല്യം) );
    പ്രതിജ്ഞാ( pStr2==0 || !വ്യഞ്ജകം_Has_Property(pStr2, EP_Int_മൂല്യം) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    എങ്കിൽ( noCase && !pParse->db->mallocFailed ){
      പൂർണ്ണസംഖ്യ i;
      അക്ഷരം c;
      pTerm->wtFlags |= TERM_LIKE;
      വേണ്ടി(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    എങ്കിൽ( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      എങ്കിൽ( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        എങ്കിൽ( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      ആകുമ്പോൾ( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      പ്രതിജ്ഞാ( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_ദ്വിമാന;
    pNewExpr1 = sqlite3_വ്യഞ്ജകം_Dup(db, p_ഇടത്, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_വ്യഞ്ജകം_Add_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_വ്യഞ്ജകം);
    idxNew1 = where_Clause_തിരുകുക(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_വ്യഞ്ജകം_Dup(db, p_ഇടത്, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_വ്യഞ്ജകം_Add_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_വ്യഞ്ജകം);
    idxNew2 = where_Clause_തിരുകുക(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    വ്യഞ്ജകം_Analyze(pSrc, pWC, idxNew1);
    വ്യഞ്ജകം_Analyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    എങ്കിൽ( isComplete ){
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
  എങ്കിൽ( (p_വ്യഞ്ജകം->op==TK_EQ || p_വ്യഞ്ജകം->op==TK_IS)
   && (n_ഇടത് = sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം->p_ഇടത്))>1
   && sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം->p_വലത്)==n_ഇടത്
   && ( (p_വ്യഞ്ജകം->p_ഇടത്->flags & EP_xIsSelect)==0
     || (p_വ്യഞ്ജകം->p_വലത്->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    പൂർണ്ണസംഖ്യ i;
    വേണ്ടി(i=0; i<n_ഇടത്; i++){
      പൂർണ്ണസംഖ്യ idxNew;
      വ്യഞ്ജകം *pNew;
      വ്യഞ്ജകം *p_ഇടത് = sqlite3_വ്യഞ്ജകം_For_Vector_Field(pParse, p_വ്യഞ്ജകം->p_ഇടത്, i, n_ഇടത്);
      വ്യഞ്ജകം *p_വലത് = sqlite3_വ്യഞ്ജകം_For_Vector_Field(pParse, p_വ്യഞ്ജകം->p_വലത്, i, n_ഇടത്);

      pNew = sqlite3PExpr(pParse, p_വ്യഞ്ജകം->op, p_ഇടത്, p_വലത്);
      transferJoinMarkings(pNew, p_വ്യഞ്ജകം);
      idxNew = where_Clause_തിരുകുക(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      വ്യഞ്ജകം_Analyze(pSrc, pWC, idxNew);
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
  അല്ലെങ്കിൽ എങ്കിൽ( p_വ്യഞ്ജകം->op==TK_IN
   && pTerm->u.x.iField==0
   && p_വ്യഞ്ജകം->p_ഇടത്->op==TK_VECTOR
   && ALWAYS( വ്യഞ്ജകം_Use_XSelect(p_വ്യഞ്ജകം) )
   && (p_വ്യഞ്ജകം->x.pSelect->pPrior==0 || (p_വ്യഞ്ജകം->x.pSelect->selFlags & SF_മൂല്യങ്ങൾ))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p_വ്യഞ്ജകം->x.pSelect->pWin==0
#endif
   && (p_വ്യഞ്ജകം->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_വ്യഞ്ജകം->x.pSelect->pEList->n_വ്യഞ്ജകം <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    പൂർണ്ണസംഖ്യ i;
    പ്രതിജ്ഞാ( pTerm->nChild==0 );
    വേണ്ടി(i=0; i<sqlite3_വ്യഞ്ജകം_Vector_Size(p_വ്യഞ്ജകം->p_ഇടത്); i++){
      പൂർണ്ണസംഖ്യ idxNew;
      idxNew = where_Clause_തിരുകുക(pWC, p_വ്യഞ്ജകം, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      വ്യഞ്ജകം_Analyze(pSrc, pWC, idxNew);
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
  അല്ലെങ്കിൽ എങ്കിൽ( pWC->op==TK_AND ){
    വ്യഞ്ജകം *p_വലത് = 0, *p_ഇടത് = 0;
    പൂർണ്ണസംഖ്യ res = isAuxiliaryVtabOperator(db, p_വ്യഞ്ജകം, &eOp2, &p_ഇടത്, &p_വലത്);
    ആകുമ്പോൾ( res-- > 0 ){
      പൂർണ്ണസംഖ്യ idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereq_വ്യഞ്ജകം;

      prereq_വ്യഞ്ജകം = sqlite3_Where_വ്യഞ്ജകം_Usage(pMaskSet, p_വലത്);
      prereqColumn = sqlite3_Where_വ്യഞ്ജകം_Usage(pMaskSet, p_ഇടത്);
      എങ്കിൽ( (prereq_വ്യഞ്ജകം & prereqColumn)==0 ){
        വ്യഞ്ജകം *p_New_വ്യഞ്ജകം;
        p_New_വ്യഞ്ജകം = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3_വ്യഞ്ജകം_Dup(db, p_വലത്, 0));
        എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p_വ്യഞ്ജകം, EP_OuterON) && p_New_വ്യഞ്ജകം ){
          വ്യഞ്ജകം_Set_Property(p_New_വ്യഞ്ജകം, EP_OuterON);
          p_New_വ്യഞ്ജകം->w.iJoin = p_വ്യഞ്ജകം->w.iJoin;
        }
        idxNew = where_Clause_തിരുകുക(pWC, p_New_വ്യഞ്ജകം, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_വലത് = prereq_വ്യഞ്ജകം | extra_വലത്;
        pNewTerm->ഇടത്_Cursor = p_ഇടത്->iTable;
        pNewTerm->u.x.ഇടത്_Column = p_ഇടത്->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(വ്യഞ്ജകം*, p_ഇടത്, p_വലത്);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereq_വലത് |= extra_വലത്;
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
ശൂന്യം sqlite3WhereSplit(WhereClause *pWC, വ്യഞ്ജകം *p_വ്യഞ്ജകം, u8 op){
  വ്യഞ്ജകം *pE2 = sqlite3_വ്യഞ്ജകം_Skip_Collate_And_Likely(p_വ്യഞ്ജകം);
  pWC->op = op;
  പ്രതിജ്ഞാ( pE2!=0 || p_വ്യഞ്ജകം==0 );
  എങ്കിൽ( pE2==0 ) തിരികെ;
  എങ്കിൽ( pE2->op!=op ){
    where_Clause_തിരുകുക(pWC, p_വ്യഞ്ജകം, 0);
  }അല്ലെങ്കിൽ{
    sqlite3WhereSplit(pWC, pE2->p_ഇടത്, op);
    sqlite3WhereSplit(pWC, pE2->p_വലത്, op);
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
സ്ഥിതം ശൂന്യം where_Add_Limit_വ്യഞ്ജകം(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  പൂർണ്ണസംഖ്യ iReg,           /* Register that will hold value of the limit/offset */
  വ്യഞ്ജകം *p_വ്യഞ്ജകം,        /* Expression that defines the limit/offset */
  പൂർണ്ണസംഖ്യ iCsr,           /* Cursor to which the constraint applies */
  പൂർണ്ണസംഖ്യ eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  വ്യഞ്ജകം *pNew;
  പൂർണ്ണസംഖ്യ iVal = 0;

  എങ്കിൽ( sqlite3_വ്യഞ്ജകം_Is_Integer(p_വ്യഞ്ജകം, &iVal, pParse, 0) && iVal>=0 ){
    വ്യഞ്ജകം *pVal = sqlite3_വ്യഞ്ജകം_Int32(db, iVal);
    എങ്കിൽ( pVal==0 ) തിരികെ;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }അല്ലെങ്കിൽ{
    വ്യഞ്ജകം *pVal = sqlite3_വ്യഞ്ജകം_Alloc(db, TK_REGISTER, 0, 0);
    എങ്കിൽ( pVal==0 ) തിരികെ;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  എങ്കിൽ( pNew ){
    WhereTerm *pTerm;
    പൂർണ്ണസംഖ്യ idx;
    idx = where_Clause_തിരുകുക(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->ഇടത്_Cursor = iCsr;
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
ശൂന്യം SQLITE_NOINLINE sqlite3WhereAddLimit(WhereClause *pWC, Select *p){
  പ്രതിജ്ഞാ( p!=0 && p->pLimit!=0 );                 /* 1 -- checked by caller */
  എങ്കിൽ( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    വ്യഞ്ജകം_List *pOrderBy = p->pOrderBy;
    പൂർണ്ണസംഖ്യ iCsr = p->pSrc->a[0].iCursor;
    പൂർണ്ണസംഖ്യ ii;

    /* Check condition (4). Return early if it is not met. */
    വേണ്ടി(ii=0; ii<pWC->nTerm; ii++){
      എങ്കിൽ( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        പ്രതിജ്ഞാ( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        പ്രതിജ്ഞാ( pWC->a[ii].eOperator==WO_ROWVAL );
        തുടരുക;
      }
      എങ്കിൽ( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        തുടരുക;
      }
      എങ്കിൽ( pWC->a[ii].ഇടത്_Cursor==iCsr && pWC->a[ii].prereq_വലത്==0 ) തുടരുക;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      എങ്കിൽ( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        എങ്കിൽ( pParent->ഇടത്_Cursor==iCsr
         && pParent->prereq_വലത്==0
         && pParent->nChild==1
        ){
          തുടരുക;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      തിരികെ;
    }

    /* Check condition (5). Return early if it is not met. */
    എങ്കിൽ( pOrderBy ){
      വേണ്ടി(ii=0; ii<pOrderBy->n_വ്യഞ്ജകം; ii++){
        വ്യഞ്ജകം *p_വ്യഞ്ജകം = pOrderBy->a[ii].p_വ്യഞ്ജകം;
        എങ്കിൽ( p_വ്യഞ്ജകം->op!=TK_COLUMN ) തിരികെ;
        എങ്കിൽ( p_വ്യഞ്ജകം->iTable!=iCsr ) തിരികെ;
        എങ്കിൽ( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) തിരികെ;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    പ്രതിജ്ഞാ( p->pLimit->op==TK_LIMIT );
    എങ്കിൽ( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_വ്യഞ്ജകം(pWC, p->iOffset, p->pLimit->p_വലത്,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    എങ്കിൽ( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_വ്യഞ്ജകം(pWC, p->iLimit, p->pLimit->p_ഇടത്,
                        iCsr, SQLITE_INDEX_CONSTRAINT_LIMIT);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
ശൂന്യം sqlite3WhereClauseInit(
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
ശൂന്യം sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  പ്രതിജ്ഞാ( pWC->nTerm>=pWC->nBase );
  എങ്കിൽ( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    പൂർണ്ണസംഖ്യ i;
    /* Verify that every term past pWC->nBase is virtual */
    വേണ്ടി(i=pWC->nBase; i<pWC->nTerm; i++){
      പ്രതിജ്ഞാ( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    ആകുമ്പോൾ(1){
      പ്രതിജ്ഞാ( a->eMatchOp==0 || a->eOperator==WO_AUX );
      എങ്കിൽ( a->wtFlags & TERM_DYNAMIC ){
        sqlite3_വ്യഞ്ജകം_Delete(db, a->p_വ്യഞ്ജകം);
      }
      എങ്കിൽ( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        എങ്കിൽ( a->wtFlags & TERM_ORINFO ){
          പ്രതിജ്ഞാ( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }അല്ലെങ്കിൽ{
          പ്രതിജ്ഞാ( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      എങ്കിൽ( a==aLast ) നിർത്തുക;
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
സ്ഥിതം SQLITE_NOINLINE Bitmask sqlite3_Where_വ്യഞ്ജകം_Usage_Full(
  WhereMaskSet *pMaskSet,
  വ്യഞ്ജകം *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  എങ്കിൽ( p->p_ഇടത് ) mask |= sqlite3_Where_വ്യഞ്ജകം_Usage_NN(pMaskSet, p->p_ഇടത്);
  എങ്കിൽ( p->p_വലത് ){
    mask |= sqlite3_Where_വ്യഞ്ജകം_Usage_NN(pMaskSet, p->p_വലത്);
    പ്രതിജ്ഞാ( p->x.pList==0 );
  }അല്ലെങ്കിൽ എങ്കിൽ( വ്യഞ്ജകം_Use_XSelect(p) ){
    എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= വ്യഞ്ജകം_Select_Usage(pMaskSet, p->x.pSelect);
  }അല്ലെങ്കിൽ എങ്കിൽ( p->x.pList ){
    mask |= sqlite3_Where_വ്യഞ്ജകം_List_Usage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  എങ്കിൽ( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && വ്യഞ്ജകം_Use_YWin(p) ){
    പ്രതിജ്ഞാ( p->y.pWin!=0 );
    mask |= sqlite3_Where_വ്യഞ്ജകം_List_Usage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3_Where_വ്യഞ്ജകം_List_Usage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3_Where_വ്യഞ്ജകം_Usage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  തിരികെ mask;
}
Bitmask sqlite3_Where_വ്യഞ്ജകം_Usage_NN(WhereMaskSet *pMaskSet, വ്യഞ്ജകം *p){
  എങ്കിൽ( p->op==TK_COLUMN && !വ്യഞ്ജകം_Has_Property(p, EP_FixedCol) ){
    തിരികെ sqlite3WhereGetMask(pMaskSet, p->iTable);
  }അല്ലെങ്കിൽ എങ്കിൽ( വ്യഞ്ജകം_Has_Property(p, EP_TokenOnly|EP_Leaf) ){
    പ്രതിജ്ഞാ( p->op!=TK_IF_NULL_ROW );
    തിരികെ 0;
  }
  തിരികെ sqlite3_Where_വ്യഞ്ജകം_Usage_Full(pMaskSet, p);
}
Bitmask sqlite3_Where_വ്യഞ്ജകം_Usage(WhereMaskSet *pMaskSet, വ്യഞ്ജകം *p){
  തിരികെ p ? sqlite3_Where_വ്യഞ്ജകം_Usage_NN(pMaskSet,p) : 0;
}
Bitmask sqlite3_Where_വ്യഞ്ജകം_List_Usage(WhereMaskSet *pMaskSet, വ്യഞ്ജകം_List *pList){
  പൂർണ്ണസംഖ്യ i;
  Bitmask mask = 0;
  എങ്കിൽ( pList ){
    വേണ്ടി(i=0; i<pList->n_വ്യഞ്ജകം; i++){
      mask |= sqlite3_Where_വ്യഞ്ജകം_Usage(pMaskSet, pList->a[i].p_വ്യഞ്ജകം);
    }
  }
  തിരികെ mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
ശൂന്യം sqlite3_Where_വ്യഞ്ജകം_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  പൂർണ്ണസംഖ്യ i;
  വേണ്ടി(i=pWC->nTerm-1; i>=0; i--){
    വ്യഞ്ജകം_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
ശൂന്യം sqlite3WhereTabFuncArgs(
  Parse *pParse,                    /* Parsing context */
  SrcItem *pItem,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  Table *pTab;
  പൂർണ്ണസംഖ്യ j, k;
  വ്യഞ്ജകം_List *pArgs;
  വ്യഞ്ജകം *pColRef;
  വ്യഞ്ജകം *pTerm;
  എങ്കിൽ( pItem->fg.isTabFunc==0 ) തിരികെ;
  pTab = pItem->pSTab;
  പ്രതിജ്ഞാ( pTab!=0 );
  എങ്കിൽ( !IsVirtual(pTab) ){
    sqlite3_Error_സന്ദേശം(pParse, "'%s' is not a function", pItem->zName);
    തിരികെ;
  }
  pArgs = pItem->u1.pFuncArg;
  എങ്കിൽ( pArgs==0 ) തിരികെ;
  വേണ്ടി(j=k=0; j<pArgs->n_വ്യഞ്ജകം; j++){
    വ്യഞ്ജകം *pRhs;
    u32 joinType;
    ആകുമ്പോൾ( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    എങ്കിൽ( k>=pTab->nCol ){
      sqlite3_Error_സന്ദേശം(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      തിരികെ;
    }
    pColRef = sqlite3_വ്യഞ്ജകം_Alloc(pParse->db, TK_COLUMN, 0, 0);
    എങ്കിൽ( pColRef==0 ) തിരികെ;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    പ്രതിജ്ഞാ( വ്യഞ്ജകം_Use_YTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3_വ്യഞ്ജകം_Col_Used(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_വ്യഞ്ജകം_Dup(pParse->db, pArgs->a[j].p_വ്യഞ്ജകം, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    എങ്കിൽ( pItem->fg.jointype & (JT_ഇടത്|JT_വലത്) ){
      testcase( pItem->fg.jointype & JT_ഇടത് );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_വലത് ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }അല്ലെങ്കിൽ{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_Set_Join_വ്യഞ്ജകം(pTerm, pItem->iCursor, joinType);
    where_Clause_തിരുകുക(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
പൂർണ്ണസംഖ്യ sqlite3WhereLoopBloomable(സ്ഥിരം WhereLoop *pLoop){
  എങ്കിൽ( pLoop->wsFlags & WHERE_IPK ) തിരികെ 1;
  എങ്കിൽ( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) തിരികെ 0;
  തിരികെ sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
