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
#அந்தர்பாவ "sqliteInt.h"
#அந்தர்பாவ "whereInt.h"

/* Forward declarations */
நிலையான வெறுமை கோவை_Analyze(SrcList*, WhereClause*, முழு);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
நிலையான வெறுமை whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
நிலையான வெறுமை whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
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
நிலையான முழு where_Clause_செருகு(WhereClause *pWC, கோவை *p, u16 wtFlags){
  WhereTerm *pTerm;
  முழு idx;
  testcase( wtFlags & TERM_VIRTUAL );
  எனில்( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3WhereMalloc(pWC->pWInfo, அளவு(pWC->a[0])*pWC->nSlot*2 );
    எனில்( pWC->a==0 ){
      எனில்( wtFlags & TERM_DYNAMIC ){
        sqlite3_கோவை_Delete(db, p);
      }
      pWC->a = pOld;
      திரும்பு 0;
    }
    memcpy(pWC->a, pOld, அளவு(pWC->a[0])*pWC->nTerm);
    pWC->nSlot = pWC->nSlot*2;
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  எனில்( (wtFlags & TERM_VIRTUAL)==0 ) pWC->nBase = pWC->nTerm;
  எனில்( p && கோவை_Has_Property(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }இல்லையெனில்{
    pTerm->truthProb = 1;
  }
  pTerm->p_கோவை = sqlite3_கோவை_Skip_Collate_And_Likely(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  memset(&pTerm->eOperator, 0,
         அளவு(WhereTerm) - offsetof(WhereTerm,eOperator));
  திரும்பு idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", "IS", and "IS NULL"
*/
நிலையான முழு allowedOp(முழு op){
  ப்ரதிஜ்ஞா( TK_GT>TK_EQ && TK_GT<TK_GE );
  ப்ரதிஜ்ஞா( TK_LT>TK_EQ && TK_LT<TK_GE );
  ப்ரதிஜ்ஞா( TK_LE>TK_EQ && TK_LE<TK_GE );
  ப்ரதிஜ்ஞா( TK_GE==TK_EQ+4 );
  ப்ரதிஜ்ஞா( TK_IN<TK_EQ );
  ப்ரதிஜ்ஞா( TK_IS<TK_EQ );
  ப்ரதிஜ்ஞா( TK_ISNULL<TK_EQ );
  எனில்( op>TK_GE ) திரும்பு 0;
  எனில்( op>=TK_EQ ) திரும்பு 1;
  திரும்பு op==TK_IN || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
*/
நிலையான u16 கோவை_Commute(Parse *pParse, கோவை *p_கோவை){
  எனில்( p_கோவை->p_இடது->op==TK_VECTOR
   || p_கோவை->p_வலது->op==TK_VECTOR
   || sqlite3_இருமம்_Compare_Coll_Seq(pParse, p_கோவை->p_இடது, p_கோவை->p_வலது) !=
      sqlite3_இருமம்_Compare_Coll_Seq(pParse, p_கோவை->p_வலது, p_கோவை->p_இடது)
  ){
    p_கோவை->flags ^= EP_Commuted;
  }
  SWAP(கோவை*,p_கோவை->p_வலது,p_கோவை->p_இடது);
  எனில்( p_கோவை->op>=TK_GT ){
    ப்ரதிஜ்ஞா( TK_LT==TK_GT+2 );
    ப்ரதிஜ்ஞா( TK_GE==TK_LE+2 );
    ப்ரதிஜ்ஞா( TK_GT>TK_EQ );
    ப்ரதிஜ்ஞா( TK_GT<TK_LE );
    ப்ரதிஜ்ஞா( p_கோவை->op>=TK_GT && p_கோவை->op<=TK_GE );
    p_கோவை->op = ((p_கோவை->op-TK_GT)^2)+TK_GT;
  }
  திரும்பு 0;
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
நிலையான u16 operatorMask(முழு op){
  u16 c;
  ப்ரதிஜ்ஞா( allowedOp(op) );
  எனில்( op>=TK_EQ ){
    ப்ரதிஜ்ஞா( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }இல்லையெனில் எனில்( op==TK_IN ){
    c = WO_IN;
  }இல்லையெனில் எனில்( op==TK_ISNULL ){
    c = WO_ISNULL;
  }இல்லையெனில்{
    ப்ரதிஜ்ஞா( op==TK_IS );
    c = WO_IS;
  }
  ப்ரதிஜ்ஞா( op!=TK_ISNULL || c==WO_ISNULL );
  ப்ரதிஜ்ஞா( op!=TK_IN || c==WO_IN );
  ப்ரதிஜ்ஞா( op!=TK_EQ || c==WO_EQ );
  ப்ரதிஜ்ஞா( op!=TK_LT || c==WO_LT );
  ப்ரதிஜ்ஞா( op!=TK_LE || c==WO_LE );
  ப்ரதிஜ்ஞா( op!=TK_GT || c==WO_GT );
  ப்ரதிஜ்ஞா( op!=TK_GE || c==WO_GE );
  ப்ரதிஜ்ஞா( op!=TK_IS || c==WO_IS );
  திரும்பு c;
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
நிலையான முழு isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  கோவை *p_கோவை,      /* Test this expression */
  கோவை **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  முழு *pisComplete, /* True if the only wildcard is % in the last character */
  முழு *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  மாறிலி u8 *z = 0;           /* String on RHS of LIKE operator */
  கோவை *p_வலது, *p_இடது;      /* Right and left size of LIKE operator */
  கோவை_List *pList;           /* List of operands to the LIKE operator */
  u8 c;                      /* One character in z[] */
  முழு cnt;                   /* Number of non-wildcard prefix characters */
  u8 wc[4];                  /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_மதிப்பு *pVal = 0;
  முழு op;                    /* Opcode of pRight */
  முழு rc;                    /* Result code to return */

  எனில்( !sqlite3IsLikeFunction(db, p_கோவை, pnoCase, (எழுத்து*)wc) ){
    திரும்பு 0;
  }
#ifdef SQLITE_EBCDIC
  எனில்( *pnoCase ) திரும்பு 0;
#endif
  ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
  pList = p_கோவை->x.pList;
  p_இடது = pList->a[1].p_கோவை;

  p_வலது = sqlite3_கோவை_Skip_Collate(pList->a[0].p_கோவை);
  op = p_வலது->op;
  எனில்( op==TK_VARIABLE && (db->flags & SQLITE_EnableQPSG)==0 ){
    Vdbe *pReprepare = pParse->pReprepare;
    முழு iCol = p_வலது->iColumn;
    pVal = sqlite3_Vdbe_Get_Bound_மதிப்பு(pReprepare, iCol, SQLITE_AFF_BLOB);
    எனில்( pVal && sqlite3_மதிப்பு_type(pVal)==SQLITE_TEXT ){
      z = sqlite3_மதிப்பு_text(pVal);
    }
    sqlite3VdbeReprepareOnBind(pParse->pVdbe, iCol, 0);
    ப்ரதிஜ்ஞா( p_வலது->op==TK_VARIABLE || p_வலது->op==TK_REGISTER );
  }இல்லையெனில் எனில்( op==TK_STRING ){
    ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_வலது, EP_Int_மதிப்பு) );
     z = (u8*)p_வலது->u.zToken;
  }
  எனில்( z ){
    /* Count the number of prefix bytes prior to the first wildcard,
    ** U+fffd character, or malformed utf-8. If the underlying database
    ** has a UTF16LE encoding, then only consider ASCII characters.  Note that
    ** the encoding of z[] is UTF8 - we are dealing with only UTF8 here in this
    ** code, but the database engine itself might be processing content using a
    ** different encoding. */
    cnt = 0;
    போது( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
      எனில்( c==wc[3] && z[cnt]>0 && z[cnt]<0x80 ){
        cnt++;
      }இல்லையெனில் எனில்( c>=0x80 ){
        மாறிலி u8 *z2 = z+cnt-1;
        எனில்( c==0xff || sqlite3Utf8Read(&z2)==0xfffd  /* bad utf-8 */
         || ENC(db)==SQLITE_UTF16LE 
        ){
          cnt--;
          நிறுத்து;
        }இல்லையெனில்{
          cnt = (முழு)(z2-z);
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
    எனில்( (cnt>1 || (cnt>0 && z[0]!=wc[3])) && ALWAYS(255!=(u8)z[cnt-1]) ){
      கோவை *pPrefix;

      /* A "complete" match if the pattern ends with "*" or "%" */
      *pisComplete = c==wc[0] && z[cnt+1]==0 && ENC(db)!=SQLITE_UTF16LE;

      /* Get the pattern prefix.  Remove all escapes from the prefix. */
      pPrefix = sqlite3_கோவை(db, TK_STRING, (எழுத்து*)z);
      எனில்( pPrefix ){
        முழு iFrom, iTo;
        எழுத்து *zNew;
        ப்ரதிஜ்ஞா( !கோவை_Has_Property(pPrefix, EP_Int_மதிப்பு) );
        zNew = pPrefix->u.zToken;
        zNew[cnt] = 0;
        க்கு(iFrom=iTo=0; iFrom<cnt; iFrom++){
          எனில்( zNew[iFrom]==wc[3] ) iFrom++;
          zNew[iTo++] = zNew[iFrom];
        }
        zNew[iTo] = 0;
        ப்ரதிஜ்ஞா( iTo>0 );

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
        எனில்( p_இடது->op!=TK_COLUMN
         || sqlite3_கோவை_Affinity(p_இடது)!=SQLITE_AFF_TEXT
         || (ALWAYS( கோவை_Use_YTab(p_இடது) )
             && ALWAYS(p_இடது->y.pTab)
             && IsVirtual(p_இடது->y.pTab))  /* Might be numeric */
        ){
          முழு is_இலக்கம்;
          இரட்டை rDummy;
          ப்ரதிஜ்ஞா( zNew[iTo]==0 );
          is_இலக்கம் = sqlite3AtoF(zNew, &rDummy);
          எனில்( is_இலக்கம்<=0 ){
            எனில்( iTo==1 && zNew[0]=='-' ){
              is_இலக்கம் = +1;
            }இல்லையெனில்{
              zNew[iTo-1]++;
              is_இலக்கம் = sqlite3AtoF(zNew, &rDummy);
              zNew[iTo-1]--;
            }
          }
          எனில்( is_இலக்கம்>0 ){
            sqlite3_கோவை_Delete(db, pPrefix);
            sqlite3_மதிப்பு_Free(pVal);
            திரும்பு 0;
          }
        }
      }
      *ppPrefix = pPrefix;

      /* If the RHS pattern is a bound parameter, make arrangements to
      ** reprepare the statement when that parameter is rebound */
      எனில்( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeReprepareOnBind(v, p_வலது->iColumn, 0);
        ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_வலது, EP_Int_மதிப்பு) );
        எனில்( *pisComplete && p_வலது->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */
          முழு r1 = sqlite3GetTempReg(pParse);
          sqlite3_கோவை_Code_Target(pParse, p_வலது, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }இல்லையெனில்{
      z = 0;
    }
  }

  rc = (z!=0);
  sqlite3_மதிப்பு_Free(pVal);
  திரும்பு rc;
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

/*
** If pExpr is one of "like", "glob", "match", or "regexp", then
** return the corresponding SQLITE_INDEX_CONSTRAINT_xxxx value.
** If not, return 0.
**
** pExpr is guaranteed to be a TK_FUNCTION.
*/
முழு sqlite3_கோவை_Is_Like_Operator(மாறிலி கோவை *p_கோவை){
  நிலையான மாறிலி கட்டமைப்பு {
    மாறிலி எழுத்து *zOp;
    unsigned எழுத்து eOp;
  } aOp[] = {
    { "match",  SQLITE_INDEX_CONSTRAINT_MATCH },
    { "glob",   SQLITE_INDEX_CONSTRAINT_GLOB },
    { "like",   SQLITE_INDEX_CONSTRAINT_LIKE },
    { "regexp", SQLITE_INDEX_CONSTRAINT_REGEXP }
  };
  முழு i;
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_FUNCTION );
  ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
  க்கு(i=0; i<ArraySize(aOp); i++){
    எனில்( sqlite3StrICmp(p_கோவை->u.zToken, aOp[i].zOp)==0 ){
      திரும்பு aOp[i].eOp;
    }
  }
  திரும்பு 0;
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
நிலையான முழு isAuxiliaryVtabOperator(
  sqlite3 *db,                    /* Parsing context */
  கோவை *p_கோவை,                    /* Test this expression */
  unsigned எழுத்து *peOp2,           /* OUT: 0 for MATCH, or else an op2 value */
  கோவை **pp_இடது,                  /* Column expression to left of MATCH/op2 */
  கோவை **pp_வலது                  /* Expression to left of MATCH/op2 */
){
  எனில்( p_கோவை->op==TK_FUNCTION ){
    கோவை_List *pList;
    கோவை *pCol;                     /* Column reference */
    முழு i;

    ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
    pList = p_கோவை->x.pList;
    எனில்( pList==0 || pList->n_கோவை!=2 ){
      திரும்பு 0;
    }

    /* Built-in operators MATCH, GLOB, LIKE, and REGEXP attach to a
    ** virtual table on their second argument, which is the same as
    ** the left-hand side operand in their in-fix form.
    **
    **       vtab_column MATCH expression
    **       MATCH(expression,vtab_column)
    */
    pCol = pList->a[1].p_கோவை;
    ப்ரதிஜ்ஞா( pCol->op!=TK_COLUMN || (கோவை_Use_YTab(pCol) && pCol->y.pTab!=0) );
    எனில்( கோவை_Is_Vtab(pCol) && (i = sqlite3_கோவை_Is_Like_Operator(p_கோவை))!=0 ){
      *peOp2 = i;
      *pp_வலது = pList->a[0].p_கோவை;
      *pp_இடது = pCol;
      திரும்பு 1;
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
    pCol = pList->a[0].p_கோவை;
    ப்ரதிஜ்ஞா( pCol->op!=TK_COLUMN || கோவை_Use_YTab(pCol) );
    ப்ரதிஜ்ஞா( pCol->op!=TK_COLUMN || (கோவை_Use_YTab(pCol) && pCol->y.pTab!=0) );
    எனில்( கோவை_Is_Vtab(pCol) ){
      sqlite3_vtab *pVtab;
      sqlite3_module *pMod;
      வெறுமை (*xNotUsed)(sqlite3_context*,முழு,sqlite3_மதிப்பு**);
      வெறுமை *pNotUsed;
      pVtab = sqlite3GetVTable(db, pCol->y.pTab)->pVtab;
      ப்ரதிஜ்ஞா( pVtab!=0 );
      ப்ரதிஜ்ஞா( pVtab->pModule!=0 );
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      pMod = (sqlite3_module *)pVtab->pModule;
      எனில்( pMod->xFindFunction!=0 ){
        i = pMod->xFindFunction(pVtab,2, p_கோவை->u.zToken, &xNotUsed, &pNotUsed);
        எனில்( i>=SQLITE_INDEX_CONSTRAINT_FUNCTION ){
          *peOp2 = i;
          *pp_வலது = pList->a[1].p_கோவை;
          *pp_இடது = pCol;
          திரும்பு 1;
        }
      }
    }
  }இல்லையெனில் எனில்( p_கோவை->op>=TK_EQ ){
    /* Comparison operators are a common case.  Save a few comparisons for
    ** that common case by terminating early. */
    ப்ரதிஜ்ஞா( TK_NE < TK_EQ );
    ப்ரதிஜ்ஞா( TK_ISNOT < TK_EQ );
    ப்ரதிஜ்ஞா( TK_NOTNULL < TK_EQ );
    திரும்பு 0;
  }இல்லையெனில் எனில்( p_கோவை->op==TK_NE || p_கோவை->op==TK_ISNOT || p_கோவை->op==TK_NOTNULL ){
    முழு res = 0;
    கோவை *p_இடது = p_கோவை->p_இடது;
    கோவை *p_வலது = p_கோவை->p_வலது;
    ப்ரதிஜ்ஞா( p_இடது->op!=TK_COLUMN || (கோவை_Use_YTab(p_இடது) && p_இடது->y.pTab!=0) );
    எனில்( கோவை_Is_Vtab(p_இடது) ){
      res++;
    }
    ப்ரதிஜ்ஞா( p_வலது==0 || p_வலது->op!=TK_COLUMN
            || (கோவை_Use_YTab(p_வலது) && p_வலது->y.pTab!=0) );
    எனில்( p_வலது && கோவை_Is_Vtab(p_வலது) ){
      res++;
      SWAP(கோவை*, p_இடது, p_வலது);
    }
    *pp_இடது = p_இடது;
    *pp_வலது = p_வலது;
    எனில்( p_கோவை->op==TK_NE ) *peOp2 = SQLITE_INDEX_CONSTRAINT_NE;
    எனில்( p_கோவை->op==TK_ISNOT ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOT;
    எனில்( p_கோவை->op==TK_NOTNULL ) *peOp2 = SQLITE_INDEX_CONSTRAINT_ISNOTNULL;
    திரும்பு res;
  }
  திரும்பு 0;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
நிலையான வெறுமை transferJoinMarkings(கோவை *pDerived, கோவை *pBase){
  எனில்( pDerived && கோவை_Has_Property(pBase, EP_OuterON|EP_InnerON) ){
    pDerived->flags |= pBase->flags & (EP_OuterON|EP_InnerON);
    pDerived->w.iJoin = pBase->w.iJoin;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
நிலையான வெறுமை markTermAsChild(WhereClause *pWC, முழு iChild, முழு iParent){
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;
  ப்ரதிஜ்ஞா(   pWC->a[iParent].nChild <  UMXV(pWC->a[0].nChild) );
  pWC->a[iParent].nChild++;
  testcase( pWC->a[iParent].nChild == UMXV(pWC->a[0].nChild) );

}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
நிலையான WhereTerm *whereNthSubterm(WhereTerm *pTerm, முழு N){
  எனில்( pTerm->eOperator!=WO_AND ){
    திரும்பு N==0 ? pTerm : 0;
  }
  எனில்( N<pTerm->u.pAndInfo->wc.nTerm ){
    திரும்பு &pTerm->u.pAndInfo->wc.a[N];
  }
  திரும்பு 0;
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
நிலையான வெறுமை whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  கோவை *pNew;            /* New virtual expression */
  முழு op;                /* Operator for the combined expression */
  முழு idxNew;            /* Index in pWC of the next virtual term */
  கோவை *pA, *pB;         /* Expressions associated with pOne and pTwo */

  எனில்( (pOne->wtFlags | pTwo->wtFlags) & TERM_VNULL ) திரும்பு;
  எனில்( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) திரும்பு;
  எனில்( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) திரும்பு;
  எனில்( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) திரும்பு;
  pA = pOne->p_கோவை;
  pB = pTwo->p_கோவை;
  ப்ரதிஜ்ஞா( pA->p_இடது!=0 && pA->p_வலது!=0 );
  ப்ரதிஜ்ஞா( pB->p_இடது!=0 && pB->p_வலது!=0 );
  எனில்( sqlite3_கோவை_Compare(0,pA->p_இடது, pB->p_இடது, -1) )  திரும்பு;
  எனில்( sqlite3_கோவை_Compare(0,pA->p_வலது, pB->p_வலது,-1) ) திரும்பு;
  எனில்( கோவை_Has_Property(pA,EP_Commuted)!=கோவை_Has_Property(pB,EP_Commuted) ){
    திரும்பு;
  }
  /* If we reach this point, it means the two subterms can be combined */
  எனில்( (eOp & (eOp-1))!=0 ){
    எனில்( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }இல்லையெனில்{
      ப்ரதிஜ்ஞா( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3_கோவை_Dup(db, pA, 0);
  எனில்( pNew==0 ) திரும்பு;
  க்கு(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ ப்ரதிஜ்ஞா( op<TK_GE ); }
  pNew->op = op;
  idxNew = where_Clause_செருகு(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  கோவை_Analyze(pSrc, pWC, idxNew);
}

#எனில் !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
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
நிலையான வெறுமை கோவை_Analyze_Or_Term(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  முழு idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  கோவை *p_கோவை = pTerm->p_கோவை;             /* The expression of the term */
  முழு i;                                  /* Loop counters */
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
  ப்ரதிஜ்ஞா( (pTerm->wtFlags & (TERM_DYNAMIC|TERM_ORINFO|TERM_ANDINFO))==0 );
  ப்ரதிஜ்ஞா( p_கோவை->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, அளவு(*pOrInfo));
  எனில்( pOrInfo==0 ) திரும்பு;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  memset(pOrWc->aStatic, 0, அளவு(pOrWc->aStatic));
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, p_கோவை, TK_OR);
  sqlite3_Where_கோவை_Analyze(pSrc, pOrWc);
  எனில்( db->mallocFailed ) திரும்பு;
  ப்ரதிஜ்ஞா( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  க்கு(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    எனில்( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      ப்ரதிஜ்ஞா( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRawNN(db, அளவு(*pAndInfo));
      எனில்( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        முழு j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pOrTerm->இடது_Cursor = -1;
        pAndWC = &pAndInfo->wc;
        memset(pAndWC->aStatic, 0, அளவு(pAndWC->aStatic));
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->p_கோவை, TK_AND);
        sqlite3_Where_கோவை_Analyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        எனில்( !db->mallocFailed ){
          க்கு(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            ப்ரதிஜ்ஞா( pAndTerm->p_கோவை );
            எனில்( allowedOp(pAndTerm->p_கோவை->op)
             || pAndTerm->eOperator==WO_AUX
            ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->இடது_Cursor);
            }
          }
        }
        indexable &= b;
      }
    }இல்லையெனில் எனில்( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }இல்லையெனில்{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->இடது_Cursor);
      எனில்( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->இடது_Cursor);
      }
      indexable &= b;
      எனில்( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }இல்லையெனில்{
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
  pTerm->இடது_Cursor = -1;
  எனில்( indexable ){
    pWC->hasOr = 1;
  }

  /* For a two-way OR, attempt to implementation case 2.
  */
  எனில்( indexable && pOrWc->nTerm==2 ){
    முழு iOne = 0;
    WhereTerm *pOne;
    போது( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      முழு iTwo = 0;
      WhereTerm *pTwo;
      போது( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
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
  எனில்( chngToIN ){
    முழு okToChngToIN = 0;     /* True if the conversion to IN is valid */
    முழு iColumn = -1;         /* Column index on lhs of IN operator */
    முழு iCursor = -1;         /* Table cursor common to all terms */
    முழு j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    க்கு(j=0; j<2 && !okToChngToIN; j++){
      கோவை *p_இடது = 0;
      pOrTerm = pOrWc->a;
      க்கு(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        ப்ரதிஜ்ஞா( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OK;
        எனில்( pOrTerm->இடது_Cursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          ப்ரதிஜ்ஞா( j==1 );
          தொடர்;
        }
        எனில்( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->இடது_Cursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or followed by an inverted copy (t2.b==t1.a).  Skip this term
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          ப்ரதிஜ்ஞா( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          தொடர்;
        }
        ப்ரதிஜ்ஞா( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        iColumn = pOrTerm->u.x.இடது_Column;
        iCursor = pOrTerm->இடது_Cursor;
        p_இடது = pOrTerm->p_கோவை->p_இடது;
        நிறுத்து;
      }
      எனில்( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        ப்ரதிஜ்ஞா( j==1 );
        ப்ரதிஜ்ஞா( IsPowerOfTwo(chngToIN) );
        ப்ரதிஜ்ஞா( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        நிறுத்து;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      க்கு(; i>=0 && okToChngToIN; i--, pOrTerm++){
        ப்ரதிஜ்ஞா( pOrTerm->eOperator & WO_EQ );
        ப்ரதிஜ்ஞா( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        எனில்( pOrTerm->இடது_Cursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OK;
        }இல்லையெனில் எனில்( pOrTerm->u.x.இடது_Column!=iColumn || (iColumn==XN_கோவை
               && sqlite3_கோவை_Compare(pParse, pOrTerm->p_கோவை->p_இடது, p_இடது, -1)
        )){
          okToChngToIN = 0;
        }இல்லையெனில்{
          முழு aff_இடது, aff_வலது;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          aff_வலது = sqlite3_கோவை_Affinity(pOrTerm->p_கோவை->p_வலது);
          aff_இடது = sqlite3_கோவை_Affinity(pOrTerm->p_கோவை->p_இடது);
          எனில்( aff_வலது!=0 && aff_வலது!=aff_இடது ){
            okToChngToIN = 0;
          }இல்லையெனில்{
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
    எனில்( okToChngToIN ){
      கோவை *pDup;            /* A transient duplicate expression */
      கோவை_List *pList = 0;   /* The RHS of the IN operator */
      கோவை *p_இடது = 0;       /* The LHS of the IN operator */
      CollSeq *pCollSeq = 0; /* Collating sequence to use */
      கோவை *pNew;            /* The complete IN operator */

      க்கு(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        கோவை *pThis;
        எனில்( (pOrTerm->wtFlags & TERM_OK)==0 ) தொடர்;
        ப்ரதிஜ்ஞா( pOrTerm->eOperator & WO_EQ );
        ப்ரதிஜ்ஞா( (pOrTerm->eOperator & (WO_OR|WO_AND))==0 );
        ப்ரதிஜ்ஞா( pOrTerm->இடது_Cursor==iCursor );
        ப்ரதிஜ்ஞா( pOrTerm->u.x.இடது_Column==iColumn );
        pThis = pOrTerm->p_கோவை;
        pDup = sqlite3_கோவை_Dup(db, pThis->p_வலது, 0);
        pList = sqlite3_கோவை_List_Append(pWInfo->pParse, pList, pDup);
        எனில்( p_இடது==0 ){
          p_இடது = pThis->p_இடது;
          pCollSeq = sqlite3_கோவை_Compare_Coll_Seq(pParse, pThis);
        }இல்லையெனில்{
          ப்ரதிஜ்ஞா( 0==sqlite3_கோவை_Compare(pParse, 
                        sqlite3_கோவை_Skip_Collate(pThis->p_இடது),
                        sqlite3_கோவை_Skip_Collate(p_இடது), -1) );
          எனில்( pCollSeq!=sqlite3_கோவை_Compare_Coll_Seq(pParse, pThis) ){
            p_இடது = 0;  /* Collating sequence mismatch */
            நிறுத்து;
          }
        }
      }
      எனில்( p_இடது==0 ){
        pNew = 0;  /* Collating sequence mismatch */
      }இல்லையெனில்{
        pDup = sqlite3_கோவை_Dup(db, p_இடது, 0);
        எனில்( sqlite3_கோவை_Coll_Seq(pParse, pDup)!=pCollSeq
         && pCollSeq!=0
        ){
          ப்ரதிஜ்ஞா( pCollSeq->zName!=0 );
          pDup = sqlite3_கோவை_Add_Collate_String(pParse, pDup, pCollSeq->zName);
        }
        pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0);
      }
      எனில்( pNew ){
        முழு idxNew;
        transferJoinMarkings(pNew, p_கோவை);
        ப்ரதிஜ்ஞா( கோவை_Use_XList(pNew) );
        pNew->x.pList = pList;
        idxNew = where_Clause_செருகு(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        கோவை_Analyze(pSrc, pWC, idxNew);
        /* pTerm = &pWC->a[idxTerm]; // would be needed if pTerm where reused */
        markTermAsChild(pWC, idxNew, idxTerm);
      }இல்லையெனில்{
        sqlite3_கோவை_List_Delete(db, pList);
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
நிலையான முழு termIsEquivalence(Parse *pParse, கோவை *p_கோவை, SrcList *pSrc){
  எழுத்து aff1, aff2;
  எனில்( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) திரும்பு 0;  /* (1) */
  எனில்( p_கோவை->op!=TK_EQ && p_கோவை->op!=TK_IS ) திரும்பு 0;                 /* (2) */
  எனில்( கோவை_Has_Property(p_கோவை, EP_OuterON|EP_Collate) ) திரும்பு 0;        /* (3) */
  ப்ரதிஜ்ஞா( pSrc!=0 );
  எனில்( p_கோவை->op==TK_IS
   && pSrc->nSrc>=2
   && (pSrc->a[0].fg.jointype & JT_LTORJ)!=0
  ){
    திரும்பு 0;                                                          /* (4) */
  }
  aff1 = sqlite3_கோவை_Affinity(p_கோவை->p_இடது);
  aff2 = sqlite3_கோவை_Affinity(p_கோவை->p_வலது);
  எனில்( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    திரும்பு 0;                                                          /* (5) */
  }
  எனில்( !sqlite3_கோவை_Coll_Seq_Match(pParse, p_கோவை->p_இடது, p_கோவை->p_வலது) ){
    திரும்பு 0;                                                          /* (6) */
  }
  திரும்பு 1;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
நிலையான Bitmask கோவை_Select_Usage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  போது( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3_Where_கோவை_List_Usage(pMaskSet, pS->pEList);
    mask |= sqlite3_Where_கோவை_List_Usage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3_Where_கோவை_List_Usage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3_Where_கோவை_Usage(pMaskSet, pS->pWhere);
    mask |= sqlite3_Where_கோவை_Usage(pMaskSet, pS->pHaving);
    எனில்( ALWAYS(pSrc!=0) ){
      முழு i;
      க்கு(i=0; i<pSrc->nSrc; i++){
        எனில்( pSrc->a[i].fg.isSubquery ){
          mask |= கோவை_Select_Usage(pMaskSet, pSrc->a[i].u4.pSubq->pSelect);
        }
        எனில்( pSrc->a[i].fg.isUsing==0 ){
          mask |= sqlite3_Where_கோவை_Usage(pMaskSet, pSrc->a[i].u3.pOn);
        }
        எனில்( pSrc->a[i].fg.isTabFunc ){
          mask |= sqlite3_Where_கோவை_List_Usage(pMaskSet, pSrc->a[i].u1.pFuncArg);
        }
      }
    }
    pS = pS->pPrior;
  }
  திரும்பு mask;
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
நிலையான SQLITE_NOINLINE முழு கோவை_Might_Be_Indexed2(
  SrcList *pFrom,        /* The FROM clause */
  முழு *aiCurCol,         /* Write the referenced table cursor and column here */
  கோவை *p_கோவை,           /* An operand of a comparison operator */
  முழு j                  /* Start looking with the j-th pFrom entry */
){
  Index *pIdx;
  முழு i;
  முழு iCur;
  செய்{
    iCur = pFrom->a[j].iCursor;
    க்கு(pIdx=pFrom->a[j].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      எனில்( pIdx->a_Col_கோவை==0 ) தொடர்;
      க்கு(i=0; i<pIdx->n_சாவி_Col; i++){
        எனில்( pIdx->aiColumn[i]!=XN_கோவை ) தொடர்;
        ப்ரதிஜ்ஞா( pIdx->b_Has_கோவை );
        எனில்( sqlite3_கோவை_Compare_Skip(p_கோவை,pIdx->a_Col_கோவை->a[i].p_கோவை,iCur)==0
         && !sqlite3_கோவை_Is_Constant(0,pIdx->a_Col_கோவை->a[i].p_கோவை)
        ){
          aiCurCol[0] = iCur;
          aiCurCol[1] = XN_கோவை;
          திரும்பு 1;
        }
      }
    }
  }போது( ++j < pFrom->nSrc );
  திரும்பு 0;
}
நிலையான முழு கோவை_Might_Be_Indexed(
  SrcList *pFrom,        /* The FROM clause */
  முழு *aiCurCol,         /* Write the referenced table cursor & column here */
  கோவை *p_கோவை,           /* An operand of a comparison operator */
  முழு op                 /* The specific comparison operator */
){
  முழு i;

  /* If this expression is a vector to the left or right of a
  ** inequality constraint (>, <, >= or <=), perform the processing
  ** on the first element of the vector.  */
  ப்ரதிஜ்ஞா( TK_GT+1==TK_LE && TK_GT+2==TK_LT && TK_GT+3==TK_GE );
  ப்ரதிஜ்ஞா( TK_IS<TK_GE && TK_ISNULL<TK_GE && TK_IN<TK_GE );
  ப்ரதிஜ்ஞா( op<=TK_GE );
  எனில்( p_கோவை->op==TK_VECTOR && (op>=TK_GT && ALWAYS(op<=TK_GE)) ){
    ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
    p_கோவை = p_கோவை->x.pList->a[0].p_கோவை;
  }

  எனில்( p_கோவை->op==TK_COLUMN ){
    aiCurCol[0] = p_கோவை->iTable;
    aiCurCol[1] = p_கோவை->iColumn;
    திரும்பு 1;
  }

  க்கு(i=0; i<pFrom->nSrc; i++){
    Index *pIdx;
    க்கு(pIdx=pFrom->a[i].pSTab->pIndex; pIdx; pIdx=pIdx->pNext){
      எனில்( pIdx->a_Col_கோவை ){
        திரும்பு கோவை_Might_Be_Indexed2(pFrom,aiCurCol,p_கோவை,i);
      }
    }
  }
  திரும்பு 0;
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
நிலையான வெறுமை கோவை_Analyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  முழு idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  கோவை *p_கோவை;                     /* The expression to be analyzed */
  Bitmask prereq_இடது;              /* Prerequisites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequisites of pExpr */
  Bitmask extra_வலது = 0;          /* Extra dependencies on LEFT JOIN */
  கோவை *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  முழு isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  முழு noCase = 0;                  /* uppercase equivalent to lowercase */
  முழு op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */
  unsigned எழுத்து eOp2 = 0;          /* op2 value for LIKE/REGEXP/GLOB */
  முழு n_இடது;                       /* Number of elements on left side vector */

  எனில்( db->mallocFailed ){
    திரும்பு;
  }
  ப்ரதிஜ்ஞா( pWC->nTerm > idxTerm );
  pTerm = &pWC->a[idxTerm];
#ifdef SQLITE_DEBUG
  pTerm->iTerm = idxTerm;
#endif
  pMaskSet = &pWInfo->sMaskSet;
  p_கோவை = pTerm->p_கோவை;
  ப்ரதிஜ்ஞா( p_கோவை!=0 ); /* Because malloc() has not failed */
  ப்ரதிஜ்ஞா( p_கோவை->op!=TK_AS && p_கோவை->op!=TK_COLLATE );
கோவை_Analyze_restart:
  pMaskSet->bVarSelect = 0;
  prereq_இடது = sqlite3_Where_கோவை_Usage(pMaskSet, p_கோவை->p_இடது);
  op = p_கோவை->op;
  எனில்( op==TK_IN ){
    ப்ரதிஜ்ஞா( p_கோவை->p_வலது==0 );
    எனில்( sqlite3_கோவை_Check_IN(pParse, p_கோவை) ) திரும்பு;
    எனில்( கோவை_Use_XSelect(p_கோவை) ){
      pTerm->prereq_வலது = கோவை_Select_Usage(pMaskSet, p_கோவை->x.pSelect);
    }இல்லையெனில்{
      pTerm->prereq_வலது = sqlite3_Where_கோவை_List_Usage(pMaskSet, p_கோவை->x.pList);
    }
    prereqAll = prereq_இடது | pTerm->prereq_வலது;
  }இல்லையெனில்{
    pTerm->prereq_வலது = sqlite3_Where_கோவை_Usage(pMaskSet, p_கோவை->p_வலது);
    எனில்( p_கோவை->p_இடது==0
     || கோவை_Has_Property(p_கோவை, EP_xIsSelect|EP_IfNullRow)
     || p_கோவை->x.pList!=0
    ){
      prereqAll = sqlite3_Where_கோவை_Usage_NN(pMaskSet, p_கோவை);
    }இல்லையெனில்{
      prereqAll = prereq_இடது | pTerm->prereq_வலது;
    }
  }
  எனில்( pMaskSet->bVarSelect ) pTerm->wtFlags |= TERM_VARSELECT;

#ifdef SQLITE_DEBUG
  எனில்( prereqAll!=sqlite3_Where_கோவை_Usage_NN(pMaskSet, p_கோவை) ){
    printf("\n*** Incorrect prereqAll computed for:\n");
    sqlite3_மரம்_View_கோவை(0,p_கோவை,0);
    ப்ரதிஜ்ஞா( 0 );
  }
#endif

  எனில்( கோவை_Has_Property(p_கோவை, EP_OuterON|EP_InnerON) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, p_கோவை->w.iJoin);
    எனில்( கோவை_Has_Property(p_கோவை, EP_OuterON) ){
      prereqAll |= x;
      extra_வலது = x-1;  /* ON clause terms may not be used with an index
                         ** on left table of a LEFT JOIN.  Ticket #3015 */
    }இல்லையெனில் எனில்( (prereqAll>>1)>=x ){
      கோவை_Clear_Property(p_கோவை, EP_InnerON);
    }
  }
  pTerm->prereqAll = prereqAll;
  pTerm->இடது_Cursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  எனில்( allowedOp(op) ){
    முழு aiCurCol[2];
    கோவை *p_இடது = sqlite3_கோவை_Skip_Collate(p_கோவை->p_இடது);
    கோவை *p_வலது = sqlite3_கோவை_Skip_Collate(p_கோவை->p_வலது);
    u16 opMask = (pTerm->prereq_வலது & prereq_இடது)==0 ? WO_ALL : WO_EQUIV;

    எனில்( pTerm->u.x.iField>0 ){
      ப்ரதிஜ்ஞா( op==TK_IN );
      ப்ரதிஜ்ஞா( p_இடது->op==TK_VECTOR );
      ப்ரதிஜ்ஞா( கோவை_Use_XList(p_இடது) );
      p_இடது = p_இடது->x.pList->a[pTerm->u.x.iField-1].p_கோவை;
    }

    எனில்( கோவை_Might_Be_Indexed(pSrc, aiCurCol, p_இடது, op) ){
      pTerm->இடது_Cursor = aiCurCol[0];
      ப்ரதிஜ்ஞா( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pTerm->u.x.இடது_Column = aiCurCol[1];
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    எனில்( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    எனில்( p_வலது
     && கோவை_Might_Be_Indexed(pSrc, aiCurCol, p_வலது, op)
     && !கோவை_Has_Property(p_வலது, EP_FixedCol)
    ){
      WhereTerm *pNew;
      கோவை *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      ப்ரதிஜ்ஞா( pTerm->u.x.iField==0 );
      எனில்( pTerm->இடது_Cursor>=0 ){
        முழு idxNew;
        pDup = sqlite3_கோவை_Dup(db, p_கோவை, 0);
        எனில்( db->mallocFailed ){
          sqlite3_கோவை_Delete(db, pDup);
          திரும்பு;
        }
        idxNew = where_Clause_செருகு(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        எனில்( idxNew==0 ) திரும்பு;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        எனில்( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        ப்ரதிஜ்ஞா( pWInfo->pTabList!=0 );
        எனில்( termIsEquivalence(pParse, pDup, pWInfo->pTabList) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }இல்லையெனில்{
        pDup = p_கோவை;
        pNew = pTerm;
      }
      pNew->wtFlags |= கோவை_Commute(pParse, pDup);
      pNew->இடது_Cursor = aiCurCol[0];
      ப்ரதிஜ்ஞா( (pTerm->eOperator & (WO_OR|WO_AND))==0 );
      pNew->u.x.இடது_Column = aiCurCol[1];
      testcase( (prereq_இடது | extra_வலது) != prereq_இடது );
      pNew->prereq_வலது = prereq_இடது | extra_வலது;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }இல்லையெனில்
    எனில்( op==TK_ISNULL
     && !கோவை_Has_Property(p_கோவை,EP_OuterON)
     && 0==sqlite3_கோவை_Can_Be_Null(p_இடது)
    ){
      ப்ரதிஜ்ஞா( !கோவை_Has_Property(p_கோவை, EP_Int_மதிப்பு) );
      p_கோவை->op = TK_TRUEFALSE;  /* See tag-20230504-1 */
      p_கோவை->u.zToken = "false";
      கோவை_Set_Property(p_கோவை, EP_IsFalse);
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
  இல்லையெனில் எனில்( p_கோவை->op==TK_BETWEEN && pWC->op==TK_AND ){
    கோவை_List *pList;
    முழு i;
    நிலையான மாறிலி u8 ops[] = {TK_GE, TK_LE};
    ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
    pList = p_கோவை->x.pList;
    ப்ரதிஜ்ஞா( pList!=0 );
    ப்ரதிஜ்ஞா( pList->n_கோவை==2 );
    ப்ரதிஜ்ஞா( pWC->a[idxTerm].nChild==0 );
    க்கு(i=0; i<2; i++){
      கோவை *p_New_கோவை;
      முழு idxNew;
      p_New_கோவை = sqlite3PExpr(pParse, ops[i],
                             sqlite3_கோவை_Dup(db, p_கோவை->p_இடது, 0),
                             sqlite3_கோவை_Dup(db, pList->a[i].p_கோவை, 0));
      transferJoinMarkings(p_New_கோவை, p_கோவை);
      idxNew = where_Clause_செருகு(pWC, p_New_கோவை, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      கோவை_Analyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#எனில் !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  இல்லையெனில் எனில்( p_கோவை->op==TK_OR && !கோவை_Has_Property(p_கோவை, EP_Collate) ){
    கோவை *pAlt = sqlite3_கோவை_Simplified_And_Or(p_கோவை);
    எனில்( pAlt!=p_கோவை ){
      pTerm->p_கோவை = p_கோவை = sqlite3_கோவை_Skip_Collate_And_Likely(pAlt);
      செல் கோவை_Analyze_restart;
    }
    ப்ரதிஜ்ஞா( pWC->op==TK_AND );
    கோவை_Analyze_Or_Term(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */
  /* The form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** The virtual term must be tagged with TERM_VNULL.
  */
  இல்லையெனில் எனில்( p_கோவை->op==TK_NOTNULL ){
    எனில்( p_கோவை->p_இடது->op==TK_COLUMN
     && p_கோவை->p_இடது->iColumn>=0
     && !கோவை_Has_Property(p_கோவை, EP_OuterON)
    ){
      கோவை *p_New_கோவை;
      கோவை *p_இடது = p_கோவை->p_இடது;
      முழு idxNew;
      WhereTerm *pNewTerm;
 
      p_New_கோவை = sqlite3PExpr(pParse, TK_GT,
                              sqlite3_கோவை_Dup(db, p_இடது, 0),
                              sqlite3_கோவை_Alloc(db, TK_NULL, 0, 0));
 
      idxNew = where_Clause_செருகு(pWC, p_New_கோவை,
                                TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
      எனில்( idxNew ){
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_வலது = 0;
        pNewTerm->இடது_Cursor = p_இடது->iTable;
        pNewTerm->u.x.இடது_Column = p_இடது->iColumn;
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
  இல்லையெனில் எனில்( p_கோவை->op==TK_FUNCTION
   && pWC->op==TK_AND
   && isLikeOrGlob(pParse, p_கோவை, &pStr1, &isComplete, &noCase)
  ){
    கோவை *p_இடது;       /* LHS of LIKE/GLOB operator */
    கோவை *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    கோவை *pNewExpr1;
    கோவை *pNewExpr2;
    முழு idxNew1;
    முழு idxNew2;
    மாறிலி எழுத்து *zCollSeqName;     /* Name of collating sequence */
    மாறிலி u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    ப்ரதிஜ்ஞா( கோவை_Use_XList(p_கோவை) );
    p_இடது = p_கோவை->x.pList->a[1].p_கோவை;
    pStr2 = sqlite3_கோவை_Dup(db, pStr1, 0);
    ப்ரதிஜ்ஞா( pStr1==0 || !கோவை_Has_Property(pStr1, EP_Int_மதிப்பு) );
    ப்ரதிஜ்ஞா( pStr2==0 || !கோவை_Has_Property(pStr2, EP_Int_மதிப்பு) );


    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    எனில்( noCase && !pParse->db->mallocFailed ){
      முழு i;
      எழுத்து c;
      pTerm->wtFlags |= TERM_LIKE;
      க்கு(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    எனில்( !db->mallocFailed ){
      u8 *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      எனில்( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        எனில்( *pC=='A'-1 ) isComplete = 0;
        *pC = sqlite3UpperToLower[*pC];
      }

      /* Increment the value of the last utf8 character in the prefix. */
      போது( *pC==0xBF && pC>(u8*)pStr2->u.zToken ){
        *pC = 0x80;
        pC--;
      }
      ப்ரதிஜ்ஞா( *pC!=0xFF );        /* isLikeOrGlob() guarantees this */
      (*pC)++;
    }
    zCollSeqName = noCase ? "NOCASE" : sqlite3_Str_இருமம்;
    pNewExpr1 = sqlite3_கோவை_Dup(db, p_இடது, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3_கோவை_Add_Collate_String(pParse,pNewExpr1,zCollSeqName),
           pStr1);
    transferJoinMarkings(pNewExpr1, p_கோவை);
    idxNew1 = where_Clause_செருகு(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    pNewExpr2 = sqlite3_கோவை_Dup(db, p_இடது, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3_கோவை_Add_Collate_String(pParse,pNewExpr2,zCollSeqName),
           pStr2);
    transferJoinMarkings(pNewExpr2, p_கோவை);
    idxNew2 = where_Clause_செருகு(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    கோவை_Analyze(pSrc, pWC, idxNew1);
    கோவை_Analyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    எனில்( isComplete ){
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
  எனில்( (p_கோவை->op==TK_EQ || p_கோவை->op==TK_IS)
   && (n_இடது = sqlite3_கோவை_Vector_Size(p_கோவை->p_இடது))>1
   && sqlite3_கோவை_Vector_Size(p_கோவை->p_வலது)==n_இடது
   && ( (p_கோவை->p_இடது->flags & EP_xIsSelect)==0
     || (p_கோவை->p_வலது->flags & EP_xIsSelect)==0)
   && pWC->op==TK_AND
  ){
    முழு i;
    க்கு(i=0; i<n_இடது; i++){
      முழு idxNew;
      கோவை *pNew;
      கோவை *p_இடது = sqlite3_கோவை_For_Vector_Field(pParse, p_கோவை->p_இடது, i, n_இடது);
      கோவை *p_வலது = sqlite3_கோவை_For_Vector_Field(pParse, p_கோவை->p_வலது, i, n_இடது);

      pNew = sqlite3PExpr(pParse, p_கோவை->op, p_இடது, p_வலது);
      transferJoinMarkings(pNew, p_கோவை);
      idxNew = where_Clause_செருகு(pWC, pNew, TERM_DYNAMIC|TERM_SLICE);
      கோவை_Analyze(pSrc, pWC, idxNew);
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
  இல்லையெனில் எனில்( p_கோவை->op==TK_IN
   && pTerm->u.x.iField==0
   && p_கோவை->p_இடது->op==TK_VECTOR
   && ALWAYS( கோவை_Use_XSelect(p_கோவை) )
   && (p_கோவை->x.pSelect->pPrior==0 || (p_கோவை->x.pSelect->selFlags & SF_மதிப்புகள்))
#ifndef SQLITE_OMIT_WINDOWFUNC
   && p_கோவை->x.pSelect->pWin==0
#endif
   && (p_கோவை->x.pSelect->selFlags & SF_MinMaxAgg)==0
   && pWC->op==TK_AND
   && p_கோவை->x.pSelect->pEList->n_கோவை <= UMXV(pTerm->nChild)
   /* ^-- See bug 2026-06-04T10:00:49Z */
  ){
    முழு i;
    ப்ரதிஜ்ஞா( pTerm->nChild==0 );
    க்கு(i=0; i<sqlite3_கோவை_Vector_Size(p_கோவை->p_இடது); i++){
      முழு idxNew;
      idxNew = where_Clause_செருகு(pWC, p_கோவை, TERM_VIRTUAL|TERM_SLICE);
      pWC->a[idxNew].u.x.iField = i+1;
      கோவை_Analyze(pSrc, pWC, idxNew);
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
  இல்லையெனில் எனில்( pWC->op==TK_AND ){
    கோவை *p_வலது = 0, *p_இடது = 0;
    முழு res = isAuxiliaryVtabOperator(db, p_கோவை, &eOp2, &p_இடது, &p_வலது);
    போது( res-- > 0 ){
      முழு idxNew;
      WhereTerm *pNewTerm;
      Bitmask prereqColumn, prereq_கோவை;

      prereq_கோவை = sqlite3_Where_கோவை_Usage(pMaskSet, p_வலது);
      prereqColumn = sqlite3_Where_கோவை_Usage(pMaskSet, p_இடது);
      எனில்( (prereq_கோவை & prereqColumn)==0 ){
        கோவை *p_New_கோவை;
        p_New_கோவை = sqlite3PExpr(pParse, TK_MATCH,
            0, sqlite3_கோவை_Dup(db, p_வலது, 0));
        எனில்( கோவை_Has_Property(p_கோவை, EP_OuterON) && p_New_கோவை ){
          கோவை_Set_Property(p_New_கோவை, EP_OuterON);
          p_New_கோவை->w.iJoin = p_கோவை->w.iJoin;
        }
        idxNew = where_Clause_செருகு(pWC, p_New_கோவை, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        pNewTerm = &pWC->a[idxNew];
        pNewTerm->prereq_வலது = prereq_கோவை | extra_வலது;
        pNewTerm->இடது_Cursor = p_இடது->iTable;
        pNewTerm->u.x.இடது_Column = p_இடது->iColumn;
        pNewTerm->eOperator = WO_AUX;
        pNewTerm->eMatchOp = eOp2;
        markTermAsChild(pWC, idxNew, idxTerm);
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;
        pNewTerm->prereqAll = pTerm->prereqAll;
      }
      SWAP(கோவை*, p_இடது, p_வலது);
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  testcase( pTerm!=&pWC->a[idxTerm] );
  pTerm = &pWC->a[idxTerm];
  pTerm->prereq_வலது |= extra_வலது;
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
வெறுமை sqlite3WhereSplit(WhereClause *pWC, கோவை *p_கோவை, u8 op){
  கோவை *pE2 = sqlite3_கோவை_Skip_Collate_And_Likely(p_கோவை);
  pWC->op = op;
  ப்ரதிஜ்ஞா( pE2!=0 || p_கோவை==0 );
  எனில்( pE2==0 ) திரும்பு;
  எனில்( pE2->op!=op ){
    where_Clause_செருகு(pWC, p_கோவை, 0);
  }இல்லையெனில்{
    sqlite3WhereSplit(pWC, pE2->p_இடது, op);
    sqlite3WhereSplit(pWC, pE2->p_வலது, op);
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
நிலையான வெறுமை where_Add_Limit_கோவை(
  WhereClause *pWC,   /* Add the constraint to this WHERE clause */
  முழு iReg,           /* Register that will hold value of the limit/offset */
  கோவை *p_கோவை,        /* Expression that defines the limit/offset */
  முழு iCsr,           /* Cursor to which the constraint applies */
  முழு eMatchOp        /* SQLITE_INDEX_CONSTRAINT_LIMIT or _OFFSET */
){
  Parse *pParse = pWC->pWInfo->pParse;
  sqlite3 *db = pParse->db;
  கோவை *pNew;
  முழு iVal = 0;

  எனில்( sqlite3_கோவை_Is_Integer(p_கோவை, &iVal, pParse, 0) && iVal>=0 ){
    கோவை *pVal = sqlite3_கோவை_Int32(db, iVal);
    எனில்( pVal==0 ) திரும்பு;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }இல்லையெனில்{
    கோவை *pVal = sqlite3_கோவை_Alloc(db, TK_REGISTER, 0, 0);
    எனில்( pVal==0 ) திரும்பு;
    pVal->iTable = iReg;
    pNew = sqlite3PExpr(pParse, TK_MATCH, 0, pVal);
  }
  எனில்( pNew ){
    WhereTerm *pTerm;
    முழு idx;
    idx = where_Clause_செருகு(pWC, pNew, TERM_DYNAMIC|TERM_VIRTUAL);
    pTerm = &pWC->a[idx];
    pTerm->இடது_Cursor = iCsr;
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
வெறுமை SQLITE_NOINLINE sqlite3WhereAddLimit(WhereClause *pWC, Select *p){
  ப்ரதிஜ்ஞா( p!=0 && p->pLimit!=0 );                 /* 1 -- checked by caller */
  எனில்( p->pGroupBy==0
   && (p->selFlags & (SF_Distinct|SF_Aggregate))==0             /* 2 */
   && (p->pSrc->nSrc==1 && IsVirtual(p->pSrc->a[0].pSTab))      /* 3 */
  ){
    கோவை_List *pOrderBy = p->pOrderBy;
    முழு iCsr = p->pSrc->a[0].iCursor;
    முழு ii;

    /* Check condition (4). Return early if it is not met. */
    க்கு(ii=0; ii<pWC->nTerm; ii++){
      எனில்( pWC->a[ii].wtFlags & TERM_CODED ){
        /* This term is a vector operation that has been decomposed into
        ** other, subsequent terms.  It can be ignored. See tag-20220128a */
        ப்ரதிஜ்ஞா( pWC->a[ii].wtFlags & TERM_VIRTUAL );
        ப்ரதிஜ்ஞா( pWC->a[ii].eOperator==WO_ROWVAL );
        தொடர்;
      }
      எனில்( pWC->a[ii].nChild ){
        /* If this term has child terms, then they are also part of the
        ** pWC->a[] array. So this term can be ignored, as a LIMIT clause
        ** will only be added if each of the child terms passes the
        ** (leftCursor==iCsr) test below.  */
        தொடர்;
      }
      எனில்( pWC->a[ii].இடது_Cursor==iCsr && pWC->a[ii].prereq_வலது==0 ) தொடர்;

      /* If this term has a parent with exactly one child, and the parent will
      ** be passed through to xBestIndex, then this term can be ignored.  */
      எனில்( pWC->a[ii].iParent>=0 ){
        WhereTerm *pParent = &pWC->a[ pWC->a[ii].iParent ];
        எனில்( pParent->இடது_Cursor==iCsr
         && pParent->prereq_வலது==0
         && pParent->nChild==1
        ){
          தொடர்;
        }
      }

      /* This term will not be passed through. Do not add a LIMIT clause. */
      திரும்பு;
    }

    /* Check condition (5). Return early if it is not met. */
    எனில்( pOrderBy ){
      க்கு(ii=0; ii<pOrderBy->n_கோவை; ii++){
        கோவை *p_கோவை = pOrderBy->a[ii].p_கோவை;
        எனில்( p_கோவை->op!=TK_COLUMN ) திரும்பு;
        எனில்( p_கோவை->iTable!=iCsr ) திரும்பு;
        எனில்( pOrderBy->a[ii].fg.sortFlags & KEYINFO_ORDER_BIGNULL ) திரும்பு;
      }
    }

    /* All conditions are met. Add the terms to the where-clause object. */
    ப்ரதிஜ்ஞா( p->pLimit->op==TK_LIMIT );
    எனில்( p->iOffset!=0 && (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_கோவை(pWC, p->iOffset, p->pLimit->p_வலது,
                        iCsr, SQLITE_INDEX_CONSTRAINT_OFFSET);
    }
    எனில்( p->iOffset==0 || (p->selFlags & SF_Compound)==0 ){
      where_Add_Limit_கோவை(pWC, p->iLimit, p->pLimit->p_இடது,
                        iCsr, SQLITE_INDEX_CONSTRAINT_LIMIT);
    }
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
வெறுமை sqlite3WhereClauseInit(
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
வெறுமை sqlite3WhereClauseClear(WhereClause *pWC){
  sqlite3 *db = pWC->pWInfo->pParse->db;
  ப்ரதிஜ்ஞா( pWC->nTerm>=pWC->nBase );
  எனில்( pWC->nTerm>0 ){
    WhereTerm *a = pWC->a;
    WhereTerm *aLast = &pWC->a[pWC->nTerm-1];
#ifdef SQLITE_DEBUG
    முழு i;
    /* Verify that every term past pWC->nBase is virtual */
    க்கு(i=pWC->nBase; i<pWC->nTerm; i++){
      ப்ரதிஜ்ஞா( (pWC->a[i].wtFlags & TERM_VIRTUAL)!=0 );
    }
#endif
    போது(1){
      ப்ரதிஜ்ஞா( a->eMatchOp==0 || a->eOperator==WO_AUX );
      எனில்( a->wtFlags & TERM_DYNAMIC ){
        sqlite3_கோவை_Delete(db, a->p_கோவை);
      }
      எனில்( a->wtFlags & (TERM_ORINFO|TERM_ANDINFO) ){
        எனில்( a->wtFlags & TERM_ORINFO ){
          ப்ரதிஜ்ஞா( (a->wtFlags & TERM_ANDINFO)==0 );
          whereOrInfoDelete(db, a->u.pOrInfo);
        }இல்லையெனில்{
          ப்ரதிஜ்ஞா( (a->wtFlags & TERM_ANDINFO)!=0 );
          whereAndInfoDelete(db, a->u.pAndInfo);
        }
      }
      எனில்( a==aLast ) நிறுத்து;
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
நிலையான SQLITE_NOINLINE Bitmask sqlite3_Where_கோவை_Usage_Full(
  WhereMaskSet *pMaskSet,
  கோவை *p
){
  Bitmask mask;
  mask = (p->op==TK_IF_NULL_ROW) ? sqlite3WhereGetMask(pMaskSet, p->iTable) : 0;
  எனில்( p->p_இடது ) mask |= sqlite3_Where_கோவை_Usage_NN(pMaskSet, p->p_இடது);
  எனில்( p->p_வலது ){
    mask |= sqlite3_Where_கோவை_Usage_NN(pMaskSet, p->p_வலது);
    ப்ரதிஜ்ஞா( p->x.pList==0 );
  }இல்லையெனில் எனில்( கோவை_Use_XSelect(p) ){
    எனில்( கோவை_Has_Property(p, EP_VarSelect) ) pMaskSet->bVarSelect = 1;
    mask |= கோவை_Select_Usage(pMaskSet, p->x.pSelect);
  }இல்லையெனில் எனில்( p->x.pList ){
    mask |= sqlite3_Where_கோவை_List_Usage(pMaskSet, p->x.pList);
  }
#ifndef SQLITE_OMIT_WINDOWFUNC
  எனில்( (p->op==TK_FUNCTION || p->op==TK_AGG_FUNCTION) && கோவை_Use_YWin(p) ){
    ப்ரதிஜ்ஞா( p->y.pWin!=0 );
    mask |= sqlite3_Where_கோவை_List_Usage(pMaskSet, p->y.pWin->pPartition);
    mask |= sqlite3_Where_கோவை_List_Usage(pMaskSet, p->y.pWin->pOrderBy);
    mask |= sqlite3_Where_கோவை_Usage(pMaskSet, p->y.pWin->pFilter);
  }
#endif
  திரும்பு mask;
}
Bitmask sqlite3_Where_கோவை_Usage_NN(WhereMaskSet *pMaskSet, கோவை *p){
  எனில்( p->op==TK_COLUMN && !கோவை_Has_Property(p, EP_FixedCol) ){
    திரும்பு sqlite3WhereGetMask(pMaskSet, p->iTable);
  }இல்லையெனில் எனில்( கோவை_Has_Property(p, EP_TokenOnly|EP_Leaf) ){
    ப்ரதிஜ்ஞா( p->op!=TK_IF_NULL_ROW );
    திரும்பு 0;
  }
  திரும்பு sqlite3_Where_கோவை_Usage_Full(pMaskSet, p);
}
Bitmask sqlite3_Where_கோவை_Usage(WhereMaskSet *pMaskSet, கோவை *p){
  திரும்பு p ? sqlite3_Where_கோவை_Usage_NN(pMaskSet,p) : 0;
}
Bitmask sqlite3_Where_கோவை_List_Usage(WhereMaskSet *pMaskSet, கோவை_List *pList){
  முழு i;
  Bitmask mask = 0;
  எனில்( pList ){
    க்கு(i=0; i<pList->n_கோவை; i++){
      mask |= sqlite3_Where_கோவை_Usage(pMaskSet, pList->a[i].p_கோவை);
    }
  }
  திரும்பு mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause. 
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
வெறுமை sqlite3_Where_கோவை_Analyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  முழு i;
  க்கு(i=pWC->nTerm-1; i>=0; i--){
    கோவை_Analyze(pTabList, pWC, i);
  }
}

/*
** For table-valued-functions, transform the function arguments into
** new WHERE clause terms. 
**
** Each function argument translates into an equality constraint against
** a HIDDEN column in the table.
*/
வெறுமை sqlite3WhereTabFuncArgs(
  Parse *pParse,                    /* Parsing context */
  SrcItem *pItem,                   /* The FROM clause term to process */
  WhereClause *pWC                  /* Xfer function arguments to here */
){
  Table *pTab;
  முழு j, k;
  கோவை_List *pArgs;
  கோவை *pColRef;
  கோவை *pTerm;
  எனில்( pItem->fg.isTabFunc==0 ) திரும்பு;
  pTab = pItem->pSTab;
  ப்ரதிஜ்ஞா( pTab!=0 );
  எனில்( !IsVirtual(pTab) ){
    sqlite3_Error_செய்தி(pParse, "'%s' is not a function", pItem->zName);
    திரும்பு;
  }
  pArgs = pItem->u1.pFuncArg;
  எனில்( pArgs==0 ) திரும்பு;
  க்கு(j=k=0; j<pArgs->n_கோவை; j++){
    கோவை *pRhs;
    u32 joinType;
    போது( k<pTab->nCol && (pTab->aCol[k].colFlags & COLFLAG_HIDDEN)==0 ){k++;}
    எனில்( k>=pTab->nCol ){
      sqlite3_Error_செய்தி(pParse, "too many arguments on %s() - max %d",
                      pTab->zName, j);
      திரும்பு;
    }
    pColRef = sqlite3_கோவை_Alloc(pParse->db, TK_COLUMN, 0, 0);
    எனில்( pColRef==0 ) திரும்பு;
    pColRef->iTable = pItem->iCursor;
    pColRef->iColumn = k++;
    ப்ரதிஜ்ஞா( கோவை_Use_YTab(pColRef) );
    pColRef->y.pTab = pTab;
    pItem->colUsed |= sqlite3_கோவை_Col_Used(pColRef);
    pRhs = sqlite3PExpr(pParse, TK_UPLUS,
        sqlite3_கோவை_Dup(pParse->db, pArgs->a[j].p_கோவை, 0), 0);
    pTerm = sqlite3PExpr(pParse, TK_EQ, pColRef, pRhs);
    எனில்( pItem->fg.jointype & (JT_இடது|JT_வலது) ){
      testcase( pItem->fg.jointype & JT_இடது );  /* testtag-20230227a */
      testcase( pItem->fg.jointype & JT_வலது ); /* testtag-20230227b */
      joinType = EP_OuterON;
    }இல்லையெனில்{
      testcase( pItem->fg.jointype & JT_LTORJ ); /* testtag-20230227c */
      joinType = EP_InnerON;
    }
    sqlite3_Set_Join_கோவை(pTerm, pItem->iCursor, joinType);
    where_Clause_செருகு(pWC, pTerm, TERM_DYNAMIC);
  }
}

/*
** Return true if the WhereLoop pLoop can be use a Bloom filter.
** tag-202607231411
*/
முழு sqlite3WhereLoopBloomable(மாறிலி WhereLoop *pLoop){
  எனில்( pLoop->wsFlags & WHERE_IPK ) திரும்பு 1;
  எனில்( NEVER((pLoop->wsFlags & WHERE_INDEXED)==0) ) திரும்பு 0;
  திரும்பு sqlite3IndexBloomable(pLoop->u.btree.pIndex, pLoop->u.btree.nEq);
}
